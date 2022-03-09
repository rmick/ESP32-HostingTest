
unsigned long   lastHostTime            = millis();
unsigned long   assignPlayerSentTime    = millis();
unsigned long   ackFailedSentTime       = millis();



//constants - value is P data

const uint8_t   cTaggerRequestToJoin        = 16;
const uint8_t   cHostAssignPlayer           = 01;
const uint8_t   cTaggerAckPlayerAssign      = 17;
const uint8_t   cAckFailed                  = 15;
//const uint8_t   cHostLtarHostAnnounce       = 129;
const uint8_t   cTaggerLtarRequestToJoin    = 130;
//const uint8_t   cHostLtarAssignPlayer       = 131;
const uint8_t   cTaggerLtarAckPlayerAssign  = 132;
const uint8_t   cHostLtarRelease            = 135;
const uint8_t   cHostLtarPlayerDone         = 244;

//constants - value is just a number
const uint8_t   cHostAnnounce               = 255; //02, 03, 04, 05, 06, 07, 08, 09, 10, 11, 12, 13, 14, 129-Ltar
const uint8_t   cIdle                       = 00;
const uint8_t   cHostPlayerDone             = 222;
const uint8_t   cAnnounceGame               = 200;
const uint8_t   cAssignPlayerSent           = 233;

//stateMachine
uint8_t         stateMachine                = cAnnounceGame;


//variables
int             taggerHostedID      = 0;
uint8_t         teamToBeHosted      = 0;
uint8_t         playerToBeHosted    = 3;

uint8_t         gameID              = 42;
uint8_t         taggerID            = 0;
uint8_t         flags               = 0;

const uint8_t   cMinCodeLength      = 2;
uint8_t         codeLength          = 0;

const uint8_t   cLttoMode           = 0;
const uint8_t   cLtarMode           = 1;
const uint16_t  cHostInterval       = 750;
bool            ltarHostMode        = false;
bool            firstTimeAnnounceNewPlayer = true;

unsigned int IRdataRx[500];              //holding IR byte array (data is ms)


#define PACKET                  'P'
#define DATA                    'D'
#define CHECKSUM                'C'
#define TAG                     'T'
#define BEACON                  'Z'
#define LTAR_BEACON             'E'
#define IS_LTAR                 true

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

int8_t hostGame(bool _cLTARmode, int _teamNum, int _playerNum, int _cancelButtonPin)    //returns TaggerID
{
    taggerHostedID      = 0;
    teamToBeHosted      = _teamNum;
    playerToBeHosted    = _playerNum;
    ltarHostMode        = _cLTARmode;
    
    while (taggerHostedID == 0)
    {
        sendHostMessage();
        getJoinerMessage();
        if(_cancelButtonPin == true)
        {
            Serial.println("Cancelling...");
            taggerHostedID = -99;  
        }

    }
    stateMachine = cAnnounceGame;
    return taggerHostedID;
}

/////////////////////////////////////////////////////////////////////////////

bool checkCancel()
{
    
// TBD 
   
}

/////////////////////////////////////////////////////////////////////////////

void sendHostMessage()
{
    switch (stateMachine)
    {
        case cAnnounceGame:
            digitalWrite(cBlueLed,  HIGH);
            digitalWrite(cRedLed,    LOW);
            digitalWrite(cGreenLed,  LOW);
            announceGame();
            break;

        case cHostAssignPlayer:
            digitalWrite(cBlueLed,  HIGH);
            digitalWrite(cRedLed,   HIGH);
            digitalWrite(cGreenLed,  LOW);
            assignPlayer();
            break;

        case cAssignPlayerSent:
            digitalWrite(cBlueLed,  HIGH);
            digitalWrite(cRedLed,   HIGH);
            digitalWrite(cGreenLed, HIGH);
            checkAckPlayerTimeOut();
            break;

        case cHostLtarRelease:
            releaseLtar();
            break;
        
        case cHostPlayerDone:
            if(ltarHostMode)    stateMachine = cHostLtarRelease;
            else 
            {
                    Serial.println  ("HostPlayerDone");
                    Serial.print    ("\ttaggerID = ");
                    Serial.println  (taggerID);
                    Serial.print    ("taggerHostedID = " + String(taggerHostedID));
                taggerHostedID = taggerID;
                    Serial.print    (" == ");
                    Serial.println  (taggerHostedID);
            }
             digitalWrite(cBlueLed,  LOW);
            digitalWrite(cRedLed,    LOW);
            digitalWrite(cGreenLed,  LOW);
            break;

        case cHostLtarPlayerDone:
            taggerHostedID = taggerID;
            break;

        case cAckFailed:
            sendAckFailedMessage();
            digitalWrite(cBlueLed,   LOW);
            digitalWrite(cRedLed,   HIGH);
            digitalWrite(cGreenLed,  LOW);
            break;

        case cIdle:
            //Do nothing - we are receiving a valid reply.
             digitalWrite(cBlueLed,  LOW);
            digitalWrite(cRedLed,    LOW);
            digitalWrite(cGreenLed,  LOW);
            break;
    }
}

/////////////////////////////////////////////////////////////////////////////

