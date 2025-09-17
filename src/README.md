# Initial Setup

To adjust ledstrip to piano adjust amount of leds and add an offset, on my piano I have a 25 led offset to be the right size

LED_LEFT_MARGIN    0      // LEDs to skip on the left
LED_RIGHT_MARGIN   25     // LEDs to skip on the right
ORIENT_RIGHT_TO_LEFT 0    // set to 1 if your strip enters from the right

Additionally you can use set the colors of the leds


// Black-key color 
WHITE_R 255
WHITE_G 255
WHITE_B 255

// Black-key color 
BLACK_R 255
BLACK_G 200
BLACK_B 0

# Running the project - first calibration

1) ensure both arduino and piano are connected
2) start Hairless (ref. https://projectgus.github.io/hairless-midiserial/#downloads)
3) set ports in hairless app (MIDI IN: keyboard, Serial port: Arduino)

# Running the project with a midi file

1) start loop midi and add a new port (toArduino) (ref. https://www.tobias-erichsen.de/software/loopmidi.html)
2) I'm using Soundfont from https://falcosoft.hu/softwares.html#midiplayer, 
   In settings, set output to the "toArduino" port
3) In hairless, set midi in to "toArduino" and set serial to the actual arduino
4) play midi file in Soundfont