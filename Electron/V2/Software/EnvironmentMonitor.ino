// This #include statement was automatically added by the Particle IDE.
#include "HTU21D/HTU21D.h"

// This #include statement was automatically added by the Particle IDE.
#include "SFE_BMP180.h"

// Temperature and humidity
HTU21D humiditySensor;

// Pressure sensor. See: https://github.com/sparkfun/BMP180_Breakout_Arduino_Library
// Code to read the BMP180 is from the Sparkfun.com example.
SFE_BMP180 pressure;
bool hasPressureSensor = true;

// TEMT6000 analog input.
int lightSensorPin = A1;

// Output LED pin (onboard LED)
int ledPin = D7;

// Measured values.
int lightLevel = 0;
double humidity = 0;
double temperature = 0;
float measuredPressure = 0;
float relativePressure = 0;

// Set your altitude here.
// Cambridge, UK is between 6 and 20 meters above sea-level
float altitude = 13.0;

void setup() {
    pinMode(lightSensorPin, INPUT);
    pinMode(ledPin, OUTPUT);

    if (!humiditySensor.begin()) {
        // Publish a status message to indicate a fault. 
        // This can be picked up by Tinamous.com and will show on the timeline.
        Particle.publish("status", "Humidity Sensor setup failed.");
    }
    
    if (!pressure.begin()) {
        Particle.publish("status", "Could not find a valid BMP180 sensor.");
        hasPressureSensor = false;
    }

    // Publish a status message to say we're online and the software version.
    Particle.publish("Status", "Environment Sensor Online. V0.4.0");
}

void loop() {
    // Indicate we are measuring.
    digitalWrite(ledPin, HIGH);
    
    readAndPublishMeasurements();
    
    digitalWrite(ledPin, LOW);
    
    sleep();
}

void sleep() {
    // Sleep for 10 seconds before measuring again.
    delay(10000); 
}

// Publish the measured values as SENML format
// so that Tinamous knows how to parse it and 
// record the data.
void readAndPublishMeasurements() {
    String senml;
    senml.reserve(250);
    
    lightLevel = analogRead(lightSensorPin);
    senml = "{'n':'Light','v':'" + String(lightLevel) + "'}";
    
    // a value > 100 indicates a fault.
    humidity = humiditySensor.readHumidity();
    if (humidity < 100) {
        senml += ",{'n':'Humidity','v':'" + String(humidity) + "'}";
    }
    
    // a value > 100 indicates a fault.
    temperature = humiditySensor.readTemperature();
    if (temperature < 100) {
        senml += ",{'n':'Temperature','v':'" + String(temperature) + "'}";
    }
    
    if (hasPressureSensor) {
        measuredPressure = readPressure();
        if (measuredPressure > 0) {
            senml += ",{'n':'Pressure','v':'" + String(measuredPressure) + "'}";
        }
        
        relativePressure = getRelativePressure(measuredPressure);
        if (relativePressure > 0) {
            senml += ",{'n':'RelativePressure','v':'" + String(relativePressure) + "'}";
        }
    }
    
    Particle.publish("senml", "{e:[ " + senml + "]}", 600, PRIVATE);
    // Small delay to ensure the measurements get sent before 
    // possible sleep.
    delay(1000);
}


double readPressure() {

    char status;

    status = pressure.startTemperature();
    if (status == 0)
    {
      Particle.publish("status", "error starting temperature measurement");
      return 0;
    }
  
    // Wait for the measurement to complete:
    delay(status);

    // Retrieve the completed temperature measurement:
    // Note that the measurement is stored in the variable T.
    // Function returns 1 if successful, 0 if failure.

    double T;
    status = pressure.getTemperature(T);
    if (status == 0)
    {
        Particle.publish("status", "error retrieving temperature measurement");
        return 0;
    }
    
     // Start a pressure measurement:
     // The parameter is the oversampling setting, from 0 to 3 (highest res, longest wait).
     // If request is successful, the number of ms to wait is returned.
     // If request is unsuccessful, 0 is returned.

    status = pressure.startPressure(3);
    if (status == 0)
    {
        Particle.publish("status", "error starting pressure measurement");
        return 0;
    }
    
    // Wait for the measurement to complete:
    delay(status);

    // Retrieve the completed pressure measurement:
    // Note that the measurement is stored in the variable P.
    // Note also that the function requires the previous temperature measurement (T).
    // (If temperature is stable, you can do one temperature measurement for a number of pressure measurements.)
    // Function returns 1 if successful, 0 if failure.

    double P;
    status = pressure.getPressure(P,T);
    if (status == 0)
    {
        Particle.publish("status", "error retrieving pressure measurement");
        return 0;
    }
          
    // return absolute pressure.
    return P;
}

double getRelativePressure(double P) {
    // The pressure sensor returns abolute pressure, which varies with altitude.
    // To remove the effects of altitude, use the sealevel function and your current altitude.
    // This number is commonly used in weather reports.
    // Parameters: P = absolute pressure in mb, ALTITUDE = current altitude in m.
    // Result: p0 = sea-level compensated pressure in mb
    
    // Store relative pressure in global variable.
    return pressure.sealevel(P, altitude);
}