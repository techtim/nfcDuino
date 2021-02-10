/**************************************************************************/
/*!

*/
/**************************************************************************/

#include "Keyboard.h"

#include "Adafruit_PN532.h"
#include <Wire.h>
#include <FastLED.h>

// If using the breakout or shield with I2C, define just the pins connected
// to the IRQ and reset lines.  Use the values below (2, 3) for the shield!
#define PN532_IRQ (21)
#define PN532_RESET (20)  // Not connected by default on the NFC Shield

//////////////// LED CONF ///////////////////

#define LED_PIN 9
#define NUM_LEDS 200
#define LED_TYPE SK6812
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];

///////////////// EDIT //////////////////////

int BRIGHTNESS = 160;

CRGB COLOR_OK = CRGB(0, 50, 250);
CRGB COLOR_WRONG = CRGB(250, 0, 0);
CRGB COLOR_NO = CRGB(0, 0, 0);

const uint8_t cardUidLen = 6;
const uint8_t cardUid[cardUidLen] = {0xA7, 0x96, 0x4C, 0xB5, 0xB5, 0xB5};

const char cardKeyOk = '1';

const char cardKeyError = '0';

/////////////////////////////////////////////

int check(Adafruit_PN532 &nfc, const uint8_t uidCheck[]);

// Or use this line for a breakout or shield with an I2C connection:
Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET);

void toLog(const char * str){
  if (Serial)
    Serial.println(str);
}

void setup(void) {
  Keyboard.begin();
  Serial.begin(115200);
//  while (!Serial) delay(10);  // for Leonardo/Micro/Zero

  toLog("Hello!");

  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (!versiondata) {
    toLog("Didn't find PN53x board");
    while (1)
      ;  // halt
  }
  // Got ok data, print it out!
//  Serial.print("Found chip PN5");
//  Serial.println((versiondata >> 24) & 0xFF, HEX);
//  Serial.print("Firmware ver. ");
//  Serial.print((versiondata >> 16) & 0xFF, DEC);
//  Serial.print('.');
//  Serial.println((versiondata >> 8) & 0xFF, DEC);

  // Set the max number of retry attempts to read from a card
  // This prevents us from waiting forever for a card, which is
  // the default behaviour of the PN532. 0xFF for blocking infinity wait
  nfc.setPassiveActivationRetries(0x1F);

  nfc.SAMConfig();

  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS)
      .setCorrection(TypicalLEDStrip);

  FastLED.setBrightness(BRIGHTNESS);

  for (int led = 0; led < NUM_LEDS; ++led) leds[led] = CRGB::Black;

  FastLED.show();

  toLog("Waiting for an ISO14443A Card ...");

}

int prevState = -1;

void loop(void) {
      
  int state = check(nfc, cardUid);

  if (state == 1) {
    toLog(" Success");
    if (state != prevState)
      Keyboard.write(cardKeyOk);
    for (int led = 0; led < NUM_LEDS; ++led) {
      leds[led] = COLOR_OK;
    }
  } else if (state == 0) {
    toLog(" Wrong");
    if (state != prevState)
      Keyboard.write(cardKeyError);
    for (int led = 0; led < NUM_LEDS; ++led) {
      leds[led] = COLOR_WRONG;
    }
  } else if (state == -1) {
    toLog("No Card");
    for (int led = 0; led < NUM_LEDS; ++led) {
      leds[led] = COLOR_NO;
    }
  }

  prevState = state;

  FastLED.show();
//  FastLED.delay(100);
}

int check(Adafruit_PN532 &nfc, const uint8_t uidCheck[]) {
  uint8_t success;
  uint8_t uid[] = {0, 0, 0, 0, 0, 0, 0};  // Buffer to store the returned UID
  uint8_t uidLength;  // Length of the UID (4 or 7 bytes depending on ISO14443A
                      // card type)

  // Wait for an NTAG203 card.  When one is found 'uid' will be populated with
  // the UID, and uidLength will indicate the size of the UUID (normally 7)
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);

  if (!success) return -1;

  Serial.print("UID Value: ");
  for (uint8_t i = 0; i < uidLength; i++) {
    Serial.print(" 0x");
    Serial.print(uid[i], HEX);
  }
  Serial.println("");

  if (uidLength == cardUidLen) {
    for (int i = 0; i < cardUidLen; ++i)
      if (uid[i] != uidCheck[i]) success = false;
  } else {
    success = false;
    Serial.println(
        "This doesn't seem to be an NTAG203 tag (UUID length != 4 bytes)!");
  }

  Serial.flush();

  // Wait 0.1 second before continuing
  delay(100);
  return success ? 1 : 0;
}
