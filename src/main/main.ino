

#include "HomeSpan.h" 
#include "DEV_LED.h"          // NEW! Include this new file, DEV_LED.h, which will be fully explained below

void setup() {

  // First light! Control an LED from HomeKit!
  
  // Example 5 expands on Example 2 by adding in the code needed to actually control LEDs connected to the ESP32 from HomeKit.
  // In Example 2 we built out all the functionality to create a "Tile" Acessories inside HomeKit that displayed an on/off light, but
  // these control did not actually operate anything on the ESP32.  To operate actual devices HomeSpan needs to be programmed to
  // respond to "update" requests from HomeKit by performing some form of operation.
  
  // Though HomeKit itself sends "update" requests to individual Characteristics, this is not intuitive and leads to complex coding requirements
  // when a Service has more than one Characteristic, such as both "On" and "Brightness."  To make this MUCH easier for the user, HomeSpan
  // uses a framework in which Services are updated instead of individual Characteristics.  It does so by calling the update() method of
  // each Service with flags indicating all the Characteristics in that Service that HomeKit requested to update.  The user simply
  // implements code to perform the actual operation, and returns either true or false if the update was successful.  HomeSpan takes care of all
  // the underlying nuts and bolts.

  // Every Service defined in HomeKit, such as Service:LightBulb and Service:Fan (and even Service::AccessoryInformation) implements an update()
  // method that, as a default, does nothing but returns a value of true.  To actually operate real devices you need to over-ride this default update()
  // method with your own code.  The easiest way to do this is by creating a DERIVED class based on one of the built-in HomeSpan Services.
  // Within this derived class you can perform initial set-up routines (if needed), over-ride the update() method with your own code, and even create
  // any other methods or class-specific variables you need to fully operate complex devices.  Most importantly, the derived class can take arguments
  // so that you can make them more generic, re-use them multiple times (as will be seen below), and convert them to standalone modules (also shown below).

  // All of the HomeKit Services implemented by HomeSpan can be found in the Services.h file.  Any can be used as the parent for a derived Service.

  // We begin by repeating nearly the same code from Example 2, but with a few key changes. For ease of reading, all prior comments have been removed
  // from lines that simply repeat Example 2, and new comments have been added to explictly show the new code.

  Serial.begin(115200);

  homeSpan.begin(Category::Lighting,"RF Fireplace Control");
  
  new SpanAccessory(); 
  
    new Service::AccessoryInformation(); 
      new Characteristic::Identify();                
      new Characteristic::Manufacturer("Rob");   // Manufacturer of the Accessory (arbitrary text string, and can be the same for every Accessory)
//      new Characteristic::SerialNumber("123-ABC");    // Serial Number of the Accessory (arbitrary text string, and can be the same for every Accessory)
      //new Characteristic::Model("120-Volt Lamp");     // Model of the Accessory (arbitrary text string, and can be the same for every Accessory)
/     new Characteristic::FirmwareRevision("0.1");    // Firmware of the Accessory (arbitrary text string, and can be the same for every Accessory)

  new Service::LightBulb();
  new Characteristic::On();    
  
  new DEV_LED(16);        // this instantiates a new LED Service.  Where is this defined?  What happpened to Characteristic::On?  Keep reading...


void loop(){
  
  homeSpan.poll();
  
} // end of loop()



struct DEV_LED : Service::LightBulb {               // First we create a derived class from the HomeSpan LightBulb Service

  int ledPin;                                       // this variable stores the pin number defined for this LED
  SpanCharacteristic *power;                        // here we create a generic pointer to a SpanCharacteristic named "power" that we will use below

  // Next we define the constructor for DEV_LED.  Note that it takes one argument, ledPin,
  // which specifies the pin to which the LED is attached.
  
  DEV_LED(int ledPin) : Service::LightBulb(){
    

    power=new Characteristic::On();                 // this is where we create the On Characterstic we had previously defined in setup().  Save this in the pointer created above, for use below
    this->ledPin=ledPin;                            // don't forget to store ledPin...
    pinMode(ledPin,OUTPUT);                         // ...and set the mode for ledPin to be an OUTPUT (standard Arduino function)
    
  } // end constructor

  // Finally, we over-ride the default update() method with instructions that actually turn on/off the LED.  Note update() returns type boolean

  boolean update(){            

    digitalWrite(ledPin,power->getNewVal());        // use a standard Arduino function to turn on/off ledPin based on the return of a call to power->getNewVal() (see below for more info)
   
    return(true);                                   // return true to indicate the update was successful (otherwise create code to return false if some reason you could not turn on the LED)
  
  } // update
};




 ********************************************************************************/

 
/////////////////////////////////////////////////////////
//                                                     //
//   HomeSpan Reference Sketch: Air Purifier Service   //
//                                                     //
/////////////////////////////////////////////////////////

// As of iOS 17.2.1, the Home App provides the following Air Purifier functionality:
// 
//  * a Master Switch allows you to set the Active Characteristic to either ACTIVE or INACTIVE
//  * a duplicate of this same functionality is available via a toggle button labeled "Mode" on the 
//    Settings Screen that also allows you to set the Active Characteristic to either ACTIVE or INACTIVE
//  * two selector buttons on the Settings Screen allow you to set the TargetAirPurifierState Characteristic
//    to either AUTO or MANUAL
//
//    NOTE: When changing the state of the Accessory from INACTIVE to ACTIVE via either the Master Switch or
//    the duplicate "Mode" toggle on the Settings Screen, the Home App automatically sets TargetAirPurifierState
//    to AUTO.  If you want to operate in MANUAL mode, you must select that option *after* first setting
//    the Accessory to ACTIVE.  In other words, the Home App always "starts" the Purifier in AUTO mode
//
 
#include "HomeSpan.h"
#include "RF_Signals.h"
#define RF_TX_PIN 21
const int repeat_count=5;
const int tick_len=100;


struct DEV_FIREPLACE : Service::Switch {
  //Characteristic::On power; // This is in some versions, probably old?
  SpanCharacteristic *power;                        // here we create a generic pointer to a SpanCharacteristic named "power" that we will use below
  DEV_FIREPLACE() : Service::Switch(){
    RFControl rf(RF_TX_PIN,true);  // create an instance of RFControl, use default 1 ms clock (the bool true)
    rf.clear();                     // clear the pulse train memory buffer
    power=new Characteristic::On();
  }
  boolean update() override {
    if power->getNewVal<bool>() !=power->getVal<bool> {
      if power->getNewVal<bool>() {
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
  homeSpan.setLogLevel(2);
  homeSpan.begin(Category::Lighting,"Fireplace");
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
