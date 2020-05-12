/**The MIT License (MIT)

Copyright (c) 2018 by Daniel Eichhorn - ThingPulse

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

See more at https://thingpulse.com
*/
#include <ESPWiFi.h>
#include <ESPHTTPClient.h>
#include <JsonListener.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
// time
#include <time.h>      // time() ctime()
#include <sys/time.h>  // struct timeval
#include <coredecls.h> // settimeofday_cb()
//#include "SSD1306Wire.h"
#include "SH1106Wire.h"
#include "OLEDDisplayUi.h"
#include <Wire.h>
#include "OpenWeatherMapCurrent.h"
#include "OpenWeatherMapForecast.h"
#include "WeatherStationFonts.h"
#include "WeatherStationImages.h"

// Create the Lightsensor instance
#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10
#define SEALEVELPRESSURE_HPA (1013.25)
Adafruit_BME280 bme; // I2C
//DHTesp dht;
/***************************
 * Begin Settings
 **************************/

// WIFI
const char *WIFI_SSID = "GotRidOfComcast";
const char *WIFI_PWD = "theirservicesucked";

//Soil Moisture Sensor
const int ANALOG_PIN = A0;
const int AirValue = 790;   //Moisture1 Air Value
const int WaterValue = 480; //Moisture1 Water Value
int soilMoistureValue = 0;
int soilmoisturepercent = 0;
String percent_str = "N/A";

//Declare Temperature/Humidity
String humi1;
String temp1;
String pres1; //added this

//Timezone Settings
#define TZ -5     // (utc+) TZ in hours
#define DST_MN 60 // use 60mn for summer time in some countries

// Display Settings
const int I2C_DISPLAY_ADDRESS = 0x3c;
#if defined(ESP8266)
const int SDA_PIN = D2;
const int SDC_PIN = D1;
const int MOISTURE_PIN_1 = D5;
const int MOISTURE_PIN_2 = D6;
const int RELAY_PIN = D7;
#else
const int SDA_PIN = 4; //D2;
const int SDC_PIN = 5; //D1;
const int MOISTURE_PIN_1 = 14; //D5
const int MOISTURE_PIN_2 = 12;  //D6
const int RELAY_PIN = 13; //D7
#endif

// OpenWeatherMap Settings
// Sign up here to get an API key:
// https://docs.thingpulse.com/how-tos/openweathermap-key/
String OPEN_WEATHER_MAP_APP_ID = "02a19f4506b3008018c8f690e62db526";
/*
Go to https://openweathermap.org/find?q= and search for a location. Go through the
result set and select the entry closest to the actual location you want to display 
data for. It'll be a URL like https://openweathermap.org/city/2657896. The number
at the end is what you assign to the constant below.
 */
String OPEN_WEATHER_MAP_LOCATION_ID = "4255056";

// Pick a language code from this list:
// Arabic - ar, Bulgarian - bg, Catalan - ca, Czech - cz, German - de, Greek - el,
// English - en, Persian (Farsi) - fa, Finnish - fi, French - fr, Galician - gl,
// Croatian - hr, Hungarian - hu, Italian - it, Japanese - ja, Korean - kr,
// Latvian - la, Lithuanian - lt, Macedonian - mk, Dutch - nl, Polish - pl,
// Portuguese - pt, Romanian - ro, Russian - ru, Swedish - se, Slovak - sk,
// Slovenian - sl, Spanish - es, Turkish - tr, Ukrainian - ua, Vietnamese - vi,
// Chinese Simplified - zh_cn, Chinese Traditional - zh_tw.
String OPEN_WEATHER_MAP_LANGUAGE = "en";
const uint8_t MAX_FORECASTS = 4;
const int UPDATE_INTERVAL_SECS = 10 * 60; // Update every 20 minutes
unsigned long delayTime;
const boolean IS_METRIC = false;

