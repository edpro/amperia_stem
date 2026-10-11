#pragma once

#include "AmperiaDescriptors.h"

namespace amperia
{
namespace protocol
{
    // RegisterCodec converts the library's in-memory device description and
    // current sensor values into the byte sequences returned for Amperia
    // protocol registers.
    //
    // It deliberately does not use Wire, Wire1, DREADY, millis(), or any
    // board-specific API. AmperiaDeviceServer handles I2C transport; this class
    // only answers: "which bytes belong to the requested register?"
    //
    // The codec does not copy or own the descriptors or value buffers passed
    // to its constructor. They must remain alive and at the same addresses for
    // the entire lifetime of the codec. Sensor classes satisfy this by storing
    // descriptors and measurement arrays as members rather than local values.
    class RegisterCodec
    {
    public:
        // sensors points to an array of sensor descriptors.
        //
        // sensorCount is the number of descriptors currently visible through
        // the protocol. sensorCapacity is the physical number of elements in
        // the array. Statically allocated multi-sensor containers may start
        // with sensorCount == 0 and expose configured descriptors later via
        // setSensorCount(). If sensorCapacity is omitted, it equals the initial
        // sensorCount.
        RegisterCodec(DeviceDescriptor &device,
                      SensorDescriptor sensors[],
                      int sensorCount,
                      int sensorCapacity = -1) :
            m_device(device),
            m_sensors(sensors),
            m_sensorCount(sensorCount < 0 ? 0 : sensorCount),
            m_sensorCapacity(
                sensorCapacity < 0
                    ? m_sensorCount
                    : (sensorCapacity < m_sensorCount
                           ? m_sensorCount
                           : sensorCapacity))
        {}

        // Encode one requested register into buffer.
        //
        // Returns the number of bytes written. A return value of 0 means that
        // the register or sensor index is invalid, or that buffer is too small.
        // The caller owns buffer and sends the returned bytes through the
        // selected transport.
        [[nodiscard]]
        int getData(uint8_t reg, uint8_t *buffer, size_t bufferSize);

        // Return the largest response size required by the currently active
        // descriptors. A transport can use this to validate its TX capacity.
        size_t requiredBufferSize() const;

        // Number of sensor descriptors currently exposed to the multimeter.
        inline int sensorsCnt() const
        {
            return m_sensorCount;
        }

        // Update the active descriptor count after a statically allocated
        // multi-sensor device has finished configuring its sensor list. The
        // value is clamped to 0..sensorCapacity. The constructor's capacity
        // argument must therefore match the real descriptor-array capacity.
        inline void setSensorCount(int sensorCount)
        {
            if (sensorCount < 0) {
                m_sensorCount = 0;
            } else if (sensorCount > m_sensorCapacity) {
                m_sensorCount = m_sensorCapacity;
            } else {
                m_sensorCount = sensorCount;
            }
        }

        // Operation mode is stored in the device descriptor because it is a
        // protocol-visible property. AmperiaDeviceServer uses these methods
        // when the host reads or writes the operation-mode register.
        inline uint8_t getOperationMode() const
        {
            return m_device.operationMode;
        }

        inline void setOperationMode(uint8_t value)
        {
            m_device.operationMode = value;
        }

    private:
        // Non-owning references/pointers to storage owned by a sensor class.
        DeviceDescriptor &m_device;
        SensorDescriptor *m_sensors;
        int m_sensorCount;
        const int m_sensorCapacity;

        [[nodiscard]]
        int getDeviceData(uint8_t reg,
                          uint8_t *buffer,
                          size_t bufferSize) const;

        [[nodiscard]]
        int getSensorData(uint8_t sensorIndex,
                          uint8_t reg,
                          uint8_t *buffer,
                          size_t bufferSize) const;
    };
}
}
