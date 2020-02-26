
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
const int pcbYellowPin = 9; // LHS
const int pcbGreenPin = 6; // Middle
const int pcbRedPin = 5; // RHS

const int LED_A = 2;
const int LED_B = 3;
const int LED_C = A2;
const int LED_D = A1;
const int LED_E = 12;
const int LED_F = 4;
const int LED_G = 13;

const int digit1 = A3;
const int digit2 = A6;
const int digit3 = A7;

const int switch1 = 10;


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
//BLEService batteryService("180F");

// BLE Battery Level Characteristic
// standard 16-bit characteristic UUID
//BLEUnsignedCharCharacteristic batteryLevelChar("2A19",  BLERead | BLENotify);

// https://github.com/bneedhamia/DHTBLESensor/blob/master/DHTBLESensor.ino
//BLEService envService("181A");
//BLEUnsignedShortCharacteristic bleHumidityPC("2A6F", BLERead | BLENotify);
//BLEShortCharacteristic bleTemperatureC("2A6E", BLERead | BLENotify);

int oldBatteryLevel = 0;  // last battery level reading from analog input
long previousMillis = 0;  // last time the battery level was checked, in ms



// buffer to read samples into, each sample is 16-bits
short sampleBuffer[256];

// number of samples read
volatile int samplesRead;

// A, B, C, D, E, F, G
bool segments[][7] = {{true, true, true, true, true, true, false}, // 0
                      {false, true, true, false, false, false, false }, // 1
                      {true, true, false, true, true, false, true}, // 2
                      {true, true, true, true, false, false, true}, // 3
                      {false, true, true, false, false, true, true}, // 4
                      {true, false, true, true, false, true, true}, // 5
                      {true, false, true, true, true, true, true}, // 6
                      {true, true, true, false, false, false, false}, // 7
                      {true, true, true, true, true, true, true}, // 8
                      {true, true, true, true, false, true, true}, // 9
                      {true, true, true, false, true, true, true}, // A
                      {false, false, true, true, true, true, true}, // B
                      {true, false, false, true, true, true, false}, // C
                      {false, true, true, true, true, false, true}, // D
                      {true, false, false, true, true, true, true}, // E
                      {true, false, false, false, true, true, true}, // F
                      {true, true, true, true, true, true, false}, // Q (needs extra segment (C on Digit 3). // 16
                      {false, false, false, false, false, false, true}, // - 17
                      
};

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

  pinMode(LED_A, OUTPUT);
  pinMode(LED_B, OUTPUT);
  pinMode(LED_C, OUTPUT);
  pinMode(LED_D, OUTPUT);
  pinMode(LED_E, OUTPUT);
  pinMode(LED_F, OUTPUT);
  pinMode(LED_G, OUTPUT);

  pinMode(digit1, OUTPUT);
  pinMode(digit2, OUTPUT);
  pinMode(digit3, OUTPUT);

  pinMode(switch1, INPUT_PULLUP);

  

  // Set the analog reference to 1v5 as wel 
  //analogReference(AR_INTERNAL1V65);

  // TODO: Analog in for battery voltage.

  // begin initialization
  /*
  if (!BLE.begin()) {
    Serial.println("starting BLE failed!");
    digitalWrite(pcbRedPin, HIGH);

    while (1);
  } else {
 
    // Set a local name for the BLE device
    // This name will appear in advertising packets
    // and can be used by remote devices to identify this BLE device
    // The name can be changed but maybe be truncated based on space left in advertisement packet
    

    
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
  */
  
  
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
  //digitalWrite(LED_BUILTIN, LOW);
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

   Serial.print("Humidity    = ");
  Serial.print(humidity);
  Serial.println(" %");

  Serial.print("Pressure = ");
  Serial.print(pressure);
  Serial.println(" kPa");
}

/*

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

*/


