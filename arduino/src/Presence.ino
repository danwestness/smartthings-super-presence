//*****************************************************************************
/// @file
/// @brief
///   Arduino SmartThings Shield LED Example with Network Status
/// @note
///              ______________
///             |              |
///             |         SW[] |
///             |[]RST         |
///             |         AREF |--
///             |          GND |--
///             |           13 |--
///             |           12 |--
///             |           11 |--
///           --| 3.3V      10 |--
///           --| 5V         9 |--
///           --| GND        8 |--
///           --| GND          |
///           --| Vin        7 |--X Button: Not Present
///             |            6 |--X Button: Present
///           --| A0         5 |--X
///           --| A1    ( )  4 |--X LED
///           --| A2         3 |--X THING_RX
///           --| A3  ____   2 |--X THING_TX
///           --| A4 |    |  1 |--
///           --| A5 |    |  0 |--
///             |____|    |____|
///                  |____|
///
//*****************************************************************************
#include <SoftwareSerial.h>   //TODO need to set due to some weird wire language linker, should we absorb this whole library into smartthings
#include <SmartThings.h>

//*****************************************************************************
// Pin Definitions    | | | | | | | | | | | | | | | | | | | | | | | | | | | | |
//                    V V V V V V V V V V V V V V V V V V V V V V V V V V V V V
//*****************************************************************************
// #define PIN_LED         13
#define PIN_THING_RX    3
#define PIN_THING_TX    2
#define PIN_LED  4
#define PIN_BTN_ON  6
#define PIN_BTN_OFF  7
#define CRLF "\r\n"

// no-cost stream operator as described at
// http://arduiniana.org/libraries/streaming/
// http://playground.arduino.cc/Main/StreamingOutput
template<class T>
inline Print &operator <<( Print &obj, T arg ) {
    obj.print( arg );
    return obj;
}
//*****************************************************************************
// Global Variables   | | | | | | | | | | | | | | | | | | | | | | | | | | | | |
//                    V V V V V V V V V V V V V V V V V V V V V V V V V V V V V
//*****************************************************************************
SmartThingsCallout_t
messageCallout;    // call out function forward decalaration
SmartThings smartthing( PIN_THING_RX, PIN_THING_TX,
                        messageCallout ); // constructor

bool isDebugEnabled = true;     // enable or disable debug in this example
bool present = false;
int stateNetwork;               // state of the network

//*****************************************************************************
// Local Functions  | | | | | | | | | | | | | | | | | | | | | | | | | | | | | |
//                  V V V V V V V V V V V V V V V V V V V V V V V V V V V V V V
//*****************************************************************************
void on() {
    present = true;                // save state as on
    digitalWrite( PIN_LED, HIGH ); // turn LED on
    // smartthing.shieldSetLED( 0, 0, 2 );
    smartthing.send( "on" );       // send message to cloud
}

//*****************************************************************************
void off() {
    present = false;              // set state to off
    digitalWrite( PIN_LED, LOW ); // turn LED off
    // smartthing.shieldSetLED( 0, 0, 0 );
    smartthing.send( "off" );     // send message to cloud
}

//*****************************************************************************
void poll() {
    // smartthing.shieldSetLED( 0, 0, 0 );
    if( present ) {
        smartthing.send( "on" );     // send message to cloud
    } else {
        smartthing.send( "off" );    // send message to cloud
    }
}


//*****************************************************************************
void setNetworkStateLED() {
    SmartThingsNetworkState_t tempState = smartthing.shieldGetLastNetworkState();

    if ( tempState != stateNetwork ) {
        switch ( tempState ) {
        case STATE_NO_NETWORK:
            if ( isDebugEnabled ) Serial.println( "NO_NETWORK" );
            smartthing.shieldSetLED( 2, 0, 0 ); // red
            break;
        case STATE_JOINING:
            if ( isDebugEnabled ) Serial.println( "JOINING" );
            smartthing.shieldSetLED( 0, 2, 0 ); // green
            break;
        case STATE_JOINED:
            if ( isDebugEnabled ) Serial.println( "JOINED" );
            smartthing.shieldSetLED( 0, 0, 0 ); // off
            break;
        case STATE_JOINED_NOPARENT:
            if ( isDebugEnabled ) Serial.println( "JOINED_NOPARENT" );
            smartthing.shieldSetLED( 0, 2, 2 ); // ??
            break;
        case STATE_LEAVING:
            if ( isDebugEnabled ) Serial.println( "LEAVING" );
            smartthing.shieldSetLED( 2, 0, 0 ); // red
            break;
        default:
        case STATE_UNKNOWN:
            if ( isDebugEnabled ) Serial.println( "UNKNOWN" );
            smartthing.shieldSetLED( 2, 0, 2 ); // purple
            break;
        }
        stateNetwork = tempState;
    }
}

//*****************************************************************************
// API Functions    | | | | | | | | | | | | | | | | | | | | | | | | | | | | | |
//                  V V V V V V V V V V V V V V V V V V V V V V V V V V V V V V
//*****************************************************************************
void setup() {
    // setup hardware pins
    pinMode( PIN_BTN_ON, INPUT );
    pinMode( PIN_BTN_OFF, INPUT );
    pinMode( PIN_LED, OUTPUT );
    digitalWrite( PIN_LED, LOW ); // set value to LOW (off) to match stateLED=0

    present = false;              // matches state of hardware pin set below
    stateNetwork = STATE_JOINED;  // set to joined to keep state off if off

    if( isDebugEnabled ) {
        Serial.begin( 9600 );
        Serial.println( "Setup..." );
    }
    delay( 2000 );
}

void loop() {
    checkPresence();
    smartthing.run();
    setNetworkStateLED();
}

void checkPresence() {
    // Check button states
    int on_btn_val = digitalRead( PIN_BTN_ON );
    int off_btn_val = digitalRead( PIN_BTN_OFF );

    // Set status based on button presses
    if( on_btn_val == HIGH ) {
        if( isDebugEnabled ) Serial.println( "on pressed" );
        on();
    }
    if( off_btn_val == HIGH ) {
        if( isDebugEnabled ) Serial.println( "off pressed" );
        off();
    }
}

void messageCallout( String message ) {
    // if debug is enabled print out the received message
    if( isDebugEnabled ) {
        Serial << "Received message: '" << message << "'" CRLF;
    }

    // Dispatch based on message
    if( message.equals( "on" ) ) {
        on();
    } else if( message.equals( "off" ) ) {
        off();
    } else if( message.equals( "poll" ) ) {
        poll();
    }
}
