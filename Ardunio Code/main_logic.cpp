
// #include <stdlib.h>
// #include <String.h>
#include <SoftwareSerial.h>

SoftwareSerial ser(2, 3); // RX, TX

// =====================================================//
// Thinkspeak and thinktweet setup //
String reset = "AT+RST";
String thingSpeak = "api.thingspeak.com";
String urlEncoded = "application/x-www-form-urlencoded";
String cmd;

String apiWriteKey = "2W54NENKHMAMNK71";
String apiReadKey = "N0KL9SHVN3B1YOD7";

String twitterAPIKey = "Y6OS7WNU7BSQDADR";
String tweetURI = "/apps/thingtweet/1/statuses/update";
// =====================================================//

void postThinkSpeak(int fieldNo)
{
    String getStr = "/update?api_key=" + apiWriteKey + "&field1=" + 1;
    if (fieldNo > 1)
    {
        for (int i = 2; i < fieldNo + 2; i++)
        {
            getStr += "&field" + String(i) + "=1";
        }
    }
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
// think tweet function//
// create a POST request header structure

void postThinkSpeak()
{
    String payload = postRequest(tweetURI, thingSpeak, urlEncoded, tempAlertTweet());
}

// =============== functions below used to connect to thinkspeak to retrieve data ================//

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
    String getString = "GET https://" + url + "\r\n";
    return getString;
}

// tweet message + api key + temperature
String tempAlertTweet()
{
    // tweet message
    String tempAlert = "api_key=" + twitterAPIKey + "&status=Door has been Open! [" + String(millis()) + "]";
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
