#include <Wire.h>
#include "RTClib.h"
#include <LiquidCrystal_I2C.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

#define LED_PIN 6
#define LED_COUNT 8
#define DS1307_ADDRESS 0x68

LiquidCrystal_I2C lcd(0x27, 16, 2); // 0x27 là địa chỉ của lcd 16x2
RTC_DS1307 RTC;

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_RGB + NEO_KHZ800);

String message;
String message1 = "ELG";
String power = "off";
int mode = 0;
int modeCount = 5;
int color[] = {255, 255, 255};
int r = 255, g = 0, b = 0;
int countChase = 0;
int chaseMode = 0;
long  rainbowCount = 0;
byte zero = 0x00;
bool scheduleMode = false;
int startHour = 0;
int startMinute = 0;
int endHour = 0;
int endMinute = 0;
char buf[50];
char c = ' ' ;
int u;
unsigned long previousMillis = 0;

void setup() {
  pinMode(13, OUTPUT);
  Serial.begin(9600);

  strip.begin();
  strip.show();
  strip.setBrightness(100);
  // setup LCD
  lcd.init();
  lcd.backlight();
  // Serial.begin(9600);
  Wire.begin();
  RTC.begin();
  if (! RTC.isrunning()) {
    Serial.println("RTC is NOT running!");
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }
}

void loop() {
  while (Serial.available())
  {
    delay(10);
    char c = Serial.read();
    if (c == ' ') break;
    message += c;
  }

  if (message.length() > 0)
  {
    Serial.println(message);
    if (message == "power") {
      power = power == "on" ? "off" : "on" ;
    } else if (message == "mode") {
      mode = (mode + 1) % modeCount;
    } else if (message.substring(0, 1) == "r") {
      color[0] = message.substring(message.indexOf('r') + 1, message.indexOf('m')).toInt();
      color[1] = message.substring(message.indexOf('g') + 1, message.indexOf('n')).toInt();
      color[2] = message.substring(message.indexOf('b') + 1, message.indexOf('o')).toInt();
    } else if (message.substring(0, 1) == "b") {
      strip.setBrightness(message.substring(1, 4).toInt());
    } else if (message.substring(0, 1) == "s") {
      updateDateTime(message);
    } else if (message.substring(0, 1) == "a") {
      String startTime = message.substring(1, message.indexOf('b') + 1);
      String endTime = message.substring(message.indexOf('b') + 1, message.indexOf('c') + 1);

      startHour = startTime.substring(0, startTime.indexOf(':')).toInt();
      startMinute = startTime.substring(startTime.indexOf(':') + 1, startTime.indexOf('b')).toInt();

      endHour = endTime.substring(0, endTime.indexOf(':')).toInt();
      endMinute = endTime.substring(endTime.indexOf(':') + 1, endTime.indexOf('c')).toInt();

      scheduleMode = true;
    } else if (message == "canncel") {
      scheduleMode = false;
    }
    else if (message.substring(0, 1) == "t"){
      message1 = message.substring(1);
      message1.replace("*" , " ");
      lcd.clear();
    }
  }

  message = "";

  if (power == "on")
  {
    switch (mode) {
      case 0:
        for (int i = 0; i < strip.numPixels(); i++) {
          strip.setPixelColor(i, color[1], color[0], color[2]);
        }
        strip.show();
        break;
      case 1:
        blinkColor(20);
        break;
      case 2:
        colorCycle(20);
        break;
      case 3:
        chase(20);
      case 4:
        rainbow(20);
    }
  }
  if (power == "off")
  {
    for (int i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, 0, 0, 0);
    }
    strip.show();
  }

  //LCD
  DateTime now = RTC.now();


  lcd.setCursor(2, 0);
  lcd.print(now.hour(), DEC);
  lcd.print(":");
  lcd.print(now.minute(), DEC);
  lcd.print(":");
  lcd.print(now.second(), DEC);
  lcd.print(" ");

  lcd.setCursor(11, 0);
  lcd.print(now.day(), DEC);
  lcd.print("/");
  lcd.print(now.month(), DEC);

  if (message1.length() >= 16 )
  {
    lcd.setCursor(0, 1);
    for ( int letter = u; letter <= u + 15; letter ++)
    {
      lcd.print(message1[letter]);
    } if ( u == 0) {
      delay(500);
    };
    unsigned long currentMillis = millis();
    if (u == message1.length() - 16)
    {
      u = 0;
    };
    if (currentMillis - previousMillis >= 350)
    { previousMillis = currentMillis;
      u++;
    }
  }
  else {
    lcd.setCursor(0, 1);
    lcd.print(message1);
  }
  // schedule mode
  if (scheduleMode) {
    // start
    if (startHour == now.hour() && startMinute == now.minute()) {
      power = "on";
//      lcd.backlight();
    }
    //end
    if (endHour == now.hour() && endMinute == now.minute()) {
      power = "off";
//      lcd.noBacklight();
    }
  }
}

