#pragma once

#include <Arduino.h>
#include <Wire.h>

#include "../boards/UnoR4WifiShieldPins.h"
#include "../protocol/AmperiaProtocol.h"

// Generic reader for one external EdPro/Amperia sensor connected to the
// controller-side I2C bus of the EdPro shield.
//
// The class discovers protocol metadata instead of relying on product names.
// It keeps the received value bytes unchanged and converts them only when a
// typed accessor such as valueAsFloat() is called.
class AmperiaSensorClient
{
public:
    class Sensor
    {
    public:
        const char* name() const;
        const char* unit() const;
        uint16_t dimension() const;
        uint16_t dataPeriodMs() const;
        amperia::protocol::DataType valueType() const;
        float valueResolution() const;
        float valueMin() const;
        float valueMax() const;
        uint16_t flags() const;
        uint32_t timestampMs() const;

        bool hasValues() const;

        // The returned bytes are exactly the value portion received from the
        // sensor. They remain valid until the next discover() or update of
        // this record.
        const uint8_t* rawValues() const;
        size_t rawValuesSize() const;

        // Convert one component to a physical floating-point value.
        // Integer protocol types are multiplied by valueResolution; Float is
        // returned unchanged. Char is converted to its unsigned byte value.
        bool valueAsFloat(size_t component, float& value) const;

        // Convenience overload. Returns 0.0f for an invalid component or when
        // no value has been received; use the bool overload to detect errors.
        float valueAsFloat(size_t component) const;

        // Short form for simple sketches. value() reads the first component.
        float value(size_t component = 0) const;

        bool usesDiscouragedDimension() const;

    private:
        friend class AmperiaSensorClient;

        Sensor();
        void reset();

        char m_name[amperia::protocol::kSensorNameSize + 1];
        char m_unit[amperia::protocol::kUnitNameSize + 1];
        uint16_t m_dimension;
        uint16_t m_dataPeriodMs;
        amperia::protocol::DataType m_valueType;
        float m_valueResolution;
        float m_valueMin;
        float m_valueMax;
        uint16_t m_flags;
        uint32_t m_timestampMs;
        uint8_t m_rawValues[amperia::protocol::kMaxValueBytes];
        size_t m_rawValuesSize;
        bool m_hasValues;
    };

    enum class Error : uint8_t
    {
        None,
        NotStarted,
        AnotherClientActive,
        RegisterReadFailed,
        UnsupportedProtocolVersion,
        InvalidSensorDescriptor,
        InvalidSensorIndex
    };

    static constexpr uint32_t kDefaultI2cClockHz = 50000;
    static constexpr uint16_t kRegisterSelectDelayMs = 5;
    static constexpr uint16_t kRetryDelayMs = 10;
    static constexpr uint8_t kReadAttempts = 2;

    AmperiaSensorClient();
    ~AmperiaSensorClient();

    // The client owns interrupt-registration state through a static callback
    // trampoline and keeps a pointer to the selected TwoWire instance.
    AmperiaSensorClient(const AmperiaSensorClient&) = delete;
    AmperiaSensorClient& operator=(const AmperiaSensorClient&) = delete;
    AmperiaSensorClient(AmperiaSensorClient&&) = delete;
    AmperiaSensorClient& operator=(AmperiaSensorClient&&) = delete;

    // UNO R4 WiFi shield defaults: Wire reads the external sensor and A3 is
    // the sensor-to-Arduino DREADY input. Arduino interrupt callbacks do not
    // carry an object pointer, so only one interrupt-driven client may be
    // active at a time.
    bool begin();

    // Advanced initialization for another TwoWire instance or DREADY pin.
    // Set wireInit=false when the application has already initialized and
    // configured the bus. In that case the client does not change its clock.
    bool begin(TwoWire& wire, uint8_t dataReadyPin, bool wireInit = true);

    // Detach the DREADY interrupt. The I2C bus is intentionally not ended,
    // because other application devices may use the same controller bus.
    void end();

    bool isStarted() const;

    // Read device and logical-sensor metadata. This does not write operation
    // mode or other configuration registers on the external sensor.
    // Protocol versions newer than this library are accepted under the
    // forward-compatible rule that the v1 core register contract is preserved.
    bool discover();

    // Read a complete, consistent snapshot of all discovered logical sensors.
    // If any register read fails, no sensor record is updated.
    bool update();

    // Read one logical sensor. This is useful when only one record is needed;
    // it does not require or clear the DREADY flag.
    bool updateSensor(size_t sensorIndex);

    // Update only after a DREADY rising edge. The flag is cleared before the
    // I2C reads so a new pulse arriving during the update is not lost.
    bool updateIfReady();
    bool dataReady() const;
    void clearDataReady();

    const char* deviceName() const;
    uint8_t hardwareVersion() const;
    uint8_t firmwareVersion() const;
    uint8_t protocolVersion() const;
    uint8_t operationMode() const;

    // sensorCount() is capped at the protocol-supported maximum. The reported
    // count and truncation flag preserve what the external device announced.
    size_t sensorCount() const;
    uint8_t reportedSensorCount() const;
    bool sensorListTruncated() const;

    // Valid indexes are 0..sensorCount()-1.
    const Sensor& sensor(size_t sensorIndex) const;
    const Sensor* sensorOrNull(size_t sensorIndex) const;

    // Find the first logical sensor whose name contains namePart.
    // Returns nullptr if no discovered sensor matches.
    const Sensor* findSensor(const char* namePart) const;

    // Short form for simple sketches. value(0) reads the first logical sensor;
    // the optional second argument selects one of its components.
    float value(size_t sensorIndex, size_t component = 0) const;

    Error lastError() const;
    uint8_t failedRegister() const;

private:
    static AmperiaSensorClient* s_activeClient;
    static void handleDataReadyInterrupt();

    void resetDiscovery();
    void setError(Error error, uint8_t reg = 0);

    bool readRegister(uint8_t reg, uint8_t* buffer, size_t size);
    bool readFixedText(uint8_t reg,
                       char* destination,
                       size_t protocolFieldSize);
    bool discoverSensor(size_t sensorIndex);
    bool readSensorPayload(size_t sensorIndex,
                           uint8_t* payload,
                           size_t payloadCapacity,
                           size_t& payloadSize);
    bool commitSensorPayload(size_t sensorIndex,
                             const uint8_t* payload,
                             size_t payloadSize);

    TwoWire* m_wire;
    uint8_t m_dataReadyPin;
    bool m_started;
    bool m_interruptAttached;
    volatile bool m_dataReady;

    char m_deviceName[amperia::protocol::kDeviceNameSize + 1];
    uint8_t m_hardwareVersion;
    uint8_t m_firmwareVersion;
    uint8_t m_protocolVersion;
    uint8_t m_operationMode;
    uint8_t m_reportedSensorCount;
    size_t m_sensorCount;
    bool m_sensorListTruncated;

    Sensor m_sensors[amperia::protocol::kMaxLogicalSensors];

    Error m_lastError;
    uint8_t m_failedRegister;
};
