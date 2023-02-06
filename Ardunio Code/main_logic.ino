#include <ArduinoJson.h>

// import libraries
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>
#include <SoftwareSerial.h>

// ====== import from header file ====== //
// #include "thinkspeak.h"
// ==================== Defining sensors  ====================//

// for ser //
SoftwareSerial ser(2, 3); // RX, TX

// for RFID //
#define SS_PIN 10
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance.

// For LCD //
LiquidCrystal_I2C lcd(0x27, 16, 2);

// pin for the buzzer //
static int buzzerPin = 8;

// pin for servo motor //
Servo myservo;
static int servoPin = 9;

// ultrasonic pins //
static int Trig = 6; // Trig connected to pin 6
static int Echo = 7; // Echo connected to pin 7

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
    Serial.println(F("Starting..."));
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
    Serial.print(F("Read RFID UID: "));
    String uid = "";
    for (byte i = 0; i < mfrc522.uid.size; i++)
    {
        // putting in uid string //
        uid += String(mfrc522.uid.uidByte[i] < 0x10 ? F(" 0") : F(" "));
        uid += String(mfrc522.uid.uidByte[i], HEX);

        Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? F(" 0") : F(" "));
        Serial.print(mfrc522.uid.uidByte[i], HEX);
    }
    Serial.println(uid);

    // check uid //
    // other card uid (blue one) is  AC 31 A9 33 ac 31 a9 33 //
    if (uid == " 45 a6 f5 2a")
    {
        uid = "45";
        return uid;
    }
    // else if (uid == " AC 31 A9 33 ac 31 a9 33")
    // {
    //     uid = "31";
    //     return uid;
    // }
    else
    { // for invalid RFID //
        return "false";
    }
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
    if (cm <= 20)
    {
        Serial.print(F("Distance Detected: "));
        Serial.print(cm);
        Serial.print(F(" cm"));
        Serial.println();
        delay(500);
    }
    else
    {
        Serial.print(F("No nearby objects detected..."));
        Serial.println(F("Detecting any remote operations..."));
    }
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
        lcd.print(F("Welcome!"));
        lcd.setCursor(0, 1);
        lcd.print(F("Turning on RFID..."));
        delay(1000);
        lcd.setCursor(0, 0);
        lcd.begin(16, 2); // reset the LCD //
        lcd.print(F("RFID ready!"));
        lcd.setCursor(0, 1);
        lcd.print(F("Scan card now..."));

        String validRFID = RFID();
        if (validRFID != "false" && validRFID != ".")
        {                     // valid RFID card = Unlock DOOR //
            lcd.begin(16, 2); // reset the LCD //
            delay(500);
            lcd.print(F("Valid Card..."));
            lcd.setCursor(0, 1);
            lcd.print(F("Welcome Home!"));
            Buzzer(buzzerPin, "Correct RFID"); // sound the buzzer for correct //
            servo_motor();                     // opening of door //
            lcd.begin(16, 2);                  // reset the LCD //
            lcd.print(F("Offing LCD..."));

            // send thinkspeak and thinktweet //
            // validRFID contains the uid of the RFID //
            tcp_connect();
            postThinkTweet(F("Door Open")); // send to twitter, informing the owner //
            tcp_connect();
            postThinkSpeak_OPEN(validRFID);
            // Break out of loop //
            lcd.begin(16, 2); // reset the LCD //
            break;
        }
        else if (validRFID == "false")
        { // invalid RFID //
            lcd.setCursor(0, 0);
            lcd.begin(16, 2); // reset the LCD //
            delay(500);
            lcd.print(F("Invalid Card!"));
            lcd.setCursor(0, 1);
            lcd.print(F("Please Try again!"));
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
        lcd.print(F("DOOR IS LOCKED"));
        lcd.setCursor(0, 1);
        lcd.print(F("for 10 minutes.."));
        // notifying of owner via app by thinkspeak //
        tcp_connect();
        postThinkSpeak_FAIL(); // upload to thinkspeak that door has been opened //
        tcp_connect();
        postThinkTweet(F("Incorrect attempts warning ")); // send to twitter, informing the owner //

        delay(2000);      // 2 sec waiting time + upload time //
        lcd.begin(16, 2); // reset the LCD //
    }

    // Read from thinkspeak to check if there any remote controls being activated //
    tcp_connect();
    String remoteOperation = readThinkSpeak();
    // for remote operation: //
    if (remoteOperation == "remote open door")
    {
        lcd.begin(16, 2); // reset the LCD //
        lcd.print(F("Welcome Home!"));
        Buzzer(buzzerPin, "Correct RFID"); // sound the buzzer for correct //
        servo_motor();                     // opening of door //
        // post back to thinkspeak field 4 a value of 0 //
        lcd.begin(16, 2); // reset the LCD //
        tcp_connect();
        remoteReplyThinkspeak();
        tcp_connect();
        postThinkTweet("Door Open");
        tcp_connect();
        postThinkSpeak_OPEN("Remote");
    }
}

