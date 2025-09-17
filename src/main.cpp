#include <FastLED.h>
#include <MIDI.h>

// Forward declarations
void handleNoteOff(byte ch, byte note, byte velocity);

// ---------------- User config ----------------
#define NUM_LEDS   144            // change to your strip length
#define DATA_PIN   11
#define CLOCK_PIN  13
#define BRIGHTNESS 64            // be kind to eyes & PSU
// Map: which real MIDI note is your LEFTMOST physical key?
#define NOTE_START 36            // PSR-E373 low C is usually 36 (C2)
// ---------------------------------------------

CRGB leds[NUM_LEDS];

// ---- MIDI @ 115200 for Hairless ----
struct HairlessBaud : public midi::DefaultSettings {
  static const long BaudRate = 115200;
};
MIDI_CREATE_CUSTOM_INSTANCE(HardwareSerial, Serial, MIDI, HairlessBaud);

// --- simple LED state buffer so we can redraw colors if needed ---
bool ledOn[NUM_LEDS] = {false};

// Helpers
inline bool noteInRange(byte note) {
  return (note >= NOTE_START) && (note < (NOTE_START + NUM_LEDS));
}
inline uint16_t noteToIndex(byte note) {
  // left-to-right strip: lowest note is index 0
  return (uint16_t)(note - NOTE_START);
  // If your strip enters from the RIGHT, flip it:
  // return (uint16_t)((NOTE_START + NUM_LEDS - 1) - note);
}

void setKeyColor(uint16_t idx, uint8_t r, uint8_t g, uint8_t b) {
  leds[idx].setRGB(r, g, b);
}

void lightOn(uint16_t idx, byte velocity) {
  // velocity 1..127 → brightness scaling (coarse)
  uint8_t v = constrain(map(velocity, 1, 127, 32, 255), 0, 255);
  // pick a color; pure white scaled by velocity:
  setKeyColor(idx, v, v, v);
  ledOn[idx] = true;
}

void lightOff(uint16_t idx) {
  setKeyColor(idx, 0, 0, 0);
  ledOn[idx] = false;
}

void handleNoteOn(byte ch, byte note, byte velocity) {
  if (velocity == 0) { 
    handleNoteOff(ch, note, 64); 
    return; 
  } // running status
  if (!noteInRange(note)) return;
  lightOn(noteToIndex(note), velocity);
  FastLED.show();
}

void handleNoteOff(byte ch, byte note, byte velocity) {
  if (!noteInRange(note)) return;
  lightOff(noteToIndex(note));
  FastLED.show();
}

void setup() {
  FastLED.addLeds<APA102, DATA_PIN, CLOCK_PIN, BGR>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.clear(true);

  MIDI.setHandleNoteOn(handleNoteOn);
  MIDI.setHandleNoteOff(handleNoteOff);
  MIDI.begin(MIDI_CHANNEL_OMNI);
}

void loop() {
  MIDI.read(); // poll MIDI
}
