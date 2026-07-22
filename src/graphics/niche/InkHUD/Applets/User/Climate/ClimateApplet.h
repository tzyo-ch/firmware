#ifdef MESHTASTIC_INCLUDE_INKHUD

#pragma once

#include "configuration.h"

#include "graphics/niche/InkHUD/Applet.h"

#include "mesh/SinglePortModule.h"
#include "mesh/generated/meshtastic/telemetry.pb.h"

namespace NicheGraphics::InkHUD
{

class ClimateApplet : public Applet, public SinglePortModule
{
  public:
    ClimateApplet() : SinglePortModule("ClimateApplet", meshtastic_PortNum_TELEMETRY_APP) {}

    void onRender(bool full) override;

  protected:
    void onActivate() override { loopbackOk = true; }
    void onDeactivate() override { loopbackOk = false; }
    ProcessMessage handleReceived(const meshtastic_MeshPacket &mp) override;

  private:
    bool haveAirQuality = false;
    bool haveCo2 = false;
    bool haveTemperature = false;
    bool haveHumidity = false;
    int32_t co2Ppm = 0;
    float temperatureC = 0.0f;
    float humidityPercent = 0.0f;

    std::string getClockText() const;
    std::string getDateText() const;
    std::string formatMetric(float value, const char *suffix) const;
};

} // namespace NicheGraphics::InkHUD

#endif
