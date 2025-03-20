#ifndef ANALOG_H
#define ANALOG_H

#include <Arduino.h>
#include "esp_adc_cal.h"   // For ADC calibration functions
#include "driver/adc.h"    // For ADC driver functions

// Structure to hold ADC calibration characteristics.
typedef struct {
    esp_adc_cal_characteristics_t *adc_chars;
    adc_atten_t current_atten;
} adc_calibration_data_t;

// Global array to store calibration data for each ADC1 channel.
// (On the ESP32-S2, the number of available ADC channels may vary.)
#define ADC1_CHANNEL_COUNT 10
extern adc_calibration_data_t adc1_cal_data[ADC1_CHANNEL_COUNT];
extern bool adc1_cal_initialized[ADC1_CHANNEL_COUNT];

/**
 * @brief Helper function to map an Arduino pin number to an ADC1 channel on the ESP32-S2.
 *
 * This example mapping is based on an ESP32-S2 board (such as the Saola-1)
 * where analog pins are typically A0 through A6 mapped to GPIO4–GPIO10.
 * Adjust this mapping to match your board’s specifications.
 *
 * @param pin The Arduino pin number.
 * @return The ADC1 channel (as defined by the driver) or -1 if the pin is invalid.
 */
static inline int get_adc1_channel(int pin) {
    switch(pin) {
        case 4:  return ADC1_CHANNEL_1;  // A0: GPIO4
        case 5:  return ADC1_CHANNEL_2;  // A1: GPIO5
        case 6:  return ADC1_CHANNEL_3;  // A2: GPIO6
        case 7:  return ADC1_CHANNEL_4;  // A3: GPIO7
        case 8:  return ADC1_CHANNEL_5;  // A4: GPIO8
        case 9:  return ADC1_CHANNEL_6;  // A5: GPIO9
        case 10: return ADC1_CHANNEL_7;  // A6: GPIO10
        default: return -1;              // Invalid pin for ADC1 on ESP32-S2.
    }
}

/**
 * @brief Reads an analog value in millivolts from the specified pin.
 *
 * This function configures the ADC channel with the given attenuation, performs oversampling
 * (if requested), and converts the raw ADC reading into millivolts using calibration data.
 *
 * @param pin The Arduino pin number to read.
 * @param attenuation The ADC attenuation setting (e.g., ADC_ATTEN_DB_0, ADC_ATTEN_DB_2_5, ADC_ATTEN_DB_6, ADC_ATTEN_DB_11).
 * @param oversampling The number of samples to average (set to 1 to disable oversampling).
 * @return The analog reading in millivolts, or -1 if an error occurred.
 */
int analogReadMillivolts(int pin, adc_atten_t attenuation, int oversampling);

#endif // ANALOG_H
