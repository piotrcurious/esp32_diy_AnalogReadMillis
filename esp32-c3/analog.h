#ifndef ANALOG_H
#define ANALOG_H

#include <Arduino.h>
#include "esp_adc_cal.h" // For ADC calibration
#include "driver/adc.h"   // For ADC driver

// Define a structure to hold ADC calibration characteristics
typedef struct {
  esp_adc_cal_characteristics_t *adc_chars;
  adc_atten_t current_atten;
} adc_calibration_data_t;

// Global array to store calibration data for each ADC channel (ADC1)
// For ESP32-C3, only 6 ADC1 channels are available.
#define ADC1_CHANNEL_COUNT 6
extern adc_calibration_data_t adc1_cal_data[ADC1_CHANNEL_COUNT];
extern bool adc1_cal_initialized[ADC1_CHANNEL_COUNT];

// Helper function to get the ADC channel number from the Arduino pin number for ADC1
// For ESP32-C3, we assume that analog pins A0–A5 correspond to GPIO0–GPIO5.
static inline int get_adc1_channel(int pin) {
  switch(pin) {
    case 0: return ADC1_CHANNEL_0; // GPIO0, A0
    case 1: return ADC1_CHANNEL_1; // GPIO1, A1
    case 2: return ADC1_CHANNEL_2; // GPIO2, A2
    case 3: return ADC1_CHANNEL_3; // GPIO3, A3
    case 4: return ADC1_CHANNEL_4; // GPIO4, A4
    case 5: return ADC1_CHANNEL_5; // GPIO5, A5
    default: return -1; // Invalid pin for ADC1 on ESP32-C3
  }
}

/**
 * @brief Replacement for analogReadMillivolts on ESP32-C3 using ESP-IDF functions.
 *
 * This function reads the analog value from the specified pin, using calibration data
 * and supporting oversampling. It also allows setting the attenuation before reading.
 *
 * @param pin The Arduino pin number (corresponding to a GPIO on the ESP32-C3).
 * @param attenuation The ADC attenuation setting (ADC_ATTEN_DB_0, ADC_ATTEN_DB_2_5,
 * ADC_ATTEN_DB_6, ADC_ATTEN_DB_11).
 * @param oversampling The number of samples to read and average for oversampling.
 * A value of 1 disables oversampling. Higher values increase effective resolution at the cost
 * of reading time.
 * @return The analog reading in millivolts, or -1 if an error occurred.
 */
int analogReadMillivolts(int pin, adc_atten_t attenuation, int oversampling);

#endif // ANALOG_H