void getJoinerMessage()
{
    if(getIR())
    {
        switch(irRx.readMessageType())
        {
            case PACKET:
            {
                int _irMessageData = irRx.readRawDataPacket();
                switch(_irMessageData)
                {
                    case cTaggerRequestToJoin:
                    case cTaggerLtarRequestToJoin:
                        stateMachine = cTaggerRequestToJoin;
                        break;
                    
                    case cTaggerAckPlayerAssign:
                    case cTaggerLtarAckPlayerAssign:
                        stateMachine = cTaggerAckPlayerAssign;
                        break;
                }
                break;
            }

            case DATA:
            case CHECKSUM:
                switch(stateMachine)
                {
                    case cTaggerRequestToJoin:
                        processRTJ();
                        break;

                    case cTaggerAckPlayerAssign:
                         processAPA();
                         break;
                }
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//  Announce methods
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void announceGame()
{
    //check timer and send Announce game message
    if( (millis() - lastHostTime) > cHostInterval)
    {
        Serial.print("\tstate = cAnnounceGame. Announcing Game: - \tTeam:"); Serial.print(teamToBeHosted); 
        Serial.print(",  Player:"); Serial.println(playerToBeHosted);   
        lastHostTime = millis();
        
        if(ltarHostMode)    irTx.hostPlayerToGame(teamToBeHosted, playerToBeHosted, 129, gameID,  10,  25, 100,  15,  20,  32,   1, 1);
        else                irTx.hostPlayerToGame(teamToBeHosted, playerToBeHosted,   2, gameID,  10,  25, 100,  15,  20,  32,   1);                     
    }
}

/////////////////////////////////////////////////////////////////////////////

void assignPlayer()
{
    Serial.print("\tstate = cHostAssignPlayer. PlayerID = "); Serial.println(playerToBeHosted);
    if(ltarHostMode)        irTx.assignPlayer(gameID, taggerID, teamToBeHosted, playerToBeHosted, IS_LTAR);
    else                    irTx.assignPlayer(gameID, taggerID, teamToBeHosted, playerToBeHosted);
    assignPlayerSentTime = millis();
    stateMachine = cAssignPlayerSent;
}

/////////////////////////////////////////////////////////////////////////////

void checkAckPlayerTimeOut()
{   
    if( (millis() - assignPlayerSentTime) > 1500)
    {
        Serial.print("No 'Joined' Response from Tagger");
        stateMachine = cAckFailed;
    }
}

/////////////////////////////////////////////////////////////////////////////

uint8_t failedAckCount = 0;

void sendAckFailedMessage()
{
    Serial.println("\tsendAckFailedMessage");

    while(failedAckCount <= 5)
    {
        if( (millis() - ackFailedSentTime) > 500)
        {
            Serial.print("sending AckFailed Message # ");Serial.println(failedAckCount++);
            if(ltarHostMode)    irTx.assignPlayerFailed(gameID, taggerID, IS_LTAR);
            else                irTx.assignPlayerFailed(gameID, taggerID);
        }
    }
    stateMachine = cHostAnnounce;
}

/////////////////////////////////////////////////////////////////////////////

void releaseLtar()
{
    Serial.println("\treleaseLtarMessage");
    irTx.ltarAssignPlayerSuccess(gameID, teamToBeHosted, playerToBeHosted);
    stateMachine = cHostLtarPlayerDone;
    
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//  Response methods
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

bool getIR()
{
    bool _returnValue = false;
    codeLength = 0;
    digitalWrite(cBlueLed, !digitalRead(cBlueLed));    
    codeLength = irRx.readIR(IRdataRx,sizeof(IRdataRx));
    if (codeLength > cMinCodeLength) _returnValue = true;
    return _returnValue;
}

/////////////////////////////////////////////////////////////////////////////

void processRTJ()
{        
    Serial.println("\trequestToJoin");
    static int byteCount = 1;
    switch(byteCount++)
    {
        case 1:     //GameID
            Serial.println("-------Player Responded -------");
            gameID = irRx.readRawDataPacket();
            Serial.print("\t\tRTJ - GameID = ");Serial.println(gameID); 
            break;
        case 2:     //TaggerID
            taggerID = irRx.readRawDataPacket();
            Serial.print("\t\tRTJ - TaggerID = ");Serial.println(taggerID);
            break;
        case 3:     //Flags
            flags = irRx.readRawDataPacket();
            Serial.print("\t\tRTJ - Flags = ");Serial.println(flags);
            break;
        case 4:     //Checksum or Ltar SmartDeviceInfo
            if(ltarHostMode)
            {
                Serial.print("\t\tRTJ - Smartdevice Info = ");Serial.println(irRx.readRawDataPacket() );
            }
            else
            {
                Serial.print("\t\tRTJ - Checksum = ");Serial.println(irRx.readRawDataPacket() );Serial.println("\t\t-----------------");
                byteCount = 1;
                stateMachine = cHostAssignPlayer;
            }
            break;
        case 5: //LTAR checksum
            Serial.print("\t\tRTJ - Ltar Checksum = ");Serial.println(irRx.readRawDataPacket() );Serial.println("\t\t-----------------");
            byteCount = 1;
            stateMachine = cHostAssignPlayer;
            break;
    }
}

/////////////////////////////////////////////////////////////////////////////

void processAPA()
{
    Serial.println("\tackPlayerAssign");
    static int byteCount = 1;
    switch(byteCount++)
    {
        case 1:     //GameID
            Serial.println("-------Player Ackd Assign -------");
            gameID = irRx.readRawDataPacket();
            Serial.print("\t\tAPA - GameID = ");Serial.println(gameID); 
            break;
        case 2:     //TaggerID
            taggerID = irRx.readRawDataPacket();
            Serial.print("\t\tAPA - TaggerID = ");Serial.println(taggerID);
            break;
        case 3:     //Checksum
            Serial.print("\t\tAPA - Checksum = ");Serial.println(irRx.readRawDataPacket() );Serial.println("\t\t-----------------");
            byteCount = 1;
            stateMachine = cHostPlayerDone;
            break;
    }
}

/////////////////////////////////////////////////////////////////////////////
