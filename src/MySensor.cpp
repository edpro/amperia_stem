#include "MySensor.h"
#include "internal/AmperiaText.h"


// ----------------GeneralSensor--------------------------

GeneralSensor::~GeneralSensor() = default;

bool GeneralSensor::begin()
{
#if defined(ARDUINO_UNOR4_WIFI)
    return begin(
        Wire1,
        amperia::board::unoR4Wifi::kMultimeterDataReadyPin,
        true);
#else
    // No implicit pin or Wire choice is safe on unsupported boards. Future
    // platform adapters can provide a board-specific default without changing
    // the protocol or sensor classes.
    return false;
#endif
}

bool GeneralSensor::begin(TwoWire& wire,
                          uint8_t dataReadyPin,
                          bool wireInit)
{
    return deviceServer.begin(
        wire,
        dataReadyPin,
        protocolCodec(),
        wireInit);
}

void GeneralSensor::signalDataReady(uint16_t pulseWidthUs)
{
    deviceServer.signalDataReady(pulseWidthUs);
}

// ----------------SimpleSensor--------------------------
SimpleSensor::SimpleSensor(
    const char* deviceName,
    const char* sensorName,
    const char* sensorUnit)
: device{},
  sensorDescriptors{},
  registerCodec(device, sensorDescriptors, 1)
{
    amperia::detail::copyText(
        device.name,
        sizeof(device.name),
        deviceName);

    device.hardwareVersion = 1;
    device.firmwareVersion = 1;
    device.PROTOCOL_VERSION = amperia::protocol::kProtocolVersion;
    device.numSensors = 1;
    device.operationMode = 0;

    amperia::protocol::SensorDescriptor& sensor = sensorDescriptors[0];
    amperia::detail::copyText(
        sensor.name,
        sizeof(sensor.name),
        sensorName);
    amperia::detail::copyText(
        sensor.unit,
        sizeof(sensor.unit),
        sensorUnit);

    sensor.dimension = 1;
    sensor.dataPeriod = amperia::kDefaultDataPeriodMs;
    sensor.valueType = amperia::protocol::DataType::Float;
    sensor.valueResolution = 0.01f;
    sensor.valueMin = 0.001f;
    sensor.valueMax = 1000.0f;

    memset(sensor.keyName, 0, sizeof(sensor.keyName));
    memset(sensor.keyUnit, 0, sizeof(sensor.keyUnit));
    sensor.keyType = amperia::protocol::DataType::Char;
    
    sensor.flags = 0;
    sensor.timestamp = 0;

    sensor.values = reinterpret_cast<uint8_t*>(&currentReading);
    sensor.keys = nullptr;

}

void SimpleSensor::set(float newValue)
{
    currentReading = newValue;
}

void SimpleSensor::update(float newValue)
{
    set(newValue);
}

void SimpleSensor::setDataPeriodMs(uint16_t periodMs)
{
    sensorDescriptors[0].dataPeriod = periodMs;
}

void SimpleSensor::publish()
{
    publishAt(millis());
}

void SimpleSensor::publish(float value)
{
    set(value);
    publish();
}

void SimpleSensor::publishAt(uint32_t timestampMs)
{
    setTimestamp(timestampMs);
    signalDataReady();
}

void SimpleSensor::setTimestamp(uint32_t ts)
{
    sensorDescriptors[0].timestamp = ts;
}

// ----------------AgentSensor--------------------------

AgentSensor::AgentSensor(
    const char* deviceName,
    const char* sensorName,
    const char* sensorUnit,
    AgentMeasurementFunction function)
: SimpleSensor(deviceName, sensorName, sensorUnit),
  measurementFunction(function)
{}

void AgentSensor::setMeasurementFunction(AgentMeasurementFunction function)
{
    measurementFunction = function;
}

bool AgentSensor::hasMeasurementFunction() const
{
    return measurementFunction != nullptr;
}

bool AgentSensor::measure()
{
    if (measurementFunction == nullptr) {
        return false;
    }

    float measuredValue = 0.0f;
    if (!measurementFunction(measuredValue)) {
        return false;
    }

    SimpleSensor::set(measuredValue);
    return true;
}

bool AgentSensor::measureAndPublish()
{
    return measureAndPublish(millis());
}

bool AgentSensor::measureAndPublish(uint32_t timestampMs)
{
    if (!measure()) {
        return false;
    }

    SimpleSensor::publishAt(timestampMs);
    return true;
}

// ----------------NDimSensor--------------------------

