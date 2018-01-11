# Environment Monitor ThingySticks


Particle Photon or Electron based board to measure Humidity, Temperature, Light level and pressure.

* Compaible with Particle Power Shield to allow completly wireless monitoring.
* Compaible with Particle Asset Sheild to allow location information to be included with sensor data.
* Works with Photon or Electron.



## V2:

* Temperature, Humidity (HTU21D)
* Pressure (BMP180)
* Light (TEMT6000)
* Uses a Particle Photon or Electron
* Can piggyback onto Particle Asset Tracker and other ThingySticks prototype PCBs

Available on Tindie: https://www.tindie.com/products/ThingySticks/environment-sensor-for-the-photon-or-electron/

## V3:


Uses a BME680 - These are currently very hard (i.e. impossible) to get hold of. They were expected to be sold over a year ago, hence V3 of the monitor is on-hold.

## V4:

* Total Volatile Organic Compounds (TVOC) (CCS811)
* Equivelant CO2 (CCS811)
* Temperature, Humidity and Pressure (BME280)
* Light (TEMT6000)
* Uses a Particle Photon or Electron
* Can piggyback onto Particle Asset Tracker and other ThingySticks prototype PCBs

Available on Tindie: https://www.tindie.com/products/ThingySticks/air-quality-sensor-bme280ccs811-thingystick/


Build Files
-----------

Files used by the CI server to automate Eagle as part of a build process.

Clean.bat - As it says, pre build clean.
Build.bat - The build tasks (Generate images of the schematic, PCB top, bottom, all and measurements layers).
Distribute.bat - Copy the images to the appropriate places (S3....)