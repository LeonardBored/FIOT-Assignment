
// import libraries
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>

#include <SoftwareSerial.h>
#include <stdlib.h>
#include <String.h>

// import header file for thinkspeak + think tweet //
#include "thinkspeak.h"

// ==================== Defining sensors  ====================//

// for ser //
// SoftwareSerial ser(10, 11); // RX, TX
SoftwareSerial ser(2, 3); // RX, TX


// for RFID //
#define SS_PIN 10
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance.

// For LCD //
LiquidCrystal_I2C lcd(0x27, 16, 2);

// pin for the buzzer //
const int buzzerPin = 8;

// pin for servo motor //
Servo myservo;
const int servoPin = 9;

// ultrasonic pins //
const int Trig = 6; // Trig connected to pin 6
const int Echo = 7; // Echo connected to pin 7

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
void Buzzer(int buzzerPin, String condition)
{
    if (condition == "Wrong RFID")
    {
        for (int i = 0; i < 3; i++)
        {
            tone(buzzerPin, 262); // play a C note
            delay(100);           // wait for 0.5 seconds
            noTone(buzzerPin);    // stop the tone
            delay(100);           // wait for 1 second
        }
    }
    else if (condition == "Correct RFID")
    {
        tone(buzzerPin, 440, 500); // generate a tone of frequency 440Hz for 200ms
        delay(200);                // delay for 200ms
    }
}

/// RFID Code //
String RFID()
{
    // Look for new cards
    if (!mfrc522.PICC_IsNewCardPresent())
    {
        return ".";
    }

    // Select one of the cards
    if (!mfrc522.PICC_ReadCardSerial())
    {
        return ".";
    }

    // Print the UID of the card
    Serial.print("UID: ");
    String uid = "";
    for (byte i = 0; i < mfrc522.uid.size; i++)
    {
        // putting in uid string //
        uid += String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
        uid += String(mfrc522.uid.uidByte[i], HEX);

        Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
        Serial.print(mfrc522.uid.uidByte[i], HEX);
    }
    Serial.println(uid);

    // check uid //
    // other card uid (blue one) is  AC 31 A9 33 ac 31 a9 33 //
    if (uid == " 45 a6 f5 2a")
    {
        return String(uid);
    }
    else
    { // for invalid RFID //
        return "false";
    }

    // Dump debug info about the card. PICC_HaltA() is automatically called.
    mfrc522.PICC_DumpToSerial(&(mfrc522.uid));
}

// servo motor Code //
void servo_motor()
{
    myservo.write(0);
    delay(10000);      // after 10s, door will close //
    myservo.write(90); // sets the servo to the default position
}

// Ultrasonic ranger code //
long ultrasonic_ranger()
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
    return cm;
}

// ========================= Above are functions for the individual sensors =========================//

// ========================= Below is the main Loop =========================//
void loop()
{

    // first, use Ultrasonic sensor to detect whether there is an object within 1m //
    long dist = ultrasonic_ranger();
    // storing of number of wrong attempts //
    int wrongCount = 0;
    // storing of inactivity as a count //
    int inactiveCount = 0;

    while (dist > 0 && dist <= 20 && wrongCount < 5 && inactiveCount < 10)
    { // when ultrasonic detects an object from it with distance less than or = to 1m, it will on LCD
        lcd.setCursor(0, 0);
        lcd.begin(16, 2); // reset the LCD //
        delay(500);
        lcd.print("Welcome!");
        lcd.setCursor(0, 1);
        lcd.print("Turning on RFID...");
        delay(1000);
        lcd.setCursor(0, 0);
        lcd.begin(16, 2); // reset the LCD //
        delay(500);
        lcd.print("RFID ready!");
        lcd.setCursor(0, 1);
        lcd.print("Scan card now...");
        delay(3000);

        String validRFID = RFID();
        if (validRFID != "false" && validRFID != ".")
        {                     // valid RFID card = Unlock DOOR //
            lcd.begin(16, 2); // reset the LCD //
            delay(500);
            lcd.print("Valid Card!");
            Buzzer(buzzerPin, "Correct RFID"); // sound the buzzer for correct //
            servo_motor();                     // opening of door //
            // send thinkspeak and thinktweet //
            // validRFID contains the uid of the RFID //
            postThinkSpeak(2); // upload for 2 fields //
            
            // Break out of loop //
            delay(300);
            lcd.begin(16, 2); // reset the LCD //
            lcd.print("Offing LCD...");
            delay(5000);
            lcd.begin(16, 2); // reset the LCD //
            break;
        }
        else if (validRFID == "false")
        { // invalid RFID //
            lcd.setCursor(0, 0);
            lcd.begin(16, 2); // reset the LCD //
            delay(500);
            lcd.print("Invalid Card!");
            lcd.setCursor(0, 1);
            lcd.print("Please Try again!");
            delay(500);
            Buzzer(buzzerPin, "Wrong RFID"); // sound the buzzer for incorrect //
            wrongCount += 1;
        }
        else // for the case where no RFId scan //
        {
            delay(100);
            inactiveCount += 1;
            lcd.begin(16, 2); // reset the LCD //
        }
    }

    if (wrongCount == 5)
    { // after 5 wrong attempts, door will be locked for 10 min and owner will be notified //
      // first lock door for 10 min //
      // for this demo, door will be locked for 10s //
        lcd.setCursor(0, 0);
        lcd.begin(16, 2); // reset the LCD //
        lcd.print("DOOR IS LOCKED");
        lcd.setCursor(0, 1);
        lcd.print("for 10 minutes...");
        delay(15000);

        // notifying of owner via app by thinkspeak //
    }
}
