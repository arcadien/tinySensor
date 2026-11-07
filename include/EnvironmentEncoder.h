#pragma once

/*
 * Common entry point for the environment radio encoders (temperature,
 * humidity, pressure).
 *
 * The protocol is chosen at build time (USE_OREGON / USE_LACROSSE), so the
 * "interface" is static: both encoders get an ApplyMeasurements() overload
 * and main.cpp uses the EnvironmentEncoder alias. No virtual table, no
 * indirect call: the generated code is the same as calling the encoder
 * directly.
 *
 * Luminosity is not handled here: Oregon has no light frame, and a Lacrosse
 * frame carrying luminosity cannot carry anything else. It uses its own
 * Lacrosse encoder in main.cpp, whatever the environment protocol.
 */

#include <LacrosseWS7000.h>
#include <Measurements.h>
#include <Oregon_v3.h>

/*
 * Loads the available measurements into the encoder.
 * Returns true when a frame must be sent, i.e. when a temperature is
 * available. Fields not present keep the value of a previous cycle.
 */
inline bool ApplyMeasurements(OregonV3 &encoder, const Measurements &m,
                              bool lowBattery) {
  if (m.Has(Measurements::TEMPERATURE)) {
    encoder.SetTemperature(m.temperature);
  }
  if (m.Has(Measurements::HUMIDITY)) {
    encoder.SetHumidity(static_cast<uint8_t>(m.humidity));
  }
  if (m.Has(Measurements::PRESSURE)) {
    encoder.SetPressure(static_cast<uint16_t>(m.pressure));
  }
  if (!m.Has(Measurements::TEMPERATURE)) {
    return false;
  }
  encoder.SetBatteryLow(lowBattery);
  return true;
}

// Lacrosse WS7000 frames have no battery-low flag: lowBattery is ignored.
inline bool ApplyMeasurements(LacrosseWS7000 &encoder, const Measurements &m,
                              bool /* lowBattery */) {
  if (m.Has(Measurements::TEMPERATURE)) {
    encoder.SetTemperature(m.temperature);
  }
  if (m.Has(Measurements::HUMIDITY)) {
    encoder.SetHumidity(m.humidity);
  }
  if (m.Has(Measurements::PRESSURE)) {
    encoder.SetPressure(m.pressure);
  }
  return m.Has(Measurements::TEMPERATURE);
}

#if defined(USE_LACROSSE)
typedef LacrosseWS7000 EnvironmentEncoder;
inline void ConfigureEncoder(EnvironmentEncoder &encoder) {
  encoder.SetAddress(LACROSSE_ID);
}
#elif defined(USE_OREGON)
typedef OregonV3 EnvironmentEncoder;
inline void ConfigureEncoder(EnvironmentEncoder &encoder) {
  encoder.SetChannel(OREGON_CHANNEL);
  encoder.SetRollingCode(OREGON_RCODE);
}
#endif
