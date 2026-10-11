#include "AmperiaSensorClient.h"

#include <string.h>

namespace
{
    static_assert(
        amperia::protocol::kMaxSensorPayloadSize <= 255,
        "The client passes register response sizes through Wire's byte-sized API.");

    uint16_t readUint16Le(const uint8_t* data)
    {
        return static_cast<uint16_t>(data[0]) |
               (static_cast<uint16_t>(data[1]) << 8);
    }

    uint32_t readUint32Le(const uint8_t* data)
    {
        return static_cast<uint32_t>(data[0]) |
               (static_cast<uint32_t>(data[1]) << 8) |
               (static_cast<uint32_t>(data[2]) << 16) |
               (static_cast<uint32_t>(data[3]) << 24);
    }

    float readFloatLe(const uint8_t* data)
    {
        const uint32_t bits = readUint32Le(data);
        float value = 0.0f;
        memcpy(&value, &bits, sizeof(value));
        return value;
    }

    int16_t readInt16Le(const uint8_t* data)
    {
        return static_cast<int16_t>(readUint16Le(data));
    }

    int32_t readInt32Le(const uint8_t* data)
    {
        return static_cast<int32_t>(readUint32Le(data));
    }

    uint8_t sensorRegister(size_t sensorIndex, uint8_t registerOffset)
    {
        const size_t address =
            static_cast<size_t>(amperia::protocol::kSensorRegisterStep) *
                sensorIndex +
            registerOffset;
        return static_cast<uint8_t>(address);
    }
}

AmperiaSensorClient* AmperiaSensorClient::s_activeClient = nullptr;

AmperiaSensorClient::Sensor::Sensor()
{
    reset();
}

void AmperiaSensorClient::Sensor::reset()
{
    memset(m_name, 0, sizeof(m_name));
    memset(m_unit, 0, sizeof(m_unit));
    memset(m_rawValues, 0, sizeof(m_rawValues));

    m_dimension = 0;
    m_dataPeriodMs = 0;
    m_valueType = amperia::protocol::DataType::None;
    m_valueResolution = 0.0f;
    m_valueMin = 0.0f;
    m_valueMax = 0.0f;
    m_flags = 0;
    m_timestampMs = 0;
    m_rawValuesSize = 0;
    m_hasValues = false;
}

const char* AmperiaSensorClient::Sensor::name() const
{
    return m_name;
}

const char* AmperiaSensorClient::Sensor::unit() const
{
    return m_unit;
}

uint16_t AmperiaSensorClient::Sensor::dimension() const
{
    return m_dimension;
}

uint16_t AmperiaSensorClient::Sensor::dataPeriodMs() const
{
    return m_dataPeriodMs;
}

amperia::protocol::DataType AmperiaSensorClient::Sensor::valueType() const
{
    return m_valueType;
}

float AmperiaSensorClient::Sensor::valueResolution() const
{
    return m_valueResolution;
}

float AmperiaSensorClient::Sensor::valueMin() const
{
    return m_valueMin;
}

float AmperiaSensorClient::Sensor::valueMax() const
{
    return m_valueMax;
}

uint16_t AmperiaSensorClient::Sensor::flags() const
{
    return m_flags;
}

uint32_t AmperiaSensorClient::Sensor::timestampMs() const
{
    return m_timestampMs;
}

bool AmperiaSensorClient::Sensor::hasValues() const
{
    return m_hasValues;
}

const uint8_t* AmperiaSensorClient::Sensor::rawValues() const
{
    return m_rawValues;
}

size_t AmperiaSensorClient::Sensor::rawValuesSize() const
{
    return m_rawValuesSize;
}

