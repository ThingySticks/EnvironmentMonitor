/*
  This example reads audio data from the on-board PDM microphones, and prints
  out the samples to the Serial console. The Serial Plotter built into the
  Arduino IDE can be used to plot the audio data (Tools -> Serial Plotter)

  Circuit:
  - Arduino Nano 33 BLE board

  This example code is in the public domain.
*/

#include <PDM.h>
#include <Arduino_LSM9DS1.h>
#include <Arduino_APDS9960.h>
#include <Arduino_LPS22HB.h>
#include <Arduino_HTS221.h>
#include <ArduinoBLE.h>

// RGB led 
const int rgbRedPin = 22;
const int rgbGreenPin = 23;
const int rgbBluePin = 24;

// PCB Red/Yellow/Green
const int pcbRedPin = 5;
const int pcbYellowPin = 6;
const int pcbGreenPin = 9;

// TODO: Battery voltate monitor!
// TODO: Diode on battery!

// Low power notes:

// MP34DT05-A 1.6 - 3.6V. Tyical 1.8 (Mic)
// LSM9DS3  1.9 V to 3.6 V (Accel)
// APDS9960 2.4 - 3.6 (Typical 3.0) (Light)
// LPS22HB  1.7 to 3.6 V (Pressure)
// HTS221  1.7 to 3.6 V (Temperature / Humidity)
// NINA-B3 1.7 to 3.6 (Typical 3.3)
// DL3 - R + G + B LED (Blue won't work at 3v)

// Extra...
// BME680 1.71 to 3.6


// BLE Battery Service
BLEService batteryService("180F");

// BLE Battery Level Characteristic
// standard 16-bit characteristic UUID
BLEUnsignedCharCharacteristic batteryLevelChar("2A19",  BLERead | BLENotify);

// https://github.com/bneedhamia/DHTBLESensor/blob/master/DHTBLESensor.ino
BLEService envService("181A");
BLEUnsignedShortCharacteristic bleHumidityPC("2A6F", BLERead | BLENotify);
BLEShortCharacteristic bleTemperatureC("2A6E", BLERead | BLENotify);

int oldBatteryLevel = 0;  // last battery level reading from analog input
long previousMillis = 0;  // last time the battery level was checked, in ms



// buffer to read samples into, each sample is 16-bits
short sampleBuffer[256];

// number of samples read
volatile int samplesRead;

void setup() {
  Serial.begin(9600);
  delay(2000);
  
  pinMode(LED_BUILTIN, OUTPUT); // initialize the built-in LED pin to indicate when a central is connected
  digitalWrite(LED_BUILTIN, HIGH);
  // RGB LED (DL3)
  //pinMode(rgbRedPin, OUTPUT);
  //pinMode(rgbGreenPin, OUTPUT);
  //pinMode(rgbBluePin, OUTPUT);

  pinMode(pcbRedPin, OUTPUT);
  pinMode(pcbYellowPin, OUTPUT);
  pinMode(pcbGreenPin, OUTPUT);

  // Set the analog reference to 1v5 as wel 
  //analogReference(AR_INTERNAL1V65);

  // TODO: Analog in for battery voltage.

  // begin initialization
  
  if (!BLE.begin()) {
    Serial.println("starting BLE failed!");
    digitalWrite(pcbRedPin, HIGH);

    while (1);
  } else {
 
      /* Set a local name for the BLE device
       This name will appear in advertising packets
       and can be used by remote devices to identify this BLE device
       The name can be changed but maybe be truncated based on space left in advertisement packet
    */
    
    BLE.setLocalName("environmentMonitor");
    
    BLE.setAdvertisedService(batteryService); // add the service UUID
    batteryService.addCharacteristic(batteryLevelChar); // add the battery level characteristic
    BLE.addService(batteryService); // Add the battery service
    
    batteryLevelChar.writeValue(oldBatteryLevel); // set initial value for this characteristic


    BLE.setAdvertisedService(envService);
    envService.addCharacteristic(bleTemperatureC); 
    envService.addCharacteristic(bleHumidityPC); 
    BLE.addService(envService); 

    bleTemperatureC.writeValue(1*100.0);
    bleHumidityPC.writeValue(1*100.0);
  
    // Start advertising BLE.  It will start continuously transmitting BLE
    // advertising packets and will be visible to remote BLE central devices
    // until it receives a new connection 
  
    // start advertising
    BLE.advertise();
  
    Serial.println("Bluetooth device active, waiting for connections...");
  }
  
  
  // configure the audio data receive callback
  //PDM.onReceive(onPDMdata);

  // optionally set the gain, defaults to 20
  // PDM.setGain(30);

  // initialize PDM with:
  // - one channel (mono mode)
  // - a 16 kHz sample rate
  /*
  if (!PDM.begin(1, 16000)) {
    Serial.println("Failed to start PDM!");
    while (1);
  }
  */


  // Temperature / Humidity.


  if (!HTS.begin()) {
    digitalWrite(pcbRedPin, HIGH);
    Serial.println("Failed to initialize humidity temperature sensor!");
    while (1);
  }

/*
  if (!BARO.begin()) {
    Serial.println("Failed to initialize pressure sensor!");
    while (1);
  }

  if (!APDS.begin()) {
    Serial.println("Error initializing APDS9960 sensor!");
  }

  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }
  */
  digitalWrite(LED_BUILTIN, LOW);
}

