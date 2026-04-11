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

}

void loop()
{
    BoutonStartStop.Surveille();
    BoutonMoins.Surveille();
    if (BoutonStartStop.EventBouton())
    {
        if (BoutonStartStop.EtatBouton == 0)
        {
            mylcd.drawText(0, LargScreen - 8, "Wait for Press");
            Serial.println("Mode WAIT");
            led_record.Stop();
        }
        else
        {
            mylcd.drawText(0, LargScreen - 8, "Record              ");
            Serial.println("Mode RECORD");
            led_record.Reset();
        }
    }

    if (BoutonStartStop.EtatBouton == 1)
    {
        // Mode Record - Enregistrement du point courant
        unsigned long CurrentTime = millis();
        if (CurrentTime - LastTime > DelayMesure)
        {
            indiceLecture++;
            LastTime = CurrentTime;
            int ReadMeasure = analogRead(PinCapteur);
            MonAxe.AddPlot1(indiceLecture, float(ReadMeasure) / ValMaxAnalogRead * ValMaxVoltmetre);
        }
    }
    else
    {
        // Mode Wait
        if (BoutonMoins.EventBouton())
        {
            Serial.println("Push Moins");
            if (BoutonMoins.LongPress == 1)
            {
                //Reset de l'enregistrement
                Serial.println("Reset");
                MonAxe.NbVal1=0;
                indiceLecture=0;
                mylcd.drawText(0, LargScreen - 8, "Reset Line1      ",COLOR_GREENYELLOW);
                delay(500);
                MonAxe.ClearAll();
                mylcd.drawText(0, LargScreen - 8, "Wait for Press");

            }
        }
    }

    if (BoutonStartStop.LongPress == 1)
    {
        if (BoutonStartStop.EtatBouton == 1)
        {
            mylcd.drawText(0, LargScreen - 8, "CURSOR           ");
        }
        // mylcd.drawText(0, LargScreen - 8, "Cursor Analysis        ");
        bool FlagAnalyseCurseur = 1; // Tant que l'on déplace le curseur sans appuie long sur Start Stop
        int IndicePtCourant = MonAxe.NbVal1;
        int LastIndicePtCourant = IndicePtCourant;
        while (FlagAnalyseCurseur)
        {
            BoutonMoins.Surveille();
            BoutonStartStop.Surveille();

            if (BoutonStartStop.EventBouton())
            {

                // Demande de sortie ou IndicePtCourant +1
                if (BoutonStartStop.LongPress)
                { // demande de sortie
                    BoutonStartStop.EtatBouton = 0;
                    BoutonStartStop.LongPress = 0;
                    FlagAnalyseCurseur = 0; // Fin d'analyse

                    // Efface l'ancien curseur
                    MonAxe.drawCursor(LastIndicePtCourant, COLOR_BLACK);
                    MonAxe.drawLine1(COLOR_LIGHTBLUE);
                    mylcd.drawText(0, LargScreen - 8, "Wait for Press       ");
                    Serial.println("Mode Back WAIT");
                }
                else
                {
                    IndicePtCourant = min(IndicePtCourant + 1, MonAxe.NbVal1);
                    Serial.printf("Indice: %i\n", IndicePtCourant);
                    // Efface l'ancien curseur
                    MonAxe.drawCursor(LastIndicePtCourant, COLOR_BLACK);
                    MonAxe.drawLine1(COLOR_LIGHTBLUE);

                    MonAxe.drawCursor(IndicePtCourant, COLOR_RED);
                    LastIndicePtCourant = IndicePtCourant;
                }
            }
            if (BoutonMoins.EventBouton())
            {
                // Demande de sortie ou IndicePtCourant +1

                IndicePtCourant = max(IndicePtCourant - 1, 0);
                Serial.printf("Indice: %i", IndicePtCourant);
                Serial.println();
                // Efface l'ancien curseur
                MonAxe.drawCursor(LastIndicePtCourant, COLOR_BLACK);
                MonAxe.drawLine1(COLOR_LIGHTBLUE);

                MonAxe.drawCursor(IndicePtCourant, COLOR_RED);
                LastIndicePtCourant = IndicePtCourant;
            }
            led_record.Update();
        }
    }
    led_record.Update();
}