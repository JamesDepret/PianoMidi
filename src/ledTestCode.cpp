// #include <Arduino.h>
// /*
//  * Basic MIDI Visualizer
//  * by David Madison © 2017
//  * www.partsnotincluded.com
//  * 
//  * This is a basic MIDI visualizer using addressable LEDs, to demonstrate how
//  * Arduino's MIDI library works. Playing a note turns an LED on, stopping a note
//  * turns the LED off. Continuous controllers 21, 22, and 23 adjust the RGB color.  
//  * 
//  */

// #include <FastLED.h>
// //#include <Adafruit_NeoPixel.h> // Uncomment if you're using RGBW LEDs

// #include <FastLED.h>

// // How many leds in your strip?
// #define NUM_LEDS 144

// // For led chips like Neopixels, which have a data line, ground, and power, you just
// // need to define DATA_PIN.  For led chipsets that are SPI based (four wires - data, clock,
// // ground, and power), like the LPD8806 define both DATA_PIN and CLOCK_PIN
// #define DATA_PIN 11
// #define CLOCK_PIN 13
// int index = 0;
// int speed = 50;
// // Define the array of leds
// CRGB leds[NUM_LEDS];

// void setup() {
//   // FastLED.addLeds<WS2801, RGB>(leds, NUM_LEDS);
//   FastLED.addLeds<APA102, DATA_PIN, CLOCK_PIN, BGR>(leds, NUM_LEDS);
// }

// void loop() {
//   // Turn the LED on, then pause
//   leds[index] = CRGB::Red;
//   FastLED.show();
//   delay(speed);
//   // Now turn the LED off, then pause
//   leds[index] = CRGB::Black;
//   FastLED.show();
//   delay(speed);

//   // Turn the LED on, then pause
//   leds[index] = CRGB::Green;
//   FastLED.show();
//   delay(speed);
//   // Now turn the LED off, then pause
//   leds[index] = CRGB::Black;
//   FastLED.show();
//   delay(speed);

//   // Turn the LED on, then pause
//   leds[index] = CRGB::Blue;
//   FastLED.show();
//   delay(speed);
//   // Now turn the LED off, then pause
//   leds[index] = CRGB::Black;
//   FastLED.show();
//   delay(speed);

//   index++;
//   if(index >= NUM_LEDS){
//     index = 0;
//   }
// }