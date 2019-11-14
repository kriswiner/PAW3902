# PAW3902
Optical flow sensor with three light levels

Basic sketch for PAW3902 optical flow sensor using motion pin as data ready interrupt. 

![PAW3902](https://user-images.githubusercontent.com/6698410/68826664-ac3db980-0653-11ea-9e62-be59510a926d.jpg)
*PAW3902 breakout board on an STM32L476 Dragonfly Development Board*

Added automatic switching between light modes depending on shutter and data quality (features). 

Added frame capture of 35 x 35 pixel 8-bit gray scale imaged. Frame rate is a miserable ~5 seconds or so. But this capability is useful for basic feature detection and low light/IR "photography", since the sensor sensitivity extends to the IR (940 nm) and an IR illumination led can be added and synced to the sensor frames.
