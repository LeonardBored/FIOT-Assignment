#include <SoftwareSerial.h>
// #include <stdlib.h>
// #include <String.h>

String reset = "AT+RST";
String thingSpeak = "api.thingspeak.com";
String urlEncoded = "application/x-www-form-urlencoded";
String cmd;

// replace with your Thingspeak channel's API key!!!
String apiWriteKey = "2W54NENKHMAMNK71";
String apiReadKey = "N0KL9SHVN3B1YOD7";

String twitterAPIKey = "Y6OS7WNU7BSQDADR";
String tweetURI = "/apps/thingtweet/1/statuses/update";

// UNO's D10 connected to ESP's TX
// UNO's D11 connected to ESP's RX via resistor network

SoftwareSerial ser(10, 11); // RX, TX
// LED at D13
int ledPin = 5;
// Potentiometer at A0
int potPin = 0;
int buzPin = 8;
int buttonPin = 7;

void setup()
{
    pinMode(ledPin, OUTPUT);   // LED
    pinMode(buttonPin, INPUT); // Button input
    pinMode(potPin, INPUT);    // potentialmeter
    pinMode(buzPin, OUTPUT);   // Buzzer

    Serial.begin(9600);
    while (!Serial)
    {
    }
    Serial.println("Starting...");
    ser.begin(9600);
}

void loop()
{
    // Read potentiometer value
    int sensorValue = analogRead(A0);             // 10 bit result: 0 - 1023
    float voltage = sensorValue * (5.0 / 1023.0); // 0V - 5V
    String temp = String(voltage);                // convert to string
    Serial.println("Voltage: " + temp);

    String getStr = "/update?api_key=" + apiWriteKey + "&field1=" + temp;

    // when button pressed, led and buzzer on //
    if (digitalRead(buttonPin) == HIGH)
    {
        digitalWrite(ledPin, HIGH);
        digitalWrite(buzPin, HIGH);
        delay(1000);
        digitalWrite(ledPin, LOW);
        digitalWrite(buzPin, LOW);
        getStr += "&field2=1";
        getStr += "&field3=1";
        // think tweet//
        // create a POST request header structure
        String payload = postRequest(tweetURI, thingSpeak, urlEncoded, tempAlertTweet());

        // TCP connect
        tcp_connect(cmd);

        // send data length
        if (postStatus(cmd, payload))
        {
            // thingspeak needs at least 15 sec delay between updates
            delay(5000);
        }
        else
        {
            ser.println("AT+CIPCLOSE");
            // alert user
            Serial.println("AT+CIPCLOSE");
        }
        delay(5000);
    }
    else
    {
        getStr += "&field2=0";
        getStr += "&field3=0";
    }
    digitalWrite(ledPin, LOW);
    digitalWrite(buzPin, LOW);

    // for updating thinkspeak //
    String payload2 = getRequest(thingSpeak + getStr, thingSpeak);

        // TCP connect
        tcp_connect(cmd);

        // send data length
        if (postStatus(cmd, payload2))
        {
            // thingspeak needs at least 15 sec delay between updates
            delay(5000);
        }
        else
        {
            ser.println("AT+CIPCLOSE");
            // alert user
            Serial.println("AT+CIPCLOSE");
        }
        delay(5000);

}

// TCP connection
bool tcp_connect(String tcp_connect_cmd)
{
    // TCP connection
    tcp_connect_cmd = "AT+CIPSTART=\"TCP\",\"";
    tcp_connect_cmd += "184.106.153.149"; // api.thingspeak.com
    tcp_connect_cmd += "\",80";
    ser.println(tcp_connect_cmd);
    Serial.println(tcp_connect_cmd);
    // check for connection error
    if (ser.find("Error") || ser.find("closed"))
    {
        Serial.println("AT+CIPSTART error");
        return;
    }
}

// POST request structure for thinkTweet
String postRequest(String url, String host, String content_type, String message)
{
    String postString = "POST " + url + " HTTP/1.1\r\n" +
                        "Host: " + host + "\r\n" +
                        "Content-Type: " + content_type + "\r\n" +
                        "Connection: " + "close\r\n" +
                        "Content-Length: " + message.length() + "\r\n" +
                        "Cache-Control: " + "no-cache\r\n" +
                        +"\r\n" + message;
    return postString;
}

// updating thinkspeak fields //
String getRequest(String url, String host)
{
    String getString = "GET https://" + url +"\r\n";
    return getString;
}

// tweet message + api key + temperature
String tempAlertTweet()
{
    // tweet message
    String tempAlert = "api_key=" + twitterAPIKey + "&status=Buzzer and LED On [" + String(millis()) + "]";
    return tempAlert;
}

// send payload to server via esp8266
bool postStatus(String command, String message)
{
    command = "AT+CIPSEND=";
    command += String(message.length());
    Serial.println(command);
    ser.println(command);
    if (ser.find(">"))
    {
        ser.print(message);
        Serial.print(message);
        return;
    }
}

