#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>


#define RST_PIN 9
#define SS_PIN 10

//Motor Pin Setup 
const int motorLatch = 12; 
const int motorClock = 4;
const int motorEnable = 7;
const int motorData   = 8;

// PWM Speed Control Pins
const int motorPwm_M3 = 6; // Right Wheel
const int motorPwm_M2 = 3; // Left Wheel (M2 uses Pin 3)

const int buzzerPin = A0;

// LCD Setup
LiquidCrystal_I2C lcd(0x27, 16, 2);  

// RFID pins
MFRC522 mfrc522(SS_PIN, RST_PIN);

// card code
String whiteCardUID = "F3405D28"; 
String blueKeyUID   = "13F09813";

// 
bool isSchoolZone = false;

void setup() {
  Serial.begin(9600);

  // Output pins
  pinMode(buzzerPin, OUTPUT);
  pinMode(motorClock, OUTPUT);
  pinMode(motorEnable, OUTPUT);
  pinMode(motorData, OUTPUT);
  
  pinMode(motorPwm_M3, OUTPUT);
  pinMode(motorPwm_M2, OUTPUT); //2 motor connected in m2 and m3 
  
  digitalWrite(motorEnable, LOW); // Turn Motor Driver ON

  // LCD Initialization
  lcd.init();
  lcd.backlight();
  
  // Default Message
  lcd.setCursor(0, 0);
  lcd.print("No School Zone");
  lcd.setCursor(0, 1);
  lcd.print("> Normal Speed <");
  
  beep(2); 

  // RFID Initialization
  SPI.begin();
  mfrc522.PCD_Init();

  // Start with Normal Speed (Both Motors)
  setMotors(255); 
}

void loop() {
  // Check for new card
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  // Read UID
  String scannedUID = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    scannedUID += String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
    scannedUID += String(mfrc522.uid.uidByte[i], HEX);
  }
  scannedUID.toUpperCase();
  
  Serial.print("UID: "); Serial.println(scannedUID);

  //logic
  //White Card -> Activate School Zone
  if (scannedUID == whiteCardUID) {
    Serial.println(">> SCHOOL ZONE ACTIVATED!");
    
    // Only update LCD if mode changes
    if (!isSchoolZone) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(" SCHOOL ZONE! ");
        lcd.setCursor(0, 1);
        lcd.print("Speed: SLOW");
        isSchoolZone = true;
    }
    
    beep(10); 
    setMotors(140); // Slow speed (M3 & M2)
  }
  
  //Blue Key -> Deactivate School Zone
  else if (scannedUID == blueKeyUID) {
    Serial.println(">> NORMAL ROAD!");
    
    // Only update LCD if mode changes
    if (isSchoolZone) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("No School Zone");
        lcd.setCursor(0, 1);
        lcd.print("Normal Speed");
        isSchoolZone = false;
    }
    
    beep(1); 
    setMotors(255); // Full Speed (M3 & M2)
  }
  
  //Unknown Card
  else {
    Serial.println(">> Unknown Card");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(" ACCESS DENIED! ");
    beep(2); 
    
    delay(2000); 
    
    // Restore display
    lcd.clear();
    if(isSchoolZone) {
        lcd.setCursor(0, 0); lcd.print(" SCHOOL ZONE! ");
        lcd.setCursor(0, 1); lcd.print("Speed: SLOW");
    } else {
        lcd.setCursor(0, 0); lcd.print("No School Zone");
        lcd.setCursor(0, 1); lcd.print("Normal Speed");
    }
  }

  delay(1000); 
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}

//Control M3 & M2
void setMotors(int speed) {
  
  analogWrite(motorPwm_M3, speed);
  analogWrite(motorPwm_M2, speed); // Speed for M2
  
  //Make Pin 12 OUTPUT
  pinMode(motorLatch, OUTPUT); 
  
  
  digitalWrite(motorLatch, LOW);
  shiftOut(motorData, motorClock, MSBFIRST, 130); 
  digitalWrite(motorLatch, HIGH);
  
  //Make Pin 12 input again
  pinMode(motorLatch, INPUT); 
}

// Buzzer Function
void beep(int times) {
  for(int i=0; i<times; i++) {
    digitalWrite(buzzerPin, HIGH);
    delay(100);
    digitalWrite(buzzerPin, LOW);
    delay(100);
  }
}