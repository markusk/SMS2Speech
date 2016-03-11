// For Adafruit FONA
#include <Adafruit_FONA.h>

// For serial communication to the FONA module
#include <SoftwareSerial.h>

// Adafruit FONA pins (GSM)  @todo: update pins to user a real HW serial port on Arduino/seeduino mega!! < < < <
#define FONA_TX      16
#define FONA_RX      17
#define FONA_RST     18


//------------------------------------------------------------
// Adafruit FONA stuff
//------------------------------------------------------------
//const int buttonPin = A0; // Pushbutton
int8_t smsnum = 0; // the number of available SMS's
int returnValue = 0;
// this is a large buffer for replies (SMS)
char replybuffer[160]; // < < < < max 160 character length ! ! !!

SoftwareSerial fonaSS = SoftwareSerial(FONA_TX, FONA_RX);
Adafruit_FONA fona = Adafruit_FONA(FONA_RST);

// store the state if FONA is okay or not (init okay etc.)
boolean FONAstate = false;
//------------------------------------------------------------



//-------  from main.h  -------------------------------

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
  // initialize serial communication on the USB port
  Serial.begin(9600);

  // reserve 200 bytes for the inputString
  inputString.reserve(stringSize);
  command.reserve(stringSize);
  stringComplete = false;  // Flag, String komplett empfangen
  //-------------------------------------------------------------------------------------------------


  //-------------------------------------------------------------------------------------------------
  // initialize some digital pins as an output.
  //-------------------------------------------------------------------------------------------------

//  pinMode(RGBLED5red, OUTPUT);

  //-------------------------------------------------------------------------------------------------

}


void loop()
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
        greenLED(OFF);
        yellowLED(OFF);
        
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
  else
  // GSM init (FONA) = "gsmi"
  if (command == "*gsmi#")
  {
    // FONA init in setup() okay?    
    if (FONAstate == false)
    {
      // answer "error"
      if (Serial.print("*err#") < 5)
      {
        // ERROR!!
        delay(10000);
        return;
      }
      // write all data immediately!
      Serial.flush();
    }
    else
    {
      // answer "ok"
      if (Serial.print("*gsmi#") < 6)
      {
        // ERROR!!
        delay(10000);
        return;
      }
      // write all data immediately!
      Serial.flush();
    }
  } // gsmi
  else
  // GSM PIN unlock (FONA) =  "gsmp"
  if (command == "*gsmp#")
  {
    if (unlockSIM() == -1)
    {
      // ERROR

      // store state
      FONAstate = false;

      // all LEDs red
      allLEDsRed();

      // answer "ok"
      if (Serial.print("*err#") < 5)
      {
        // ERROR!!
        delay(10000);
        return;
      }
      // write all data immediately!
      Serial.flush();
    }
    else
    {
      // OKAY

      // store state
      FONAstate = true;      

      // answer "ok"
      if (Serial.print("*gsmp#") < 6)
      {
        // ERROR!!
        delay(10000);
        return;
      }
      // write all data immediately!
      Serial.flush();
    } // PIN okay
  } // gsmp
  // (get) GSM status (FONA) = "gsms"
  if (command == "*gsms#")
  {
    // read the network/cellular status
    uint8_t networkStatus = fona.getNetworkStatus();

    if (Serial.print("*") < 1)
    {
      // ERROR!!
      delay(10000);
      return;
    }
    // write all data immediately!
    Serial.flush();

    // print network status
    if (Serial.print( networkStatus ) < 1)
    {
      // ERROR!!
      delay(10000);
      return;
    }
    // write all data immediately!
    Serial.flush();

    if (Serial.print("#") < 1)
    {
      // ERROR!!
      delay(10000);
      return;
    }
    // write all data immediately!
    Serial.flush();
  } // gsms
  else
  // SMS_COUNT / SMS_CHECK = "smsc"
  if (command == "*smsc#")
  {
    // read and store(!) the number of SMS's
    smsnum = fona.getNumSMS();

    // success ?
    if (smsnum < 0)
    {
      // answer with "ERROR"
      Serial.print("*err#");
      Serial.flush();
    }
    else
    {
      if (Serial.print("*") < 1)
      {
        // ERROR!!
        delay(10000);
        return;
      }
      // write all data immediately!
      Serial.flush();

      // print no of SMS
      if (Serial.print( smsnum ) < 1)
      {
        // ERROR!!
        delay(10000);
        return;
      }
      // write all data immediately!
      Serial.flush();

      if (Serial.print("#") < 1)
      {
        // ERROR!!
        delay(10000);
        return;
      }
      // write all data immediately!
      Serial.flush();
    } // okay
  } // smsc
  else
  // read Last SMS = "smsl"
  if (command == "*smsl#")
  {
    // Retrieve content of SMS No. "smsnum"
    uint16_t smslength;

    // pass in buffer and max len (255)!
    if (! fona.readSMS(smsnum, replybuffer, 250, &smslength))
    {
      // answer with "ERROR"
      Serial.print("*err#");
      Serial.flush();
    }
    else
    {
      // success
      if (Serial.print("*") < 1)
      {
        // ERROR!!
        delay(10000);
        return;
      }
      // write all data immediately!
      Serial.flush();

      // print SMS content
      if (Serial.print( replybuffer ) < 1)
      {
        // ERROR!!
        delay(10000);
        return;
      }
      // write all data immediately!
      Serial.flush();

      if (Serial.print("#") < 1)
      {
        // ERROR!!
        delay(10000);
        return;
      }
      // write all data immediately!
      Serial.flush();
    } // okay
  } // smsl

  // no valid command found (i.e. *wtf# )
  // delete command string
  command = "";

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