bool AmperiaSensorClient::Sensor::valueAsFloat(size_t component,
                                               float& value) const
{
    if (!m_hasValues || component >= m_dimension) {
        return false;
    }

    const size_t elementSize = amperia::protocol::dataTypeSize(m_valueType);
    if (elementSize == 0) {
        return false;
    }

    const size_t offset = component * elementSize;
    if (offset + elementSize > m_rawValuesSize) {
        return false;
    }

    const uint8_t* source = m_rawValues + offset;

    switch (m_valueType) {
        case amperia::protocol::DataType::Char:
            value = static_cast<float>(source[0]);
            return true;
        case amperia::protocol::DataType::Sint8:
            value = static_cast<float>(static_cast<int8_t>(source[0])) *
                    m_valueResolution;
            return true;
        case amperia::protocol::DataType::Uint8:
            value = static_cast<float>(source[0]) * m_valueResolution;
            return true;
        case amperia::protocol::DataType::Sint16:
            value = static_cast<float>(readInt16Le(source)) *
                    m_valueResolution;
            return true;
        case amperia::protocol::DataType::Uint16:
            value = static_cast<float>(readUint16Le(source)) *
                    m_valueResolution;
            return true;
        case amperia::protocol::DataType::Sint32:
            value = static_cast<float>(readInt32Le(source)) *
                    m_valueResolution;
            return true;
        case amperia::protocol::DataType::Uint32:
            value = static_cast<float>(readUint32Le(source)) *
                    m_valueResolution;
            return true;
        case amperia::protocol::DataType::Float:
            value = readFloatLe(source);
            return true;
        default:
            return false;
    }
}

float AmperiaSensorClient::Sensor::valueAsFloat(size_t component) const
{
    float value = 0.0f;
    valueAsFloat(component, value);
    return value;
}

float AmperiaSensorClient::Sensor::value(size_t component) const
{
    return valueAsFloat(component);
}

bool AmperiaSensorClient::Sensor::usesDiscouragedDimension() const
{
    return m_dimension > amperia::protocol::kRecommendedSensorDimension;
}

AmperiaSensorClient::AmperiaSensorClient() :
    m_wire(nullptr),
    m_dataReadyPin(0),
    m_started(false),
    m_interruptAttached(false),
    m_dataReady(false),
    m_hardwareVersion(0),
    m_firmwareVersion(0),
    m_protocolVersion(0),
    m_operationMode(0),
    m_reportedSensorCount(0),
    m_sensorCount(0),
    m_sensorListTruncated(false),
    m_lastError(Error::None),
    m_failedRegister(0)
{
    memset(m_deviceName, 0, sizeof(m_deviceName));
}

AmperiaSensorClient::~AmperiaSensorClient()
{
    end();
}

bool AmperiaSensorClient::begin()
{
    return begin(
        Wire,
        amperia::board::unoR4Wifi::kSensorDataReadyPin,
        true);
}

bool AmperiaSensorClient::begin(TwoWire& wire,
                                uint8_t dataReadyPin,
                                bool wireInit)
{
    if (s_activeClient != nullptr && s_activeClient != this) {
        setError(Error::AnotherClientActive);
        return false;
    }

    end();

    m_wire = &wire;
    m_dataReadyPin = dataReadyPin;
    m_dataReady = false;
    resetDiscovery();

    if (wireInit) {
        m_wire->begin();
        m_wire->setClock(kDefaultI2cClockHz);
    }

    pinMode(m_dataReadyPin, INPUT);
    const int interruptNumber = digitalPinToInterrupt(m_dataReadyPin);

#if defined(NOT_AN_INTERRUPT)
    if (interruptNumber != NOT_AN_INTERRUPT) {
#endif
        s_activeClient = this;
        attachInterrupt(interruptNumber, handleDataReadyInterrupt, RISING);
        m_interruptAttached = true;
#if defined(NOT_AN_INTERRUPT)
    }
#endif

    m_started = true;
    setError(Error::None);
    return true;
}

