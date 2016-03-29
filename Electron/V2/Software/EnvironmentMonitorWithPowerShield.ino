// This #include statement was automatically added by the Particle IDE.
#include "PowerShield/PowerShield.h"

// This #include statement was automatically added by the Particle IDE.
#include "HTU21D/HTU21D.h"

// Temperature and humidity
HTU21D humiditySensor;

// Battery monitor
PowerShield batteryMonitor;

// TEMT6000 analog input.
int lightSensorPin = A1;

// Output LED pin (onboard LED)
int ledPin = D7;

// Ground this pin to allow the 
// photon to go into deep sleep
int keepAwakePin = D6;

// ALERT interrupt from MAX17043 (optional solder bridge)
int batteryAlertPin = D3;

int lightLevel = 0;
double humidity = 0;
double temperature = 0;
double faultCode = 0;
bool hasBatteryPack = false;
float cellVoltage = 0;
float stateOfCharge = 0;


void setup() {
    //pinMode(lightSensorPin, INPUT);
    pinMode(ledPin, OUTPUT);
    pinMode(keepAwakePin, INPUT_PULLUP);
    
    // Take control of the spark LED and make it dimmer as it's very
    // bright by default.
    RGB.control(true);
    RGB.brightness(10);
    
    // Set blue to show initializing.
    RGB.color(0, 0, 255);
    
    if (!humiditySensor.begin()) {
        RGB.color(255, 0, 00);
        Particle.publish("Status", "Humidity Sensor setup failed.");
    }
    
    // Set-up the battery monitor.
    // May not be fitted.
    Wire.begin(); 

    batteryMonitor.reset();
    batteryMonitor.quickStart();
    int batteryMonitorVersion = batteryMonitor.getVersion();
    if (batteryMonitorVersion < 65535) {
        hasBatteryPack = true;
    }
    
    Particle.publish("Status", "Environment 0.2.1");
    Particle.publish("Status", "Battery Monitor Version: " + String(batteryMonitorVersion));
    delay(1000);
    
    RGB.color(0, 255, 0);
}

void loop() {
    digitalWrite(ledPin, HIGH);
    
    lightLevel = analogRead(lightSensorPin);
    readHumidity();
    readTemperature();
    
    if (hasBatteryPack) {
        cellVoltage = batteryMonitor.getVCell();
        stateOfCharge = batteryMonitor.getSoC();
    }
    
    Particle.publish("senml", "{e:[{'n':'ps-voltage','v':'" + String(cellVoltage) + "'},{'n':'ps-soc','v':'" + String(stateOfCharge) + "'},{'n':'Light','v':'" + String(lightLevel) + "'},{'n':'Humidity','v':'" + String(humidity) + "'},{'n':'Temperature','v':'" + String(temperature) + "'} ]}", 60, PRIVATE);
    
    // Delay for publish.
    delay(2000);
    
    digitalWrite(ledPin, LOW);
    
    
    // if keepAwakePin is high then keep the photon
    // awake, otherwise use a deep sleep to save battery
    // Useful when 
    int keepAwake = digitalRead(keepAwakePin);
    if (keepAwake) {
        delay(20000); 
    } else {
        //System.sleep(SLEEP_MODE_DEEP, 60);
        delay(20000); 
    }
}

double readHumidity() {
    double h = humiditySensor.readHumidity();
    if (h < 100) {
        humidity = (double)h;
    } else {
        RGB.color(255, 0, 0);
        faultCode = h;
    }
}

double readTemperature() {
    double t = humiditySensor.readTemperature();
    if (t<100) {
        temperature = (double)t;
    } else {
        RGB.color(255, 0, 0);
        faultCode = t;
    }
}