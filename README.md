# Joint&Waves: Wearable Acoustic Joint Diagnosis System

[cite_start]**Design and development of a wearable system for diagnosing joint conditions via acoustic analysis of movement-induced sounds.** [cite: 12, 13, 14]

![Device Outside](hardware/device_outside.jpg) *[Insert your image path here]*
![Device Inside](hardware/device_inside.jpg) *[Insert your image path here]*

## Overview
[cite_start]Knee osteoarthritis (OA) affects over 365 million people globally, with standard diagnostics relying on costly, radiation-heavy methods like X-rays and MRIs[cite: 19, 22]. [cite_start]Joint&Waves is a low-cost, non-invasive wearable device utilizing Vibroarthrography and Edge AI[cite: 24]. [cite_start]It performs real-time acoustic analysis of joint sounds to classify conditions as Normal, Osteoarthritis, or Ligament Injury, enabling continuous joint health tracking[cite: 24, 85].

## Hardware Architecture
[cite_start]The data acquisition node is engineered for optimal patient comfort and non-intrusive monitoring[cite: 38, 44].
* [cite_start]**Microcontroller:** ESP32-S3 Module[cite: 70].
* [cite_start]**Sensor:** SPH0645 MEMS microphone for high-fidelity acoustic capture[cite: 68].
* [cite_start]**Power:** Safely managed Li-Po battery with a modern Type-C charging interface[cite: 42, 43].
* [cite_start]**Dimensions:** Ultra-compact design measuring 3.75x2.7x2.35 cm[cite: 42].
* [cite_start]**Endurance:** Sustains up to 40 minutes of continuous data acquisition and on-device AI inference per full charge[cite: 43].

## Signal Processing & Edge AI Pipeline
[cite_start]By computing the diagnosis directly on the edge node, the system significantly reduces energy overhead and secures raw acoustic data[cite: 108].
1. [cite_start]**Acquisition:** Captures raw joint sound during movement[cite: 65, 77]. [cite_start]Spectral analysis confirms critical features are concentrated within the 0-8000 Hz frequency band[cite: 106].
2. [cite_start]**Signal Processing:** Filters the raw sound and extracts key acoustic features, prioritizing mean amplitude, spectral entropy, and mean frequency[cite: 78, 79, 103, 120]. 
3. [cite_start]**Inference:** A highly optimized Random Forest TinyML model runs directly on the ESP32-S3 to classify the joint condition[cite: 39, 86]. 
4. [cite_start]**Transmission:** The diagnosis result is transmitted via BLE to a PC or smartphone for real-time visualization[cite: 74, 75].

## Performance Metrics
[cite_start]The Random Forest model demonstrates strong operational stability and classification capability without signal degradation[cite: 86, 105]. 
* [cite_start]**Normal Joints:** 100% accuracy in classification[cite: 87].
* [cite_start]**Pathological Joints:** Strong differentiation capabilities, with an 83% recall for Osteoarthritis and 85.7% recall for Ligament Injuries[cite: 92, 93]. 
* *[Insert your additional extended performance metrics, ROC curves, or latency stats here]*

## Repository Contents
* `/hardware`: Device schematics and internal/external photographs.
* `/firmware`: ESP32-S3 C++ code for I2S microphone interfacing, DSP filtering, and BLE transmission.
* `/model`: Feature extraction scripts and TinyML model deployment files.
* `/docs`: Project poster and extended evaluation charts.

## Authors and Acknowledgements
* [cite_start]**Authors:** Nguyen Tan Tri, Ta Hoang Truc Tho[cite: 15].
* **Acknowledgements:** Special thanks to Assoc. [cite_start]Prof. Le Ngoc Bich for the opportunity to design this wearable device, and to Nguyen Nhat Minh and Ta Minh Tri for their research and development support[cite: 122, 123].