// Adjust according to your language
const String WDAY_NAMES[] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};
const String MONTH_NAMES[] = {"JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};

/***************************
 * End Settings
 **************************/
// Initialize the oled display for address 0x3c
// sda-pin=14 and sdc-pin=12
//SSD1306Wire     display(I2C_DISPLAY_ADDRESS, SDA_PIN, SDC_PIN);
SH1106Wire display(I2C_DISPLAY_ADDRESS, SDA_PIN, SDC_PIN);
OLEDDisplayUi ui(&display);

OpenWeatherMapCurrentData currentWeather;
OpenWeatherMapCurrent currentWeatherClient;

OpenWeatherMapForecastData forecasts[MAX_FORECASTS];
OpenWeatherMapForecast forecastClient;

#define TZ_MN ((TZ)*60)
#define TZ_SEC ((TZ)*3600)
#define DST_SEC ((DST_MN)*60)
time_t now;

// flag changed in the ticker function every 10 minutes
bool readyForWeatherUpdate = false;

String lastUpdate = "--";

long timeSinceLastWUpdate = 0;

//declaring prototypes
void drawProgress(OLEDDisplay *display, int percentage, String label);
void updateData(OLEDDisplay *display);
void drawBME(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y);
void drawDateTime(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y);
void drawCurrentWeather(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y);
void drawForecast(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y);
void drawLightSensor(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y);
void drawSoilMoistureSensorOne(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y);
void drawSoilMoistureSensorTwo(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y);
void drawForecastDetails(OLEDDisplay *display, int x, int y, int dayIndex);
void drawHeaderOverlay(OLEDDisplay *display, OLEDDisplayUiState *state);
void setReadyForWeatherUpdate();

// Add frames
// this array keeps function pointers to all frames
// frames are the single views that slide from right to left
FrameCallback frames[] = {drawDateTime, drawCurrentWeather, drawForecast, drawBME, drawLightSensor, drawSoilMoistureSensorOne, drawSoilMoistureSensorTwo};
int numberOfFrames = 7;

OverlayCallback overlays[] = {drawHeaderOverlay};
int numberOfOverlays = 1;

void setup()
{
  Serial.begin(115200);
  
  //Set Pin I/O
  pinMode(MOISTURE_PIN_1, OUTPUT);
  pinMode(MOISTURE_PIN_2, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);

  digitalWrite(MOISTURE_PIN_1, 0);
  digitalWrite(MOISTURE_PIN_2, 0);
  digitalWrite(RELAY_PIN, 0);
  
  Serial.println();
  Serial.println(F("BME280 test"));
  bool status;
  status = bme.begin(0x76);
  if (!status)
  {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1)
      ;
    Serial.println("-- Default Test --");
    delayTime = 1000;

    Serial.println();
  }
  // initialize dispaly
  display.init();
  display.clear();
  display.display();

  //display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setContrast(255);
  WiFi.begin(WIFI_SSID, WIFI_PWD);
  int counter = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
    display.clear();
    display.drawString(64, 10, "Connecting to WiFi");
    display.drawXbm(40, 30, 8, 8, counter % 3 == 0 ? activeSymbole : inactiveSymbole);
    display.drawXbm(54, 30, 8, 8, counter % 3 == 1 ? activeSymbole : inactiveSymbole);
    display.drawXbm(68, 30, 8, 8, counter % 3 == 2 ? activeSymbole : inactiveSymbole);
    display.display();

    counter++;
  }
  // Get time from network time service
  configTime(TZ_SEC, DST_SEC, "pool.ntp.org");

  ui.setTargetFPS(30);

  ui.setActiveSymbol(activeSymbole);
  ui.setInactiveSymbol(inactiveSymbole);

  // You can change this to
  // TOP, LEFT, BOTTOM, RIGHT
  ui.setIndicatorPosition(BOTTOM);

  // Defines where the first frame is located in the bar.
  ui.setIndicatorDirection(LEFT_RIGHT);

  // You can change the transition that is used
  // SLIDE_LEFT, SLIDE_RIGHT, SLIDE_TOP, SLIDE_DOWN
  ui.setFrameAnimation(SLIDE_LEFT);

  ui.setFrames(frames, numberOfFrames);

  ui.setOverlays(overlays, numberOfOverlays);

  // Inital UI takes care of initalising the display too.
  ui.init();

  Serial.println("");

  updateData(&display);
}

