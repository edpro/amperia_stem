#pragma once

#include <stddef.h>
#include <stdint.h>
#include <Wire.h>

#include "../protocol/AmperiaRegisterCodec.h"

namespace amperia
{
namespace device
{
    // Device-side transport for the Amperia sensor protocol.
    //
    // The server exposes one RegisterCodec over an Arduino TwoWire instance
    // operating in peripheral mode and signals new measurements through a
    // separate DREADY pin. It deliberately contains no knowledge of concrete
    // sensor classes such as SimpleSensor or NDimSensor.
    //
    // Arduino's TwoWire callbacks do not carry a user context, therefore the
    // current implementation supports one active device-side server per MCU.
    class AmperiaDeviceServer
    {
    public:
        static constexpr size_t kReceiveBufferSize = 4;
        static constexpr size_t kTransmitBufferSize = 32;
        static constexpr uint16_t kDefaultDataReadyPulseUs = 10;

        AmperiaDeviceServer();
        ~AmperiaDeviceServer();

        // The static TwoWire callback trampoline can refer to exactly one
        // concrete server object, so copying or moving a server is unsafe.
        AmperiaDeviceServer(const AmperiaDeviceServer&) = delete;
        AmperiaDeviceServer& operator=(const AmperiaDeviceServer&) = delete;
        AmperiaDeviceServer(AmperiaDeviceServer&&) = delete;
        AmperiaDeviceServer& operator=(AmperiaDeviceServer&&) = delete;

        // Bind the server to a concrete Wire interface and protocol codec.
        // When initializeWire is false, the caller remains responsible for
        // configuring the bus; callbacks are registered in either case.
        bool begin(TwoWire &wire,
                   uint8_t dataReadyPin,
                   protocol::RegisterCodec &codec,
                   bool initializeWire = true);

        // Disconnect this object from the static callback trampoline. The
        // TwoWire bus itself is not stopped because it may be shared by other
        // code on future platforms.
        void end();

        // Notify the multimeter that fresh values are available.
        void signalDataReady(
            uint16_t pulseWidthUs = kDefaultDataReadyPulseUs);

        bool isStarted() const { return m_started; }

    private:
        static AmperiaDeviceServer *s_activeServer;

        static void receiveThunk(int byteCount);
        static void requestThunk();

        void handleReceive(int byteCount);
        void handleRequest();

        TwoWire *m_wire;
        protocol::RegisterCodec *m_codec;
        uint8_t m_dataReadyPin;
        uint8_t m_requestedRegister;
        uint8_t m_receiveBuffer[kReceiveBufferSize];
        uint8_t m_transmitBuffer[kTransmitBufferSize];
        bool m_started;
    };
}
}
