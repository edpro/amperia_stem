#pragma once

#include <stddef.h>
#include <stdint.h>

namespace amperia
{
namespace protocol
{
    // Current protocol limits. The multimeter can display four components,
    // but using more than three is discouraged because the fourth component
    // is not presented well by the current UI.
    constexpr size_t kMaxLogicalSensors             = 5;
    constexpr size_t kMaxSensorDimension            = 4;
    constexpr size_t kRecommendedSensorDimension    = 3;

    constexpr size_t kMaxFieldSize                  = 16;
    constexpr size_t kDeviceNameSize                = 16;
    constexpr size_t kSensorNameSize                = 16;
    constexpr size_t kUnitNameSize                  = 12;
    constexpr size_t kKeyNameSize                   = 12;
    constexpr size_t kTimestampSize                 = 4;
    constexpr size_t kMaxValueBytes                 = kMaxSensorDimension * 4;
    constexpr size_t kMaxSensorPayloadSize          = kTimestampSize + kMaxValueBytes;

    // Compatibility policy: v1 is the minimum supported protocol. Newer
    // versions are accepted as long as they keep the v1 core register contract
    // backward-compatible.
    constexpr uint8_t kMinimumSupportedProtocolVersion = 1;
    constexpr uint8_t kProtocolVersion                  = 1;
    constexpr uint8_t kI2cAddress                       = 0x04;

    inline bool isProtocolVersionCompatible(uint8_t version)
    {
        return version >= kMinimumSupportedProtocolVersion;
    }

    constexpr uint8_t kSensorRegisterBase           = 0x20;
    constexpr uint8_t kSensorRegisterStep           = 0x20;

    namespace registers
    {
        constexpr uint8_t kReserved                 = 0x00;
        constexpr uint8_t kDeviceName               = 0x01;
        constexpr uint8_t kHardwareVersion          = 0x02;
        constexpr uint8_t kFirmwareVersion          = 0x03;
        constexpr uint8_t kProtocolVersion          = 0x04;
        constexpr uint8_t kSensorCount              = 0x05;
        constexpr uint8_t kOperationMode            = 0x06;

        constexpr uint8_t kSensorName               = 0x20;
        constexpr uint8_t kSensorUnit               = 0x21;
        constexpr uint8_t kSensorDimension          = 0x22;
        // uint16_t, milliseconds.
        constexpr uint8_t kSensorDataPeriod         = 0x23;
        constexpr uint8_t kSensorValueType          = 0x24;
        constexpr uint8_t kSensorValueResolution    = 0x25;
        constexpr uint8_t kSensorValueMin           = 0x26;
        constexpr uint8_t kSensorValueMax           = 0x27;
        constexpr uint8_t kSensorKeyName            = 0x28;
        constexpr uint8_t kSensorKeyUnit            = 0x29;
        constexpr uint8_t kSensorKeyType            = 0x2A;
        constexpr uint8_t kSensorKeyResolution      = 0x2B;
        constexpr uint8_t kSensorKeyMin             = 0x2C;
        constexpr uint8_t kSensorKeyMax             = 0x2D;
        constexpr uint8_t kSensorFlags              = 0x2E;
        constexpr uint8_t kSensorTimestamp          = 0x2F;
        constexpr uint8_t kSensorValues             = 0x30;
        constexpr uint8_t kSensorKeys               = 0x31;
    }

    enum class SensorFlags : uint16_t
    {
        DynamicKeys = 1 << 0,
        HideOnHost  = 1 << 1,
        HideOnWeb   = 1 << 2,
        GraphLine   = 0b000 << 3,
        GraphBars   = 0b001 << 3,
    };

    inline SensorFlags operator|(SensorFlags a, SensorFlags b)
    {
        return static_cast<SensorFlags>(
            static_cast<uint16_t>(a) | static_cast<uint16_t>(b));
    }

    enum class DataType : uint8_t
    {
        None   = 0,
        Char   = 1,
        Sint8  = 2,
        Uint8  = 3,
        Sint16 = 4,
        Uint16 = 5,
        Sint32 = 6,
        Uint32 = 7,
        Float  = 8,
    };

    inline size_t dataTypeSize(DataType dataType)
    {
        switch (dataType) {
            case DataType::Char:
            case DataType::Sint8:
            case DataType::Uint8:
                return 1;
            case DataType::Sint16:
            case DataType::Uint16:
                return 2;
            case DataType::Sint32:
            case DataType::Uint32:
            case DataType::Float:
                return 4;
            default:
                return 0;
        }
    }

    inline bool isSupportedDimension(size_t dimension)
    {
        return dimension > 0 && dimension <= kMaxSensorDimension;
    }

    inline bool isRecommendedDimension(size_t dimension)
    {
        return dimension > 0 && dimension <= kRecommendedSensorDimension;
    }

    static_assert(sizeof(float) == 4,
                  "The Amperia protocol requires 32-bit IEEE-754 float values.");
    static_assert(sizeof(uint32_t) == kTimestampSize,
                  "The Amperia protocol requires a 32-bit timestamp.");
    static_assert(sizeof(DataType) == 1,
                  "The Amperia protocol encodes DataType in one byte.");
}
}