NDimSensor::NDimSensor(
    const char* deviceName,
    const char* sensorName,
    const char* sensorUnit,
    int dimension)
: device{},
  totalDimensions(
      dimension >= 1 &&
      dimension <= static_cast<int>(amperia::protocol::kMaxSensorDimension)
          ? static_cast<uint8_t>(dimension)
          : 1),
  sensorDescriptors{},
  values{},
  registerCodec(device, sensorDescriptors, 1),
  validConfiguration(
      dimension >= 1 &&
      dimension <= static_cast<int>(amperia::protocol::kMaxSensorDimension))
{
    amperia::detail::copyText(
        device.name,
        sizeof(device.name),
        deviceName);
    device.hardwareVersion = 1;
    device.firmwareVersion = 1;
    device.PROTOCOL_VERSION = amperia::protocol::kProtocolVersion;
    device.numSensors = 1;
    device.operationMode = 0;

    amperia::protocol::SensorDescriptor& sensor = sensorDescriptors[0];
    amperia::detail::copyText(
        sensor.name,
        sizeof(sensor.name),
        sensorName);
    amperia::detail::copyText(
        sensor.unit,
        sizeof(sensor.unit),
        sensorUnit);

    sensor.dimension = totalDimensions;
    sensor.dataPeriod = amperia::kDefaultDataPeriodMs;
    sensor.valueType = amperia::protocol::DataType::Float;
    sensor.valueResolution = 0.01f;

    // TODO(protocol-validation): Check on a real Amperia Multimeter whether
    // per-component resolutions are read, ignored, or affect compatibility.
    // The canonical scalar valueResolution above remains authoritative.
    for (uint8_t index = 0;
         index < amperia::protocol::kMaxSensorDimension;
         ++index) {
        sensor.resolutions[index] = sensor.valueResolution;
    }

    sensor.valueMin = 0.0f;
    sensor.valueMax = 1000.0f;
    sensor.keyType = amperia::protocol::DataType::Char;
    sensor.flags = 0;
    sensor.timestamp = 0;
    sensor.values = reinterpret_cast<uint8_t*>(values);
    sensor.keys = nullptr;
}

NDimSensor::~NDimSensor() = default;

bool NDimSensor::isValid() const
{
    return validConfiguration;
}

uint8_t NDimSensor::dimension() const
{
    return totalDimensions;
}

bool NDimSensor::set(int index, float value)
{
    if (index < 0 || index >= totalDimensions) {
        return false;
    }

    values[index] = value;
    return true;
}

bool NDimSensor::set(int index, int32_t value)
{
    return set(index, static_cast<float>(value));
}

void NDimSensor::update(int index, float value)
{
    (void)set(index, value);
}

void NDimSensor::update(int index, int32_t value)
{
    (void)set(index, value);
}

void NDimSensor::setDataPeriodMs(uint16_t periodMs)
{
    sensorDescriptors[0].dataPeriod = periodMs;
}

void NDimSensor::setTimestamp(uint32_t ts)
{
    sensorDescriptors[0].timestamp = ts;
}

void NDimSensor::publish()
{
    publish(millis());
}

void NDimSensor::publish(uint32_t timestampMs)
{
    setTimestamp(timestampMs);
    signalDataReady();
}

bool NDimSensor::publishValues(const float* newValues, size_t count)
{
    if (!validConfiguration || newValues == nullptr ||
        count != totalDimensions) {
        return false;
    }

    for (size_t index = 0; index < count; ++index) {
        values[index] = newValues[index];
    }

    publish();
    return true;
}

bool NDimSensor::publish(float value0, float value1)
{
    const float newValues[] = {value0, value1};
    return publishValues(newValues, 2);
}

bool NDimSensor::publish(float value0, float value1, float value2)
{
    const float newValues[] = {value0, value1, value2};
    return publishValues(newValues, 3);
}

bool NDimSensor::publish(
    float value0, float value1, float value2, float value3)
{
    const float newValues[] = {value0, value1, value2, value3};
    return publishValues(newValues, 4);
}

// ----------------NSensors--------------------------
namespace
{
    constexpr size_t kMaxStoredValues =
        amperia::protocol::kMaxLogicalSensors *
        amperia::protocol::kMaxSensorDimension;

    uint8_t clampCapacity(int requested, size_t maximum)
    {
        if (requested <= 0) {
            return 0;
        }

        if (static_cast<size_t>(requested) > maximum) {
            return static_cast<uint8_t>(maximum);
        }

        return static_cast<uint8_t>(requested);
    }

