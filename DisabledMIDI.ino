#include <RK002.h>

// Disabled messages for loopback cable mode
bool RK002_onPolyPressure(byte channel, byte key, byte value) {
  return false;
}

bool RK002_onControlChange(byte channel, byte nr, byte value) {
  return false;
}

bool RK002_onChannelPressure(byte channel, byte val) {
  return false;
}

bool RK002_onPitchBend(byte channel, int val) {
  return false;
}

bool RK002_onSystemExclusive(unsigned n, const byte *data) {
  return false;
}

bool RK002_onTimeCodeQuarterFrame(byte mtc) {
  return false;
}

bool RK002_onSongSelect(byte songsel) {
  return false;
}

bool RK002_onSongPosition(unsigned pos) {
  return false;
}

bool RK002_onTuneRequest() {
  return false;
}

bool RK002_onActiveSensing() {
  return false;
}

bool RK002_onReset() {
  return false;
}