void AmperiaSensorClient::end()
{
    if (m_interruptAttached) {
        detachInterrupt(digitalPinToInterrupt(m_dataReadyPin));
        m_interruptAttached = false;
    }

    if (s_activeClient == this) {
        s_activeClient = nullptr;
    }

    m_started = false;
    m_dataReady = false;
    m_wire = nullptr;
}

bool AmperiaSensorClient::isStarted() const
{
    return m_started;
}

bool AmperiaSensorClient::discover()
{
    if (!m_started || m_wire == nullptr) {
        setError(Error::NotStarted);
        return false;
    }

    resetDiscovery();

    uint8_t oneByte[1] = {0};

    if (!readFixedText(
            amperia::protocol::registers::kDeviceName,
            m_deviceName,
            amperia::protocol::kDeviceNameSize) ||
        !readRegister(
            amperia::protocol::registers::kHardwareVersion,
            oneByte,
            sizeof(oneByte))) {
        resetDiscovery();
        return false;
    }
    m_hardwareVersion = oneByte[0];

    if (!readRegister(
            amperia::protocol::registers::kFirmwareVersion,
            oneByte,
            sizeof(oneByte))) {
        resetDiscovery();
        return false;
    }
    m_firmwareVersion = oneByte[0];

    if (!readRegister(
            amperia::protocol::registers::kProtocolVersion,
            oneByte,
            sizeof(oneByte))) {
        resetDiscovery();
        return false;
    }

    const uint8_t reportedProtocolVersion = oneByte[0];
    if (!amperia::protocol::isProtocolVersionCompatible(
            reportedProtocolVersion)) {
        resetDiscovery();
        m_protocolVersion = reportedProtocolVersion;
        setError(
            Error::UnsupportedProtocolVersion,
            amperia::protocol::registers::kProtocolVersion);
        return false;
    }
    m_protocolVersion = reportedProtocolVersion;

    if (!readRegister(
            amperia::protocol::registers::kSensorCount,
            oneByte,
            sizeof(oneByte))) {
        resetDiscovery();
        return false;
    }
    m_reportedSensorCount = oneByte[0];
    m_sensorListTruncated =
        m_reportedSensorCount > amperia::protocol::kMaxLogicalSensors;
    m_sensorCount = m_sensorListTruncated
        ? amperia::protocol::kMaxLogicalSensors
        : m_reportedSensorCount;

    if (!readRegister(
            amperia::protocol::registers::kOperationMode,
            oneByte,
            sizeof(oneByte))) {
        resetDiscovery();
        return false;
    }
    m_operationMode = oneByte[0];

    for (size_t sensorIndex = 0;
         sensorIndex < m_sensorCount;
         ++sensorIndex) {
        if (!discoverSensor(sensorIndex)) {
            const Error error = m_lastError;
            const uint8_t failedRegister = m_failedRegister;
            resetDiscovery();
            setError(error, failedRegister);
            return false;
        }
    }

    setError(Error::None);
    return true;
}

bool AmperiaSensorClient::update()
{
    if (!m_started || m_wire == nullptr) {
        setError(Error::NotStarted);
        return false;
    }

    if (m_sensorCount == 0) {
        setError(Error::None);
        return true;
    }

    uint8_t payloads[amperia::protocol::kMaxLogicalSensors]
                    [amperia::protocol::kMaxSensorPayloadSize] = {{0}};
    size_t payloadSizes[amperia::protocol::kMaxLogicalSensors] = {0};

    for (size_t sensorIndex = 0;
         sensorIndex < m_sensorCount;
         ++sensorIndex) {
        if (!readSensorPayload(
                sensorIndex,
                payloads[sensorIndex],
                sizeof(payloads[sensorIndex]),
                payloadSizes[sensorIndex])) {
            return false;
        }
    }

    for (size_t sensorIndex = 0;
         sensorIndex < m_sensorCount;
         ++sensorIndex) {
        if (!commitSensorPayload(
                sensorIndex,
                payloads[sensorIndex],
                payloadSizes[sensorIndex])) {
            setError(Error::InvalidSensorDescriptor);
            return false;
        }
    }

    setError(Error::None);
    return true;
}