    void appendToken(char* destination,
                     size_t destinationSize,
                     const char* token)
    {
        if (destinationSize == 0 || token == nullptr || token[0] == '\0') {
            return;
        }

        const size_t used = strlen(destination);
        if (used >= destinationSize - 1) {
            return;
        }

        if (used != 0) {
            strncat(destination, " ", destinationSize - strlen(destination) - 1);
        }
        strncat(destination, token, destinationSize - strlen(destination) - 1);
    }

    void initializeCommonSensorFields(amperia::protocol::SensorDescriptor& sensor,
                                      const char* name,
                                      const char* unit,
                                      uint16_t dimension,
                                      float resolution,
                                      float* values)
    {
        memset(&sensor, 0, sizeof(sensor));
        amperia::detail::copyText(
            sensor.name,
            sizeof(sensor.name),
            name);
        amperia::detail::copyText(
            sensor.unit,
            sizeof(sensor.unit),
            unit);
        sensor.dimension = dimension;
        sensor.dataPeriod = amperia::kDefaultDataPeriodMs;
        sensor.valueType = amperia::protocol::DataType::Float;
        sensor.valueResolution = resolution;
        sensor.valueMin = -50.0f;
        sensor.valueMax = 5000.0f;
        sensor.values = reinterpret_cast<uint8_t*>(values);
        sensor.keys = nullptr;
    }
}

NSensors::NSensors(const char* deviceName) :
    combinedDevice{},
    sensorDescriptors{},
    sensorValues{},
    maxFloats(amperia::protocol::kMaxLogicalSensors),
    sensorCount(0),
    totalComponents(0),
    sensorOffsets{},
    registerCodec(
        combinedDevice,
        sensorDescriptors,
        0,
        amperia::protocol::kMaxLogicalSensors),
    finalized(false),
    started(false)
{
    amperia::detail::copyText(
        combinedDevice.name,
        sizeof(combinedDevice.name),
        deviceName);
    combinedDevice.PROTOCOL_VERSION = amperia::protocol::kProtocolVersion;
    combinedDevice.hardwareVersion = 1;
    combinedDevice.firmwareVersion = 1;
}

NSensors::NSensors(const char* deviceName, int totalExpectedFloats) :
    combinedDevice{},
    sensorDescriptors{},
    sensorValues{},
    maxFloats(clampCapacity(
        totalExpectedFloats,
        amperia::protocol::kMaxLogicalSensors)),
    sensorCount(0),
    totalComponents(0),
    sensorOffsets{},
    registerCodec(
        combinedDevice,
        sensorDescriptors,
        0,
        amperia::protocol::kMaxLogicalSensors),
    finalized(false),
    started(false)
{
    amperia::detail::copyText(
        combinedDevice.name,
        sizeof(combinedDevice.name),
        deviceName);
    combinedDevice.PROTOCOL_VERSION = amperia::protocol::kProtocolVersion;
    combinedDevice.hardwareVersion = 1;
    combinedDevice.firmwareVersion = 1;
}

int NSensors::addSensorImpl(
    const char* sensorName,
    const char* unit,
    bool requireConfigurable)
{
    if ((requireConfigurable && (finalized || started)) ||
        sensorName == nullptr || unit == nullptr ||
        sensorCount >= amperia::protocol::kMaxLogicalSensors ||
        totalComponents >= maxFloats) {
        return -1;
    }

    const int handle = sensorCount;
    const uint8_t offset = totalComponents;
    amperia::protocol::SensorDescriptor& sensor = sensorDescriptors[sensorCount];
    initializeCommonSensorFields(
        sensor,
        sensorName,
        unit,
        1,
        0.001f,
        &sensorValues[offset]);

    amperia::detail::copyText(
        sensor.keyName,
        sizeof(sensor.keyName),
        sensorName);
    amperia::detail::copyText(
        sensor.keyUnit,
        sizeof(sensor.keyUnit),
        unit);
    sensor.flags = 1;

    sensorOffsets[sensorCount] = offset;
    ++sensorCount;
    ++totalComponents;

    finalized = false;
    combinedDevice.numSensors = 0;
    registerCodec.setSensorCount(0);
    return handle;
}

int NSensors::addSensor(const char* name, const char* unit)
{
    return addSensorImpl(name, unit, true);
}

bool NSensors::addDevice(const char* sensorName, const char* unit)
{
    return addSensorImpl(sensorName, unit, false) >= 0;
}