void showDigit(int value) {

  if (value < 0) {
    digitalWrite(LED_A, LOW);
    digitalWrite(LED_B, LOW);
    digitalWrite(LED_C, LOW);
    digitalWrite(LED_D, LOW);
    digitalWrite(LED_E, LOW);
    digitalWrite(LED_F, LOW);
    digitalWrite(LED_G, LOW);
  } else {
    digitalWrite(LED_A, segments[value][0]);
    digitalWrite(LED_B, segments[value][1]);
    digitalWrite(LED_C, segments[value][2]);
    digitalWrite(LED_D, segments[value][3]);
    digitalWrite(LED_E, segments[value][4]);
    digitalWrite(LED_F, segments[value][5]);
    digitalWrite(LED_G, segments[value][6]);
  }
}

void showNumber(float value, bool temperature, bool humidity, bool airQuality, int delayms) {

  int intValue = 0;
  int digit1value = -1;
  int digit2value = -1;
  bool dp = false;
  bool q = false;

  if (value < -9) {
    // We can't show this.
    digit1value = 17; // -
    digit2value = 17; // -
  } else if (value < 0) {
    // Show negative.
    digit1value = 17; // -ve
    // Cast to integer and make +ve (i.e. -6.1 -> 6)
    digit2value = - ((int)value);
  } else if (value >-0.1 && value < 0.1) {
    // 0.0
    digit1value = 0;
    digit2value = 0;
    dp = true;
  } else if (value < 10) {
    // Single digit so we can show x.x
    intValue = (int) (value * 10); // 3.4 -> 34
    
    digit1value = intValue / 10;
    digit2value = intValue - (digit1value * 10);
    dp = true;
  } else if (value > 99) {
    // out of range
    digit1value = 17;
    digit2value = 17;
  }
  else {
    digit1value = value / 10; // 17 -> 1
    digit2value = value - (digit1value * 10); // (17 -> 7)
  }


  // Now light up the LEDs to show the digit values...
  int digitDelay = 2;  
  for (int i = 0; i< (delayms / digitDelay); i++) {
    
    for (int digit=0; digit<3; digit++) {
      
      // All off whilst we setup the segments.
      enableDigit(0);

      if (digit == 0) {
        showDigit(digit1value);
      } else if (digit == 1) {
        showDigit(digit2value);
      } else {
        // Digit 3.
        digitalWrite(LED_A, false);
        digitalWrite(LED_B, humidity);
        digitalWrite(LED_C, q);
        digitalWrite(LED_D, airQuality);
        digitalWrite(LED_E, dp);
        digitalWrite(LED_F, false);
        digitalWrite(LED_G, temperature);
      }

      enableDigit(digit+1);
      delay(digitDelay);
    }
    
  }
}

void enableDigit(int digit) {
      // Enable the appropriate digit.
      switch (digit) {
        case 1:
          digitalWrite(digit1, HIGH);
          digitalWrite(digit2, LOW);
          digitalWrite(digit3, HIGH);  
          break;
        case 2:
          digitalWrite(digit1, LOW);
          digitalWrite(digit2, HIGH);
          digitalWrite(digit3, HIGH);  
          break;
        case 3:
          digitalWrite(digit1, LOW);
          digitalWrite(digit2, LOW);
          digitalWrite(digit3, LOW);   // digit 3 is active low.
          break;
        default:
          // all off.
          digitalWrite(digit1, LOW);
          digitalWrite(digit2, LOW);
          digitalWrite(digit3, HIGH);  
          break;
      }  
}

int loopCounter = 0;

