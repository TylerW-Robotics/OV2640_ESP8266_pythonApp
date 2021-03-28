# OV2640_ESP8266_pythonApp
This repository is a ESP8266 Nodemcu using the OV2640 ArduCam is take a simple 320x240 photo. After the photo is taken, It will send serial data to the python computer app to display the new photo. 

The ArduCam OV2640 is a 2MP that uses I2C and SPI communication.
The I2C communication is used for setting up the camera modes and for capturing a photo.
The SPI communication simply collects that data of photo. In this case, we collect the color of every pixel.
The pixel color is in RGB565 (2 Bytes per Pixel).

The collected data will be sent to a python app on the computer, through serial communication. 
The pixel array is then converted into a visible photo. 

## Code details
**ESP8266**

