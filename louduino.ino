/*
* Louduino.ino  Make an LED light up according to sound level
*               measured by a microphone. Sound level is measured
*               by collecting a series of samples and calculating
*               the root mean square (rms). LED brightness is
*               simulated via pulse-width modulation (PWM).
*                
* Board:  Arduino UNO Rev 3.0 (ATMega238P)
* Input:  Electret MAX4466 mic+amp (VCC: 3.3V, GND, OUT: pin A_0)
* Output: MOSFET IRF520 via pin 11 (PWM), switching the LED circuit
* Analog reference: 3.3V via Arduino UNO 3.3V pin
* 
* Author: kf03w5t5741l (2019)
* 
 */
 
#define LED_PIN 11        // the PWM pin the LED is attached to
#define POT_PIN A1        // Potentiometer input pin
#define BUTTON_PIN 2
#define MIC_IN A0
#define USER_MODES 4
#define BOUNCE_DELAY 50
#define MIC_OFFSET 506
#define SAMPLE_RATE 50 // Number of samples to collect for rms
#define FADE_RATIO 10

byte brightness = 0;    // how bright the LED is
int fadeDefault = 5;
int fadeAmount = fadeDefault;    // how many points to fade the LED by
unsigned long peakTime = 0;   // measure time since last audio peak

byte userMode = 0;
byte userModeChanged = 1;

int lastButtonState = 0;
unsigned long lastButtonTime = 0; // millis() outputs an unsigned long

int flickerFrequency = 333;
byte flickerStatus = 0;
unsigned long flickerTime = 0; // millis() outputs an unsigned long

byte receivedByte = 0;

// the setup routine runs once when you press reset:
void setup() {
  analogReference(EXTERNAL);
  // declare pin 9 to be an output:
  pinMode(LED_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(MIC_IN, INPUT);
  pinMode(BUTTON_PIN, INPUT);

  // initialize serial communication at 115200 bits per second:
  Serial.begin(115200);
}

// the loop routine runs over and over again forever:
void loop() {

  // read sensitivity from user-controlled potentiometer
  int pot_value = analogRead(POT_PIN);
  if (pot_value <= 0) {
    pot_value = 1;
  }
  pot_value = map(pot_value, 0, 1023, 0, 255);
      
  if (userMode == 0) {
      digitalWrite(LED_BUILTIN, HIGH);
      if (userModeChanged == 1) {
        brightness = 0;
        fadeAmount = fadeDefault;
        userModeChanged = 0;
        Serial.println("Usermode changed.");
        Serial.println(userMode);
        Serial.println(userModeChanged);
        Serial.println(brightness);
        Serial.println(flickerTime);
        Serial.println(flickerFrequency);
      }
      
      analogWrite(LED_PIN, map(brightness, 0, 255, 0, pot_value));
      
      // change the brightness for next time through the loop:
      brightness = brightness + fadeAmount;

      // reverse the direction of the fading at the ends of the fade:
      if ((int) brightness <= 0 || (int) brightness >= 255) {
        fadeAmount = -fadeAmount;
      }
      delay(50);
  }

  else if (userMode == 1) {
    if (userModeChanged == 1) {
      flickerTime = 0;
      flickerFrequency = 333;
      brightness = 0;
      userModeChanged = 0;
      Serial.println("Usermode changed.");
      Serial.println(userMode);
      Serial.println(userModeChanged);
      Serial.println(brightness);
      Serial.println(flickerTime);
      Serial.println(flickerFrequency);
    }

    // flicker onboard LED
    if (millis() - flickerTime > flickerFrequency) {
      flickerStatus = flickerStatus ^ 1;  // XOR the current LED status with 1 to flip the bit
      digitalWrite(LED_BUILTIN, flickerStatus);
      flickerTime = millis();
    }

    int sensitivity = map(pot_value, 0, 255, 1, 100);

    // calculate the root mean square of a series of sound samples
    // formula: rms = sqrt((sample_1 ^2 + sample_2 ^2 ...) / samples)
    unsigned long sample_accumulator = 0;
    for (int counter = 0; counter < SAMPLE_RATE; counter++) {
      //sample_accumulator += pow(analogRead(MIC_IN) - MIC_OFFSET, 2);
      sample_accumulator += pow(analogRead(MIC_IN) - MIC_OFFSET, 2);
    }
    double rms = sqrt((double) sample_accumulator / SAMPLE_RATE);
  
    // calculate LED brightness (0-255) based on rms and send to LED
    int new_brightness = (rms + 1) * pow((float) sensitivity / 10, 2);
    //int brightness = rms * 10;
    if (new_brightness > 255) {
      new_brightness = 255;
    }
    else if (new_brightness < 50) {
      new_brightness = 0;
    }
    if (new_brightness >= brightness) {
      brightness = new_brightness;
    }
    else {
      brightness -= ((brightness + FADE_RATIO) / FADE_RATIO);
    }
    analogWrite(LED_PIN, brightness);
  }

  else if (userMode == 2) {
      if (userModeChanged == 1) {
        flickerTime = 0;
        flickerFrequency = 100;
        brightness = 0;
        userModeChanged = 0;
        Serial.println("Usermode changed.");
        Serial.println(userMode);
        Serial.println(userModeChanged);
        Serial.println(brightness);
        Serial.println(flickerTime);
        Serial.println(flickerFrequency);
      }

      // flicker onboard LED
      if (millis() - flickerTime > flickerFrequency) {
        flickerStatus = flickerStatus ^ 1;  // XOR the current LED status with 1 to flip the bit
        digitalWrite(LED_BUILTIN, flickerStatus);
        flickerTime = millis();
      }
      

      //updateBrightness(receivedByte); WORK IN PROGRESS - see below!
      brightness = receivedByte;
      analogWrite(LED_PIN, map(brightness, 0, 255, 0, pot_value));
  }

  else if (userMode == 3) {
      if (userModeChanged == 1) {
        digitalWrite(LED_BUILTIN, LOW);
        brightness = 0;
        analogWrite(LED_PIN, pot_value);
        delay(1);
        userModeChanged = 0;
        Serial.println("Usermode changed.");
        Serial.println(userMode);
        Serial.println(userModeChanged);
        Serial.println(brightness);
        Serial.println(flickerTime);
        Serial.println(flickerFrequency);
      }
      analogWrite(LED_PIN, pot_value);
  }

  else {
        Serial.println("Help! Default case executed for reasons known only to you-know-who.");
  }

  checkButton();
}

void checkButton() {
  int buttonState = digitalRead(BUTTON_PIN);
  
  if (buttonState != lastButtonState) {
    
    if ((millis() - lastButtonTime) > BOUNCE_DELAY) {
      
      if (buttonState == HIGH) {
        updateUserMode();
      }
      
    }
    
    lastButtonState = buttonState;
    lastButtonTime = millis();
  } 
}

void serialEvent() {
  
  if (userMode == 1) {
   Serial.readBytes(&receivedByte, 1); 
  }
  
}

void updateUserMode() {

  userMode = (userMode + 1) % USER_MODES;
  
  userModeChanged = 1;
  Serial.println("Changing usermode...");
  Serial.println(userMode);
  Serial.println(userModeChanged);
}