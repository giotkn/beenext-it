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
constexpr uint8_t VENT12V_PIN = A0;
constexpr uint8_t RGB_RED_PIN = 5;
constexpr uint8_t RGB_GREEN_PIN = 6;
constexpr uint8_t RGB_BLUE_PIN = 9;
constexpr uint8_t NEOPIXELS_PIN = 2;
constexpr uint8_t NEOPIXELS_SIZE = 4;
constexpr uint8_t STD_SPEED = 100; //Vitesse du ventilateur standard (sans contraintes)
constexpr uint8_t FAN_MIN_SPEED = 0;
constexpr uint8_t FAN_MAX_SPEED = 100;
constexpr uint8_t TEMP_ALERT_THRESHOLD = 71;
constexpr uint16_t LCD_REFRESH_INTERVAL = 1000;
constexpr float VOLTS_PER_STEP = 0.5 / 1023.0;
constexpr bool ECO_MODE = true;

struct rgb {
	uint8_t r, g, b;
};
constexpr rgb colors[6] {
				{0, 0, 175},    // Bleu clair - <15°C
    {0, 0, 255},    // Bleu foncé - <25°C
    {255, 255, 100},// Jaune - <40°C
    {255, 140, 0},  // Orange - <55°C
    {255, 0, 0},    // Rouge - <71°C
    {255, 0, 0}     // Rouge (clignotement) - Alerte
};

//Cache variables
uint16_t VATILATORS_SPEED = STD_SPEED;
float TEMPERATURE = -100;
unsigned long LCD_UPDATE = 0;

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

  //Setup bas niveau (contrôle des PWM nativement)
  cli(); //(cli => Clear Interrupts) Désactivation des interuptions, arrêt des programmes en arrière plan pour une configuration bas niveau

  //Attribution des LEDs aux différents PIN
  pinMode(RGB_RED_PIN, OUTPUT);
  pinMode(RGB_GREEN_PIN, OUTPUT);
  pinMode(RGB_BLUE_PIN, OUTPUT);
  pinMode(VENT12V_PIN, OUTPUT);

  TCCR0A = (1 << COM0A1) | (1 << COM0B1) | (1 << WGM01) | (1 << WGM00);
  TCCR0B = (1 << CS01) | (1 << CS00);

  TCCR1A = (1 << COM1A1) | (1 << WGM10);
  TCCR1B = (1 << WGM12) | (1 << CS11) | (1 << CS10);

  sei(); //Sauvegarde et fermeture des modifications

  //Configuration de l'écran LCD I2C
  lcd.init();
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);

  //Mode économie d'énergie (simplifié)
		if(ECO_MODE) {
    lcd.noBlacklight();
    Logger::log("Mode économie d'énergie activé'");
  } else {
    lcd.blacklight();
  }

  //Configuration de la bande NeoPixels RGB
  pixels.begin();
  pixels.show();
	
  //Attribution des "ventilateurs" (ici des servomoteur)
  //aux points d'attaches 13, 12 et 11. 
  vents[0].attach(13);
  vents[1].attach(12);
  vents[2].attach(11);

  setSpeed(STD_SPEED);
}

void loop() {
  float t = readTensor();

  if(t < 0) {
    return;
  }

		if(abs(t - TEMPERATURE) < 0.5) return;

	 if(t > TEMP_ALERT_THRESHOLD) {
				//Si la température dépasse 71, déclenche un clignotement
    for (int clignotement = 5; clignotement > 0; --clignotement) {
       setColor(255, 0, 0); //Rouge
      	delay(500);
      	setColor(0, 0, 0);
      	delay(500);
    }
    //Arrêt d'urgence
    Logger::warning("Boum");
				exit(0);
		}

  const rgb color =
    t <= 15 ? colors[0] :
    t <= 25 ? colors[1] :
    t <= 40 ? colors[2] :
    t <= 55 ? colors[3] :
                 colors[4];
	
		setColor(color[0], color[1], color[2]);
		setSpeed(t);

		if (millis() - LCD_UPDATE >= LCD_REFRESH_INTERVAL) {
				Logger::display(t);
		}

  delay(ECO_MODE ? 2000 : 1000);
}

class Logger {
public:
    static void log(const String& m) {
        Serial.println("[Log]" + m);
    }
    
    static void warning(const String& m) {
        Serial.println("[Warning] " + m);
    }

	   static void display(float temp) {
	       lcd.clear();
	       lcd.print(temp);
	   }
};


float readTensor() {
//Methode pour obtenir la valeur du point annalogique A0
  //A l'occurence, le capteur de chaleur
  int value = analogRead(TMP_PIN);
  
  //Arduino supporte des messages de 10 bits, pouvant donc produire 1024 valeurs différentes (0 à 1023)
  //U = (Valeur / 1023) x Ualimentation. Où Ualimentation = 5.0 V
  //équivalant à U = value * (Ualimentation / 1023.0)

  float voltage = value * VOLTS_PER_STEP;
  return (voltage - 0.5) * 100.0;
}

void setSpeed(float temp) {
    const int speed = constrains(map(temp, 0, TEMP_ALERT_THRESHOLD, 0, 180), FAN_MIN_SPEED, FAN_MAX_SPEED);

    for (auto& vent : vents) {
        vent.write(speed);
    }
}

void setColor(int red, int green, int blue) {
  //Optimisation (native) https://docs.arduino.cc/tutorials/generic/secrets-of-arduino-pwm/
  OCR0B = red;
  OCR0A = green;
  OCR1A = blue; 

  setPixelsColor(red, green, blue);
}

void setPixelsColor(int red, int green, int blue) {
  const int color = pixels.Color(red, green, blue);
  for (int i = 0; NEOPIXELS_SIZE > i; ++i)
				pixels.setPixelColor(i, color);
  pixels.show();
}