bool AmperiaSensorClient::updateSensor(size_t sensorIndex)
{
    if (!m_started || m_wire == nullptr) {
        setError(Error::NotStarted);
        return false;
    }

    if (sensorIndex >= m_sensorCount) {
        setError(Error::InvalidSensorIndex);
        return false;
    }

    uint8_t payload[amperia::protocol::kMaxSensorPayloadSize] = {0};
    size_t payloadSize = 0;

    if (!readSensorPayload(
            sensorIndex,
            payload,
            sizeof(payload),
            payloadSize)) {
        return false;
    }

    if (!commitSensorPayload(sensorIndex, payload, payloadSize)) {
        setError(Error::InvalidSensorDescriptor);
        return false;
    }

    setError(Error::None);
    return true;
}

bool AmperiaSensorClient::updateIfReady()
{
    if (!m_dataReady) {
        return false;
    }

    // Clear first. A new edge during update() sets the flag again.
    m_dataReady = false;
    return update();
}

bool AmperiaSensorClient::dataReady() const
{
    return m_dataReady;
}

void AmperiaSensorClient::clearDataReady()
{
    m_dataReady = false;
}

const char* AmperiaSensorClient::deviceName() const
{
    return m_deviceName;
}

uint8_t AmperiaSensorClient::hardwareVersion() const
{
    return m_hardwareVersion;
}

uint8_t AmperiaSensorClient::firmwareVersion() const
{
    return m_firmwareVersion;
}

uint8_t AmperiaSensorClient::protocolVersion() const
{
    return m_protocolVersion;
}

uint8_t AmperiaSensorClient::operationMode() const
{
    return m_operationMode;
}

size_t AmperiaSensorClient::sensorCount() const
{
    return m_sensorCount;
}

uint8_t AmperiaSensorClient::reportedSensorCount() const
{
    return m_reportedSensorCount;
}

bool AmperiaSensorClient::sensorListTruncated() const
{
    return m_sensorListTruncated;
}

const AmperiaSensorClient::Sensor&
AmperiaSensorClient::sensor(size_t sensorIndex) const
{
    return m_sensors[sensorIndex];
}

const AmperiaSensorClient::Sensor*
AmperiaSensorClient::sensorOrNull(size_t sensorIndex) const
{
    return sensorIndex < m_sensorCount ? &m_sensors[sensorIndex] : nullptr;
}

const AmperiaSensorClient::Sensor*
AmperiaSensorClient::findSensor(const char* namePart) const
{
    if (namePart == nullptr || namePart[0] == '\0') {
        return nullptr;
    }

    for (size_t sensorIndex = 0;
         sensorIndex < m_sensorCount;
         ++sensorIndex) {
        if (strstr(m_sensors[sensorIndex].name(), namePart) != nullptr) {
            return &m_sensors[sensorIndex];
        }
    }

    return nullptr;
}

float AmperiaSensorClient::value(size_t sensorIndex, size_t component) const
{
    const Sensor* source = sensorOrNull(sensorIndex);
    return source != nullptr ? source->value(component) : 0.0f;
}

AmperiaSensorClient::Error AmperiaSensorClient::lastError() const
{
    return m_lastError;
}

uint8_t AmperiaSensorClient::failedRegister() const
{
    return m_failedRegister;
}

void AmperiaSensorClient::handleDataReadyInterrupt()
{
    if (s_activeClient != nullptr) {
        s_activeClient->m_dataReady = true;
    }
}

void AmperiaSensorClient::resetDiscovery()
{
    memset(m_deviceName, 0, sizeof(m_deviceName));
    m_hardwareVersion = 0;
    m_firmwareVersion = 0;
    m_protocolVersion = 0;
    m_operationMode = 0;
    m_reportedSensorCount = 0;
    m_sensorCount = 0;
    m_sensorListTruncated = false;

    for (size_t i = 0;
         i < amperia::protocol::kMaxLogicalSensors;
         ++i) {
        m_sensors[i].reset();
    }
}

