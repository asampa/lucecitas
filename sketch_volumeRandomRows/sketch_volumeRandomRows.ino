// NeoPixel test program showing use of the WHITE channel for RGBW
// pixels only (won't look correct on regular RGB NeoPixel strips).
#include <PDM.h>
#include <arduinoFFT.h>

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

// -------- Initialize mic ---------
arduinoFFT FFT = arduinoFFT();

// line output to serial
char linebuf_all[200];

// define FFT parameters
#define SAMPLES 256
#define SAMPLING_FREQUENCY 16000
// buffer to read samples into, each sample is 16-bits
short wform[SAMPLES];
// FFT real and imaginary vectors
double vReal[SAMPLES];
double vImag[SAMPLES];

// number of samples read
volatile int samplesRead;

// final result from FFT
double ftsum = 0.0;

// short pause between sensor reads
short srelax = 40;

int ledState = LOW;

// ---------- Initialize LED -----------
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
  // MIC SETUP
  Serial.begin(115200);
  delay(100);

  // sound
  PDM.onReceive(onPDMdata);
  delay(100);
  PDM.begin(1, SAMPLING_FREQUENCY);
  delay(100);

  pinMode(LED_BUILTIN, OUTPUT);

  // Let's allow things to settle down.
  delay(100);

  // LED SETUP

  // These lines are specifically to support the Adafruit Trinket 5V 16 MHz.
  // Any other board, you can remove this part (but no harm leaving it):
#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
#endif
  // END of Trinket-specific code.

  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(BRIGHTNESS); // Set BRIGHTNESS to about 1/5 (max = 255)

  // Let's allow things to settle down.
  delay(100);

}

int red, blue;

void loop() {
  // READ MIC 

  // wait for sound samples to be read
  if (samplesRead) {
    //delay(srelax);
    for (int i = 0; i < SAMPLES; i++) {
      // load the waveform into the FFT real vector
      vReal[i] = double(wform[i]);
      // FFT imaginary vector is zero
      vImag[i] = 0.0;
    }

    // compute the spectrum
    // at the end of the sequence, vReal will contain the spectrum
    FFT.Windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    FFT.Compute(vReal, vImag, SAMPLES, FFT_FORWARD);
    FFT.ComplexToMagnitude(vReal, vImag, SAMPLES);

    // calculate the sum of all spectral components
    // with log10() to adjust for perceptual scale
    ftsum = 0.0;
    // don't start i at 0, low frequencies are too noisy
    // stop at sR / 2 since the spectrum is repeated symmetrically after that
    // (that's how FFT works)
    for (int i = 8; i < samplesRead / 2; i++) {
      ftsum += log10(vReal[i]);
    }

    // clear the samples read count
    samplesRead = 0;
  }

  // prepare the line output with all data
  sprintf(linebuf_all,
    "Sound level: %u",int(ftsum));

  // send data out
  Serial.println(linebuf_all);

  // blink the LED every cycle
  // (heartbeat indicator)
  //ledState = ledState ? LOW: HIGH;
  //digitalWrite(LED_BUILTIN,  ledState);

  //delay(srelax);
  // UPDATE LED
  if(ftsum>100){
  introduce_bottom_row(strip.Color(random(0, 255), random(0, 255), random(0, 255)));
  delay(transform_ftsum_to_millis(ftsum));
  Serial.printf("delay:");
  Serial.println(transform_ftsum_to_millis(ftsum));
  }else{
    Serial.println("no sound no delay.");
    
  }
}

int transform_ftsum_to_millis(double ftsum){
  // ftsum values between 40 and 700.
  //return int(10000/(ftsum-70));
  return max(0,  min(int(10000/(ftsum-100)),1000));
}


void onPDMdata() {
  // query the number of bytes available
  int bytesAvailable = PDM.available();

  // read into the sample buffer
  PDM.read(wform, bytesAvailable);

  // 16-bit, 2 bytes per sample
  samplesRead = bytesAvailable / 2;
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
