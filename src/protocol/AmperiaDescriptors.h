#pragma once

#include "AmperiaProtocol.h"

namespace amperia
{
namespace protocol
{
    struct DeviceDescriptor
    {
        char name[kDeviceNameSize];
        uint8_t hardwareVersion;
        uint8_t firmwareVersion;
        uint8_t PROTOCOL_VERSION;
        // Current supported range: 0..kMaxLogicalSensors.
        uint8_t numSensors;
        uint8_t operationMode;
    };

    struct SensorDescriptor
    {
        char name[kSensorNameSize];
        char unit[kUnitNameSize];

        // Supported range: 1..4. Values above 3 are discouraged because
        // the current multimeter UI displays the fourth component poorly.
        uint16_t dimension;

        // Measurement update period in milliseconds.
        uint16_t dataPeriod;

        DataType valueType;
        float valueResolution;

        // TODO(protocol-validation): The inherited protocol model contains
        // both valueResolution and resolutions[4]. Keep both fields until a
        // real Amperia Multimeter test establishes whether resolutions[4] is
        // queried, ignored, interpreted, or causes a compatibility problem.
        float resolutions[kMaxSensorDimension];

        float valueMin;
        float valueMax;
        char keyName[kKeyNameSize];
        char keyUnit[kUnitNameSize];
        DataType keyType;
        float keyResolution;
        float keyMin;
        float keyMax;
        uint16_t flags;
        uint32_t timestamp;
        uint8_t *values;
        uint8_t *keys;

        inline bool hasFlag(SensorFlags value) const
        {
            return (flags & static_cast<uint16_t>(value)) ==
                   static_cast<uint16_t>(value);
        }

        inline size_t valuesSize() const
        {
            return dimension * dataTypeSize(valueType);
        }

        inline size_t keysSize() const
        {
            return dimension * dataTypeSize(keyType);
        }
    };
}
}
