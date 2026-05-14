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

#define PinCapteur 35

int ValMaxAnalogRead = 4095;
float ValMaxVoltmetre = 32;
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

// declare le timer entre deux mesures
#include <Guptime.h>
Guptimer timer_measure;
Guptimer timer_display;

// Flag pour effacer le dernier enreg quand un nouveau record commence
bool Flag_efface_record_qd_enreg;

// configuration du capteur INA219
#include <Wire.h>
#include <Adafruit_INA219.h>

// Création de l'instance
Adafruit_INA219 Sensor_ina219;
// init des variables de lecture
// float shuntvoltage = 0;
float busvoltage = 0;
float current_mA = 0;
// float loadvoltage = 0;
float power_mW = 0;

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
    MonAxe.FlagLinetype1 = 1;
    MonAxe.InitAxes();

    led_record.Stop();

    // parametres de frequences
    // entre deux mesures
    timer_measure.Init(100);
    // entre deux affichages
    timer_display.Init(1000);

    //Configuration INA219
    while (!Serial)
    {
        delay(10);
    } // Attente de l'ouverture du moniteur série

    mylcd.println("Initialisation du INA219...");

    // Initialisation du capteur (adresse par défaut 0x40)
    if (!Sensor_ina219.begin())
    {
        mylcd.println("Erreur: INA219 introuvable");
        while (1)
        {
            delay(10);
        }
    }
    // Reset complet du capteur
    Wire.beginTransmission(0x40);
    Wire.write(0x00); // Registre de configuration
    Wire.write(0x80); // Bit de Reset
    Wire.write(0x00);
    Wire.endTransmission();

    delay(100);
    Sensor_ina219.setCalibration_32V_2A(); // Re-calibrer proprement

    // OPTIONNEL : Calibration
    // Par défaut, la bibliothèque configure le capteur pour 32V et 2A.
    // Pour plus de précision sur de petits courants (ex: < 400mA), on peut utiliser :
    // ina219.setCalibration_16V_400mA();

    mylcd.drawText(0, LargScreen - 8, "Mode WAIT         ");
    Serial.println("Mode WAIT");
}