void NSensors::finalizeConfiguration()
{
    combinedDevice.numSensors = sensorCount;

    for (uint8_t index = 0; index < sensorCount; ++index) {
        sensorDescriptors[index].timestamp = 1000;
    }

    registerCodec.setSensorCount(sensorCount);
    finalized = true;
}

void NSensors::finalize()
{
    finalizeConfiguration();
}

bool NSensors::begin()
{
    if (!finalized) {
        finalizeConfiguration();
    }

    if (started) {
        return true;
    }

    started = GeneralSensor::begin();
    return started;
}

bool NSensors::begin(TwoWire& wire,
                     uint8_t dataReadyPin,
                     bool wireInit)
{
    if (!finalized) {
        finalizeConfiguration();
    }

    if (started) {
        return true;
    }

    started = GeneralSensor::begin(wire, dataReadyPin, wireInit);
    return started;
}

NSensors::~NSensors() = default;

bool NSensors::setValue(int handle, float value, bool requireFinalized)
{
    if ((requireFinalized && !finalized) ||
        handle < 0 || handle >= sensorCount) {
        return false;
    }

    const uint8_t dataIndex = sensorOffsets[handle];
    if (dataIndex >= totalComponents || dataIndex >= maxFloats) {
        return false;
    }

    sensorValues[dataIndex] = value;
    return true;
}

bool NSensors::set(int handle, float value)
{
    return setValue(handle, value, true);
}

bool NSensors::setDataPeriodMs(int handle, uint16_t periodMs)
{
    if (started || handle < 0 || handle >= sensorCount) {
        return false;
    }

    sensorDescriptors[static_cast<uint8_t>(handle)].dataPeriod = periodMs;
    return true;
}

void NSensors::update(int devIdx, float value)
{
    (void)setValue(devIdx, value, false);
}

void NSensors::setTimestamp(uint32_t ts)
{
    for (uint8_t index = 0; index < sensorCount; ++index) {
        sensorDescriptors[index].timestamp = ts;
    }
}

void NSensors::publish()
{
    publish(millis());
}

void NSensors::publish(uint32_t timestampMs)
{
    setTimestamp(timestampMs);
    signalDataReady();
}

//-----------Combined Sensor-----------
CombinedSensor::CombinedSensor(const char* deviceName) :
    combinedDevice{},
    sensorDescriptors{},
    sensorValues{},
    maxFloats(static_cast<uint8_t>(kMaxStoredValues)),
    sensorDescriptorCount(0),
    deviceCount(0),
    totalComponents(0),
    deviceOffsets{},
    deviceDimensions{},
    deviceDescriptorOffsets{},
    deviceDescriptorCounts{},
    registerCodec(
        combinedDevice,
        sensorDescriptors,
        0,
        amperia::protocol::kMaxLogicalSensors),
    finalized(false),
    started(false)
{
    amperia::detail::copyText(
        combinedDevice.name,
        sizeof(combinedDevice.name),
        deviceName);
    combinedDevice.PROTOCOL_VERSION = amperia::protocol::kProtocolVersion;
    combinedDevice.hardwareVersion = 1;
    combinedDevice.firmwareVersion = 1;
}

CombinedSensor::CombinedSensor(int totalExpectedFloats) :
    combinedDevice{},
    sensorDescriptors{},
    sensorValues{},
    maxFloats(clampCapacity(totalExpectedFloats, kMaxStoredValues)),
    sensorDescriptorCount(0),
    deviceCount(0),
    totalComponents(0),
    deviceOffsets{},
    deviceDimensions{},
    deviceDescriptorOffsets{},
    deviceDescriptorCounts{},
    registerCodec(
        combinedDevice,
        sensorDescriptors,
        0,
        amperia::protocol::kMaxLogicalSensors),
    finalized(false),
    started(false)
{
    amperia::detail::copyText(
        combinedDevice.name,
        sizeof(combinedDevice.name),
        "AmperiaShield");
    combinedDevice.PROTOCOL_VERSION = amperia::protocol::kProtocolVersion;
    combinedDevice.hardwareVersion = 1;
    combinedDevice.firmwareVersion = 1;
}