float temperature;
float humidity;
float pressure;

void checkTemperature() {
  temperature = HTS.readTemperature();
  humidity    = HTS.readHumidity();
  pressure = BARO.readPressure();

  Serial.print("Temperature = ");
  Serial.print(temperature);
  Serial.println(" °C");

  // Don't use for low power mode.
  if (temperature < 25 && temperature > 18) {
    digitalWrite(pcbRedPin, LOW);
    digitalWrite(pcbYellowPin, LOW);
    digitalWrite(pcbGreenPin, HIGH);

    //digitalWrite(rgbRedPin, LOW);
    //digitalWrite(rgbGreenPin, LOW);
    //digitalWrite(rgbBluePin, HIGH);
  } else   {
    digitalWrite(pcbRedPin, LOW);
    digitalWrite(pcbYellowPin, HIGH);
    digitalWrite(pcbGreenPin, LOW);

    //digitalWrite(rgbRedPin, HIGH);
    //digitalWrite(rgbGreenPin, LOW);
    //digitalWrite(rgbBluePin, LOW);
  }

  Serial.print("Humidity    = ");
  Serial.print(humidity);
  Serial.println(" %");

  Serial.print("Pressure = ");
  Serial.print(pressure);
  Serial.println(" kPa");
}

void checkLightLevels() {
  while (! APDS.colorAvailable()) {
    delay(5);
  }
  
  int r, g, b;
  APDS.readColor(r, g, b);
  
  Serial.print("Colour:  ");
  Serial.print('\t');
  Serial.print("r = ");
  Serial.print(r);
  Serial.print('\t');
  Serial.print("g = ");
  Serial.print(g);
  Serial.print('\t');
  Serial.print("b = ");
  Serial.print(b);
  Serial.println();
}

float x, y, z;
 
void checkAcceleration() {
 

  if (IMU.accelerationAvailable()) {
        IMU.readAcceleration(x, y, z);

        Serial.print("Accel: ");
        Serial.print('\t');
        
        Serial.print(x);
        Serial.print('\t');
        Serial.print(y);
        Serial.print('\t');
        Serial.println(z);
  }

}

void checkGyro() {
  if (IMU.gyroscopeAvailable()) {
        IMU.readGyroscope(x, y, z);

        Serial.print("Gyro: ");
        Serial.print('\t');
        Serial.print(x);
        Serial.print('\t');
        Serial.print(y);
        Serial.print('\t');
        Serial.println(z);
  }
}

void checkMagneticField() {

  if (IMU.magneticFieldAvailable()) {
        IMU.readMagneticField(x, y, z);

        Serial.print("Magnetic Field: ");
        Serial.print('\t');
        Serial.print(x);
        Serial.print('\t');
        Serial.print(y);
        Serial.print('\t');
        Serial.println(z);
  }
}

void loop() {

  // wait for a BLE central
  BLEDevice central = BLE.central();
  handleBLE(central);
  
  //readAudioData();
  checkTemperature();   
  //checkLightLevels();
  //checkAcceleration();
  //checkGyro();
  //checkMagneticField();

  // TODO: Low power please..
  delay (5000);
}

void readAudioData() {
    // wait for samples to be read
  if (samplesRead) {

    // print samples to the serial monitor or plotter
    for (int i = 0; i < samplesRead; i++) {
      Serial.println(sampleBuffer[i]);
    }

    // clear the read count
    samplesRead = 0;
  }
}

void handleBLE(BLEDevice central) {
  // if a central is connected to the peripheral:
  if (central) {
    Serial.print("Connected to central: ");
    // print the central's BT address:
    Serial.println(central.address());
    // turn on the LED to indicate the connection:
    digitalWrite(LED_BUILTIN, HIGH);

    // check the battery level every 200ms
    // while the central is connected:
    while (central.connected()) {
      long currentMillis = millis();
      // if 1000ms have passed, check the battery level:
      if (currentMillis - previousMillis >= 1000) {
        previousMillis = currentMillis;
        updateBatteryLevel();
      }
    }
    // when the central disconnects, turn off the LED:
    digitalWrite(LED_BUILTIN, LOW);
    Serial.print("Disconnected from central: ");
    Serial.println(central.address());
  }
}

// TODO: Convert to sound level
void onPDMdata() {
  // query the number of bytes available
  //int bytesAvailable = PDM.available();

  // read into the sample buffer
  //PDM.read(sampleBuffer, bytesAvailable);

  // 16-bit, 2 bytes per sample
  //samplesRead = bytesAvailable / 2;
}


void updateBatteryLevel() {
  /* Read the current voltage level on the A0 analog input pin.
     This is used here to simulate the charge level of a battery.
  */
  int battery = analogRead(A0);
  int batteryLevel = map(battery, 0, 1023, 0, 100);

  if (batteryLevel != oldBatteryLevel) {      // if the battery level has changed
    Serial.print("Battery Level % is now: "); // print it
    Serial.println(batteryLevel);
    batteryLevelChar.writeValue(batteryLevel);  // and update the battery level characteristic
    oldBatteryLevel = batteryLevel;           // save the level for next comparison
  }

   short newTemperatureC = (short) (temperature * 100.0);
   bleTemperatureC.setValue(newTemperatureC);
   
   unsigned short newHumidity = (unsigned short) (humidity * 100.0);
   bleHumidityPC.setValue(newHumidity);
}
