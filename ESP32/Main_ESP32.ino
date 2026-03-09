// Created by Tan Tri Nguyen, BME student k23
// Biomedical Engineering Department, International University-Vietnam National University, HCMC


#include <driver/i2s.h>
#include <math.h>
#include "esp_dsp.h"             
#include "random_forest_model.h"
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

// Pin Configuration 
#define I2S_DOUT   2
#define I2S_BCLK   1
#define I2S_LRCLK  3  
#define BUTTON_PIN 13
#define RGB_PIN    48  

// Audio Configuration
#define I2S_PORT I2S_NUM_0
#define SAMPLE_RATE 16000
#define CHUNK_SIZE 1024
#define MAX_SAMPLES 48000 

int16_t audio_buffer[MAX_SAMPLES];
volatile int total_samples_recorded = 0;
int32_t raw_samples[CHUNK_SIZE];

// State Machine
enum SystemState { WAITING_BLE, IDLE, RECORDING, DONE };
SystemState current_state = WAITING_BLE;

// DSP & FFT Configuration
#define FFT_SIZE 2048
float fft_workspace[FFT_SIZE * 2]; 
float hann_window[FFT_SIZE];

// BLE Configuration
BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;

// Standard custom UUIDs for BLE UART
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };
    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

// RGB LED Helper
void setRGB(uint8_t r, uint8_t g, uint8_t b) {
  neopixelWrite(RGB_PIN, r / 4, g / 4, b / 4);
}

// Timer for non-blocking RED blink
unsigned long previousBlinkMillis = 0;
bool blinkState = false;

void extract_features_and_predict(int16_t* buffer, int length) {
  if (length < FFT_SIZE) {
    Serial.println("Not enough data for FFT.");
    return;
  }

  // 1. Time-Domain Feature Extraction (RMS & ZCR)
  long long sum_sq = 0;
  int zcr_count = 0;
  
  for (int i = 0; i < length; i++) {
    sum_sq += ((long long)buffer[i] * buffer[i]);
    if (i > 0 && ((buffer[i] >= 0 && buffer[i-1] < 0) || (buffer[i] < 0 && buffer[i-1] >= 0))) {
      zcr_count++;
    }
  }
  
  float rms_amplitude = sqrt((float)sum_sq / length);
  float zero_crossing_rate = (float)zcr_count / length;

  // 2. Frequency-Domain Feature Extraction (FFT)
  for (int i = 0; i < FFT_SIZE; i++) {
    fft_workspace[i * 2] = (float)buffer[i] * hann_window[i];
    fft_workspace[i * 2 + 1] = 0.0;
  }

  dsps_fft2r_fc32(fft_workspace, FFT_SIZE);
  dsps_bit_rev_fc32(fft_workspace, FFT_SIZE);

  float total_power = 0.0;
  float max_power = 0.0;
  int peak_bin = 0;
  float sum_freq_power = 0.0;
  float power_spectrum[FFT_SIZE / 2];

  for (int i = 0; i < FFT_SIZE / 2; i++) {
    float real = fft_workspace[i * 2];
    float imag = fft_workspace[i * 2 + 1];
    float power = (real * real) + (imag * imag);
    
    power_spectrum[i] = power;
    total_power += power;
    sum_freq_power += (power * i);
    
    if (power > max_power) {
      max_power = power;
      peak_bin = i;
    }
  }

  float freq_resolution = (float)SAMPLE_RATE / FFT_SIZE;
  float peak_frequency = peak_bin * freq_resolution;
  float mean_frequency = (total_power > 0) ? (sum_freq_power / total_power) * freq_resolution : 0.0;

  float spectral_entropy = 0.0;
  if (total_power > 0) {
    for (int i = 0; i < FFT_SIZE / 2; i++) {
      float prob = power_spectrum[i] / total_power;
      if (prob > 0) {
        spectral_entropy -= prob * log2(prob);
      }
    }
  }

  float scaled_rms = rms_amplitude; 
  float scaled_peak_freq = peak_frequency;
  float scaled_entropy = spectral_entropy; 
  float scaled_zcr = zero_crossing_rate; 
  float scaled_mean_freq = mean_frequency;

  // 3. Format Array for emlearn Decision Tree
  float features[5];
  features[0] = scaled_rms;
  features[1] = scaled_peak_freq;
  features[2] = scaled_entropy;
  features[3] = scaled_zcr;
  features[4] = scaled_mean_freq;

  // 4. Run Inference
  int32_t prediction = joint_status_rf_predict(features, 5);

  // 5. Output Diagnosis
  Serial.print("--- VAG DIAGNOSIS RESULT --- \nCondition: ");
  String resultStr = "Unknown";
  if (prediction == 0) {
      resultStr = "Normal";
  } else if (prediction == 1) {
      resultStr = "Osteoarthritis";
  } else if (prediction == 2) {
      resultStr = "Ligament Injury";
  } 
  
  Serial.println(resultStr);
  Serial.println("----------------------------");

  // Send result over BLE
  if (deviceConnected) {
      pCharacteristic->setValue(resultStr.c_str());
      pCharacteristic->notify();
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  // Initialize DSP
  dsps_fft2r_init_fc32(NULL, CONFIG_DSP_MAX_FFT_SIZE);
  dsps_wind_hann_f32(hann_window, FFT_SIZE);

  // Initialize I2S
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 8,
    .dma_buf_len = CHUNK_SIZE,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0
  };

  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_BCLK,
    .ws_io_num = I2S_LRCLK,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = I2S_DOUT
  };

  i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_PORT, &pin_config);

  // Initialize BLE
  BLEDevice::init("ESP32_Joint_Diagnoser");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );
  
  pService->start();
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  BLEDevice::startAdvertising();

  Serial.println("Waiting for BLE connection...");
}

