#pragma once

#include <stddef.h>

#include "MySensor.h"
#include "client/AmperiaSensorClient.h"
#include "protocol/AmperiaProtocol.h"

// Forward measurements from one external Amperia STEM sensor to the Amperia
// Multimeter. Optional local sensors can be added before begin(); the bridge
// automatically reserves protocol slots for them.
class AmperiaBridge
{
public:
    explicit AmperiaBridge(AmperiaSensorClient& source);

    // Add a one-dimensional local sensor. Returns a small handle used by set(),
    // or -1 when no slot is available. Call before begin().
    int addSensor(const char* name, const char* unit);

    // Add a local sensor with 1..4 homogeneous components. This overload
    // preserves the historical numeric component labels: "1", "2", "3", "4".
    int addSensor(const char* name, const char* unit, size_t dimension);

    // Optional component-name overloads. Existing sketches keep their current
    // wire metadata unchanged; these forms only change labels when explicitly
    // requested by the sketch.
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

    // Set the measurement period metadata for a local sensor. Call before
    // begin(). Returns false for an invalid handle or after configuration.
    bool setDataPeriodMs(int localSensorHandle, uint16_t periodMs);

    // Configure the multimeter output from the source metadata and any local
    // sensors previously added with addSensor(). The source must already have
    // completed discover().
    bool begin();

    // Read the external sensor and copy its measurements into the output.
    bool update();

    // Do the same only after a DREADY edge from the external sensor.
    bool updateIfReady();

    // Update a one-dimensional local sensor.
    bool set(int localSensorHandle, float value);

    // Update one component of a multi-component local sensor.
    bool set(int localSensorHandle, size_t component, float value);

    // Publish one consistent snapshot to the multimeter.
    void publish();

    bool isStarted() const;
    size_t forwardedSensorCount() const;
    size_t localSensorCount() const;

private:
    struct LocalSensor
    {
        char name[amperia::protocol::kSensorNameSize + 1];
        char unit[amperia::protocol::kUnitNameSize + 1];
        char componentNames[amperia::protocol::kMaxSensorDimension]
                           [amperia::protocol::kKeyNameSize + 1];
        uint8_t dimension;
        uint16_t dataPeriodMs;
    };

    int addSensorWithComponents(
        const char* name,
        const char* unit,
        size_t dimension,
        const char* const* componentNames);

    bool copySourceMeasurements();

    AmperiaSensorClient& m_source;
    CombinedSensor m_output;
    LocalSensor m_localSensors[amperia::protocol::kMaxLogicalSensors];
    int m_forwardedOutputHandles[amperia::protocol::kMaxLogicalSensors];
    int m_localOutputHandles[amperia::protocol::kMaxLogicalSensors];
    size_t m_localSensorCount;
    size_t m_forwardedSensorCount;
    bool m_started;
};
