// For Adafruit FONA
#include <Adafruit_FONA.h>

//-----------------------------
// Disable FONA debugging!
//-----------------------------
#ifdef ADAFRUIT_FONA_DEBUG
  #undef ADAFRUIT_FONA_DEBUG
#endif

// Arduino Pins
#define FONA_RX 2
#define FONA_TX 10 // digital 10 for Mega, otherwise digital 3
#define FONA_RST 4


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


// several FONA data whcih will be stored (for a possible later use)
uint8_t FONAtype;
uint16_t FONAvoltage = 0;
uint8_t FONAnetworkStatus = 0;
bool SIMunlocked = false;
int8_t FONAsmsnum = 0;
int8_t FONAsmsInitialNum = 0;
char FONAimei[15] = {0}; // MUST use a 16 character buffer for IMEI!
bool initialSMScounted = false;
char FONASMSbuffer[255];

//--------------------------------------------------------------------
// Delete SMS after transfer
//--------------------------------------------------------------------
const bool deleteSMSafterTransfer = false;
//--------------------------------------------------------------------




// store the state if FONA is okay or not (init okay etc.)
boolean FONAstate = false;

int returnValue = 0;


// just nice to have
#define ON            1
#define OFF            0

// just nice to have
#define ENABLE          1
#define DISABLE          0


//-------------------------------------------------------------------------------------------------
// string answer check stuff
//-------------------------------------------------------------------------------------------------
int starter    = 42; // this marks the beginning of a received string. which is '*' at the moment.
int terminator = 35; // this marks the end of a string. which is '#' at the moment.

// Puffergrösse in Bytes für den Serial port
#define stringSize 32 // this amount will be reserved later

String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete
String answer = "";
//-------------------------------------------------------------------------------------------------


void setup()
{

  //-------------------------------------------------------------------------------------------------
  // string answer check stuff
  //-------------------------------------------------------------------------------------------------
  // reserve 200 bytes for the inputString
  inputString.reserve(stringSize);
  answer.reserve(stringSize);
  stringComplete = false;  // Flag, String komplett empfangen
  //-------------------------------------------------------------------------------------------------
  
  // initialize serial communication on the USB port
  while (!Serial);
  Serial.begin(115200);

  // initialize FONA
  fonaSerial->begin(4800);
  if (! fona.begin(*fonaSerial))
  {
    Serial.println(F("Couldn't find FONA"));
    while (1);
  }
  
  // detect FONA type
  FONAtype = fona.type();
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
  // read the battery voltage and percentage  
  if (! fona.getBattVoltage(&FONAvoltage))
  {
//    Serial.println(F("Failed to read Batt"));
  }
  else
  {
//    Serial.print(F("VBat = ")); Serial.print(FONAvoltage); Serial.println(F(" mV"));
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


  //------------------------------------------------------
  // send START COMM via USB, until a computer answers
  //------------------------------------------------------
  do
  {
    Serial.print("*cstart#");
    delay(1000);

    // wait for answer on USB port
    waitForAnswer();

//    Serial.print("answer=");
//    Serial.println(answer);
    
  } while(answer != "*cstart#");
}


void loop()
{
  // battery status  *b___#
  if (! fona.getBattPercent(&FONAvoltage))
  {
    Serial.print("*b");
    Serial.print("err");
    Serial.print("#");
  }
  else
  {
    Serial.print("*b");
    Serial.print(FONAvoltage);
    Serial.print("#");
  }

  // IMEI  *i___#
  if (fona.getIMEI(FONAimei) <= 0)
  {
    Serial.print("*i");
    Serial.print("err");
    Serial.print("#");
  }
  else
  {
    Serial.print("*i");
    Serial.print(FONAimei);
    Serial.print("#");
  }

  // network/cellular status  *n___#
  FONAnetworkStatus = fona.getNetworkStatus();
  Serial.print("*n");
  Serial.print(FONAnetworkStatus);
  Serial.print("#");

  // number of SMS  *s___#
  FONAsmsnum = fona.getNumSMS();
  if (FONAsmsnum < 0)
  {
    Serial.print("*s");
    Serial.print("err");
    Serial.print("#");
  }
  else
  {
    Serial.print("*s");
    Serial.print(FONAsmsnum);
    Serial.print("#");
  }

  // read all SMS and send them via serial
  if (FONAsmsnum > 0)
  {
    // read all SMS
    uint16_t smslen;
    int8_t smsn;
    char string[7];
    
    if ( (FONAtype == FONA3G_A) || (FONAtype == FONA3G_E) )
    {
      smsn = 0; // zero indexed
      FONAsmsnum--;
    }
    else
    {
      smsn = 1;  // 1 indexed
    }
    
    for ( ; smsn <= FONAsmsnum; smsn++)
    {
      // Serial.print(F("\n\rReading SMS #")); Serial.println(smsn);

      // read SMS into buffer, max length = 250
      if (!fona.readSMS(smsn, FONASMSbuffer, 250, &smslen))
      {
        Serial.println(F("Failed!"));
        break;
      }
      
      // if the length is zero, its a special case where the index number is higher
      // so increase the max we'll look at!
      if (smslen == 0)
      {
        // Serial.println(F("[empty slot]"));
        FONAsmsnum++;
        continue;
      }
    
      // message no xx (fixed length of 2!)  "*mxx="
      Serial.print("*m");
      Serial.print(smsn);
      Serial.print("=");
       
      // SMS text / content
      //
      // Fix umlauts & Co. since there is no UTF8 during serial ASCII transfer!!
      String textWithCorrectUmlauts(FONASMSbuffer);
      // fix 'ä'
      textWithCorrectUmlauts.replace(0xE4, 'ä');    textWithCorrectUmlauts.replace(0xC4, 'Ä');
      // fix 'ö'
      textWithCorrectUmlauts.replace(0xF6, 'ö');    textWithCorrectUmlauts.replace(0xD6, 'Ö');
      // fix 'ü'
      textWithCorrectUmlauts.replace(0xFC, 'ü');    textWithCorrectUmlauts.replace(0xDC, 'Ü');
      // fix 'ß'
      textWithCorrectUmlauts.replace(0xDF, 'ß');
      // fix '€'
      textWithCorrectUmlauts.replace(0x80, '€');

      // send converted SMS content
      Serial.print(textWithCorrectUmlauts);
      Serial.print("#");

      // delete sent SMS now!
      if (deleteSMSafterTransfer == true)
      {
        // if delete SMS does not workd
        if (fona.deleteSMS(smsn) == false)
        {
          Serial.print("*m");
          Serial.print(smsn);
          Serial.print("=Error deleting SMS.");
          Serial.print("#");
        }
      }
    }
  }

  // wait some time before starting again to send all stuff again
  delay(1000);
}


// waits for an answer from computer. Has start with * and end with #.
void waitForAnswer()
{
  static uint8_t string_started = 0;  // Sind wir jetzt im String?

  
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

              // copy input string to answer string (used in loop)
              answer = inputString;

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


/*
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
*/

  // Wurde ein kompletter String empfangen und ist der Buffer ist leer?
  // delete flag
  stringComplete = false;
}

