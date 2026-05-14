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
auto led_record = JLed(32).MaxBrightness(58).On().Forever();

// test de 
#include <ShiftIn.h>

// Déclarer le décalage : 1 seul boîtier 74HC165 (8 bits)
// Paramètres : ShiftIn<nombre_de_puces>(Broche_PL, Broche_CP, Broche_Q7)
// En utilisant vos broches HSPI : PL=13, CP/Clock=14, Q7/MISO=12

ShiftIn<1> shiftRegister;

void setup()
{
    // Initialisation de la communication série à 115200 bauds
    Serial.begin(115200);
    while (!Serial)
    {
    } // Attend l'ouverture du moniteur série

    // Ordre des arguments : begin(pLoadPin, clockENPin, dataPin, clockPin)
    shiftRegister.begin(5, 17, 19, 18);

    Serial.println("--- Test 74HC165 avec Librairie ShiftIn ---");


    led_record.Stop();
}

void loop()
{
    led_record.On();
    // 1. Lire physiquement le registre à décalage
    shiftRegister.read();

    // 3. Affichage dans le moniteur série de D7 à D0
    Serial.print("État des boutons (D7->D0) : ");
    for (int i = 0; i <= 7; i++)
    {
        Serial.print(1-shiftRegister.state(i));
    }
    Serial.println();

    // 4. Attente de 1000ms (1 seconde)
    delay(1000);
    led_record.Off();
}