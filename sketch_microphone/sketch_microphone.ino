
// Outputs the sound pressure level through the com port
// based on https://github.com/FlorinAndrei/WeatherStation/blob/master/nano33/nano33.ino


// MP34DT05 sensor library
// https://www.arduino.cc/en/Reference/PDM
// https://github.com/arduino/ArduinoCore-nRF528x-mbedos/tree/master/libraries/PDM
#include <PDM.h>
// TBD
#include <arduinoFFT.h>

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

void setup() {
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
}

void loop() {

  // wait for sound samples to be read
  if (samplesRead) {
    delay(srelax);
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
  ledState = ledState ? LOW: HIGH;
  digitalWrite(LED_BUILTIN,  ledState);

  delay(srelax);
}

void onPDMdata() {
  // query the number of bytes available
  int bytesAvailable = PDM.available();

  // read into the sample buffer
  PDM.read(wform, bytesAvailable);

  // 16-bit, 2 bytes per sample
  samplesRead = bytesAvailable / 2;
}
