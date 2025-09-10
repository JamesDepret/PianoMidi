#include <Arduino.h>
/*
 * Basic MIDI Visualizer
 * by David Madison © 2017
 * www.partsnotincluded.com
 * 
 * This is a basic MIDI visualizer using addressable LEDs, to demonstrate how
 * Arduino's MIDI library works. Playing a note turns an LED on, stopping a note
 * turns the LED off. Continuous controllers 21, 22, and 23 adjust the RGB color.  
 * 
 */

#include <FastLED.h>
//#include <Adafruit_NeoPixel.h> // Uncomment if you're using RGBW LEDs

// ---- User Settings --------------------------
#define DATAPIN 6
#define NUMLEDS 60
#define BRIGHTNESS 255
#define BAUDRATE 115200 // For Hairless
#define NOTESTART 40
// ----------------------------------------

// Continuous Controller Numbers
static const uint8_t
  redCC   = 21,
  greenCC = 22,
  blueCC  = 23;

// Settings
static const uint8_t
  Pin = DATAPIN,
  NumLEDs = NUMLEDS,
  minNote = NOTESTART,
  maxNote = minNote + NumLEDs,
  maxBright = BRIGHTNESS;

// LED Color Values
uint8_t
  rVal = 255,
  gVal = 255,
  bVal = 255;

// Build the LED strip
#ifdef ADAFRUIT_NEOPIXEL_H
  Adafruit_NeoPixel strip = Adafruit_NeoPixel(NumLEDs, Pin, NEO_GRBW + NEO_KHZ800);
#elif FASTLED_VERSION
  CRGB leds[NumLEDs];
#endif

boolean ledState[NumLEDs];

// Create the MIDI Instance
#include <MIDI.h>
struct CustomBaud : public midi::DefaultSettings{
    static const long BaudRate = BAUDRATE;
};
MIDI_CREATE_CUSTOM_INSTANCE(HardwareSerial, Serial, MIDI, CustomBaud);

void setup(){
  #ifdef ADAFRUIT_NEOPIXEL_H
    strip.setBrightness(maxBright);
    strip.begin();
    strip.show();
  #elif FASTLED_VERSION
    FastLED.addLeds<WS2812B, Pin, GRB>(leds, NumLEDs);
    FastLED.setBrightness(maxBright);
    FastLED.show();  
  #endif
  
  MIDI.setHandleNoteOn(handleNoteOn);
  MIDI.setHandleNoteOff(handleNoteOff);
  MIDI.setHandleControlChange(handleControlChange);
  MIDI.begin(MIDI_CHANNEL_OMNI);
}

void loop(){
  MIDI.read(); // Check for MIDI messages every loop
}

void handleNoteOn(byte channel, byte note, byte velocity){
  // Check if note is in range
  if(note < minNote || note > maxNote){
    return;
  }
  
  ledON(note - minNote);
  show();
}

void handleNoteOff(byte channel, byte note, byte velocity){
  // Check if note is in range
  if(note < minNote || note > maxNote){
    return;
  }

  ledOFF(note - minNote);
  show();
}

void handleControlChange(byte channel, byte number, byte value){
  switch(number){
    case redCC:
      rVal = map(value, 0, 127, 0, 255);
      redraw();
      break;
    case greenCC:
      gVal = map(value, 0, 127, 0, 255);
      redraw();
      break;
    case blueCC:
      bVal = map(value, 0, 127, 0, 255);
      redraw();
      break;
  }
}

void ledON(uint8_t index){
  ledState[index] = 1;
  setLED(index, rVal, gVal, bVal);
}

void ledOFF(uint8_t index){
  ledState[index] = 0;
  setLED(index, 0, 0, 0);
}

void setLED(uint8_t index, uint8_t r, uint8_t g, uint8_t b){
  #ifdef ADAFRUIT_NEOPIXEL_H
    strip.setPixelColor(index, r, g, b);
  #elif FASTLED_VERSION
    leds[index].r = r;
    leds[index].g = g;
    leds[index].b = b;
  #endif
}

void redraw(){
  for(int i = 0; i<NumLEDs; i++){
    if(ledState[i] == 1){
      ledON(i);
    }
  }
  show();
}

void show(){
  #ifdef ADAFRUIT_NEOPIXEL_H
    strip.show();
  #elif FASTLED_VERSION
    FastLED.show();
  #endif
}