void loop()
{
  if (millis() - timeSinceLastWUpdate > (1000L * UPDATE_INTERVAL_SECS))
  {
    setReadyForWeatherUpdate();
    timeSinceLastWUpdate = millis();
  }

  if (readyForWeatherUpdate && ui.getUiState()->frameState == FIXED)
  {
    updateData(&display);
  }

  int remainingTimeBudget = ui.update();

  if (remainingTimeBudget > 0)
  {
    // You can do some work here
    // Don't do stuff if you are below your
    // time budget.
    delay(remainingTimeBudget);
  }
}

void drawProgress(OLEDDisplay *display, int percentage, String label)
{
  display->clear();
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_10);
  display->drawString(64, 10, label);
  display->drawProgressBar(2, 28, 124, 10, percentage);
  display->display();
}

void updateData(OLEDDisplay *display)
{
  drawProgress(display, 10, "Updating time...");
  drawProgress(display, 30, "Updating weather...");
  currentWeatherClient.setMetric(IS_METRIC);
  currentWeatherClient.setLanguage(OPEN_WEATHER_MAP_LANGUAGE);
  currentWeatherClient.updateCurrentById(&currentWeather, OPEN_WEATHER_MAP_APP_ID, OPEN_WEATHER_MAP_LOCATION_ID);
  drawProgress(display, 50, "Updating forecasts...");
  forecastClient.setMetric(IS_METRIC);
  forecastClient.setLanguage(OPEN_WEATHER_MAP_LANGUAGE);
  uint8_t allowedHours[] = {12};
  forecastClient.setAllowedHours(allowedHours, sizeof(allowedHours));
  forecastClient.updateForecastsById(forecasts, OPEN_WEATHER_MAP_APP_ID, OPEN_WEATHER_MAP_LOCATION_ID, MAX_FORECASTS);

  readyForWeatherUpdate = false;
  drawProgress(display, 100, "Done...");
  delay(1000);
}

void drawBME(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y)
{

  float temp1 = (IS_METRIC ? bme.readTemperature() : (1.8 * bme.readTemperature() + 32));
  float pres1 = bme.readPressure() / 100.0F;
  float humi1 = bme.readHumidity();
  delay(delayTime);
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_16);
  String humi = (IS_METRIC ? "Hum: " : "Hum: ") + String(humi1, 1) + (IS_METRIC ? "%" : "%");
  display->drawString(64 + x, y, humi);
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_16);
  String temp = (IS_METRIC ? " Temp: " : "Temp: ") + String(temp1, 1) + (IS_METRIC ? "°C" : "°F");
  display->drawString(64 + x, 15 + y, temp);
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_16);
  String pres = (IS_METRIC ? " Pres: " : "Pres: ") + String(pres1, 1) + (IS_METRIC ? "hPa" : "hPa");
  display->drawString(64 + x, 30 + y, pres);
}

void drawLightSensor(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y)
{
  float light1 = 1.00;
  delay(delayTime);
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_16);
  String light_str = "Light: " + String(light1, 1) + "%";
  display->drawString(64 + x, y, light_str);
}

void drawSoilMoistureSensorOne(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y)
{
  //Turn off the other Analog Devices to read this one
  digitalWrite(MOISTURE_PIN_2, 0);
  digitalWrite(MOISTURE_PIN_1, 1);
  Serial.println(String(MOISTURE_PIN_1, 1) + ": ON");
  float soilMoistureValue = analogRead(ANALOG_PIN); //put Sensor insert into soil
  Serial.println("Moisture 1: " + String(soilMoistureValue, 1));
  soilmoisturepercent = map(soilMoistureValue, AirValue, WaterValue, 0, 100);
  Serial.println(soilmoisturepercent);
  if(soilmoisturepercent > 100)
  {
    percent_str = "100%";
  }
  else if(soilmoisturepercent <0)
  {
    percent_str = "0%";
  }
  else if(soilmoisturepercent >0 && soilmoisturepercent < 100)
  {
    percent_str = String(soilmoisturepercent) + "%";
  }
  delay(delayTime);
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_16);
  String moist_str = "Moist1: " + percent_str;
  display->drawString(64 + x, y, moist_str);
}