void AmperiaSensorClient::setError(Error error, uint8_t reg)
{
    m_lastError = error;
    m_failedRegister = reg;
}

bool AmperiaSensorClient::readRegister(uint8_t reg,
                                       uint8_t* buffer,
                                       size_t size)
{
    if (m_wire == nullptr || buffer == nullptr || size == 0 || size > 255) {
        setError(Error::RegisterReadFailed, reg);
        return false;
    }

    for (uint8_t attempt = 0; attempt < kReadAttempts; ++attempt) {
        m_wire->beginTransmission(amperia::protocol::kI2cAddress);
        m_wire->write(reg);

        if (m_wire->endTransmission(true) == 0) {
            delay(kRegisterSelectDelayMs);

            const uint8_t requested = m_wire->requestFrom(
                static_cast<uint8_t>(amperia::protocol::kI2cAddress),
                static_cast<uint8_t>(size));

            size_t bytesRead = 0;
            while (m_wire->available() && bytesRead < size) {
                const int value = m_wire->read();
                if (value < 0) {
                    break;
                }
                buffer[bytesRead++] = static_cast<uint8_t>(value);
            }

            while (m_wire->available()) {
                m_wire->read();
            }

            if (requested == size && bytesRead == size) {
                return true;
            }
        }

        if (attempt + 1 < kReadAttempts) {
            delay(kRetryDelayMs);
        }
    }

    setError(Error::RegisterReadFailed, reg);
    return false;
}

bool AmperiaSensorClient::readFixedText(uint8_t reg,
                                        char* destination,
                                        size_t protocolFieldSize)
{
    if (destination == nullptr || protocolFieldSize == 0 ||
        protocolFieldSize > amperia::protocol::kMaxFieldSize) {
        setError(Error::RegisterReadFailed, reg);
        return false;
    }

    uint8_t bytes[amperia::protocol::kMaxFieldSize] = {0};
    if (!readRegister(reg, bytes, protocolFieldSize)) {
        return false;
    }

    memcpy(destination, bytes, protocolFieldSize);
    destination[protocolFieldSize] = '\0';
    return true;
}

