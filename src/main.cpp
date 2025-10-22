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
#define LED_LEFT_MARGIN    0      // LEDs to skip on the left
#define LED_RIGHT_MARGIN   25     // LEDs to skip on the right
#define ORIENT_RIGHT_TO_LEFT 0    // set to 1 if your strip enters from the right

// ======== BLACK-KEY COLORING ========
// C#=1, D#=3, F#=6, G#=8, A#=10
const byte BLACK_PC[5] = { 1, 3, 6, 8, 10 };

// White-key color (scaled by velocity later). Default = white.
#define WHITE_R 255
#define WHITE_G 255
#define WHITE_B 255

// Black-key color (scaled by velocity later). “Yellowish” by default.
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

// Track on/off state to allow deterministic clears
bool active[128] = { false };

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

// Turn off the LEDs for a specific note (with bounds checks)
void clearNote(byte note) {
  if (!noteInRange(note) || LED_USABLE == 0) return;
  uint16_t from, to;
  keySpan(note, from, to);
  setKeySpanColor(from, to, 0, 0, 0);
}

void handleNoteOff(byte ch, byte note, byte velocity) {
  if (!noteInRange(note) || LED_USABLE == 0) return;
  active[note] = false;
  clearNote(note);
  FastLED.show();
}

void handleNoteOn(byte ch, byte note, byte velocity) {
  if (velocity == 0) { handleNoteOff(ch, note, 64); return; } // NoteOn w/ vel=0 = NoteOff
  if (!noteInRange(note) || LED_USABLE == 0) return;

  active[note] = true;

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

// Full clear: LEDs + note state
void clearAllLedsAndState() {
  FastLED.clear();
  for (int i = 0; i < 128; ++i) active[i] = false;
  FastLED.show();
}

// Respond to Control Change to support host “panic/reset”
void handleCC(byte ch, byte controller, byte value) {
  switch (controller) {
    case 120: // All Sound Off
    case 121: // Reset All Controllers
      clearAllLedsAndState();
      break;

    case 123: { // All Notes Off
      bool any = false;
      for (byte n = 0; n < 128; ++n) {
        if (active[n]) { clearNote(n); active[n] = false; any = true; }
      }
      if (any) FastLED.show();
      break;
    }

    case 119: // Custom "Hard Reset" from host
      clearAllLedsAndState();
      break;

    default:
      // ignore other CCs
      break;
  }
}

void setup() {
  FastLED.addLeds<APA102, DATA_PIN, CLOCK_PIN, BGR>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.clear(true);

  MIDI.setHandleNoteOn(handleNoteOn);
  MIDI.setHandleNoteOff(handleNoteOff);
  MIDI.setHandleControlChange(handleCC);  // listen for panic/reset CCs
  MIDI.begin(MIDI_CHANNEL_OMNI);
}

void loop() {
  MIDI.read();
}