int CombinedSensor::addDeviceImpl(
    const char* devName,
    int sCount,
    const char** sNames,
    const char** sUnits,
    bool isGrouped,
    bool requireConfigurable)
{
    if ((requireConfigurable && (finalized || started)) ||
        devName == nullptr || sNames == nullptr || sUnits == nullptr ||
        sCount <= 0 ||
        deviceCount >= amperia::protocol::kMaxLogicalSensors) {
        return -1;
    }

    const size_t componentCount = static_cast<size_t>(sCount);
    const size_t availableConfiguredValues =
        static_cast<size_t>(maxFloats - totalComponents);
    const size_t availableStoredValues =
        kMaxStoredValues - totalComponents;
    if (componentCount > availableConfiguredValues ||
        componentCount > availableStoredValues) {
        return -1;
    }

    if (isGrouped) {
        if (!amperia::protocol::isSupportedDimension(componentCount) ||
            sensorDescriptorCount >=
                amperia::protocol::kMaxLogicalSensors) {
            return -1;
        }
    } else if (componentCount >
               amperia::protocol::kMaxLogicalSensors - sensorDescriptorCount) {
        return -1;
    }

    const int handle = deviceCount;
    const uint8_t firstValueOffset = totalComponents;
    const uint8_t firstDescriptorOffset = sensorDescriptorCount;
    deviceOffsets[deviceCount] = firstValueOffset;
    deviceDimensions[deviceCount] = static_cast<uint8_t>(componentCount);
    deviceDescriptorOffsets[deviceCount] = firstDescriptorOffset;
    deviceDescriptorCounts[deviceCount] =
        static_cast<uint8_t>(isGrouped ? 1 : componentCount);

    if (isGrouped) {
        amperia::protocol::SensorDescriptor& sensor =
            sensorDescriptors[sensorDescriptorCount];
        initializeCommonSensorFields(
            sensor,
            devName,
            sUnits[0],
            static_cast<uint16_t>(componentCount),
            0.01f,
            &sensorValues[firstValueOffset]);

        for (int index = 0; index < sCount; ++index) {
            appendToken(sensor.keyName, sizeof(sensor.keyName), sNames[index]);
            appendToken(sensor.keyUnit, sizeof(sensor.keyUnit), sUnits[index]);
        }
        sensor.flags = 1;

        ++sensorDescriptorCount;
    } else {
        for (int index = 0; index < sCount; ++index) {
            amperia::protocol::SensorDescriptor& sensor =
                sensorDescriptors[sensorDescriptorCount];

            char sensorName[amperia::protocol::kSensorNameSize] = {};
            const char* componentName = sNames[index];
            if (componentName != nullptr && componentName[0] != '\0') {
                snprintf(
                    sensorName,
                    sizeof(sensorName),
                    "%s: %s",
                    devName,
                    componentName);
            } else {
                amperia::detail::copyText(
                    sensorName,
                    sizeof(sensorName),
                    devName);
            }

            initializeCommonSensorFields(
                sensor,
                sensorName,
                sUnits[index],
                1,
                0.01f,
                &sensorValues[firstValueOffset + index]);

            ++sensorDescriptorCount;
        }
    }

    totalComponents = static_cast<uint8_t>(
        totalComponents + componentCount);
    ++deviceCount;

    finalized = false;
    combinedDevice.numSensors = 0;
    registerCodec.setSensorCount(0);
    return handle;
}

int CombinedSensor::addSensor(const char* name, const char* unit)
{
    return addSensor(name, unit, 1);
}

int CombinedSensor::addSensor(
    const char* name,
    const char* unit,
    size_t dimension)
{
    const char* componentNames[amperia::protocol::kMaxSensorDimension] = {
        "1", "2", "3", "4"
    };

    return addSensorWithComponents(
        name,
        unit,
        dimension,
        componentNames);
}

int CombinedSensor::addSensor(
    const char* name,
    const char* unit,
    const char* component0,
    const char* component1)
{
    const char* componentNames[] = {
        component0, component1
    };

    return addSensorWithComponents(
        name,
        unit,
        2,
        componentNames);
}

int CombinedSensor::addSensor(
    const char* name,
    const char* unit,
    const char* component0,
    const char* component1,
    const char* component2)
{
    const char* componentNames[] = {
        component0, component1, component2
    };

    return addSensorWithComponents(
        name,
        unit,
        3,
        componentNames);
}

int CombinedSensor::addSensor(
    const char* name,
    const char* unit,
    const char* component0,
    const char* component1,
    const char* component2,
    const char* component3)
{
    const char* componentNames[] = {
        component0, component1, component2, component3
    };

    return addSensorWithComponents(
        name,
        unit,
        4,
        componentNames);
}

