#include <Arduino.h>

////////////////////////////////////////
//      TFT (librairie Nkawu/TFT_22_ILI9225)
////////////////////////////////////////
#include <TFT_22_ILI9225.h> // Force PlatformIO à inclure le chemin dans le build
#include "GupESPTFT.h"

#include "SPI.h"

/////////////////////////////////////////////////
// paramters define for TFT

// Utilisation des pins du côté DROIT de la carte
#define TFT_LED 22  // Si tu veux la piloter, sinon branche au 3.3V
#define TFT_CLK 18  // SCK (Hardware SPI)
#define TFT_SDI 23  // MOSI (Hardware SPI)
#define TFT_RS  19  // Entre SCK (18) et MOSI (23)
#define TFT_RST 5   // Juste au-dessus de SCK (18)
#define TFT_CS  16  // Juste en dessous de la Pin 17 (qui est souvent morte)


TFTGup mylcd = TFTGup(TFT_RST, TFT_RS, TFT_CS, TFT_LED);

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
void setup()
{
    // Serial.begin(115200);

    //--- init LCD ---

    mylcd.begin();
    mylcd.setOrientation(3);
    mylcd.setFont(Terminal6x8); // height 8
    mylcd.println("LCD OK");

    MonAxe.DefineY1MinMax(0, ValMaxVoltmetre);
    MonAxe.FlagautoScaleY = 0;
    MonAxe.InitAxes();

    mylcd.drawText(0, LargScreen - 8, "Wait for Press");
}

void loop()
{
    BoutonStartStop.Surveille();
    if (BoutonStartStop.EventBouton())
    {
        if (BoutonStartStop.EtatBouton == 0)
        {
            mylcd.drawText(0, LargScreen - 8, "Wait for Press");
        }
        else
        {
            mylcd.drawText(0, LargScreen - 8, "Record              ");
        }
    }

    if (BoutonStartStop.EtatBouton == 1)
    {
        unsigned long CurrentTime = millis();
        if (CurrentTime - LastTime > DelayMesure)
        {
            indiceLecture++;
            LastTime = CurrentTime;
            int ReadMeasure = analogRead(PinCapteur);
            MonAxe.AddPlot1(indiceLecture, float(ReadMeasure) / ValMaxAnalogRead * ValMaxVoltmetre);
        }
    }
    if (BoutonStartStop.LongPress == 1)
    {
        // mylcd.drawText(0, LargScreen - 8, "Cursor Analysis        ");
        bool FlagAnalyseCurseur = 1; // Tant que l'on déplace le curseur sans appuie long sur Start Stop
        int IndicePtCourant = 100;
        int LastIndicePtCourant = IndicePtCourant;
        while (FlagAnalyseCurseur)
        {
            mylcd.drawText(0, LargScreen - 8, "CURSOR           ");
            BoutonMoins.Surveille();
            BoutonStartStop.Surveille();

            if (BoutonStartStop.EventBouton())
            {
                // Demande de sortie ou IndicePtCourant +1
                if (BoutonStartStop.LongPress)
                { // demande de sortie
                    BoutonStartStop.EtatBouton = 0;
                    BoutonStartStop.LongPress=0;
                    FlagAnalyseCurseur = 0; // Fin d'analyse
                    
                    // Efface l'ancien curseur
                    MonAxe.drawCursor(LastIndicePtCourant, COLOR_BLACK);
                    MonAxe.drawLine1(COLOR_LIGHTBLUE);
                    mylcd.drawText(0, LargScreen - 8, "Wait for Press       ");
                }
                else
                {
                     IndicePtCourant = min(IndicePtCourant + 1, 100);
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
                // Efface l'ancien curseur
                MonAxe.drawCursor(LastIndicePtCourant, COLOR_BLACK);
                MonAxe.drawLine1(COLOR_LIGHTBLUE);

                MonAxe.drawCursor(IndicePtCourant, COLOR_RED);
                LastIndicePtCourant = IndicePtCourant;
            }            
        }
    }
}