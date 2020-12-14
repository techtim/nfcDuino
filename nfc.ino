/**************************************************************************/
/*! 
    @file     readntag203.pde
    @author   KTOWN (Adafruit Industries)
    @license  BSD (see license.txt)

    This example will wait for any NTAG203 or NTAG213 card or tag,
    and will attempt to read from it.

    This is an example sketch for the Adafruit PN532 NFC/RFID breakout boards
    This library works with the Adafruit NFC breakout 
      ----> https://www.adafruit.com/products/364
 
    Check out the links above for our tutorials and wiring diagrams 
    These chips use SPI or I2C to communicate.

    Adafruit invests time and resources providing this open source code, 
    please support Adafruit and open-source hardware by purchasing 
    products from Adafruit!
*/
/**************************************************************************/
#include <Wire.h>
#include <SPI.h>

#include <Adafruit_PN532.h>

// If using the breakout or shield with I2C, define just the pins connected
// to the IRQ and reset lines.  Use the values below (2, 3) for the shield!
#define PN532_IRQ   (9)
#define PN532_RESET (3)  // Not connected by default on the NFC Shield


#include <FastLED.h>

#define LED_PIN     33
#define NUM_LEDS    500
#define BRIGHTNESS  60
#define LED_TYPE    SK6812
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];

///////////////// EDIT //////////////////////
const uint8_t cardsNumber = 1;

const uint8_t cardUids[][4] = {
  {0xA7, 0x96, 0x4C, 0xB5},
  {0x2A, 0x1F, 0x7E, 0x81},
  {0x2A, 0x1F, 0x7E, 0x81},
  {0x2A, 0x1F, 0x7E, 0x81},
  {0x2A, 0x1F, 0x7E, 0x81},
  {0x2A, 0x1F, 0x7E, 0x81},
};

const char cardKeys[] = {
  '1',
  '2',
  '3',
  '4',
  '5',
  '6'
};


/////////////////////////////////////////////

int check(Adafruit_PN532 &nfc, const uint8_t uidCheck[]);

bool cardStates[]{
  false, false, false, false, false, false
};

// Or use this line for a breakout or shield with an I2C connection:
Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET); 

void setup(void) {
  Serial.begin(115200);
  while (!Serial) delay(10); // for Leonardo/Micro/Zero

  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(  BRIGHTNESS );

  for (int led=0; led<NUM_LEDS; ++led)
     leds[led] = CRGB::Black;
     
  Serial.println("Hello!");

  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    Serial.print("Didn't find PN53x board");
    while (1); // halt
  }
  // Got ok data, print it out!
  Serial.print("Found chip PN5"); Serial.println((versiondata>>24) & 0xFF, HEX); 
  Serial.print("Firmware ver. "); Serial.print((versiondata>>16) & 0xFF, DEC); 
  Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);

  // Set the max number of retry attempts to read from a card
  // This prevents us from waiting forever for a card, which is
  // the default behaviour of the PN532.
  nfc.setPassiveActivationRetries(0xFF);
  
  // configure board to read RFID tags
  nfc.SAMConfig();

  Serial.println("Waiting for an ISO14443A Card ...");
}

void loop(void) {

 //for (int led=0; led<NUM_LEDS; ++led)
 //       leds[led].g = 0;
  for (int i=0; i < cardsNumber; ++i){
    int state = check(nfc, cardUids[i]);
    if(state == -1)
      continue;
      
    if(state == 1){
      Serial.print(i);
      Serial.println(" Success");
      for (int led=0; led<NUM_LEDS; ++led){
        leds[led].r= 200; 
        leds[led].b = 0;
      }
    } else if (state == 0) {
      Serial.print(i);
      Serial.println(" Wrong");
      for (int led=0; led<NUM_LEDS; ++led){
        leds[led].r = 0; 
        leds[led].b = 200;
      }
    }
  }
  
  FastLED.show();
  FastLED.delay(33);
}

int check(Adafruit_PN532 &nfc, const uint8_t uidCheck[]) {
  uint8_t success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)

  // Wait for an NTAG203 card.  When one is found 'uid' will be populated with
  // the UID, and uidLength will indicate the size of the UUID (normally 7)
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
  
  if (!success)
    return -1;

//  Serial.print("UID Length: ");Serial.print(uidLength, DEC);Serial.println(" bytes");
  Serial.print("UID Value: ");
  for (uint8_t i=0; i < uidLength; i++) 
  {
    Serial.print(" 0x");Serial.print(uid[i], HEX); 
  }
  Serial.println("");
  
  if (uidLength == 4)
  {
    for (int i=0; i<4; ++i)
      if (uid[i] != uidCheck[i])
        success = false;
  }
  else
  {
     Serial.println("This doesn't seem to be an NTAG203 tag (UUID length != 4 bytes)!");
  }

  Serial.flush();    

 // Wait 0.1 second before continuing
  delay(100);
  return success? 1 : 0;
}
