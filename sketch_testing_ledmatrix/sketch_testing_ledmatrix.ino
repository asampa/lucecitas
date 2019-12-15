// NeoPixel test program showing use of the WHITE channel for RGBW
// pixels only (won't look correct on regular RGB NeoPixel strips).

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1:
#define LED_PIN     6

// How many NeoPixels are attached to the Arduino?
#define LED_COUNT  64

// NeoPixel brightness, 0 (min) to 255 (max)
#define BRIGHTNESS 30

// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
// Argument 1 = Number of pixels in NeoPixel strip
// Argument 2 = Arduino pin number (most are valid)
// Argument 3 = Pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)

uint32_t previous_row_colors[8];

void setup() {
  // put your setup code here, to run once:

  // These lines are specifically to support the Adafruit Trinket 5V 16 MHz.
  // Any other board, you can remove this part (but no harm leaving it):
#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
#endif
  // END of Trinket-specific code.

  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(BRIGHTNESS); // Set BRIGHTNESS to about 1/5 (max = 255)


}

int red, blue;

void loop() {
  // put your main code here, to run repeatedly:

  introduce_bottom_row(strip.Color(random(0, 255), random(0, 255), random(0, 255)));
  delay(1000);

}

void introduce_bottom_row(uint32_t color) {
  // carry upwards the rows
  for (int i = 0; i < 7; i++) {
    paint_row_single_color(previous_row_colors[i + 1], i);
    previous_row_colors[i] = previous_row_colors[i + 1];
  }
  // paint the last row
  paint_row_single_color(color, 7);
  previous_row_colors[7] = color;

}


void paint_row_single_color(uint32_t color, int row) {
  int first_pixel = 8 * row;

  for (int i = first_pixel; i < first_pixel + 8; i++) {
    strip.setPixelColor(i, color);
  }
  strip.show();

}