bool AmperiaSensorClient::discoverSensor(size_t sensorIndex)
{
    Sensor& sensorRecord = m_sensors[sensorIndex];
    sensorRecord.reset();

    const uint8_t nameRegister = sensorRegister(
        sensorIndex,
        amperia::protocol::registers::kSensorName);
    const uint8_t unitRegister = sensorRegister(
        sensorIndex,
        amperia::protocol::registers::kSensorUnit);

    if (!readFixedText(
            nameRegister,
            sensorRecord.m_name,
            amperia::protocol::kSensorNameSize) ||
        !readFixedText(
            unitRegister,
            sensorRecord.m_unit,
            amperia::protocol::kUnitNameSize)) {
        return false;
    }

    uint8_t twoBytes[2] = {0};
    uint8_t fourBytes[4] = {0};
    uint8_t oneByte[1] = {0};

    const uint8_t dimensionRegister = sensorRegister(
        sensorIndex,
        amperia::protocol::registers::kSensorDimension);
    if (!readRegister(dimensionRegister, twoBytes, sizeof(twoBytes))) {
        return false;
    }
    sensorRecord.m_dimension = readUint16Le(twoBytes);

    const uint8_t periodRegister = sensorRegister(
        sensorIndex,
        amperia::protocol::registers::kSensorDataPeriod);
    if (!readRegister(periodRegister, twoBytes, sizeof(twoBytes))) {
        return false;
    }
    sensorRecord.m_dataPeriodMs = readUint16Le(twoBytes);

    const uint8_t typeRegister = sensorRegister(
        sensorIndex,
        amperia::protocol::registers::kSensorValueType);
    if (!readRegister(typeRegister, oneByte, sizeof(oneByte))) {
        return false;
    }
    sensorRecord.m_valueType =
        static_cast<amperia::protocol::DataType>(oneByte[0]);

    const uint8_t resolutionRegister = sensorRegister(
        sensorIndex,
        amperia::protocol::registers::kSensorValueResolution);
    if (!readRegister(resolutionRegister, fourBytes, sizeof(fourBytes))) {
        return false;
    }
    sensorRecord.m_valueResolution = readFloatLe(fourBytes);

    const uint8_t minRegister = sensorRegister(
        sensorIndex,
        amperia::protocol::registers::kSensorValueMin);
    if (!readRegister(minRegister, fourBytes, sizeof(fourBytes))) {
        return false;
    }
    sensorRecord.m_valueMin = readFloatLe(fourBytes);

    const uint8_t maxRegister = sensorRegister(
        sensorIndex,
        amperia::protocol::registers::kSensorValueMax);
    if (!readRegister(maxRegister, fourBytes, sizeof(fourBytes))) {
        return false;
    }
    sensorRecord.m_valueMax = readFloatLe(fourBytes);

    const uint8_t flagsRegister = sensorRegister(
        sensorIndex,
        amperia::protocol::registers::kSensorFlags);
    if (!readRegister(flagsRegister, twoBytes, sizeof(twoBytes))) {
        return false;
    }
    sensorRecord.m_flags = readUint16Le(twoBytes);

    if (!amperia::protocol::isSupportedDimension(
            sensorRecord.m_dimension)) {
        setError(Error::InvalidSensorDescriptor, dimensionRegister);
        return false;
    }

    const size_t elementSize =
        amperia::protocol::dataTypeSize(sensorRecord.m_valueType);
    if (elementSize == 0) {
        setError(Error::InvalidSensorDescriptor, typeRegister);
        return false;
    }

    sensorRecord.m_rawValuesSize =
        sensorRecord.m_dimension * elementSize;
    if (sensorRecord.m_rawValuesSize > sizeof(sensorRecord.m_rawValues)) {
        setError(Error::InvalidSensorDescriptor, typeRegister);
        return false;
    }

    return true;
}

bool AmperiaSensorClient::readSensorPayload(size_t sensorIndex,
                                            uint8_t* payload,
                                            size_t payloadCapacity,
                                            size_t& payloadSize)
{
    if (sensorIndex >= m_sensorCount || payload == nullptr) {
        setError(Error::InvalidSensorIndex);
        return false;
    }

    const Sensor& sensorRecord = m_sensors[sensorIndex];
    payloadSize = amperia::protocol::kTimestampSize +
                  sensorRecord.m_rawValuesSize;

    if (payloadSize > payloadCapacity) {
        setError(Error::InvalidSensorDescriptor);
        return false;
    }

    const uint8_t valuesRegister = sensorRegister(
        sensorIndex,
        amperia::protocol::registers::kSensorValues);
    return readRegister(valuesRegister, payload, payloadSize);
}

bool AmperiaSensorClient::commitSensorPayload(size_t sensorIndex,
                                              const uint8_t* payload,
                                              size_t payloadSize)
{
    if (sensorIndex >= m_sensorCount || payload == nullptr) {
        return false;
    }

    Sensor& sensorRecord = m_sensors[sensorIndex];
    const size_t expectedSize = amperia::protocol::kTimestampSize +
                                sensorRecord.m_rawValuesSize;
    if (payloadSize != expectedSize) {
        return false;
    }

    sensorRecord.m_timestampMs = readUint32Le(payload);
    memcpy(
        sensorRecord.m_rawValues,
        payload + amperia::protocol::kTimestampSize,
        sensorRecord.m_rawValuesSize);
    sensorRecord.m_hasValues = true;
    return true;
}