int CombinedSensor::addSensorWithComponents(
    const char* name,
    const char* unit,
    size_t dimension,
    const char* const* componentNames)
{
    if (name == nullptr || unit == nullptr || componentNames == nullptr ||
        !amperia::protocol::isSupportedDimension(dimension)) {
        return -1;
    }

    const char* names[amperia::protocol::kMaxSensorDimension] = {};
    const char* units[amperia::protocol::kMaxSensorDimension] = {};
    for (size_t component = 0; component < dimension; ++component) {
        if (componentNames[component] == nullptr) {
            return -1;
        }
        names[component] = componentNames[component];
        units[component] = unit;
    }

    return addDeviceImpl(
        name,
        static_cast<int>(dimension),
        names,
        units,
        true,
        true);
}

bool CombinedSensor::addDevice(const char* devName,
                               int sCount,
                               const char** sNames,
                               const char** sUnits,
                               bool isGrouped)
{
    return addDeviceImpl(
               devName,
               sCount,
               sNames,
               sUnits,
               isGrouped,
               false) >= 0;
}

void CombinedSensor::finalizeConfiguration()
{
    combinedDevice.numSensors = sensorDescriptorCount;

    for (uint8_t index = 0; index < sensorDescriptorCount; ++index) {
        sensorDescriptors[index].timestamp = 1000;
    }

    registerCodec.setSensorCount(sensorDescriptorCount);
    finalized = true;
}

void CombinedSensor::finalize()
{
    finalizeConfiguration();
}

bool CombinedSensor::begin()
{
    if (!finalized) {
        finalizeConfiguration();
    }

    if (started) {
        return true;
    }

    started = GeneralSensor::begin();
    return started;
}

bool CombinedSensor::begin(TwoWire& wire,
                           uint8_t dataReadyPin,
                           bool wireInit)
{
    if (!finalized) {
        finalizeConfiguration();
    }

    if (started) {
        return true;
    }

    started = GeneralSensor::begin(wire, dataReadyPin, wireInit);
    return started;
}

bool CombinedSensor::isFinalized() const
{
    return finalized;
}

bool CombinedSensor::setValue(
    int handle,
    size_t component,
    float value,
    bool requireFinalized)
{
    if ((requireFinalized && !finalized) ||
        handle < 0 || handle >= deviceCount) {
        return false;
    }

    const uint8_t deviceIndex = static_cast<uint8_t>(handle);
    if (component >= deviceDimensions[deviceIndex]) {
        return false;
    }

    const size_t dataIndex = deviceOffsets[deviceIndex] + component;
    if (dataIndex >= totalComponents || dataIndex >= maxFloats) {
        return false;
    }

    sensorValues[dataIndex] = value;
    return true;
}

bool CombinedSensor::set(int handle, float value)
{
    if (handle < 0 || handle >= deviceCount ||
        deviceDimensions[static_cast<uint8_t>(handle)] != 1) {
        return false;
    }

    return setValue(handle, 0, value, true);
}

bool CombinedSensor::set(int handle, size_t component, float value)
{
    return setValue(handle, component, value, true);
}

bool CombinedSensor::setDataPeriodMs(int handle, uint16_t periodMs)
{
    if (started || handle < 0 || handle >= deviceCount) {
        return false;
    }

    const uint8_t deviceIndex = static_cast<uint8_t>(handle);
    const uint8_t firstDescriptor = deviceDescriptorOffsets[deviceIndex];
    const uint8_t descriptorCount = deviceDescriptorCounts[deviceIndex];

    if (descriptorCount == 0 ||
        firstDescriptor >= sensorDescriptorCount ||
        static_cast<size_t>(firstDescriptor) + descriptorCount >
            sensorDescriptorCount) {
        return false;
    }

    for (uint8_t index = 0; index < descriptorCount; ++index) {
        sensorDescriptors[firstDescriptor + index].dataPeriod = periodMs;
    }

    return true;
}

void CombinedSensor::update(int devIdx, int dimIdx, float value)
{
    if (dimIdx < 0) {
        return;
    }

    (void)setValue(
        devIdx,
        static_cast<size_t>(dimIdx),
        value,
        false);
}

void CombinedSensor::setTimestamp(uint32_t ts)
{
    for (uint8_t index = 0; index < sensorDescriptorCount; ++index) {
        sensorDescriptors[index].timestamp = ts;
    }
}

void CombinedSensor::publish()
{
    publish(millis());
}

void CombinedSensor::publish(uint32_t timestampMs)
{
    setTimestamp(timestampMs);
    signalDataReady();
}

CombinedSensor::~CombinedSensor() = default;
