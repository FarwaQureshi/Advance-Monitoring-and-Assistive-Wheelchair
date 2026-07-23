#include "MAX30100_PulseOximeter.h"
#include <Wire.h>
#include <SoftwareSerial.h>

SoftwareSerial arduino(D7,D6);
 
#define REPORTING_PERIOD_MS     1000
uint32_t tsLastReport = 0;
 
PulseOximeter pox;

float spo2 = 0.00;
float heartrate = 0.00;

void onBeatDetected()
{
    Serial.println("Beat!"); 
}
 
void setup()
{
    Serial.begin(115200);
    arduino.begin(9600);
    Serial.print("Initializing pulse oximeter…");
 
    // Initialize the PulseOximeter instance
    // Failures are generally due to an improper I2C wiring, missing power supply
    // or wrong target chip
    if (!pox.begin()) {
        Serial.println("FAILED");
        for(;;);
    } else {
        Serial.println("SUCCESS");
    }
     pox.setIRLedCurrent(MAX30100_LED_CURR_30_6MA);
 
    // Register a callback for the beat detection
    pox.setOnBeatDetectedCallback(onBeatDetected);

}
 
void loop(){
    // Make sure to call update as fast as possible
    pox.update();
    
    if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
        heartrate = pox.getHeartRate();
        spo2 = pox.getSpO2();
        Serial.print("Heart rate:");
        Serial.print(heartrate);
        Serial.print("bpm / SpO2:");
        Serial.print(spo2);
        Serial.println("%");

        String maxstream = String(heartrate)+";"+String(spo2);
        arduino.println(maxstream);
 
        tsLastReport = millis(); 

    }      
}