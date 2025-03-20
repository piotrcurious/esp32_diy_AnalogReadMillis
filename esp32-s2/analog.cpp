#include "analog.h"

// Global calibration storage for each ADC1 channel.
adc_calibration_data_t adc1_cal_data[ADC1_CHANNEL_COUNT];
bool adc1_cal_initialized[ADC1_CHANNEL_COUNT] = { false };

int analogReadMillivolts(int pin, adc_atten_t attenuation, int oversampling) {
    int adc1_chan = get_adc1_channel(pin);
    if (adc1_chan == -1) {
        log_e("analogReadMillivolts", "Invalid pin for ADC1: %d", pin);
        return -1;
    }

    // Configure the ADC to use 13-bit resolution for ESP32-S2.
    adc1_config_width(ADC_WIDTH_BIT_13);
    esp_err_t err = adc1_config_channel_atten((adc1_channel_t)adc1_chan, attenuation);
    if (err != ESP_OK) {
        log_e("analogReadMillivolts", "Error configuring attenuation for pin %d: %s", pin, esp_err_to_name(err));
        return -1;
    }

    // Initialize or update calibration data if needed.
    if (!adc1_cal_initialized[adc1_chan] || adc1_cal_data[adc1_chan].current_atten != attenuation) {
        if (adc1_cal_data[adc1_chan].adc_chars != NULL) {
            free(adc1_cal_data[adc1_chan].adc_chars);
        }
        adc1_cal_data[adc1_chan].adc_chars = (esp_adc_cal_characteristics_t *)malloc(sizeof(esp_adc_cal_characteristics_t));
        // Characterize the ADC channel using a default Vref of 3300 mV.
        esp_adc_cal_characterize(ADC_UNIT_1, attenuation, ADC_WIDTH_BIT_13, 3300, adc1_cal_data[adc1_chan].adc_chars);
        adc1_cal_data[adc1_chan].current_atten = attenuation;
        adc1_cal_initialized[adc1_chan] = true;
    }

    int raw_adc = 0;
    if (oversampling > 1) {
        long long sum = 0;
        for (int i = 0; i < oversampling; i++) {
            sum += adc1_get_raw((adc1_channel_t)adc1_chan);
            // Optionally, add a short delay (e.g., delay(1)) for stability.
        }
        raw_adc = sum / oversampling;
    } else {
        raw_adc = adc1_get_raw((adc1_channel_t)adc1_chan);
    }

    // Convert the raw ADC reading to millivolts using calibration data.
    if (adc1_cal_initialized[adc1_chan]) {
        uint32_t voltage = esp_adc_cal_raw_to_voltage(raw_adc, adc1_cal_data[adc1_chan].adc_chars);
        return (int)voltage;
    } else {
        log_w("analogReadMillivolts", "Calibration not initialized for pin %d; returning approximate scaled raw value.", pin);
        // Fallback conversion using 13-bit maximum value (8191 = 2^13 - 1).
        return (raw_adc * 3300) / 8191;
    }
}