void loop()
{
    BoutonStartStop.Surveille();
    BoutonMoins.Surveille();
    if (BoutonStartStop.EventBouton())
    {
        if (BoutonStartStop.EtatBouton == 0)
        {
            // detection dún arret d'enregistrement
            mylcd.drawText(0, LargScreen - 8, "Wait for Press");
            Serial.println("Mode WAIT");
            MonAxe.RefreshAllLines();
            
            led_record.Stop();
            Flag_efface_record_qd_enreg=true;
            timer_measure.Init(200); // pour les affichage en mode wait
        }
        else
        {
            // Detection d'un lancement d'enregistrement
            //efface le bandeau d'affichage en continue
            mylcd.fillRectangle(0,LargScreen-16,HautScreen,LargScreen,COLOR_BLACK);

            mylcd.drawText(0, LargScreen - 8, "Record              ");
            Serial.println("Mode RECORD");
            timer_measure.Init(100); // pour les records
        
            if (Flag_efface_record_qd_enreg&&BoutonStartStop.LongPress==0)
            {
                // clear du precedent enregistrement
                MonAxe.NbVal1 = 0;
                indiceLecture = 0;
                MonAxe.ClearAll();
                Flag_efface_record_qd_enreg=false;
            }
            mylcd.setBackgroundColor(COLOR_BLACK);
            led_record.Reset();
        }
    }

    if (BoutonStartStop.EtatBouton == 1)
    {
        // Mode Record - Enregistrement du point courant
        if (timer_measure.Check()&&BoutonStartStop.PositionBouton==false&&BoutonStartStop.LongPress==0)
        {        
            //on doit eviter de mesurer quand
            // le long press se prepare avec l'appui sur le bouton
            // le long press est declenché
            timer_measure.Reset();
            indiceLecture++;
            
            float current_time=millis()/1000.0f;
            // int ReadMeasure = analogRead(PinCapteur);
            // float current_measure=float(ReadMeasure) / ValMaxAnalogRead * ValMaxVoltmetre;

              // Lecture des registres
            //shuntvoltage = ina219.getShuntVoltage_mV();
            busvoltage = Sensor_ina219.getBusVoltage_V();
            
            //current_mA = ina219.getCurrent_mA();
            // power_mW = ina219.getPower_mW();
            // loadvoltage = busvoltage + (shuntvoltage / 1000);

            MonAxe.AddPlot1(current_time, busvoltage);

            float diff_acq_time=(current_time-float(MonAxe.Line1[MonAxe.NbVal1-2][0])/100);

            Serial.printf("time %.2f | last time %.2f | delta time %.2f | Value %.2f\n",
                current_time,
                float(MonAxe.Line1[MonAxe.NbVal1-2][0])/100.0,
                diff_acq_time,
                float(busvoltage));
        }
        if (timer_display.Check())
        {
            timer_display.Reset();
            /* MAJ de l'affichage */
            MonAxe.RefreshAllLines();
            Serial.println("Refresh Display");
        }
        
    }
    else
    {
        // en Mode Wait
        // affichage de la mesure courante

        if (timer_measure.Check())
        {
            timer_measure.Reset();
            // Affichage de la valeur courante du capteur
            busvoltage = Sensor_ina219.getBusVoltage_V();
            current_mA = Sensor_ina219.getCurrent_mA();

            mylcd.setBackgroundColor(COLOR_WHITE);
            mylcd.setFont(Terminal11x16); // height 16
            mylcd.drawText(0, LargScreen-16,
                           String(busvoltage) + " V " + String(current_mA) + " mA ", COLOR_NAVY);
            mylcd.setBackgroundColor(COLOR_BLACK);
            mylcd.setFont(Terminal6x8); // height 8
        }

        if (BoutonMoins.EventBouton() && BoutonMoins.LongPress == 1)
        {
            // Reset de l'enregistrement
            Serial.println("Reset");
            MonAxe.NbVal1 = 0;
            indiceLecture = 0;
            mylcd.drawText(0, LargScreen - 8, "Reset Line1      ", COLOR_GREENYELLOW);
            delay(500);
            MonAxe.ClearAll();
            mylcd.drawText(0, LargScreen - 8, "Wait for Press");
        }

    }

    if (BoutonStartStop.LongPress == 1)
    {
        if (BoutonStartStop.EtatBouton == 1)
        {
            mylcd.drawText(0, LargScreen - 8, "CURSOR           ");
            MonAxe.RefreshAllLines();
        }
        // mylcd.drawText(0, LargScreen - 8, "Cursor Analysis        ");
        bool FlagAnalyseCurseur = 1; // Tant que l'on déplace le curseur sans appuie long sur Start Stop
        int IndicePtCourant = MonAxe.NbVal1;
        int LastIndicePtCourant = IndicePtCourant;

        // passage en mode rafale quand la fonction Cursor est active
        BoutonMoins.FlagLongPressRafale=true;
        
        while (FlagAnalyseCurseur)
        {
            // en mode Curseur bouclé
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

                    // annulation du mode rafale
                    BoutonMoins.FlagLongPressRafale = false;
                    // BoutonStartStop.FlagLongPressRafale = false; incompatible avec le stop de la fonction


                    // Efface l'ancien curseur
                    MonAxe.drawCursor(LastIndicePtCourant, COLOR_BLACK);
                    MonAxe.drawLine1(COLOR_LIGHTBLUE);
                    mylcd.drawText(0, LargScreen - 8, "Wait for Press       ");
                    Serial.println("Mode Back WAIT");
                }
                else
                {
                    // Deplacement curseur +1
                    IndicePtCourant = min(IndicePtCourant + 1, MonAxe.NbVal1);
                    Serial.printf("Indice: %i\n", IndicePtCourant);
                    // Efface l'ancien curseur
                    MonAxe.drawCursor(LastIndicePtCourant, COLOR_BLACK);
                    MonAxe.drawLine1(COLOR_LIGHTBLUE);

                    MonAxe.drawCursor(IndicePtCourant, COLOR_LIGHTGREEN);
                    LastIndicePtCourant = IndicePtCourant;
                }
            }
            if (BoutonMoins.EventBouton())
            {
                // Deplacement curseur IndicePtCourant -1

                IndicePtCourant = max(IndicePtCourant - 1, 0);
                // Efface l'ancien curseur
                MonAxe.drawCursor(LastIndicePtCourant, COLOR_BLACK);
                MonAxe.drawLine1(COLOR_LIGHTBLUE);

                MonAxe.drawCursor(IndicePtCourant, COLOR_LIGHTGREEN);
                LastIndicePtCourant = IndicePtCourant;
            }
            led_record.Update();
        }
    }
    led_record.Update();
}