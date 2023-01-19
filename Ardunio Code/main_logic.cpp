
#include <stdlib.h>
#include <String.h>
#include <SoftwareSerial.h>
#include "main_logic.ino"
// =====================================================//
// Thinkspeak and thinktweet setup //
const char *thingSpeak = "api.thingspeak.com";

// String apiWriteKey = "2W54NENKHMAMNK71";
// String apiReadKey = "N0KL9SHVN3B1YOD7";
// =====================================================//

// Thinkspeak Function //
// Field no is the field to post to thinkspeak and numberOfFields is the number of fields to post
// after the initial fieldNo. uniqueInput allows to post to thinkspeak a unique input//
void postThinkSpeak(int fieldNo, int numberOfFields, String uniqueInput)
{
    // Crafting of get request str //
    String apiWriteKey = F("Q3X0NH6PSG5W6DO1");
    String getStr = F("/update?api_key=");
    getStr += String(apiWriteKey);
    getStr += F("&field");
    getStr += String(fieldNo);
    if (uniqueInput != "")
    {
        getStr += F("=");
        getStr += String(uniqueInput);
    }
    else
    {
        getStr += F("=1");
    }
    while (numberOfFields > 1)
    {
        getStr += F("&field");
        getStr += String(fieldNo + 1);
        getStr += F("=1");
        numberOfFields -= 1;
    }

    // Crafting of get request str //
    String payload = F("GET https://");
    payload += thingSpeak + getStr;
    payload += "\r\n";
    tcp_connect();
    // Send data length & GET string
    ser.print(F("AT+CIPSEND="));
    ser.println(payload.length());
    Serial.print(F("AT+CIPSEND="));
    Serial.println(payload.length());

    delay(1000);
    ser.print(payload); // send data
    Serial.println(payload);
    delay(1000);
    ser.print(F("AT+CIPCLOSE")); // close the connection
    Serial.println(F("AT+CIPCLOSE"));
}

// think tweet function//
// Conditions : Door Open OR Incorrect attempts warning //
void postThinkTweet(String condition)
{
    String twitterAPIKey = F("Y6OS7WNU7BSQDADR");
    String tweetURI = F("api.thingspeak.com/apps/thingtweet/1/statuses/update");

    // Crafting of post request str //
    String statusMessage = tempAlertTweet(condition, twitterAPIKey, tweetURI);
    String payload = "";
    payload += "POST " + String(tweetURI) + " HTTP/1.1\r\n";
    payload += "Host: " + String(thingSpeak) + "\r\n";
    payload += "Content-Type: application/x-www-form-urlencoded\r\n";
    payload += "Connection: close\r\n";
    payload += "Content-Length: " + String(statusMessage.length()) + "\r\n";
    payload += "Cache-Control: no-cache\r\n";
    payload += "\r\n" + statusMessage;

    // TCP connect
    tcp_connect();
    // Send data length & GET string
    ser.print(F("AT+CIPSEND="));
    ser.println(payload.length());
    Serial.print(F("AT+CIPSEND="));
    Serial.println(payload.length());

    delay(1000);
    ser.print(payload); // send data
    Serial.println(payload);
    delay(1000);
    ser.print(F("AT+CIPCLOSE")); // close the connection
    Serial.println(F("AT+CIPCLOSE"));
}

// =============== functions below used to connect to thinkspeak to retrieve data ================//

// TCP connection
void tcp_connect()
{
    // Connecting to wifi //
    ser.print(F("AT+RST\r\n"));
    ser.print(F("AT+CWMODE=1\r\n"));
    ser.print("AT+CWJAP=\"" + String("T923WIFI") + "\",\"" + String("abc1234567") + "\"");
    // sendData("AT+CWJAP=\"T923WIFI\",\"abc1234567\"\r\n", 4000, DEBUG);
    ser.print(F("AT+CIPMUX=0\r\n"));
    Serial.println(F("Connected to Wifi!"));
    delay(5000);

    // Make TCP connection to thinkspeak //
    String cmd = F("AT+CIPSTART=\"TCP\",\"");
    cmd += F("184.106.153.149"); // Thingspeak.com's IP address
    // ****************************************************************** Change this!
    cmd += F("\",80\r\n");
    ser.print(cmd); // start tcp connection //
    Serial.println(cmd);
}

// tweet message + api key + door has been unlocked
String tempAlertTweet(String condition, String twitterAPIKey, String tweetURI)
{
    // tweet message
    String tempAlert = "";
    if (condition == "Door Open")
    {
        // String tempAlert = "api_key=" + String(twitterAPIKey) + "&status=Door has been Open! [" + String(millis()) + "]";
        tempAlert += F("api_key=");
        tempAlert += String(twitterAPIKey);
        tempAlert += F("&status=Door has been Open! [");
        tempAlert += String(millis());
        tempAlert += F("]");
        Serial.println(tempAlert);
        return tempAlert;
    }
    else if (condition == "Incorrect attempts warning")
    {
        // String tempAlert = "api_key=" + String(twitterAPIKey) + "&[WARNING] There has been 5 incorrect attempts made to open the door. [" + String(millis()) + "]";
        tempAlert += F("api_key=");
        tempAlert += String(twitterAPIKey);
        tempAlert += F("&[WARNING] There has been 5 incorrect attempts made to open the door. [");
        tempAlert += String(millis());
        tempAlert += F("]");
        Serial.println(tempAlert);
        return tempAlert;
    }
}