//IGNORE THIS COMPLETE COPY/PASTE INSTEAD OF A NEW METHOD
void drawSoilMoistureSensorTwo(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y)
{
  //Turn off the other Analog Devices to read this one
  digitalWrite(MOISTURE_PIN_1, 0);
  digitalWrite(MOISTURE_PIN_2, 1);
  Serial.println(String(MOISTURE_PIN_2, 1) + ": ON");
  float soilMoistureValue = analogRead(ANALOG_PIN); //put Sensor insert into soil
  Serial.println("Moisture 2: " + String(soilMoistureValue, 1));
  soilmoisturepercent = map(soilMoistureValue, AirValue, WaterValue, 0, 100);
  Serial.println(soilmoisturepercent);
  if(soilmoisturepercent > 100)
  {
    percent_str = "100%";
  }
  else if(soilmoisturepercent <0)
  {
    percent_str = "0%";
  }
  else if(soilmoisturepercent >0 && soilmoisturepercent < 100)
  {
    percent_str = String(soilmoisturepercent) + "%";
  }
  delay(delayTime);
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_16);
  String moist_str = "Moist2: " + percent_str;
  display->drawString(64 + x, y, moist_str);
}

void drawDateTime(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y)
{
  now = time(nullptr);
  struct tm *timeInfo;
  timeInfo = localtime(&now);
  char buff[16];

  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_10);
  String date = WDAY_NAMES[timeInfo->tm_wday];

  sprintf_P(buff, PSTR("%s, %02d/%02d/%04d"), WDAY_NAMES[timeInfo->tm_wday].c_str(), timeInfo->tm_mday, timeInfo->tm_mon + 1, timeInfo->tm_year + 1900);
  display->drawString(64 + x, 5 + y, String(buff));
  display->setFont(ArialMT_Plain_24);

  sprintf_P(buff, PSTR("%02d:%02d:%02d"), timeInfo->tm_hour, timeInfo->tm_min, timeInfo->tm_sec);
  display->drawString(64 + x, 15 + y, String(buff));
  display->setTextAlignment(TEXT_ALIGN_LEFT);
}

void drawCurrentWeather(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y)
{
  display->setFont(ArialMT_Plain_10);
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->drawString(64 + x, 38 + y, currentWeather.description);

  display->setFont(ArialMT_Plain_24);
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  String temp = String(currentWeather.temp, 1) + (IS_METRIC ? "°C" : "°F");
  display->drawString(60 + x, 5 + y, temp);

  display->setFont(Meteocons_Plain_36);
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->drawString(32 + x, 0 + y, currentWeather.iconMeteoCon);
}

void drawForecast(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y)
{
  drawForecastDetails(display, x, y, 0);
  drawForecastDetails(display, x + 44, y, 1);
  drawForecastDetails(display, x + 88, y, 2);
}

void drawForecastDetails(OLEDDisplay *display, int x, int y, int dayIndex)
{
  time_t observationTimestamp = forecasts[dayIndex].observationTime;
  struct tm *timeInfo;
  timeInfo = localtime(&observationTimestamp);
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_10);
  display->drawString(x + 20, y, WDAY_NAMES[timeInfo->tm_wday]);

  display->setFont(Meteocons_Plain_21);
  display->drawString(x + 20, y + 12, forecasts[dayIndex].iconMeteoCon);
  String temp = String(forecasts[dayIndex].temp, 0) + (IS_METRIC ? "°C" : "°F");
  display->setFont(ArialMT_Plain_10);
  display->drawString(x + 20, y + 34, temp);
  display->setTextAlignment(TEXT_ALIGN_LEFT);
}

void drawHeaderOverlay(OLEDDisplay *display, OLEDDisplayUiState *state)
{
  now = time(nullptr);
  struct tm *timeInfo;
  timeInfo = localtime(&now);
  char buff[14];
  sprintf_P(buff, PSTR("%02d:%02d"), timeInfo->tm_hour, timeInfo->tm_min);

  display->setColor(WHITE);
  display->setFont(ArialMT_Plain_10);
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->drawString(0, 54, String(buff));
  display->setTextAlignment(TEXT_ALIGN_RIGHT);
  String temp = String(currentWeather.temp, 1) + (IS_METRIC ? "°C" : "°F");
  display->drawString(128, 54, temp);
  display->drawHorizontalLine(0, 52, 128);
}

void setReadyForWeatherUpdate()
{
  Serial.println("Setting readyForUpdate to true");
  readyForWeatherUpdate = true;
}
