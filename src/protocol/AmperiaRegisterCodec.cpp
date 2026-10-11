#include "AmperiaRegisterCodec.h"

#include <string.h>

namespace amperia
{
namespace protocol
{
    template<typename T>
    [[nodiscard]]
    inline int copyField(const T &field, uint8_t *buffer, size_t bufferSize)
    {
        if (sizeof(field) > bufferSize) {
            return 0;
        }

        memcpy(buffer, &field, sizeof(field));
        return sizeof(field);
    }

    [[nodiscard]]
    int RegisterCodec::getData(uint8_t reg,
                               uint8_t *buffer,
                               size_t bufferSize)
    {
        if (buffer == nullptr || bufferSize == 0) {
            return 0;
        }

        if (reg < kSensorRegisterBase) {
            return getDeviceData(reg, buffer, bufferSize);
        }

        int sensorIndex = 0;
        while (reg >= kSensorRegisterBase + kSensorRegisterStep) {
            ++sensorIndex;
            reg -= kSensorRegisterStep;
        }

        if (sensorIndex < m_sensorCount) {
            return getSensorData(sensorIndex, reg, buffer, bufferSize);
        }

        return 0;
    }

    [[nodiscard]]
    int RegisterCodec::getDeviceData(uint8_t reg,
                                     uint8_t *buffer,
                                     size_t bufferSize) const
    {
        switch (reg) {
            case registers::kDeviceName:
                return copyField(m_device.name, buffer, bufferSize);
            case registers::kHardwareVersion:
                return copyField(m_device.hardwareVersion, buffer, bufferSize);
            case registers::kFirmwareVersion:
                return copyField(m_device.firmwareVersion, buffer, bufferSize);
            case registers::kProtocolVersion:
                return copyField(m_device.PROTOCOL_VERSION, buffer, bufferSize);
            case registers::kSensorCount:
                return copyField(m_device.numSensors, buffer, bufferSize);
            case registers::kOperationMode:
                return copyField(m_device.operationMode, buffer, bufferSize);
            default:
                return 0;
        }
    }

    [[nodiscard]]
    int RegisterCodec::getSensorData(uint8_t sensorIndex,
                                     uint8_t reg,
                                     uint8_t *buffer,
                                     size_t bufferSize) const
    {
        if (sensorIndex >= m_device.numSensors) {
            return 0;
        }

        const SensorDescriptor &sensor = m_sensors[sensorIndex];

        switch (reg) {
            case registers::kSensorName:
                return copyField(sensor.name, buffer, bufferSize);
            case registers::kSensorUnit:
                return copyField(sensor.unit, buffer, bufferSize);
            case registers::kSensorDimension:
                return copyField(sensor.dimension, buffer, bufferSize);
            case registers::kSensorDataPeriod:
                return copyField(sensor.dataPeriod, buffer, bufferSize);

            case registers::kSensorValueType:
                return copyField(sensor.valueType, buffer, bufferSize);
            case registers::kSensorValueResolution:
                return copyField(sensor.valueResolution, buffer, bufferSize);
            case registers::kSensorValueMin:
                return copyField(sensor.valueMin, buffer, bufferSize);
            case registers::kSensorValueMax:
                return copyField(sensor.valueMax, buffer, bufferSize);

            case registers::kSensorKeyName:
                return copyField(sensor.keyName, buffer, bufferSize);
            case registers::kSensorKeyUnit:
                return copyField(sensor.keyUnit, buffer, bufferSize);
            case registers::kSensorKeyType:
                return copyField(sensor.keyType, buffer, bufferSize);
            case registers::kSensorKeyResolution:
                return copyField(sensor.keyResolution, buffer, bufferSize);
            case registers::kSensorKeyMin:
                return copyField(sensor.keyMin, buffer, bufferSize);
            case registers::kSensorKeyMax:
                return copyField(sensor.keyMax, buffer, bufferSize);
            case registers::kSensorFlags:
                return copyField(sensor.flags, buffer, bufferSize);
            case registers::kSensorTimestamp:
                return copyField(sensor.timestamp, buffer, bufferSize);

            case registers::kSensorValues: {
                const size_t valuesSize = sensor.valuesSize();
                if (sensor.values == nullptr || valuesSize == 0 ||
                    bufferSize < kTimestampSize + valuesSize) {
                    return 0;
                }

                memcpy(buffer, &sensor.timestamp, kTimestampSize);
                memcpy(buffer + kTimestampSize, sensor.values, valuesSize);
                return static_cast<int>(valuesSize + kTimestampSize);
            }

            case registers::kSensorKeys: {
                const size_t keysSize = sensor.keysSize();
                if (sensor.keys == nullptr || keysSize == 0 ||
                    bufferSize < kTimestampSize + keysSize) {
                    return 0;
                }

                memcpy(buffer, &sensor.timestamp, kTimestampSize);
                memcpy(buffer + kTimestampSize, sensor.keys, keysSize);
                return static_cast<int>(keysSize + kTimestampSize);
            }

            default:
                return 0;
        }
    }

    size_t RegisterCodec::requiredBufferSize() const
    {
        size_t maxSize = kMaxFieldSize;

        for (int i = 0; i < m_sensorCount; ++i) {
            const SensorDescriptor &sensor = m_sensors[i];
            const size_t valuesSize = sensor.valuesSize();
            const size_t keysSize = sensor.keysSize();

            if (maxSize < valuesSize) {
                maxSize = valuesSize;
            }
            if (maxSize < keysSize) {
                maxSize = keysSize;
            }
        }

        return maxSize + kTimestampSize;
    }
}
}
