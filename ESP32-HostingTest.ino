
 /* Copyright (c) 2018 Richie Mickan. All Rights Reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>. *
 * 
 * This sketch uses the ESP32_IR_LTTO library which utilises the RMT hardware in the ESP32 to send/receive LTTO Lazertag IR codes. 
 * Two instances are created, one as Tx and the other as Rx. 
 * The library and ESP32 hardware allow up to 8 instances to be created in a single sketch (e.g. 2 x Tx and 6 x Rx)
 * which all work simultaneously. This means that any Receivers will see the IR messages sent by the Transmitters, which may cause you problems! 
 */

#include "esp32_IR_Ltto.h"

const uint8_t   cRxPin              = 17;   
const uint8_t   cTxPin              = 12;   // Pin 14 on Proto Combobulator
const uint8_t   cTxChannel          = 0;    // RMT channel to use
const uint8_t   cRxChannel          = 1;    // RMT channel to use (must be different)

const uint8_t   cBlueLed            = 27;
const uint8_t   cRedLed             = 14;   //Pin 12 on Proto Combobulator
const uint8_t   cGreenLed           = 26;
const uint8_t   cButton             = 0;
const uint8_t   cLedPin             = 25;

ESP32_IR        irRx;                       //1st instance
ESP32_IR        irTx;                       //2nd instance

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void setup() {
    Serial.begin(250000);
    delay(100);

    pinMode     (cBlueLed,  OUTPUT);
    digitalWrite(cBlueLed,     LOW);
    pinMode     (cRedLed,   OUTPUT);
    digitalWrite(cRedLed,      LOW);
    pinMode     (cGreenLed, OUTPUT);
    digitalWrite(cGreenLed,    LOW);
    pinMode     (cButton,    INPUT);
    pinMode     (cLedPin,   OUTPUT);
    digitalWrite(cLedPin,      LOW);

    Serial.println("\nConfiguring IR Rx...");
    bool rxPinOk = irRx.ESP32_IRrxPIN(cRxPin, cRxChannel);
    Serial.println("Initializing IR Rx...");
    irRx.initReceive();
    if(rxPinOk) Serial.println("Rx Init complete");
    else        Serial.println("Rx Init failed");

    Serial.println("\nConfiguring IR Tx...");
    bool txPinOk = irTx.ESP32_IRtxPIN(cTxPin,cTxChannel);
    Serial.println("Initializing IR Tx...");
    irTx.initTransmit(); //setup the ESP32 to send IR code
    if(txPinOk) Serial.println("Tx Init complete\n");
    else        Serial.println("Tx Init failed");
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

uint8_t         teamNum             = 0;
uint8_t         playerNumber        = 1;
const bool      cLTARmode           = false;
bool            cancelHosting       = false;


////////////////////////////////////////////////////
void loop() 
{   
    Serial.println("\nMAIN LOOP - Tagger Hosted: "  + (String)hostGame(cLTARmode, teamNum, playerNumber++, cButton) );
    if(playerNumber > 8)
    {
        teamNum ++;
        playerNumber = 1;
    }
    if(teamNum > 3)
    {
        teamNum = 1;
        playerNumber = 1;
    }
}  


////////////////////////////////////////////////////
//bool checkButton(void)    
//{                    
//    bool returnValue = false;
//    
//    if(digitalRead(cButton) != true)             // Button is normally high,
//    {
//        Serial.println("Cancel Hosting");
//        returnValue = true;
//    }
//    
//    return returnValue;
//}
