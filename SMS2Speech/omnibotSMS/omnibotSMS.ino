// For Adafruit FONA
#include <Adafruit_FONA.h>

// disable debugging!
#ifdef ADAFRUIT_FONA_DEBUG
  #undef ADAFRUIT_FONA_DEBUG
#endif

// the Arduino Pins
#define FONA_RX 2
#define FONA_TX 10 // digital 10 for Mega, otherwise digital 3
#define FONA_RST 4

// this is a large buffer for replies
char replybuffer[255];

// We default to using software serial. If you want to use hardware serial
// (because softserial isnt supported) comment out the following three lines 
// and uncomment the HardwareSerial line
#include <SoftwareSerial.h>
SoftwareSerial fonaSS = SoftwareSerial(FONA_TX, FONA_RX);
SoftwareSerial *fonaSerial = &fonaSS;

// Hardware serial is also possible!
//  HardwareSerial *fonaSerial = &Serial1;

// Use this for FONA 800 and 808s
Adafruit_FONA fona = Adafruit_FONA(FONA_RST);

//uint8_t readline(char *buff, uint8_t maxbuff, uint16_t timeout = 0);


// Markus
uint8_t FONAtype;
uint16_t FONAvoltage = 0;
uint8_t FONAnetworkStatus = 0;
bool SIMunlocked = false;
int8_t FONAsmsnum = 0;
int8_t FONAsmsInitialNum = 0;
bool initialSMScounted = false;

// store the state if FONA is okay or not (init okay etc.)
boolean FONAstate = false;

int returnValue = 0;



//
// the "serial" commands for the MC
//

/* Commands for interaction with the GSM FONA module
  gsmi  = init GSM module
  gsmp  = unlock GSM module with PIN
  gsms  = (get) GSM status

  smsc  = count available SMS
  smsl  = read Last SMS
  smsr  = read SMS #
  smss  = send SMS
  smsd  = delete SMS #
*/

// just nice to have
#define ON            1
#define OFF            0

// just nice to have
#define ENABLE          1
#define DISABLE          0


//-------------------------------------------------------------------------------------------------
// string command check stuff
//-------------------------------------------------------------------------------------------------
int starter    = 42; // this marks the beginning of a received string. which is '*' at the moment.
int terminator = 35; // this marks the end of a string. which is '#' at the moment.

// Puffergrösse in Bytes für den Serial port
#define stringSize 32

String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete
String command = "";
//-------------------------------------------------------------------------------------------------


void setup()
{

  //-------------------------------------------------------------------------------------------------
  // string command check stuff
  //-------------------------------------------------------------------------------------------------
  // reserve 200 bytes for the inputString
  inputString.reserve(stringSize);
  command.reserve(stringSize);
  stringComplete = false;  // Flag, String komplett empfangen
  //-------------------------------------------------------------------------------------------------
  
  // initialize serial communication on the USB port
  while (!Serial);

  Serial.begin(115200);
/*
  Serial.println(F("FONA basic test"));
  Serial.println(F("Initializing....(May take 3 seconds)"));
*/
  fonaSerial->begin(4800);
  if (! fona.begin(*fonaSerial))
  {
    Serial.println(F("Couldn't find FONA"));
    while (1);
  }
  
  FONAtype = fona.type();
//  Serial.println(F("FONA is OK"));
//  Serial.print(F("Found "));
  switch (FONAtype)
  {
    case FONA800L:
//      Serial.println(F("FONA 800L")); break; // Markus' module
    case FONA800H:
//      Serial.println(F("FONA 800H")); break;
    case FONA808_V1:
//      Serial.println(F("FONA 808 (v1)")); break;
    case FONA808_V2:
//      Serial.println(F("FONA 808 (v2)")); break;
    case FONA3G_A:
//      Serial.println(F("FONA 3G (American)")); break;
    case FONA3G_E:
//      Serial.println(F("FONA 3G (European)")); break;
    default: 
//      Serial.println(F("???")); break;
        break;
  }

  /*
   * DISABLED
   * 
 
  // Print module IMEI number.
  char imei[15] = {0}; // MUST use a 16 character buffer for IMEI!
  uint8_t imeiLen = fona.getIMEI(imei);
  if (imeiLen > 0)
  {
//    Serial.print("Module IMEI: "); Serial.println(imei);
  }


  // read the battery voltage and percentage  
  if (! fona.getBattVoltage(&FONAvoltage))
  {
//    Serial.println(F("Failed to read Batt"));
  }
  else
  {
//    Serial.print(F("VBat = ")); Serial.print(FONAvoltage); Serial.println(F(" mV"));
  }
  
  if (! fona.getBattPercent(&FONAvoltage))
  {
//    Serial.println(F("Failed to read Batt"));
  }
  else
  {
//    Serial.print(F("VPct = ")); Serial.print(FONAvoltage); Serial.println(F("%"));
  }
*/

  // unlock SIM
  if (SIMunlocked == false)
  {
    // Unlock the SIM with PIN code
    char PIN[5] = { '5', '5', '5', '5', NULL};

//    Serial.print(F("Unlocking SIM card: "));

    if (! fona.unlockSIM(PIN))
    {
      SIMunlocked = false;
//      Serial.println(F("Failed"));
    }
    else
    {
      SIMunlocked = true;
//      Serial.println(F("OK!"));
    }
  }

  do
  {
    Serial.print("*cstart#");
  } while(1);
}


