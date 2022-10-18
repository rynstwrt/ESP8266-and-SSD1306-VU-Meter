/* Includes */
#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Adafruit_SSD1306.h>
#include <bitmap.h>



/* Definitions */
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C
#define SCREEN_COLOR SSD1306_WHITE

#define VU_SAMPLE_WINDOW 25  // 25ms, AKA 40Hz

#define AUDIO_ANALOG_PIN A0 
#define AUDIO_ANALOG_MIN 0
#define AUDIO_ANALOG_MAX 1024

#define NEEDLE_CENTER_HORIZ 65  // horizontal center for needle animation
#define NEEDLE_CENTER_VERT 85  // vertical "center" (out of screen bounds) for needle animation
#define NEEDLE_ANIM_LENGTH 80 //80  // length of needle animation/arch of needle travel        



/* Variables */
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
unsigned int audioSample;
const float RADIAN = 180 / PI; 
const GFXfont* FONT = &FreeMonoBold9pt7b;



/* Helper function to display horizontally centered text */
void horizCenterPrintln(String text)
{   
    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);

    int horizOffset = round(SCREEN_WIDTH / 2.0 - w / 2.0);
    static int horizCenterPrintlnYOffset = 0; // Keeps value b/c of how static vars in a function work in C++.
    horizCenterPrintlnYOffset += h;

    display.setCursor(horizOffset, horizCenterPrintlnYOffset);
    display.print(text);
}



/* Setup function */
void setup()
{
    Serial.begin(115200);
    while (!Serial);

    Serial.println("SERIAL CONNECTED!");

    pinMode(AUDIO_ANALOG_PIN, INPUT);

    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
        Serial.println("FAILED TO CONNECT TO OLED!");
    else
        Serial.println("OLED CONNECTED!");

    display.clearDisplay();
    display.setFont(FONT);
    display.setTextSize(1);
    display.setTextColor(SCREEN_COLOR);

    horizCenterPrintln("FOR DJ <3");
    horizCenterPrintln("-Ryn");

    display.display();
    delay(2500);
}



/* Loop function */
void loop()
{
    unsigned long startMS = millis();
    unsigned int peakToPeakAmp = 0;
    unsigned int signalMax = AUDIO_ANALOG_MIN;
    unsigned int signalMin = AUDIO_ANALOG_MAX;

    while (millis() - startMS < VU_SAMPLE_WINDOW)
    {
        audioSample = analogRead(AUDIO_ANALOG_PIN);

        if (audioSample >= AUDIO_ANALOG_MAX)
            continue;

        if (audioSample > signalMax)  // saves the max levels
            signalMax = audioSample;
        else if (audioSample < signalMin)  // saves the min levels
            signalMin = audioSample;
    }

    peakToPeakAmp = signalMax - signalMin; 
    
    float meterValue = peakToPeakAmp * 330 / 1024;  // convert volts to arrow information...
    meterValue -= 34;  // ...then shift needle to zero position

    int needleXCoord = (NEEDLE_CENTER_HORIZ + (sin(meterValue / RADIAN) * NEEDLE_ANIM_LENGTH));
    int needleYCoord = (NEEDLE_CENTER_VERT - (cos(meterValue / RADIAN) * NEEDLE_ANIM_LENGTH));

    display.clearDisplay();
    display.drawBitmap(0, 0, VU_METER_BG_BITMAP, SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_COLOR);
    display.drawLine(needleXCoord, needleYCoord, NEEDLE_CENTER_HORIZ, NEEDLE_CENTER_VERT, SCREEN_COLOR);
    display.display();
}