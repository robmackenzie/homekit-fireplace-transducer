#include "HomeSpan.h"
#include "RF_Signals.h"
#define RF_TX_PIN 3
#define STATUS_LED_PIN 8
#define BUTTON_PIN 9
const int repeat_count=5;
const int tick_len=100;

struct DEV_FIREPLACE : Service::Switch {
  SpanCharacteristic *power; // here we create a generic pointer to a SpanCharacteristic named "power" that we will use below
  RFControl rf;  // Declare rf

  DEV_FIREPLACE() : Service::Switch(), rf(RF_TX_PIN, true) {
    rf.clear();  // clear the pulse train memory buffer
    power=new Characteristic::On();
  }
  boolean update() override {
    if (power->getNewVal<bool>() !=power->getVal<bool>()) {
      if (power->getNewVal<bool>()) {
        rf.start(onCommand,onCommandLength,repeat_count,tick_len);
      }
      else {
        rf.start(offCommand,offCommandLength,repeat_count,tick_len);
      }
    }
    return(true);
  }
};

void setup() {
  Serial.begin(115200);
  homeSpan.setStatusPin(STATUS_LED_PIN);
  homeSpan.setControlPin(BUTTON_PIN);
  homeSpan.setLogLevel(2);
  homeSpan.begin(Category::Lighting,"RF Fireplace Control");
  new SpanAccessory();
    new Service::AccessoryInformation();
      new Characteristic::Identify();
      //new Characteristic::Name("Fireplace");              
      new Characteristic::Manufacturer("Rob");   // Manufacturer of the Accessory (arbitrary text string, and can be the same for every Accessory)
      //new Characteristic::SerialNumber("123-ABC");    // Serial Number of the Accessory (arbitrary text string, and can be the same for every Accessory)
      //new Characteristic::Model("120-Volt Lamp");     // Model of the Accessory (arbitrary text string, and can be the same for every Accessory)
      new Characteristic::FirmwareRevision("0.1");    // Firmware of the Accessory (arbitrary text string, and can be the same for every Accessory)       
    new DEV_FIREPLACE();
}
void loop(){ 
  homeSpan.poll();  
}