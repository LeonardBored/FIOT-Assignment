
#include <stdlib.h>
#include <String.h>
#include <SoftwareSerial.h>
#define DEBUG true

SoftwareSerial ser(2, 3); // RX, TX

// =====================================================//
// Thinkspeak and thinktweet setup //
String thingSpeak = "api.thingspeak.com";

// String apiWriteKey = "2W54NENKHMAMNK71";
// String apiReadKey = "N0KL9SHVN3B1YOD7";

// =====================================================//

void postThinkSpeak(int fieldNo, int numberOfFields, String uniqueInput)
{
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

    String payload = F("GET https://");
    payload += thingSpeak + getStr;
    payload += "\r\n";
    Serial.println("Payload to send: " + payload);
    tcp_connect();
    // Send data length & GET string
    ser.println(F("AT+CIPSEND="));  
    ser.println(payload.length());
    Serial.print(F("AT+CIPSEND="));  
    Serial.println(payload.length());

    delay(1000);
    ser.println(payload);  // send data 
    Serial.println(payload);
    delay(1000);
    ser.println(F("AT+CIPCLOSE")); // close the connection 
    Serial.println(F("AT+CIPCLOSE")); 
}

// think tweet function//
void postThinkTweet(String condition)
{
    String twitterAPIKey = F("Y6OS7WNU7BSQDADR");
    String tweetURI = F("https://api.thingspeak.com/apps/thingtweet/1/statuses/update");

    String payload = "";
    String statusMessage = tempAlertTweet(condition, twitterAPIKey, tweetURI);
    Serial.println("Status Message to send: " + statusMessage);

    payload =  "POST " + String(tweetURI) + " HTTP/1.1\r\n" +
                        "Host: " + String(thingSpeak) + "\r\n" +
                        "Content-Type: application/x-www-form-urlencoded" + "\r\n" +
                        "api_key: Y6OS7WNU7BSQDADR" + "\r\n";
                        "status: " + String(statusMessage); + "\r\n";

    Serial.println("Payload to send: " + payload);
    // TCP connect
    tcp_connect();
    // Send data length & GET string
    ser.println(F("AT+CIPSEND="));  
    ser.println(payload.length());
    Serial.print(F("AT+CIPSEND="));  
    Serial.println(payload.length());

    delay(1000);
    ser.println(payload);  // send data 
    Serial.println(payload);
    delay(1000);
    ser.println(F("AT+CIPCLOSE")); // close the connection 
    Serial.println(F("AT+CIPCLOSE")); 
}

// =============== functions below used to connect to thinkspeak to retrieve data ================//

// TCP connection
void tcp_connect()
{
    ser.println(F("AT+RST\r\n"));
    ser.println(F("AT+CWMODE=1\r\n"));
    ser.println("AT+CWJAP=\"" + String("T923WIFI") + "\",\"" + String("abc1234567") + "\"");
    // sendData("AT+CWJAP=\"T923WIFI\",\"abc1234567\"\r\n", 4000, DEBUG);
    ser.println(F("AT+CIPMUX=0\r\n"));
    delay(5000);

    // Make TCP connection
    String cmd = F("AT+CIPSTART=\"TCP\",\"");
    cmd += F("184.106.153.149"); // Thingspeak.com's IP address
    // ****************************************************************** Change this!
    cmd += F("\",80\r\n");
    ser.println(cmd); // start tcp connection //
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

