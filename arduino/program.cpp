// C++ code

//Notes (Tinkercad): 
//La LED RGB doit être configuré sur RCBG
//L'écran l2c sur pcf8574-based

#include <Servo.h>
#include <LiquidCrystal.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_NeoPixel.h>
#include <math.h>

constexpr uint8_t TMP_PIN = A0;
constexpr uint8_t RGB_RED_PIN = 5;
constexpr uint8_t RGB_GREEN_PIN = 6;
constexpr uint8_t RGB_BLUE_PIN = 7;
constexpr uint8_t NEOPIXELS_PIN = 2;
constexpr uint8_t NEOPIXELS_SIZE = 4;

constexpr uint8_t STD_SPEED = 100; //Vitesse du ventilateur standard (sans contraintes)
constexpr uint8_t FAN_MIN_SPEED = 0;
constexpr uint8_t FAN_MAX_SPEED = 0;

constexpr float VOLTS_PER_STEP = 0.5 / 1023.0;
constexpr rgb colors[6] {
	{0, 0, 175},    // Bleu clair - <15°C
    {0, 0, 255},    // Bleu foncé - <25°C
    {255, 255, 100},// Jaune - <40°C
    {255, 140, 0},  // Orange - <55°C
    {255, 0, 0},    // Rouge - <71°C
    {255, 0, 0}     // Alerte
};

struct rgb {
	uint8_t r, g, b;
};

bool ECO_MODE = 0;
uint16_t VATILATORS_SPEED = STD_SPEED; //Fan = 

Adafruit_NeoPixel pixels(NEOPIXELS_SIZE, NEOPIXELS_PIN, NEO_RGB + NEO_KHZ800);
LiquidCrystal_I2C lcd(0x20,16,2);
Servo vents[3];

void setup() {
  //Mise en marche des ventilateurs
  //Mise en marche des capteurs thermiques
  	
  //Ici, ce n'est pas nécessaire :
  //Les ventilateurs/capteurs thermiques sont mis en marche automatiquement
  
  Serial.begin(115200);
  while(!Serial);

  //Configuration de l'écran LCD I2C
  lcd.init();
  lcd.begin(16, 2);
  lcd.backlight();
  
  //Configuration de la bande NeoPixels
  pixels.begin();
  pixels.show();

  //Attribution dew LEDs aux différents PIN
  pinMode(RGB_RED_PIN, OUTPUT);
  pinMode(RGB_GREEN_PIN, OUTPUT);
  pinMode(RGB_BLUE_PIN, OUTPUT);
  
  //Attribution des "ventilateurs" (ici des servomoteur)
  //aux points d'attaches 13, 12 et 11. 
  vents[0].attach(13);
  vents[1].attach(12);
  vents[2].attach(11);
}

void loop() {	
  float temperature = readTensor();
  handleTemperature(temperature);
  
  lcd.clear();
  lcd.print(temperature);

  delay(1000);
}

float readTensor() {
//Methode pour obtenir la valeur du point annalogique A0
  //A l'occurence, le capteur de chaleur
  int value = analogRead(TMP_PIN);
  
  //Arduino supporte des messages de 10 bits
  //Pouvant donc produire 1024 valeurs différentes (0 à 1023)
  //Arduino fonctionne avec une alimentation de 5V
  
  //U = (Valeur / 1023) x Ualimentation. Où Ualimentation = 5.0 V
  //équivalant à U = value * (Ualimentation / 1023.0)

  float voltage = value * VOLTS_PER_STEP;
  return (voltage - 0.5) * 100.0;
}

void handleTemperature(float temperature)
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

void setSpeed(float temp) {
    int speed = map(temp, 0, 70, 0, 180);

    for (auto& vent : vents) {
        vent.write(speed);
    }
}

void setColor(int red, int green, int blue) {
  setPixelsColor(red, green, blue);

  //TODO: Optimisation (native) https://docs.arduino.cc/tutorials/generic/secrets-of-arduino-pwm/
  analogWrite(RGB_RED_PIN, red);
  analogWrite(RGB_GREEN_PIN, green);
  analogWrite(RGB_BLUE_PIN, blue);
}

void setPixelsColor(int red, int green, int blue) {
  for (int i = 0; NEOPIXELS_SIZE > i; ++i) {
    const int color = pixels.Color(red, green, blue);
  	pixels.setPixelColor(i, color);
  }
  pixels.show();
}