
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
    String apiWriteKey = F("2W54NENKHMAMNK71");
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

    String url = F("GET https://");
    url += thingSpeak + getStr;
    url += "\r\n";
    Serial.println(url);
    tcp_connect();
    // Send data length & GET string
    ser.print(F("AT+CIPSEND="));
    ser.println(url.length());
    Serial.print(F("AT+CIPSEND="));
    Serial.println(url.length());
    delay(500);
    if (ser.find(">"))
    {
        Serial.print(">");
        sendData(url, 2000, DEBUG);
    }

    // Close connection, wait a while before repeating...
    sendData(F("AT+CIPCLOSE"), 1500, DEBUG); // thingspeak needs 15 sec delay between updates
}

// think tweet function//
void postThinkTweet(String condition)
{
    String twitterAPIKey = F("Y6OS7WNU7BSQDADR");
    String tweetURI = F("/apps/thingtweet/1/statuses/update");

    String payload = "";
    String statusMessage = tempAlertTweet(condition, twitterAPIKey, tweetURI);
    Serial.println(statusMessage);
    payload = postRequest(tweetURI, thingSpeak, statusMessage);
    Serial.println(payload);
    // TCP connect
    tcp_connect();
    // Send data length & GET string
    ser.print(F("AT+CIPSEND="));
    ser.println(payload.length());
    Serial.print(F("AT+CIPSEND="));
    Serial.println(payload.length());
    delay(500);
    if (ser.find(">"))
    {
        Serial.print(">");
        sendData(payload, 2000, DEBUG);
    }

    // Close connection, wait a while before repeating...
    sendData(F("AT+CIPCLOSE"), 1500, DEBUG); // thingspeak needs 15 sec delay between updates
}

// =============== functions below used to connect to thinkspeak to retrieve data ================//

// TCP connection
void tcp_connect()
{
    sendData(F("AT+RST\r\n"), 2000, DEBUG);
    sendData(F("AT+CWMODE=1\r\n"), 2000, DEBUG);
    sendData(F("AT+CWJAP=\"life is simple\",\"wendi103\"\r\n"), 4000, DEBUG);
    // sendData("AT+CWJAP=\"T923WIFI\",\"abc1234567\"\r\n", 4000, DEBUG);
    sendData(F("AT+CIPMUX=0\r\n"), 2000, DEBUG);
    // Make TCP connection
    String cmd = F("AT+CIPSTART=\"TCP\",\"");
    cmd += F("184.106.153.149"); // Thingspeak.com's IP address
    // ****************************************************************** Change this!
    cmd += F("\",80\r\n");
    sendData(cmd, 2000, DEBUG);
}

// POST request structure for thinkTweet
String postRequest(String url, String host, String message)
{
    String postString = "POST " + String(url) + " HTTP/1.1\r\n" +
                        "Host: " + String(host) + "\r\n" +
                        "Content-Type: application/x-www-form-urlencoded" + "\r\n" +
                        "Connection: " + "close\r\n" +
                        "Content-Length: " + String(message.length()) + "\r\n" +
                        "Cache-Control: " + "no-cache\r\n" +
                        +"\r\n" + String(message);
    return postString;
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

String sendData(String command, const int timeout, boolean debug)
{
    String response = "";
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
        Serial.print(response);
    }

    return (response);
}
