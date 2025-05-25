// C++ code



// Notes : 
//Ce code à été conçu pour être compatible avec Tinkercad
//La LED RGB doit être configuré sur RCBG
//L'écran l2c sur pcf8574-based

#include <Servo.h>
#include <LiquidCrystal.h>
#include <LiquidCrystal_I2C.h>

#define TMP_PIN A0

#define RGB_RED_PIN 5
#define RGB_GREEN_PIN 6
#define RGB_BLUE_PIN 7
#define read_tensor() (analogRead(TMP_PIN))

LiquidCrystal_I2C lcd(32,16,2);

Servo vent_1;
Servo vent_2;
Servo vent_3;

int secondes= 0; // Début du programme
int vitesse_std = 100; // Vitesse du ventilateur standard (sans contraintes) 
int vitesse_ventilateur = vitesse_std;

class Logger {
public:
  void log() {
    
  }
};

void setup()
{
  //Mise en marche des ventilateurs
  //Mise en marche des capteurs thermiques
  	
  //Ici, ce n'est pas nécessaire :
  //Les ventilateurs/capteurs thermiques sont mis en marche automatiquement
  
  //Configuration de l'écran LCD I2C
  lcd.begin(16, 2);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  
  pinMode(RGB_RED_PIN, OUTPUT);
  pinMode(RGB_GREEN_PIN, OUTPUT);
  pinMode(RGB_BLUE_PIN, OUTPUT);
  
  //Attribution des "ventilateurs" (ici des servomoteur)
  //aux points d'attaches 13, 12 et 11. 
  vent_1.attach(13);
  vent_2.attach(12);
  vent_3.attach(11);
  
  Serial.begin(9600);
}

void handle_temperature(float temperature)
{
  if(temperature < 0) {
    return;
  }
  
  if(temperature <= 15.0) {
    setColor(0, 0, 175); //Bleu clair
  
  } else if(temperature <= 25.0) {
    setColor(0, 0, 255); //Bleu foncé
  
  } else if(temperature <= 40.0) {
   	setColor(255, 255, 100); //Jaune
  
  } else if(temperature <= 55.0) {
    setColor(255, 140, 0); //Orange
  
  } else if(temperature <= 71) {
    setColor(255, 0, 0); //Rouge
  
  } else {
   	//Si la température dépasse 71, déclenche un clignotement
    for (int clignotement = 5; clignotement > 0; --clignotement) {
        setColor(255, 0, 0); //Rouge
      	delay(500);
      	setColor(0, 0, 0);
      	delay(500);
    }

    delay(5000);
    
    //Arrêt d'urgence
    Serial.print("Boum");
	//exit(0);
  }
}

void logger
{
  return new class Logger()
}

void setVitesseVentilateurs() {
  
}

void setColor(int red, int green, int blue) {
    analogWrite(RGB_RED_PIN, red);
    analogWrite(RGB_GREEN_PIN, green);
    analogWrite(RGB_BLUE_PIN, blue);
}

void loop()
{	
  //Methode pour obtenir la valeur du point annalogique A0
  //A l'occurence, le capteur de chaleur
  int value = read_tensor();
  
  //Arduino supporte des messages de 10 bits
  //Pouvant donc produire 1024 valeurs différentes (0 à 1023)
  
  //Pourquoi 5?
  //Arduino fonctionne avec une alimentation de 5V
  
  //U = (Valeur / 1023) x Ualimentation. Où Ualimentation = 5.0 V
  //équivalant à U = value * (Ualimentation / 1023.0)
  float voltage = value * (5.0 / 1023.0); 
  float temperature = (voltage - 0.5) * 100.0;

  lcd.clear();
  lcd.print(temperature);
  
  handle_temperature(temperature);
  
  delay(1000);
  secondes += 1;
}