void loop() {

  // wait for a BLE central
  //BLEDevice central = BLE.central();
  //handleBLE(central);

  digitalWrite(pcbRedPin, LOW);
  digitalWrite(pcbYellowPin, LOW);
  digitalWrite(pcbGreenPin, LOW);
  
  //readAudioData();
  checkTemperature();   
  //checkLightLevels();
  //checkAcceleration();
  //checkGyro();
  //checkMagneticField();

  boolean switchPressed = !digitalRead(switch1);

  if (loopCounter == 0) {
    Serial.println("All Off");
    digitalWrite(digit1, LOW);
    digitalWrite(digit2, LOW);
    digitalWrite(digit3, HIGH); 
    delay(500);
  }
  
  if (loopCounter == 1) {
    Serial.println("Digit 1");
    digitalWrite(digit1, HIGH);
    digitalWrite(digit2, LOW);
    digitalWrite(digit3, HIGH); 
    for (int i=0; i<16; i++) {
      Serial.println(i, DEC);
      showDigit(i);
      delay(300);
    }
  }
  
  if (loopCounter == 2) {
    Serial.println("Digit 2");
    digitalWrite(digit1, LOW);
    digitalWrite(digit2, HIGH);
    digitalWrite(digit3, HIGH); 
    for (int i=0; i<16; i++) {
      Serial.println(i, DEC);
      showDigit(i);
      delay(300);
    }
  }
  
  if (loopCounter == 3) {
    Serial.println("Number range");
    for (float t = -10; t<20; t++) {
      Serial.println(t, DEC);
      showNumber(t, false, false, false, 60);
    }
  }

  if (loopCounter == 4) {
    Serial.println("All Off");
    digitalWrite(digit1, LOW);
    digitalWrite(digit2, LOW);
    digitalWrite(digit3, HIGH); 
    delay(1000);
  }

  if (loopCounter == 5) {
    Serial.println("Temperature");
    showNumber(temperature, true, false, false, 2000);
  }

  if (loopCounter == 6) {
    Serial.println("Humidity");
    showNumber(humidity, false, true, false, 2000);
  }

  if (loopCounter == 7) {
    Serial.println("Air Q");
    showNumber(9.1, false, false, true, 1000);
  }
  
  if (loopCounter == 8) { // HS
    Serial.println("Yellow");
    enableDigit(0);
    digitalWrite(pcbYellowPin, HIGH);
    delay(500);
  }

  if (loopCounter == 9) { // Middle
    Serial.println("Green");
    enableDigit(0);
    digitalWrite(pcbGreenPin, HIGH);
    delay(500);
  }

  if (loopCounter == 10) {
    Serial.println("Red"); //RHS
    enableDigit(0);
    digitalWrite(pcbRedPin, HIGH);
    delay(500);
  }

  if (loopCounter == 11) {
    Serial.println("Red"); //RHS
    enableDigit(0);
    digitalWrite(pcbRedPin, HIGH);
    delay(500);

    for (int i=0; i<30; i++) {
      checkTemperature();  

      digitalWrite(pcbGreenPin, LOW);
      digitalWrite(pcbYellowPin, LOW);
      digitalWrite(pcbRedPin, LOW);

      if (temperature > 29) {
        digitalWrite(pcbGreenPin, HIGH);
        digitalWrite(pcbYellowPin, LOW);
        digitalWrite(pcbRedPin, LOW);  
      }else if (temperature > 27) {
        digitalWrite(pcbGreenPin, LOW);
        digitalWrite(pcbYellowPin, HIGH);
        digitalWrite(pcbRedPin, LOW);  
      } else {
        digitalWrite(pcbGreenPin, LOW);
        digitalWrite(pcbYellowPin, LOW);
        digitalWrite(pcbRedPin, LOW);  
      }
      
      showNumber(temperature, true, false, false, 1000);
      showNumber(humidity, false, true, false, 800);
    }
  }
  
  loopCounter++;
  
  Serial.print("Loop Counter: ");
  Serial.println(loopCounter, DEC);

  if (loopCounter > 12) {
    loopCounter = 0;
  }
}

/*
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
    //digitalWrite(LED_BUILTIN, HIGH);

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
    //digitalWrite(LED_BUILTIN, LOW);
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
  // Read the current voltage level on the A0 analog input pin.
  //   This is used here to simulate the charge level of a battery.
  
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
*/