void rainbow(int wait) {
  for (int i = 0; i < strip.numPixels(); i++) {
    int pixelHue = rainbowCount + (i * 65536L / strip.numPixels());
    strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
  }
  strip.show();
  delay(wait);
  rainbowCount = (rainbowCount + 256) % (5 * 65536);
}

void chase(uint8_t wait) {
  if (!chaseMode) {
    strip.setPixelColor(countChase, color[1], color[0], color[2]);
  } else {
    strip.setPixelColor(countChase, 0, 0, 0);
  }
  strip.show();
  delay(wait);
  if (countChase == (LED_COUNT - 1)) chaseMode = 1 - chaseMode;
  countChase = (countChase + 1) % LED_COUNT;
}

void colorCycle(uint8_t wait) {
  if (r > 0 && b == 0) {
    r--;
    g++;
  }
  if (g > 0 && r == 0) {
    g--;
    b++;
  }
  if (b > 0 && g == 0) {
    r++;
    b--;
  }
  for (int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, g, r, b);
  }
  strip.show();
  delay(wait);
}

void blinkColor(uint8_t wait) {
  for (int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, color[1], color[0], color[2]);
  }
  strip.show();
  delay(wait);
  for (int j = 0; j < strip.numPixels(); j++) {
    strip.setPixelColor(j, 0, 0, 0);
  }
  strip.show();
  delay(wait);
}

void updateDateTime(String dateTime) {
  byte second = (byte) dateTime.substring(1, dateTime.indexOf('m')).toInt();
  byte minute = (byte) dateTime.substring(dateTime.indexOf('m') + 1, dateTime.indexOf('h')).toInt();
  byte hour = (byte) dateTime.substring(dateTime.indexOf('h') + 1, dateTime.indexOf('w')).toInt();
  byte weekDay = (byte) dateTime.substring(dateTime.indexOf('w') + 1, dateTime.indexOf('d')).toInt();
  byte monthDay = (byte) dateTime.substring(dateTime.indexOf('d') + 1, dateTime.indexOf('t')).toInt();
  byte month = (byte) dateTime.substring(dateTime.indexOf('t') + 1, dateTime.indexOf('y')).toInt();
  byte year = (byte) (dateTime.substring(dateTime.indexOf('y') + 1, dateTime.indexOf('e')).toInt() - 2000);
  setDateTime(second, minute, hour, weekDay, monthDay, month, year);
}

void setDateTime(byte second, byte minute, byte hour   , byte weekDay, byte monthDay , byte month, byte year) {
  Wire.beginTransmission(DS1307_ADDRESS);
  Wire.write(zero); //stop Oscillator

  Wire.write(decToBcd(second));
  Wire.write(decToBcd(minute));
  Wire.write(decToBcd(hour));
  Wire.write(decToBcd(weekDay));
  Wire.write(decToBcd(monthDay));
  Wire.write(decToBcd(month));
  Wire.write(decToBcd(year));

  Wire.write(zero); //start

  Wire.endTransmission();

}

byte decToBcd(byte val) {
  // Convert normal decimal numbers to binary coded decimal
  return ( (val / 10 * 16) + (val % 10) );
}

byte bcdToDec(byte val)  {
  // Convert binary coded decimal to normal decimal numbers
  return ( (val / 16 * 10) + (val % 16) );
}
void repstr(char *str)
{
  char *p = str;
  while (*p != NULL)
  {
    if (*p == '*' || *p == '\t')
    {
      *p = c;
    }
    p++;
  }
}
