#include "analog.h"

// Global arrays for calibration data and initialization status
adc_calibration_data_t adc1_cal_data[ADC1_CHANNEL_COUNT];
bool adc1_cal_initialized[ADC1_CHANNEL_COUNT] = { false };

int analogReadMillivolts(int pin, adc_atten_t attenuation, int oversampling) {
  int adc1_chan = get_adc1_channel(pin);
  if (adc1_chan == -1) {
    log_e("analogReadMillivolts", "Invalid pin for ADC1: %d", pin);
    return -1;
  }

  // Configure ADC: use 12-bit resolution for ESP32-C3 ADC1
  adc1_config_width(ADC_WIDTH_BIT_12);
  esp_err_t err = adc1_config_channel_atten((adc1_channel_t)adc1_chan, attenuation);
  if (err != ESP_OK) {
    log_e("analogReadMillivolts", "Error configuring attenuation for pin %d: %s", pin, esp_err_to_name(err));
    return -1;
  }

  // Initialize calibration data if not already initialized or if the attenuation changed
  if (!adc1_cal_initialized[adc1_chan] || adc1_cal_data[adc1_chan].current_atten != attenuation) {
    if (adc1_cal_data[adc1_chan].adc_chars != NULL) {
      free(adc1_cal_data[adc1_chan].adc_chars);
    }
    adc1_cal_data[adc1_chan].adc_chars = (esp_adc_cal_characteristics_t *)malloc(sizeof(esp_adc_cal_characteristics_t));
    // The default Vref is assumed to be 3300 mV. Adjust if your board uses a different reference.
    esp_adc_cal_characterize(ADC_UNIT_1, attenuation, ADC_WIDTH_BIT_12, 3300, adc1_cal_data[adc1_chan].adc_chars);
    adc1_cal_data[adc1_chan].current_atten = attenuation;
    adc1_cal_initialized[adc1_chan] = true;
  }

  int raw_adc = 0;
  if (oversampling > 1) {
    long long sum = 0;
    for (int i = 0; i < oversampling; i++) {
      sum += adc1_get_raw((adc1_channel_t)adc1_chan);
      // A small delay between readings can improve stability if needed
      // delay(1);
    }
    raw_adc = sum / oversampling;
  } else {
    raw_adc = adc1_get_raw((adc1_channel_t)adc1_chan);
  }

  if (adc1_cal_initialized[adc1_chan]) {
    uint32_t voltage = esp_adc_cal_raw_to_voltage(raw_adc, adc1_cal_data[adc1_chan].adc_chars);
    return (int)voltage; // voltage is in millivolts
  } else {
    // If calibration is not initialized, return a rough estimate based on the raw value.
    log_w("analogReadMillivolts", "Calibration not initialized for pin %d, returning raw value scaled (approximate).", pin);
    return (raw_adc * 3300) / 4095; // Using 3.3V full scale with 12-bit ADC resolution
  }
}
