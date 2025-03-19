#include <Arduino.h>
#include "esp_adc_cal.h" // For ADC calibration
#include "driver/adc.h"   // For ADC driver

// Define a structure to hold ADC calibration characteristics
typedef struct {
  esp_adc_cal_characteristics_t *adc_chars;
  adc_atten_t current_atten;
} adc_calibration_data_t;

// Global array to store calibration data for each ADC channel (ADC1)
#define ADC1_CHANNEL_COUNT 10 // Assuming all ADC1 channels are potentially used
adc_calibration_data_t adc1_cal_data[ADC1_CHANNEL_COUNT];
bool adc1_cal_initialized[ADC1_CHANNEL_COUNT] = {false};

// Helper function to get the ADC channel number from the Arduino pin number for ADC1
static int get_adc1_channel(int pin) {
  if (pin == 36) return 0; // GPIO36, ADC1_CHANNEL_0
  if (pin == 37) return 1; // GPIO37, ADC1_CHANNEL_1
  if (pin == 38) return 2; // GPIO38, ADC1_CHANNEL_2
  if (pin == 39) return 3; // GPIO39, ADC1_CHANNEL_3
  if (pin == 32) return 4; // GPIO32, ADC1_CHANNEL_4
  if (pin == 33) return 5; // GPIO33, ADC1_CHANNEL_5
  if (pin == 34) return 6; // GPIO34, ADC1_CHANNEL_6
  if (pin == 35) return 7; // GPIO35, ADC1_CHANNEL_7
  if (pin == 25) return 8; // GPIO25, ADC1_CHANNEL_8
  if (pin == 26) return 9; // GPIO26, ADC1_CHANNEL_9
  return -1; // Invalid pin for ADC1
}

/**
 * @brief Replacement for analogReadMillivolts on ESP32 using IDF functions (for ESP-IDF 3.1.3).
 *
 * This function reads the analog value from the specified pin, using fuse calibration data
 * and supporting oversampling. It also allows setting the attenuation before reading.
 *
 * @param pin The Arduino pin number to read.
 * @param attenuation The ADC attenuation setting (ADC_ATTEN_0dB, ADC_ATTEN_2_5dB,
 * ADC_ATTEN_6dB, ADC_ATTEN_11dB).
 * @param oversampling The number of samples to read and average for oversampling.
 * A value of 1 disables oversampling. Higher values increase
 * resolution at the cost of reading time.
 * @return The analog reading in millivolts, or -1 if an error occurred.
 */
int analogReadMillivolts(int pin, adc_atten_t attenuation, int oversampling) {
  int adc1_chan = get_adc1_channel(pin);
  if (adc1_chan == -1) {
    log_e("analogReadMillivolts", "Invalid pin for ADC1: %d", pin);
    return -1;
  }

  // Configure ADC
  adc1_config_width(ADC_WIDTH_BIT_12); // Using 12-bit resolution
  esp_err_t err = adc1_config_channel_atten((adc1_channel_t)adc1_chan, attenuation);
  if (err != ESP_OK) {
    log_e("analogReadMillivolts", "Error configuring attenuation for pin %d: %s", pin, esp_err_to_name(err));
    return -1;
  }

  // Initialize calibration data if not already initialized or if attenuation changed
  if (!adc1_cal_initialized[adc1_chan] || adc1_cal_data[adc1_chan].current_atten != attenuation) {
    if (adc1_cal_data[adc1_chan].adc_chars != NULL) {
      free(adc1_cal_data[adc1_chan].adc_chars);
    }
    adc1_cal_data[adc1_chan].adc_chars = (esp_adc_cal_characteristics_t *)malloc(sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_characterize(ADC_UNIT_1, attenuation, ADC_WIDTH_BIT_12, 3300, adc1_cal_data[adc1_chan].adc_chars); // Assuming 3.3V Vref
    adc1_cal_data[adc1_chan].current_atten = attenuation;
    adc1_cal_initialized[adc1_chan] = true;
  }

  int raw_adc = 0;
  if (oversampling > 1) {
    long long sum = 0;
    for (int i = 0; i < oversampling; i++) {
      sum += adc1_get_raw((adc1_channel_t)adc1_chan);
      delay(1); // Small delay between readings for stability
    }
    raw_adc = sum / oversampling;
  } else {
    raw_adc = adc1_get_raw((adc1_channel_t)adc1_chan);
  }

  if (adc1_cal_initialized[adc1_chan]) {
    uint32_t voltage = esp_adc_cal_raw_to_voltage(raw_adc, adc1_cal_data[adc1_chan].adc_chars);
    return (int)voltage; // voltage is in mV
  } else {
    // If calibration failed, return the raw value (scaled approximately)
    // This is a very rough estimate and should ideally not be relied upon.
    // Consider returning an error code or a special value.
    log_w("analogReadMillivolts", "Calibration not initialized for pin %d, returning raw value scaled (approximate).", pin);
    return (raw_adc * 3300) / 4095; // Assuming 3.3V full scale and 12-bit ADC
  }
}

// Example usage in your Arduino sketch:
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("ESP32 analogReadMillivolts Replacement Example (ESP-IDF 3.1.3)");
}

void loop() {
  int pinToRead = 39; // Example pin (GPIO34)
//  adc_atten_t attenuationSetting = ADC_ATTEN_DB_11; // Example attenuation (Corrected name)
  adc_atten_t attenuationSetting = ADC_ATTEN_DB_0; // Example attenuation (Corrected name)

  int oversamplingFactor = 16; // Example oversampling factor

  int millivolts = analogReadMillivolts(pinToRead, attenuationSetting, oversamplingFactor);

  if (millivolts != -1) {
    Serial.printf("Analog reading from pin %d with attenuation %d dB and oversampling %d: %d mV\n",
                  pinToRead, (int)(attenuationSetting * 2.5), oversamplingFactor, millivolts);
  } else {
    Serial.printf("Error reading from pin %d\n", pinToRead);
  }

  delay(1000);
}
