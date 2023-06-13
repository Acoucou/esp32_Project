#include "esp_camera.h"
#include "RTOS.h"
#include <time.h>
#include <Wire.h>
#include <Adafruit_NeoPixel.h>
#include "esp_http_client.h"
#include <ArduinoJson.h>
#include <lvgl.h>
#include "lv_page_common.h"
#include <BleKeyboard.h>
#include "config.h"
#include "mqtt.h"
#include "arduinoFFT.h"

#include "arduinoFFT.h"

arduinoFFT FFT = arduinoFFT(); /* Create FFT object */
/*
These values can be changed in order to evaluate the functions
*/
#define CHANNEL A10
const uint16_t samples = 64; //This value MUST ALWAYS be a power of 2
const double samplingFrequency = 100; //Hz, must be less than 10000 due to ADC
unsigned int sampling_period_us;
unsigned long microseconds;

void PrintVector(double *vData, uint16_t bufferSize, uint8_t scaleType);

/*
These are the input and output vectors
Input vectors receive computed results from FFT
*/
double vReal[samples];
double vImag[samples];

#define SCL_INDEX 0x00
#define SCL_TIME 0x01
#define SCL_FREQUENCY 0x02
#define SCL_PLOT 0x03

void setup()
{
  sampling_period_us = round(1000000*(1.0/samplingFrequency));
  Serial.begin(115200);
  while(!Serial);
  Serial.println("Ready");
}

void loop()
{
  /*SAMPLING*/
  microseconds = micros();
  for(int i=0; i<samples; i++)
  {
      vReal[i] = analogRead(CHANNEL);
      vImag[i] = 0;
      while(micros() - microseconds < sampling_period_us){
        //empty loop
      }
      microseconds += sampling_period_us;
  }
  /* Print the results of the sampling according to time */
  Serial.println("Data:");
  PrintVector(vReal, samples, SCL_TIME);

  FFT.Windowing(vReal, samples, FFT_WIN_TYP_HAMMING, FFT_FORWARD);	/* Weigh data */
  Serial.println("Weighed data:");
  PrintVector(vReal, samples, SCL_TIME);

  FFT.Compute(vReal, vImag, samples, FFT_FORWARD); /* Compute FFT */
  Serial.println("Computed Real values:");
  PrintVector(vReal, samples, SCL_INDEX);

  Serial.println("Computed Imaginary values:");
  PrintVector(vImag, samples, SCL_INDEX);
  
  FFT.ComplexToMagnitude(vReal, vImag, samples); /* Compute magnitudes */

  Serial.println("Computed magnitudes:");
  PrintVector(vReal, (samples >> 1), SCL_FREQUENCY);
  double x = FFT.MajorPeak(vReal, samples, samplingFrequency);
  Serial.println(x, 6); //Print out what frequency is the most dominant.
  while(1); /* Run Once */
  // delay(2000); /* Repeat after delay */
}

void PrintVector(double *vData, uint16_t bufferSize, uint8_t scaleType)
{
  for (uint16_t i = 0; i < bufferSize; i++)
  {
    double abscissa;
    /* Print abscissa value */
    switch (scaleType)
    {
      case SCL_INDEX:
        abscissa = (i * 1.0);
	break;
      case SCL_TIME:
        abscissa = ((i * 1.0) / samplingFrequency);
	break;
      case SCL_FREQUENCY:
        abscissa = ((i * 1.0 * samplingFrequency) / samples);
	break;

    }
    Serial.print(abscissa, 6);
    if(scaleType==SCL_FREQUENCY)
      Serial.print("Hz");

    Serial.print(" ");
    Serial.println(vData[i], 4);
  }

  Serial.println();
}
