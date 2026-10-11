#ifndef MYSENSOR_H
#define MYSENSOR_H

#include <stdint.h>
#include <Wire.h>

#include "protocol/AmperiaDescriptors.h"
#include "protocol/AmperiaRegisterCodec.h"
#include "boards/UnoR4WifiShieldPins.h"
#include "protocol/AmperiaProtocol.h"
#include "device/AmperiaDeviceServer.h"

namespace amperia
{
    constexpr uint16_t kDefaultDataPeriodMs = 160;
}

// A measurement callback writes the measured value to `value` and returns
// true on success. Returning false keeps the previous value unpublished.
using AgentMeasurementFunction = bool (*)(float& value);

// ----------------GeneralSensor--------------------------
// Definition: Main starting point of all sensors 
// GeneralSensor -- 0.0
class GeneralSensor
{
protected:
    GeneralSensor() = default;

    // RegisterCodec stores references and pointers to descriptor storage owned
    // by the concrete sensor object. Copying or moving a sensor would leave
    // those bindings referring to the wrong object.
    GeneralSensor(const GeneralSensor&) = delete;
    GeneralSensor& operator=(const GeneralSensor&) = delete;
    GeneralSensor(GeneralSensor&&) = delete;
    GeneralSensor& operator=(GeneralSensor&&) = delete;

    // Sensor classes own descriptors and current measurements only. Protocol
    // byte serialization is delegated to RegisterCodec, while transport
    // buffers remain private to AmperiaDeviceServer.
    amperia::device::AmperiaDeviceServer deviceServer;

    virtual amperia::protocol::RegisterCodec& protocolCodec() = 0;

public:
    virtual ~GeneralSensor();

    // Arduino-style initialization for the current UNO R4 WiFi shield.
    // Uses Wire1 for the multimeter connection and A2 for DREADY.
    bool begin();

    // Advanced initialization keeps the protocol independent of a concrete
    // Arduino board and leaves the path open for future platform adapters.
    bool begin(TwoWire& wire, uint8_t dataReadyPin, bool wireInit = true);

    void signalDataReady(uint16_t pulseWidthUs =
        amperia::device::AmperiaDeviceServer::kDefaultDataReadyPulseUs);
};

// First branch -- Simple Sensors
// Definition: sensors with one measurement and one operating pin.
// SimpleSensor - 1.0
// ----------------SimpleSensor--------------------------
class SimpleSensor : public GeneralSensor
{
protected:
    amperia::protocol::DeviceDescriptor device;
    // RegisterCodec accepts a descriptor array plus an active sensor count.
    // Therefore, even a device with one logical sensor stores a one-element
    // array rather than a single standalone descriptor.
    amperia::protocol::SensorDescriptor sensorDescriptors[1];
    amperia::protocol::RegisterCodec registerCodec;

    float currentReading = 0.0f;

    amperia::protocol::RegisterCodec& protocolCodec() override {
        return registerCodec;
    }

public:
    SimpleSensor(
        const char* deviceName,
        const char* sensorName,
        const char* sensorUnit);

    void set(float newValue);

    // Compatibility alias for older sketches.
    void update(float newValue);

    // Update the metadata reported to the multimeter.
    void setDataPeriodMs(uint16_t periodMs);

    // Publish the current value to the multimeter. publish() timestamps the
    // sample with millis(). publish(value) is the short form for set(value)
    // followed by publish().
    void publish();
    void publish(float value);

    // Advanced form for an explicitly supplied timestamp.
    void publishAt(uint32_t timestampMs);

    void setTimestamp(uint32_t ts);
};


// Definition: one-dimensional sensor whose measurement is supplied by a
// user function. AgentSensor automates reading, updating and publishing while
// retaining SimpleSensor's manual API.
// AgentSensor -- 1.1
// ----------------AgentSensor--------------------------
class AgentSensor : public SimpleSensor
{
private:
    AgentMeasurementFunction measurementFunction;

public:
    AgentSensor(
        const char* deviceName,
        const char* sensorName,
        const char* sensorUnit,
        AgentMeasurementFunction function = nullptr);

    void setMeasurementFunction(AgentMeasurementFunction function);
    bool hasMeasurementFunction() const;

    // Perform one measurement. A failed callback leaves the current value and
    // timestamp unchanged.
    bool measure();

    // Measure and publish only when the callback succeeds.
    bool measureAndPublish();
    bool measureAndPublish(uint32_t timestampMs);

};

// Second branch -- N-dimension Sensors
// Definition: Sensors with two or more measurements & multiple operating pins
// ----------------NDimSensor--------------------------
class NDimSensor : public GeneralSensor
{
protected:
    amperia::protocol::DeviceDescriptor device;
    uint8_t totalDimensions;
    // RegisterCodec accepts a descriptor array plus an active sensor count.
    // Therefore, even a device with one logical sensor stores a one-element
    // array rather than a single standalone descriptor.
    amperia::protocol::SensorDescriptor sensorDescriptors[1];
    float values[amperia::protocol::kMaxSensorDimension];
    amperia::protocol::RegisterCodec registerCodec;
    bool validConfiguration;

    bool publishValues(const float* newValues, size_t count);

    amperia::protocol::RegisterCodec& protocolCodec() override {
        return registerCodec;
    }

public:
    NDimSensor(
        const char* deviceName,
        const char* sensorName,
        const char* sensorUnit,
        int dimension);

    virtual ~NDimSensor();

    // A logical sensor may contain up to four homogeneous components.
    // Dimensions 1-3 are recommended because the current multimeter UI
    // displays the fourth component less conveniently.
    bool isValid() const;
    uint8_t dimension() const;

    bool set(int index, float value);
    bool set(int index, int32_t value);

