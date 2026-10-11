#include "AmperiaDeviceServer.h"

#include <Arduino.h>
#include <string.h>

#include "../protocol/AmperiaProtocol.h"

namespace amperia
{
namespace device
{
    AmperiaDeviceServer *AmperiaDeviceServer::s_activeServer = nullptr;

    AmperiaDeviceServer::AmperiaDeviceServer() :
        m_wire(nullptr),
        m_codec(nullptr),
        m_dataReadyPin(0),
        m_requestedRegister(protocol::registers::kReserved),
        m_receiveBuffer{},
        m_transmitBuffer{},
        m_started(false)
    {}

    AmperiaDeviceServer::~AmperiaDeviceServer()
    {
        end();
    }

    bool AmperiaDeviceServer::begin(TwoWire &wire,
                                    uint8_t dataReadyPin,
                                    protocol::RegisterCodec &codec,
                                    bool initializeWire)
    {
        if (s_activeServer != nullptr && s_activeServer != this) {
            return false;
        }

        m_wire = &wire;
        m_codec = &codec;
        m_dataReadyPin = dataReadyPin;
        m_requestedRegister = protocol::registers::kReserved;
        memset(m_receiveBuffer, 0, sizeof(m_receiveBuffer));
        memset(m_transmitBuffer, 0, sizeof(m_transmitBuffer));

        pinMode(m_dataReadyPin, OUTPUT);

        // Preserve the initialization pulse sequence of the previous
        // GeneralSensor implementation: DREADY remains high while the I2C
        // peripheral and callbacks are configured, then returns low.
        digitalWrite(m_dataReadyPin, LOW);
        digitalWrite(m_dataReadyPin, HIGH);

        if (initializeWire) {
            m_wire->begin(protocol::kI2cAddress);
        }

        s_activeServer = this;
        m_wire->onReceive(receiveThunk);
        m_wire->onRequest(requestThunk);
        digitalWrite(m_dataReadyPin, LOW);
        m_started = true;

        return true;
    }

    void AmperiaDeviceServer::end()
    {
        if (s_activeServer == this) {
            s_activeServer = nullptr;
        }

        m_started = false;
        m_wire = nullptr;
        m_codec = nullptr;
    }

    void AmperiaDeviceServer::signalDataReady(uint16_t pulseWidthUs)
    {
        if (!m_started) {
            return;
        }

        digitalWrite(m_dataReadyPin, HIGH);
        delayMicroseconds(pulseWidthUs);
        digitalWrite(m_dataReadyPin, LOW);
    }

    void AmperiaDeviceServer::receiveThunk(int byteCount)
    {
        if (s_activeServer != nullptr) {
            s_activeServer->handleReceive(byteCount);
        }
    }

    void AmperiaDeviceServer::requestThunk()
    {
        if (s_activeServer != nullptr) {
            s_activeServer->handleRequest();
        }
    }

    void AmperiaDeviceServer::handleReceive(int byteCount)
    {
        if (m_wire == nullptr || m_codec == nullptr || byteCount <= 0) {
            return;
        }

        size_t bytesRead = 0;
        while (m_wire->available() && byteCount-- > 0) {
            const int value = m_wire->read();
            if (value < 0) {
                break;
            }

            if (bytesRead < sizeof(m_receiveBuffer)) {
                m_receiveBuffer[bytesRead++] = static_cast<uint8_t>(value);
            }
        }

        // Drain an oversized write so it cannot leak into the next request.
        while (m_wire->available()) {
            m_wire->read();
        }

        if (bytesRead == 0) {
            return;
        }

        m_requestedRegister = m_receiveBuffer[0];

        if (m_requestedRegister == protocol::registers::kOperationMode &&
            bytesRead >= 2) {
            m_codec->setOperationMode(m_receiveBuffer[1]);
        }
    }

    void AmperiaDeviceServer::handleRequest()
    {
        if (m_wire == nullptr || m_codec == nullptr) {
            return;
        }

        const int dataSize = m_codec->getData(
            m_requestedRegister,
            m_transmitBuffer,
            sizeof(m_transmitBuffer));

        if (dataSize <= 0) {
            m_wire->write(static_cast<uint8_t>(0));
            return;
        }

        m_wire->write(m_transmitBuffer, static_cast<size_t>(dataSize));
    }
}
}