// ========================= Below are functions used to upload / read to thinkspeak and thinktweet =========================//

#define DEBUG true

// =====================================================//
// Thinkspeak and thinktweet setup //
const char *thingSpeak = "api.thingspeak.com";

// Setup for thinkspeak //

// Thingspeak Function //
// This function updates fields 1 & 2 in thingspeak, allowing thingspeak to know that the door was open by what RFID //
// it gets a unique input, for sending the RFID number over to thinkspeak //
void postThinkSpeak_OPEN(String uniqueInput)
{
    // Crafting of get request str //
    String apiWriteKey = F("Q3X0NH6PSG5W6DO1");
    String getStr = F("/update?api_key=");
    getStr += String(apiWriteKey);
    getStr += "&field1=1";
    getStr += "&field2="; // this will send RFID card tag //
    getStr += uniqueInput;
    // Crafting of get request str //
    String payload = F("GET https://");
    payload += thingSpeak + getStr;
    payload += "\r\n";
    Serial.println();
    Serial.print(payload);
    // Send data length & GET string
    ser.print("AT+CIPSEND=");
    ser.println(payload.length());
    Serial.print(F("AT+CIPSEND="));
    Serial.println(payload.length());
    delay(500);
    if (ser.find(">"))
    {
        Serial.print(F(">"));
        sendData(payload, 1000, DEBUG);
    }

    // Close connection, wait a while before repeating...
    sendData("AT+CIPCLOSE", 10000, DEBUG); // thingspeak needs 15 sec delay between updates
}

// This function is to post to thinkspeak for after 5 incorrect attempts //
void postThinkSpeak_FAIL()
{
    // Crafting of get request str //
    String apiWriteKey = F("Q3X0NH6PSG5W6DO1");
    String getStr = F("/update?api_key=");
    getStr += String(apiWriteKey);
    getStr += F("&field3=1");
    // Crafting of get request str //
    String payload = F("GET https://");
    payload += thingSpeak + getStr;
    payload += "\r\n";
    // Send data length & GET string
    ser.print("AT+CIPSEND=");
    ser.println(payload.length());
    Serial.print(F("AT+CIPSEND="));
    Serial.println(payload.length());
    delay(500);
    if (ser.find(">"))
    {
        Serial.print(F(">"));
        sendData(payload, 1000, DEBUG);
    }

    // Close connection, wait a while before repeating...
    sendData("AT+CIPCLOSE", 10000, DEBUG); // thingspeak needs 15 sec delay between updates
}

// think tweet function//
// Conditions : Door Open OR Incorrect attempts warning //
// This will post to twitter, notifying the owner //
void postThinkTweet(String condition)
{
    String twitterAPIKey = "5YIAWRJTIVRJY6OH";
    String tweetURI = "/apps/thingtweet/1/statuses/update";

    // POST https://api.thingspeak.com/apps/thingtweet/1/statuses/update
    // api_key=Y6OS7WNU7BSQDADR
    // status=I just posted this from my thing! //

    // Crafting of post request str //
    String statusMessage = tempAlertTweet(condition, twitterAPIKey, tweetURI);
    // Example Get request  https://api.thingspeak.com/apps/thingtweet/1/statuses/update?api_key=XXXXXXXXXXXXXXXX&status=HeyWorld //
    String payload = F("GET https://api.thingspeak.com/apps/thingtweet/1/statuses/update?");
    payload += statusMessage;
    payload += "\r\n";
    Serial.println(payload);
    // Send data length & GET string
    ser.print("AT+CIPSEND=");
    ser.println(payload.length());
    Serial.print(F("AT+CIPSEND="));
    Serial.println(payload.length());
    delay(500);
    if (ser.find(">"))
    {
        Serial.print(F(">"));
        sendData(payload, 1000, DEBUG);
    }

    // Close connection, wait a while before repeating...
    sendData("AT+CIPCLOSE", 10000, DEBUG); // thingspeak needs 15 sec delay between updates
}

