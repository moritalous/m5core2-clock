#include <Arduino.h>
#include <M5Core2.h>
#include <WiFi.h>

const char WIFI_SSID[] = "";
const char WIFI_PASSWORD[] = "";

const char *ntpServer = "time.google.com";

void print(const char *c)
{
    M5.Lcd.print(c);
    Serial.print(c);
}

void println(const char *c)
{
    M5.Lcd.println(c);
    Serial.println(c);
}

void connectWiFi()
{
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    int count = 0;
    while (WiFi.status() != WL_CONNECTED)
    {
        count++;
        if (count > 100)
        {
            println("Connection Failed.");
            M5.shutdown(1);
        }

        delay(500);
        print(".");
    }
    println("connected.");
}

void initRtc(long gmtOffset)
{
    configTime(gmtOffset * 3600, 0, ntpServer);

    tm timeinfo;
    getLocalTime(&timeinfo);

    RTC_TimeTypeDef Rtctime;
    Rtctime.Hours = timeinfo.tm_hour;
    Rtctime.Minutes = timeinfo.tm_min;
    Rtctime.Seconds = timeinfo.tm_sec;
    RTC_DateTypeDef Rtcdate;
    Rtcdate.Year = timeinfo.tm_year + 1900;
    Rtcdate.Month = timeinfo.tm_mon + 1;
    Rtcdate.WeekDay = timeinfo.tm_wday;

    M5.Rtc.SetDate(&Rtcdate);
    M5.Rtc.SetTime(&Rtctime);
}

void setup()
{
    M5.begin();
    M5.IMU.Init();

    M5.Lcd.setBrightness(128);

    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.setTextSize(4);

    connectWiFi();
}

int rotation = -1;
int pos_x = 160;
int pos_y = 120;

int getRotation()
{
    float ax, ay, az;
    M5.IMU.getAccelData(&ax, &ay, &az);

    if (ax < -0.5 && (-0.5 < ay && ay < 0.5))
    {
        return 0;
    }
    if (0.5 < ax && (-0.5 < ay && ay < 0.5))
    {
        return 2;
    }
    if ((-0.5 < ax && ax < 0.5) && (ay < -0.5))
    {
        return 3;
    }
    return 1;
}

void printTime()
{
    RTC_TimeTypeDef Rtctime;
    M5.Rtc.GetTime(&Rtctime);

    char c[10];
    sprintf(c, "%02d:%02d:%02d", Rtctime.Hours, Rtctime.Minutes, Rtctime.Seconds);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.setTextSize(4);
    M5.lcd.drawString(c, pos_x, pos_y);
}

void printBattery()
{
    float batVoltage = M5.Axp.GetBatVoltage();
    float batPercentage = (batVoltage < 3.2) ? 0 : (batVoltage - 3.2) * 100;

    char c[20];
    sprintf(c, "%02.0f%% (%04.2fV)", batPercentage, batVoltage);

    M5.Lcd.setTextDatum(TL_DATUM);
    M5.Lcd.setTextSize(0);
    M5.lcd.drawString(c, 0, 0);
}

void loop()
{
    M5.update();
    M5.Lcd.setCursor(0, 0);

    int newRotation = getRotation();
    if (rotation != newRotation)
    {
        M5.Lcd.clear();
        M5.Lcd.setRotation(newRotation);
        rotation = newRotation;
        if (rotation == 0 || rotation == 2)
        {
            pos_x = 120;
            pos_y = 160;
        }
        else
        {
            pos_x = 160;
            pos_y = 120;
        }

        M5.Lcd.setTextDatum(MC_DATUM);
        M5.Lcd.setTextSize(4);

        switch (rotation)
        {
        case 0:
            initRtc(0);
            M5.lcd.drawString("London", pos_x, pos_y - 50);
            M5.lcd.drawString("UTC", pos_x, pos_y + 50);
            break;
        case 2:
            initRtc(-5);
            M5.lcd.drawString("New York", pos_x, pos_y - 50);
            M5.lcd.drawString("EST", pos_x, pos_y + 50);

            break;
        case 3:
            initRtc(-8);
            M5.lcd.drawString("Los Angeles", pos_x, pos_y - 50);
            M5.lcd.drawString("PST", pos_x, pos_y + 50);
            break;
        default:
            initRtc(9);
            M5.lcd.drawString("Tokyo", pos_x, pos_y - 50);
            M5.lcd.drawString("JST", pos_x, pos_y + 50);
            break;
        }
    }

    printTime();
    printBattery();

    delay(500);
}