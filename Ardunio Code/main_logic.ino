
// import libraries
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>

#include <SoftwareSerial.h>
#include <stdlib.h>
#include <String.h>

// Defining inputs and outputs //

// for servo motor //
Servo myservo;

// for ser //
SoftwareSerial ser(10, 11); // RX, TX

// for RFID //
#define SS_PIN 10
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance.

// For LCD //
LiquidCrystal_I2C lcd(0x27, 16, 2);

// pin for the buzzer //
const int buzzerPin = 8;

// pin for servo motor //
const int servoPin = 9;

// ultrasonic pins //
const int Trig = 13; // Trig connected to pin 13
const int Echo = 12; // Echo connected to pin 12

void setup()
{
    // ===========================================//
    // RFID Setup //
    SPI.begin();        // Init SPI bus
    mfrc522.PCD_Init(); // Init MFRC522 card
    // ===========================================//
    // LCD Setup //
    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    // ===========================================//
    // Buzzer Setup //
    pinMode(buzzerPin, OUTPUT);
    // ===========================================//
    // servo motor Setup //
    myservo.attach(servoPin);
    // ===========================================//
    // ultrasonic setup Setup //
    pinMode(Trig, OUTPUT); // UNO's output, ranger's input
    pinMode(Echo, INPUT);  // UNO's input, ranger's output
    // ===========================================//
    Serial.begin(9600);
    while (!Serial)
    {
    }
    Serial.println("Starting...");
    ser.begin(9600);
}

// ========================= Below are functions for the individual sensors =========================//
/// Buzzer Code //
void Buzzer(int buzzerPin)
{
    tone(buzzerPin, 2000); // play a tone at 2000 Hz
    delay(1000);           // wait for 1 second
    noTone(buzzerPin);     // stop the tone
    delay(1000);           // wait for 1 second
}

/// RFID Code //
void RFID()
{
    // Look for new cards
    if (!mfrc522.PICC_IsNewCardPresent())
    {
        return;
    }

    // Select one of the cards
    if (!mfrc522.PICC_ReadCardSerial())
    {
        return;
    }

    // Dump debug info about the card. PICC_HaltA() is automatically called.
    mfrc522.PICC_DumpToSerial(&(mfrc522.uid));
}

// servo motor Code //
void servo_motor()
{
    myservo.write(0);
    delay(1000);
    myservo.write(90);
    delay(1000);
    myservo.write(180);
    delay(1000);
}

// Ultrasonic ranger code //
void ultrasonic_ranger()
{
    long duration, cm;

    // 1. produce a 5us HIGH pulse in Trig
    digitalWrite(Trig, LOW);
    delayMicroseconds(2);
    digitalWrite(Trig, HIGH);
    delayMicroseconds(5);
    digitalWrite(Trig, LOW);

    // 2. measure the duration of the HIGH pulse in Echo
    //      & every 58 us is an obstacle distance of 1 cm
    duration = pulseIn(Echo, HIGH);
    cm = duration / 58;

    // 3. display the obstacle distance in serial monitor
    Serial.print(cm);
    Serial.print("cm");
    Serial.println();
    delay(500);
}

// ========================= Above are functions for the individual sensors =========================//


// ========================= Below is the main Loop =========================//
void loop()
{
    
    lcd.print("Hello, please scan your card!");
    delay(1000);
    lcd.begin(16,2); // reset the LCD //
    delay(10000000);
}
