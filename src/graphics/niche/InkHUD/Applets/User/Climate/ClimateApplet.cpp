#ifdef MESHTASTIC_INCLUDE_INKHUD

#include "./ClimateApplet.h"
#include "gps/RTC.h"
#include <cstdio>
#include <ctime>

using namespace NicheGraphics;

namespace
{

std::string trimTrailingZeros(float value)
{
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%.1f", value);
    return buf;
}

} // namespace

ProcessMessage InkHUD::ClimateApplet::handleReceived(const meshtastic_MeshPacket &mp)
{
    if (!isActive())
        return ProcessMessage::CONTINUE;

    if (mp.decoded.portnum != meshtastic_PortNum_TELEMETRY_APP)
        return ProcessMessage::CONTINUE;

    meshtastic_Telemetry telemetry = meshtastic_Telemetry_init_zero;
    const meshtastic_Data &p = mp.decoded;
    if (!pb_decode_from_bytes(p.payload.bytes, p.payload.size, &meshtastic_Telemetry_msg, &telemetry)) {
        return ProcessMessage::CONTINUE;
    }

    if (telemetry.which_variant == meshtastic_Telemetry_air_quality_metrics_tag) {
        const auto &m = telemetry.variant.air_quality_metrics;
        haveAirQuality = true;
        haveCo2 = m.has_co2;
        haveTemperature = m.has_co2_temperature;
        haveHumidity = m.has_co2_humidity;
        co2Ppm = m.has_co2 ? m.co2 : 0;
        temperatureC = m.has_co2_temperature ? m.co2_temperature : 0.0f;
        humidityPercent = m.has_co2_humidity ? m.co2_humidity : 0.0f;

        if (haveCo2 || haveTemperature || haveHumidity) {
            requestUpdate();
        }
    }

    return ProcessMessage::CONTINUE;
}

std::string InkHUD::ClimateApplet::getClockText() const
{
    uint32_t now = getValidTime(RTCQuality::RTCQualityDevice, true);
    if (now == 0) {
        return "--:--";
    }

    uint32_t hms = now % SEC_PER_DAY;
    hms = (hms + SEC_PER_DAY) % SEC_PER_DAY;
    uint32_t hour = hms / SEC_PER_HOUR;
    uint32_t minute = (hms % SEC_PER_HOUR) / SEC_PER_MIN;

    char buf[16];
    std::snprintf(buf, sizeof(buf), "%02u:%02u", hour, minute);
    return buf;
}

std::string InkHUD::ClimateApplet::getDateText() const
{
    uint32_t now = getValidTime(RTCQuality::RTCQualityDevice, true);
    if (now == 0) {
        return "--/--";
    }

    char buf[32];
    std::tm timeinfo;
    time_t epoch = static_cast<time_t>(now);
#if defined(_WIN32)
    localtime_s(&timeinfo, &epoch);
#else
    localtime_r(&epoch, &timeinfo);
#endif
    std::snprintf(buf, sizeof(buf), "%02d/%02d", timeinfo.tm_mon + 1, timeinfo.tm_mday);
    return buf;
}

std::string InkHUD::ClimateApplet::formatMetric(float value, const char *suffix) const
{
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%s%s", trimTrailingZeros(value).c_str(), suffix);
    return buf;
}

void InkHUD::ClimateApplet::onRender(bool full)
{
    drawHeader("Climate");

    printAt(0, 18, getClockText(), LEFT, TOP);
    printAt(width() - getTextWidth(getDateText()), 18, getDateText(), LEFT, TOP);

    int16_t y = 38;
    if (haveCo2) {
        printAt(0, y, "CO2", LEFT, TOP);
        printAt(width() - getTextWidth(formatMetric(static_cast<float>(co2Ppm), "ppm")), y, formatMetric(static_cast<float>(co2Ppm), "ppm"), LEFT, TOP);
        y += 14;
    }

    if (haveTemperature) {
        printAt(0, y, "Temp", LEFT, TOP);
        printAt(width() - getTextWidth(formatMetric(temperatureC, "C")), y, formatMetric(temperatureC, "C"), LEFT, TOP);
        y += 14;
    }

    if (haveHumidity) {
        printAt(0, y, "RH", LEFT, TOP);
        printAt(width() - getTextWidth(formatMetric(humidityPercent, "%")), y, formatMetric(humidityPercent, "%"), LEFT, TOP);
    }

    if (!haveAirQuality) {
        printAt(0, height() - 12, "Waiting for sensor", LEFT, TOP);
    }
}

#endif