// This function will read the selected field: 4 from thinkspeak //
// This fields are responsible for doing remote unlock //
String readThinkSpeak()
{
    // Crafting of get request str //
    String payload = F("GET https://api.thingspeak.com/channels/2008441/fields/4.json?api_key=N0KL9SHVN3B1YOD7&results=1");
    payload += F("\r\n");
    Serial.println(payload);
    // Send data length & GET string
    ser.print("AT+CIPSEND=");
    ser.println(payload.length());
    Serial.print(F("AT+CIPSEND="));
    Serial.println(payload.length());
    delay(500);

    String responseList = F("");

    if (ser.find(">"))
    {
        Serial.print(F(">"));
        Serial.print(F(""));
        responseList = sendData(payload, 1000, DEBUG);
    }
    // Close connection, wait a while before repeating...
    sendData("AT+CIPCLOSE", 2000, DEBUG);

    // Process the JSON //
    Serial.println(responseList);

    String searchString = "\"field4\":\"1\"";
    int searchIndex = responseList.indexOf(searchString);
    if (searchIndex != -1)
    {
        Serial.println(F("String Found!"));
        return "remote open door";
    }
    return "";
}

// After execution of remote opertaions, this function will send a value to the respecitive field in thinkspeak to
// mean that the function has been executed. When a new remote operation is called, thinkspeak will have a value of 1 in the field //
void remoteReplyThinkspeak()
{
    // Crafting of get request str //
    String apiWriteKey = F("Q3X0NH6PSG5W6DO1");
    String getStr = F("/update?api_key=");
    getStr += String(apiWriteKey);
    getStr += F("&field4=0");
    // Crafting of get request str //
    String payload = F("GET https://");
    payload += thingSpeak + getStr;
    payload += "\r\n";
    // Send data length & GET string
    ser.print("AT+CIPSEND=");
    ser.println(payload.length());
    Serial.print(F("AT+CIPSEND="));
    Serial.println(payload.length());
    delay(500);
    if (ser.find(">"))
    {
        Serial.print(F(">"));
        sendData(payload, 1000, DEBUG);
    }

    // Close connection, wait a while before repeating...
    sendData("AT+CIPCLOSE", 1000, DEBUG); // thingspeak needs 15 sec delay between updates
}

// =============== functions below used to connect to thinkspeak to retrieve data ================//

// TCP connection
void tcp_connect()
{
    sendData("AT+RST\r\n", 2000, DEBUG);
    sendData("AT+CWMODE=1\r\n", 2000, DEBUG);
    sendData("AT+CWJAP=\"T923WIFI\",\"abc1234567\"\r\n", 4000, DEBUG);
    // ****************************************************************** Change these!
    sendData("AT+CIPMUX=0\r\n", 2000, DEBUG);

    // Make TCP connection
    String cmd = F("AT+CIPSTART=\"TCP\",\"");
    cmd += F("184.106.153.149"); // Thingspeak.com's IP address
    // ****************************************************************** Change this!
    cmd += F("\",80\r\n");
    sendData(cmd, 2000, DEBUG);
}

// tweet message + api key + door has been unlocked
String tempAlertTweet(String condition, String twitterAPIKey, String tweetURI)
{
    // tweet message
    String tempAlert = F("");
    tempAlert += "api_key=";
    tempAlert += String(twitterAPIKey);
    if (condition == "Door Open")
    {
        // String tempAlert = "api_key=" + String(twitterAPIKey) + "&status=Door has been Open! [" + String(millis()) + "]";
        tempAlert += "&status=Door-has-been-Open!==[";
        tempAlert += String(millis());
        tempAlert += "]";
        return tempAlert;
    }
    else if (condition == "Incorrect attempts warning")
    {
        // String tempAlert = "api_key=" + String(twitterAPIKey) + "&[WARNING] There has been 5 incorrect attempts made to open the door. [" + String(millis()) + "]";
        tempAlert += "&status=[WARNING]-There-has-been-5-incorrect-attempts-made-to-open-the-door!==[";
        tempAlert += String(millis());
        tempAlert += "]'";
        return tempAlert;
    }
}

String sendData(String command, const int timeout, boolean debug)
{
    String response = F("");
    ser.print(command);
    long int time = millis();

    while ((time + timeout) > millis())
    {
        while (ser.available())
        {
            // "Construct" response from ESP01 as follows
            // - this is to be displayed on Serial Monitor.
            char c = ser.read(); // read the next character.
            response += c;
        }
    }

    if (debug)
    {
        Serial.println(response);
    }

    return (response);
}
