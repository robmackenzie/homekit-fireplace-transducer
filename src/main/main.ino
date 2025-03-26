#include "HomeSpan.h"             // include the HomeSpan library
#include "signals.h"
#define RF_TX_PIN 21
const int tick_len=100;
void setup() {    
    Serial.begin(115200);           // start the Serial interface
    Serial.flush();
    delay(100);                    // wait for interface to flush
    Serial.print("\n\nHomeSpan RF Transmitter Example\n\n");
    RFControl rf(RF_TX_PIN,true);  // create an instance of RFControl, use default 1 ms clock
    rf.clear();                     // clear the pulse train memory buffer
    Serial.println("Sending On command");
    rf.start(onCommand,onCommandLength,5,tick_len);
    Serial.println("Done On command");
    delay(5000); // Sleep 5 seconds
    Serial.println("Sending Off command");
    rf.start(offCommand,offCommandLength,5,tick_len);
    Serial.println("Done On command");
    Serial.println("Done!");
}
void loop(){

}