void loop() {
  // 1. Handle BLE Disconnect Event
  if (!deviceConnected && oldDeviceConnected) {
      delay(500); // Give the BLE stack time to reset
      BLEDevice::startAdvertising();
      oldDeviceConnected = false;
      current_state = WAITING_BLE;
      Serial.println("Device disconnected. Advertising restarted.");
  }

  // 2. Handle BLE Newly Connected Event (Blink GREEN 3 times)
  if (deviceConnected && !oldDeviceConnected) {
      Serial.println("Device Connected!");
      for (int i = 0; i < 3; i++) {
          setRGB(0, 255, 0); // Green
          delay(300);
          setRGB(0, 0, 0);   // Off
          delay(300);
      }
      oldDeviceConnected = true;
      current_state = IDLE;
      setRGB(0, 255, 0); // Use BLUE to signify connected & IDLE
  }

  // 3. Handle Waiting for Connection (Blink RED continuously)
  if (!deviceConnected) {
      unsigned long currentMillis = millis();
      if (currentMillis - previousBlinkMillis >= 500) {
          previousBlinkMillis = currentMillis;
          blinkState = !blinkState;
          if (blinkState) setRGB(255, 0, 0); 
          else setRGB(0, 0, 0);
      }
      return; // Do not allow main functionality until connected
  }

  // --- MAIN FUNCTION (Runs only when connected) ---
  bool button_pressed = (digitalRead(BUTTON_PIN) == LOW);

  if (button_pressed) {
    if (current_state == IDLE || current_state == DONE) {
      Serial.println("Recording started...");
      total_samples_recorded = 0;
      current_state = RECORDING;
      setRGB(255, 255, 0); // YELLOW: Recording active
      i2s_zero_dma_buffer(I2S_PORT);
    }

    if (total_samples_recorded < MAX_SAMPLES) {
      size_t bytes_read = 0;
      esp_err_t result = i2s_read(I2S_PORT, &raw_samples, sizeof(raw_samples), &bytes_read, portMAX_DELAY);
      
      if (result == ESP_OK) { 
        int samples_read = bytes_read / sizeof(int32_t);
        for (int i = 0; i < samples_read; i++) { 
          if (total_samples_recorded < MAX_SAMPLES) {
            audio_buffer[total_samples_recorded] = (int16_t)(raw_samples[i] >> 14);
            total_samples_recorded++;
          }
        }
      }
    }
  } else {
    if (current_state == RECORDING) {
      Serial.println("Recording stopped. Processing acoustic signal...");
      extract_features_and_predict(audio_buffer, total_samples_recorded);
      
      // 1. Show GREEN to confirm analysis and BLE transmission are complete
      setRGB(0, 255, 0); 
      delay(1500); // Keep green for 1.5 seconds 
      
      // 2. Switch to RED and set state to IDLE to indicate it is ready for the next recording
      current_state = IDLE;
      setRGB(255, 0, 0); 
    }
    
    delay(10);
  }
}
