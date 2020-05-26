# Smart Wearable to predict ASD meltdowns

ASD, which is also known as Autism Spectrum Disorder, is a brain development condition, effecting people in different ways. Moreover, this is a condition which effects an individual through his or her life. It often begins in childhood, with many displaying symptoms before the age of 1. The main problem behind this kind of disorder is generally the patients suffer from-
1. Having issue with social interaction and communication.
2. Displaying of certain repetitive pattern or behavior called as “meltdowns”.

# Architecture

<img width="599" alt="Picture 1" src="https://user-images.githubusercontent.com/22388218/82872374-9721db80-9f50-11ea-9d30-5536c91adabd.png">


The Proposed system is divided into three parts –
1. Hardware System
2. Communication Gateway
3. Cloud Application

The hardware system consists of sensors and actuators used in the device. As shown in the above Figure, I have used heart rate sensor and temperature sensor to read the body units of an ASD patient. The sensor data is read through GPIO pins of ESP32 which is a standalone Wi-Fi microcontroller available in the market.

Below Figure shows the setup of the proposed system. The circuit is fixed on hand glove. A battery is kept under the breadboard which is the main source of power. The glove will be fixed on patients’ right hand. The temperature sensor is fixed underneath the hand and pulse rate will be measured through a figure as shown in the figure.

A web page has been deployed on the server which will be monitored by a caretaker. This page has a unique ID assigned which is equal to the ID of the data stored in the firebase. The page shows the real time pulse rate and the temperature of the patient’s body.

# Visual of the Crude Setup
   <img width="385" alt="Picture 2" src="https://user-images.githubusercontent.com/22388218/82872389-9b4df900-9f50-11ea-927d-8efcd68efaf8.png">  <img width="180" alt="Picture 3" src="https://user-images.githubusercontent.com/22388218/82872392-9c7f2600-9f50-11ea-8274-0dcb782fbbee.png">  <img width="614" alt="Picture 4" src="https://user-images.githubusercontent.com/22388218/82872657-f7b11880-9f50-11ea-9d88-80aa5c2fb755.png">