void loop()
{
  static uint8_t string_started = 0;  // Sind wir jetzt im String?

delay(5000);

/*

  do
  {
    // do we have something on the USB port?
    if (Serial.available())
    {
      // get the new byte
      char inChar = (char)Serial.read();

      // max String length reached?
      if (inputString.length() >= stringSize)
      {
        
        // clear string
        inputString = "";

        // delete flags
        stringComplete = false;
        string_started = 0;
      }
      else
      {
        // Ist Puffer frei für neue Daten?
        if (stringComplete == false)
        {
          //-----------------
          // string start
          //-----------------
          // string speichern, wenn mit 'starter' begonnen!
          if (inChar == starter)
          {
            // clear the string
            inputString = "";
            // start inputString:
            inputString += inChar;

            // string has started
            string_started = 1;
          }
          else
          {
            //-----------------
            // string stop
            //-----------------
            // Ist das Ende des Strings (terminator) erreicht?
            if (inChar == terminator)
            {
              // ja, dann terminator anhängen
              inputString += inChar;

              // copy input string to command string (used in loop)
              command = inputString;

              // clear input striing
              inputString = "";

              // if the incoming character is a "terminator", set a flag
              // so //the main loop can do something about it:
              stringComplete = true;

              // reset flag
              string_started = 0;
            }
            else
            {
              //-----------------
              // string middle
              //-----------------
              // string nur speichern, wenn zuvor der starter mal war.
              if  (string_started == 1)
              {
                // Daten in Puffer speichern
                inputString += inChar;
              } // string complete
              else
              {
              } // any string
            }
          }
        } // string complete = false
      }
    } // Serial.available
  } while (stringComplete == false);

/ *
  // print the string when a newline arrives:
  if (stringComplete) 
  {
    Serial.print("stringComplete (loop):"); 

    Serial.print(inputString); 

    Serial.println("<END>"); 

    // clear the string
    inputString = "";

    stringComplete = false;
  }
* /

  // Wurde ein kompletter String empfangen und ist der Buffer ist leer?
  // delete flag
  stringComplete = false;

  //--------------------------
  // check what was received
  //--------------------------

  // RESET / INIT
  if (command == "*re#")
  {
    // answer with "ok"
    // this answer is used to see if the robot is "on"
    Serial.print("*re#");
    // write all data immediately!
    Serial.flush();
  }

*/
} // loop


// FONA: read the number of SMS's
int8_t readNumSMS()
{
  int8_t smsnum = fona.getNumSMS();


  if (smsnum < 0)
  {
    return -1;
  }

  return smsnum; 
}


// FONA: Unlock the SIM with a PIN code
int unlockSIM()
{
  char PIN[5];


  // PIN
  PIN[0] = '5';
  PIN[1] = '5';
  PIN[2] = '5';
  PIN[3] = '5';
  PIN[4] = NULL;

  // unlock
  if (! fona.unlockSIM(PIN))
  {
    // error
    return -1;
  } 
  else
  {
    // ok
    return 0;
  }        
}
