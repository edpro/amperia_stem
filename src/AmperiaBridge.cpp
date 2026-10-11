#include "AmperiaBridge.h"

#include "internal/AmperiaText.h"

AmperiaBridge::AmperiaBridge(AmperiaSensorClient& source) :
    m_source(source),
    m_output("AmperiaShield"),
    m_localSensors{},
    m_forwardedOutputHandles{},
    m_localOutputHandles{},
    m_localSensorCount(0),
    m_forwardedSensorCount(0),
    m_started(false)
{}

int AmperiaBridge::addSensor(const char* name, const char* unit)
{
    return addSensor(name, unit, 1);
}

int AmperiaBridge::addSensor(
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

int AmperiaBridge::addSensor(
    const char* name,
    const char* unit,
    const char* component0,
    const char* component1)
{
    const char* componentNames[] = {component0, component1};
    return addSensorWithComponents(name, unit, 2, componentNames);
}

int AmperiaBridge::addSensor(
    const char* name,
    const char* unit,
    const char* component0,
    const char* component1,
    const char* component2)
{
    const char* componentNames[] = {
        component0, component1, component2
    };
    return addSensorWithComponents(name, unit, 3, componentNames);
}

int AmperiaBridge::addSensor(
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
    return addSensorWithComponents(name, unit, 4, componentNames);
}

int AmperiaBridge::addSensorWithComponents(
    const char* name,
    const char* unit,
    size_t dimension,
    const char* const* componentNames)
{
    if (m_started || m_output.isFinalized() ||
        name == nullptr || unit == nullptr || componentNames == nullptr ||
        !amperia::protocol::isSupportedDimension(dimension) ||
        m_localSensorCount >= amperia::protocol::kMaxLogicalSensors) {
        return -1;
    }

    for (size_t component = 0; component < dimension; ++component) {
        if (componentNames[component] == nullptr) {
            return -1;
        }
    }

    LocalSensor& sensor = m_localSensors[m_localSensorCount];
    amperia::detail::copyText(sensor.name, sizeof(sensor.name), name);
    amperia::detail::copyText(sensor.unit, sizeof(sensor.unit), unit);
    sensor.dimension = static_cast<uint8_t>(dimension);
    sensor.dataPeriodMs = amperia::kDefaultDataPeriodMs;

    for (size_t component = 0; component < dimension; ++component) {
        amperia::detail::copyText(
            sensor.componentNames[component],
            sizeof(sensor.componentNames[component]),
            componentNames[component]);
    }

    return static_cast<int>(m_localSensorCount++);
}

bool AmperiaBridge::setDataPeriodMs(
    int localSensorHandle,
    uint16_t periodMs)
{
    if (m_started || m_output.isFinalized() || localSensorHandle < 0 ||
        static_cast<size_t>(localSensorHandle) >= m_localSensorCount) {
        return false;
    }

    m_localSensors[static_cast<size_t>(localSensorHandle)].dataPeriodMs =
        periodMs;
    return true;
}

bool AmperiaBridge::begin()
{
    if (m_started || !m_source.isStarted()) {
        return m_started;
    }

    if (!m_output.isFinalized()) {
        const size_t availableSourceSlots =
            amperia::protocol::kMaxLogicalSensors - m_localSensorCount;
        m_forwardedSensorCount = m_source.sensorCount();
        if (m_forwardedSensorCount > availableSourceSlots) {
            m_forwardedSensorCount = availableSourceSlots;
        }

        for (size_t sensorIndex = 0;
             sensorIndex < m_forwardedSensorCount;
             ++sensorIndex) {
            auto& source = m_source.sensor(sensorIndex);
            const int handle = m_output.addSensor(
                source.name(),
                source.unit(),
                source.dimension());
            if (handle < 0) {
                return false;
            }
            if (!m_output.setDataPeriodMs(handle, source.dataPeriodMs())) {
                return false;
            }

            m_forwardedOutputHandles[sensorIndex] = handle;
        }

        for (size_t localIndex = 0;
             localIndex < m_localSensorCount;
             ++localIndex) {
            LocalSensor& local = m_localSensors[localIndex];
            const char* localComponentNames[
                amperia::protocol::kMaxSensorDimension] = {
                    local.componentNames[0],
                    local.componentNames[1],
                    local.componentNames[2],
                    local.componentNames[3]
                };

            const int handle = m_output.addSensorWithComponents(
                local.name,
                local.unit,
                local.dimension,
                localComponentNames);
            if (handle < 0) {
                return false;
            }
            if (!m_output.setDataPeriodMs(handle, local.dataPeriodMs)) {
                return false;
            }

            m_localOutputHandles[localIndex] = handle;
        }
    }

    m_started = m_output.begin();
    return m_started;
}

bool AmperiaBridge::copySourceMeasurements()
{
    for (size_t sensorIndex = 0;
         sensorIndex < m_forwardedSensorCount;
         ++sensorIndex) {
        auto& source = m_source.sensor(sensorIndex);

        for (size_t component = 0;
             component < source.dimension();
             ++component) {
            float value = 0.0f;
            if (!source.valueAsFloat(component, value)) {
                return false;
            }

            if (!m_output.set(
                    m_forwardedOutputHandles[sensorIndex],
                    component,
                    value)) {
                return false;
            }
        }
    }

    return true;
}

bool AmperiaBridge::update()
{
    return m_started && m_source.update() && copySourceMeasurements();
}

bool AmperiaBridge::updateIfReady()
{
    return m_started &&
           m_source.updateIfReady() &&
           copySourceMeasurements();
}

bool AmperiaBridge::set(int localSensorHandle, float value)
{
    return set(localSensorHandle, 0, value);
}

bool AmperiaBridge::set(
    int localSensorHandle,
    size_t component,
    float value)
{
    if (!m_started || localSensorHandle < 0 ||
        static_cast<size_t>(localSensorHandle) >= m_localSensorCount) {
        return false;
    }

    LocalSensor& local =
        m_localSensors[static_cast<size_t>(localSensorHandle)];
    if (component >= local.dimension) {
        return false;
    }

    return m_output.set(
        m_localOutputHandles[static_cast<size_t>(localSensorHandle)],
        component,
        value);
}

void AmperiaBridge::publish()
{
    if (m_started) {
        m_output.publish();
    }
}

bool AmperiaBridge::isStarted() const
{
    return m_started;
}

size_t AmperiaBridge::forwardedSensorCount() const
{
    return m_forwardedSensorCount;
}

size_t AmperiaBridge::localSensorCount() const
{
    return m_localSensorCount;
}