    // Compatibility aliases for older sketches.
    void update(int index, float value);
    void update(int index, int32_t value);

    void setDataPeriodMs(uint16_t periodMs);
    void setTimestamp(uint32_t ts);
    void publish();
    void publish(uint32_t timestampMs);

    // Short forms for publishing all components of a 2D, 3D or 4D sensor.
    // They return false when the number of supplied values does not match the
    // configured sensor dimension.
    bool publish(float value0, float value1);
    bool publish(float value0, float value1, float value2);
    bool publish(float value0, float value1, float value2, float value3);

};

// Class N Sensors
// Definition: Multiple one-dimensional logical sensors exposed as one device.
class NSensors : public GeneralSensor {
protected:
    amperia::protocol::DeviceDescriptor combinedDevice;
    amperia::protocol::SensorDescriptor sensorDescriptors[
        amperia::protocol::kMaxLogicalSensors];
    float sensorValues[
        amperia::protocol::kMaxLogicalSensors *
        amperia::protocol::kMaxSensorDimension];
    uint8_t maxFloats;
    uint8_t sensorCount;
    uint8_t totalComponents;
    uint8_t sensorOffsets[amperia::protocol::kMaxLogicalSensors];
    amperia::protocol::RegisterCodec registerCodec;
    bool finalized;
    bool started;

    amperia::protocol::RegisterCodec& protocolCodec() override {
        return registerCodec;
    }

    int addSensorImpl(
        const char* sensorName,
        const char* unit,
        bool requireConfigurable);
    bool setValue(int handle, float value, bool requireFinalized);
    void finalizeConfiguration();

public:
    explicit NSensors(const char* deviceName);

    [[deprecated("use NSensors(deviceName)")]]
    NSensors(const char* deviceName, int totalExpectedFloats);

    virtual ~NSensors();

    int addSensor(const char* name, const char* unit);
    bool set(int handle, float value);
    bool setDataPeriodMs(int handle, uint16_t periodMs);

    bool begin();
    bool begin(TwoWire& wire, uint8_t dataReadyPin, bool wireInit = true);

    [[deprecated("use addSensor()")]]
    bool addDevice(const char* sensorName, const char* unit);

    [[deprecated("begin() finalizes automatically")]]
    void finalize();

    [[deprecated("use set()")]]
    void update(int devIdx, float value);

    void setTimestamp(uint32_t ts);
    void publish();
    void publish(uint32_t timestampMs);
};

// Class Combined Sensor
// Definition: Multiple logical sensors, including grouped N-dimensional ones,
// exposed as one device.
class CombinedSensor : public GeneralSensor {
private:
    amperia::protocol::DeviceDescriptor combinedDevice;
    amperia::protocol::SensorDescriptor sensorDescriptors[
        amperia::protocol::kMaxLogicalSensors];
    float sensorValues[
        amperia::protocol::kMaxLogicalSensors *
        amperia::protocol::kMaxSensorDimension];
    uint8_t maxFloats;
    uint8_t sensorDescriptorCount;
    uint8_t deviceCount;
    uint8_t totalComponents;
    uint8_t deviceOffsets[amperia::protocol::kMaxLogicalSensors];
    uint8_t deviceDimensions[amperia::protocol::kMaxLogicalSensors];
    uint8_t deviceDescriptorOffsets[amperia::protocol::kMaxLogicalSensors];
    uint8_t deviceDescriptorCounts[amperia::protocol::kMaxLogicalSensors];
    amperia::protocol::RegisterCodec registerCodec;
    bool finalized;
    bool started;

    amperia::protocol::RegisterCodec& protocolCodec() override {
        return registerCodec;
    }

    int addDeviceImpl(
        const char* devName,
        int sCount,
        const char** sNames,
        const char** sUnits,
        bool isGrouped,
        bool requireConfigurable);
    bool setValue(
        int handle,
        size_t component,
        float value,
        bool requireFinalized);
    void finalizeConfiguration();

    friend class AmperiaBridge;
    int addSensorWithComponents(
        const char* name,
        const char* unit,
        size_t dimension,
        const char* const* componentNames);

public:
    explicit CombinedSensor(const char* deviceName);

    [[deprecated("use CombinedSensor(deviceName)")]]
    CombinedSensor(int totalExpectedFloats);

    virtual ~CombinedSensor();

    int addSensor(const char* name, const char* unit);
    int addSensor(const char* name, const char* unit, size_t dimension);
    int addSensor(
        const char* name,
        const char* unit,
        const char* component0,
        const char* component1);
    int addSensor(
        const char* name,
        const char* unit,
        const char* component0,
        const char* component1,
        const char* component2);
    int addSensor(
        const char* name,
        const char* unit,
        const char* component0,
        const char* component1,
        const char* component2,
        const char* component3);

    bool set(int handle, float value);
    bool set(int handle, size_t component, float value);
    bool setDataPeriodMs(int handle, uint16_t periodMs);

    bool begin();
    bool begin(TwoWire& wire, uint8_t dataReadyPin, bool wireInit = true);

    bool isFinalized() const;

    // isGrouped = true  => one N-dimensional logical sensor.
    // isGrouped = false => sCount one-dimensional logical sensors.
    [[deprecated("use addSensor()")]]
    bool addDevice(const char* devName, int sCount, const char** sNames,
                   const char** sUnits, bool isGrouped);

    [[deprecated("begin() finalizes automatically")]]
    void finalize();

    [[deprecated("use set()")]]
    void update(int devIdx, int dimIdx, float value);

    void setTimestamp(uint32_t ts);
    void publish();
    void publish(uint32_t timestampMs);

    [[deprecated("use isFinalized()")]]
    bool hasDevice() const {
        return isFinalized();
    }
};

#endif