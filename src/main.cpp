#include <Arduino.h>

////////////////////////////////////////
//      TFT (librairie Nkawu/TFT_22_ILI9225)
////////////////////////////////////////
#include <TFT_22_ILI9225.h> // Force PlatformIO à inclure le chemin dans le build
#include "GupESPTFT.h"

#include "SPI.h"

/////////////////////////////////////////////////
// paramters define for TFT

// --- CONFIGURATION DES PINS (CÔTÉ DROIT LOLIN32 LITE) ---
#define TFT_CLK 18 // Pin native (VSPI)
#define TFT_SDI 23 // Pin native (VSPI)
#define TFT_RS 19
#define TFT_RST 17
#define TFT_CS 5 // Pin CS native (VSPI)

// Création de l'instance de l'écran
// TFT_22_ILI9225 tft(TFT_RST, TFT_RS, TFT_CS, TFT_SDI, TFT_CLK,150);

TFTGup mylcd = TFTGup(TFT_RST, TFT_RS, TFT_CS, TFT_SDI, TFT_CLK, 200);

//////////////////////////////////////////////////
// config pour Mesure voltmetre

#define PinCapteur 32

int DelayMesure = 100; // ms
unsigned long LastTime = millis();
int ValMaxAnalogRead = 4095;
float ValMaxVoltmetre = 3.3;
AxesGup MonAxe(mylcd, 0, 0, HautScreen, LargScreen - 16);
int indiceLecture = 0;

//////////////////////////////////////////////////
// config pour bouton

#include <GupButton.h>

#define PinBoutonSS 33
#define PinBoutonMoins 25

Bouton BoutonStartStop(PinBoutonSS);
Bouton BoutonMoins(PinBoutonMoins);


////////////////////////////////////////
// Config de la LED en utilisant jled
#include <jled.h>


// déclare la led
auto led_record = JLed(26).MaxBrightness(58).On().Forever();

// a developper ----------
#include <Guptime.h>
Guptimer Timer;
//------------

void setup()
{
    Serial.begin(115200);
    delay(500);
    Serial.println("--- GUP SYSTEM READY ---");

    //--- init LCD ---

    mylcd.begin();
    mylcd.setOrientation(3);
    mylcd.setFont(Terminal6x8); // height 8
    mylcd.println("LCD OK");

    MonAxe.DefineY1MinMax(0, ValMaxVoltmetre);
    MonAxe.FlagautoScaleY = 0;
    MonAxe.InitAxes();

    mylcd.drawText(0, LargScreen - 8, "Wait for Press");
    Serial.println("Mode WAIT");

    led_record.Stop();

    Timer.Init();
    
}

void loop()
{
    if (Timer.Check())
    {

        mylcd.println(String(millis())+" ms");
    }
    
}