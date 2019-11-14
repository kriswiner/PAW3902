# PAW3902
Optical flow sensor with three light levels

Basic sketch for PAW3902 optical flow sensor using motion pin as data ready interrupt. 

Added automatic switching between light modes depending on shutter and data quality (features). 

Added frame capture of 35 x 35 pixel 8-bit gray scale imaged. Frame rate is a miserable ~5 seconds or so. But this capability is useful for basic feature detection and low light/IR "photography", since the sensor sensitivity extends to the IR (940 nm) and an IR illumination led can be added and synced to the sensor frames.
