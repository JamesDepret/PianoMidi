#include <FastLED.h>
#include <MIDI.h>

// ---------------- User config ----------------
#define NUM_LEDS           144    // total LEDs on your strip
#define DATA_PIN           11
#define CLOCK_PIN          13
#define BRIGHTNESS         5

#define NOTE_START         36     // lowest MIDI note on your keyboard (PSR-E373 = C2 = 36)
#define KEY_COUNT          61     // number of keys to map (PSR-E373 = 61)

// Fine alignment controls (adjust to fit physical layout)

// ======== LED_LEFT_MARGIN / LED_RIGHT_MARGIN ========
// Number of physical LEDs to leave unused (always off) at the *left* and *right*
// ends of the strip. This is purely for alignment/centering on your piano.
// Typical use: your strip is longer than the keybed; add margins to center it.
//
// How it works:
//   LED_USABLE = NUM_LEDS - LED_LEFT_MARGIN - LED_RIGHT_MARGIN
//   PIXELS_PER_KEY = LED_USABLE / KEY_COUNT   (floating point)
// Keys are then mapped across the LED_USABLE region.
//
// Calibration tips:
//   1) Start with both margins = 0. Verify NOTE_START (lowest key) lights near
//      the physical left end of the strip.
//   2) Increase LEFT/RIGHT margins to slide/center the mapped region over the keys.
//      Example: With 144 LEDs and 61 keys, if you set LEFT=8 and RIGHT=7,
//      you’ll use 144-8-7 = 129 LEDs → about 129/61 ≈ 2.11 LEDs per key.
//   3) If margins are too large (LED_USABLE <= 0), nothing will light (guarded).
#define LED_LEFT_MARGIN    0      // LEDs to skip on the left
#define LED_RIGHT_MARGIN   25      // LEDs to skip on the right
#define ORIENT_RIGHT_TO_LEFT 0    // set to 1 if your strip enters from the right

// ======== BLACK-KEY COLORING ========
// Define the 5 black-key pitch classes in one octave (relative to C=0):
//   C#=1, D#=3, F#=6, G#=8, A#=10
// We’ll check (note % 12) against this list to decide color.
const byte BLACK_PC[5] = { 1, 3, 6, 8, 10 };

// White-key color (scaled by velocity later). Default = white.
#define WHITE_R 255
#define WHITE_G 255
#define WHITE_B 255

// Black-key color (scaled by velocity later). “Yellowish” by default.
// Tune these three to taste (e.g., 255, 200, 0 is a warm yellow).
#define BLACK_R 255
#define BLACK_G 200
#define BLACK_B 0
// ---------------------------------------------

CRGB leds[NUM_LEDS];

// ---- MIDI @ 115200 for Hairless ----
struct HairlessBaud : public midi::DefaultSettings { static const long BaudRate = 115200; };
MIDI_CREATE_CUSTOM_INSTANCE(HardwareSerial, Serial, MIDI, HairlessBaud);

// Derived layout values
const uint16_t LED_START  = LED_LEFT_MARGIN;
const uint16_t LED_END    = NUM_LEDS - 1 - LED_RIGHT_MARGIN;
const uint16_t LED_USABLE = (LED_END >= LED_START) ? (LED_END - LED_START + 1) : 0;
const float    PIXELS_PER_KEY = (KEY_COUNT > 0) ? (float)LED_USABLE / (float)KEY_COUNT : 0.0f;

// Helpers
inline bool noteInRange(byte note) {
  return (note >= NOTE_START) && (note < NOTE_START + KEY_COUNT);
}

// Is this MIDI note a black key? (Check pitch class against BLACK_PC[])
bool isBlackKey(byte note) {
  byte pc = note % 12;
  for (byte i = 0; i < 5; i++) {
    if (BLACK_PC[i] == pc) return true;
  }
  return false;
}

// Map a MIDI note to the [from..to] LED span for that key
void keySpan(byte note, uint16_t &from, uint16_t &to) {
  // key index 0..KEY_COUNT-1
  int k = (int)note - (int)NOTE_START;
  if (k < 0) k = 0;
  if (k >= KEY_COUNT) k = KEY_COUNT - 1;

  // Partition usable LEDs into KEY_COUNT bins:
  uint16_t a = LED_START + (uint16_t)floorf(k * PIXELS_PER_KEY);
  uint16_t b = LED_START + (uint16_t)floorf((k + 1) * PIXELS_PER_KEY) - 1;
  if (b < a) b = a;                           // guarantee at least 1 pixel
  if (b > LED_END) b = LED_END;               // clamp
  if (a > LED_END) a = LED_END;

  if (ORIENT_RIGHT_TO_LEFT) {
    // Mirror into right-to-left layout
    uint16_t ma = LED_END - (a - LED_START);
    uint16_t mb = LED_END - (b - LED_START);
    from = (ma < mb) ? ma : mb;
    to   = (ma > mb) ? ma : mb;
  } else {
    from = a; to = b;
  }
}

void setKeySpanColor(uint16_t from, uint16_t to, uint8_t r, uint8_t g, uint8_t b) {
  for (uint16_t i = from; i <= to; i++) leds[i].setRGB(r, g, b);
}

void handleNoteOff(byte ch, byte note, byte velocity) {
  if (!noteInRange(note) || LED_USABLE == 0) return;

  uint16_t from, to;
  keySpan(note, from, to);
  setKeySpanColor(from, to, 0, 0, 0);

  FastLED.show();
}

void handleNoteOn(byte ch, byte note, byte velocity) {
  if (velocity == 0) { handleNoteOff(ch, note, 64); return; } // NoteOn w/ vel=0 = NoteOff
  if (!noteInRange(note) || LED_USABLE == 0) return;

  uint16_t from, to;
  keySpan(note, from, to);

  // Velocity (1..127) → brightness scale (adjust 32..255 to taste)
  uint8_t v = constrain(map(velocity, 1, 127, 32, 255), 0, 255);

  // Choose color based on black/white key, then scale by velocity.
  if (isBlackKey(note)) {
    // Yellowish for black keys
    uint8_t r = (uint16_t)BLACK_R * v / 255;
    uint8_t g = (uint16_t)BLACK_G * v / 255;
    uint8_t b = (uint16_t)BLACK_B * v / 255;
    setKeySpanColor(from, to, r, g, b);
  } else {
    // White for white keys
    uint8_t r = (uint16_t)WHITE_R * v / 255;
    uint8_t g = (uint16_t)WHITE_G * v / 255;
    uint8_t b = (uint16_t)WHITE_B * v / 255;
    setKeySpanColor(from, to, r, g, b);
  }

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
  MIDI.read();
}
