#include "mqtt.h"
#include "LittleFS.h"
#include "network_secrets.h"

#include <ArduinoMqttClient.h>
#include <WiFi.h>

char ssid[] = SECRET_SSID;    // network SSID (name)
char pass[] = SECRET_PASS;    // network password (use for WPA, or use as key for WEP)

char broker[] = BROKER_IP;
int  port     = BROKER_PORT;

String moduleId = String(MODULE_ID);
String topic  = moduleId + "/#";

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

void mqttSetup(bool &isWifiConnected)
{
  //////////////////////////////////////////////
  //                  SERIAL                  //
  //////////////////////////////////////////////
  //Initialize serial and wait for port to open:
  Serial.begin(115200);
  
  //////////////////////////////////////////////
  //                    WIFI                  //
  //////////////////////////////////////////////
  // attempt to connect to WiFi network:
  Serial.print("Attempting to connect to WPA SSID: ");
  Serial.println(ssid);
  checkWifiStatus(isWifiConnected);
  Serial.println("You're connected to the network");
  Serial.println();

  //////////////////////////////////////////////
  //                    MQTT                  //
  //////////////////////////////////////////////
  // You can provide a unique client ID, if not set the library uses Arduino-millis()
  // Each client must have a unique client ID
  // mqttClient.setId("clientId");

  // You can provide a username and password for authentication
  // mqttClient.setUsernamePassword("username", "password");

  Serial.print("Attempting to connect to the MQTT broker: ");
  Serial.println(broker);

  if (!mqttClient.connect(broker, port)) 
  {
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());

    while (1);
  }

  Serial.println("You're connected to the MQTT broker!");
  Serial.println();

  Serial.print("Subscribing to topic: ");
  Serial.println(topic);
  Serial.println();

  // subscribe to a topic
  mqttClient.subscribe(topic);

  // topics can be unsubscribed using:
  // mqttClient.unsubscribe(topic);

  Serial.print("Waiting for messages on topic: ");
  Serial.println(topic);
  Serial.println();  
}

void mainMqttLoop(CardList &cardPaths, bool &isWifiConnected)
{
  checkWifiStatus(isWifiConnected);
  
  int messageSize = mqttClient.parseMessage();
  if (messageSize) {
    // we received a message, print out the topic and contents
    Serial.print("Received a message with topic '");
    Serial.print(mqttClient.messageTopic());
    Serial.print("', length ");
    Serial.print(messageSize);
    Serial.println(" bytes:");

    String new_topic = mqttClient.messageTopic();
    new_topic.replace(moduleId + "/", "");
    
    if (new_topic.startsWith("send")) // card with default back sent
    {
      new_topic.replace("send", "");
      if (new_topic.startsWith("back")) // card with specified back sent
      {
        new_topic.replace("back", "");
        File image = LittleFS.open(new_topic, "w");
        while (mqttClient.available()) 
        {
          image.write(mqttClient.read());
        }
        image.close();

        // Save a placeholder image back
        File imageBack = LittleFS.open("/b"+new_topic, "w");
        File defaultBack = LittleFS.open("/b/back.jpg", "w");

        while (defaultBack.available())
        {
          image.write(defaultBack.read());
        }
        image.close();
        defaultBack.close();
        
        Card receivedCard(new_topic, "/b"+new_topic);
        cardPaths.append(receivedCard);
        Serial.print(new_topic + " Image saved:\n");
      }
      else // card with default card back received
      {
        File image = LittleFS.open(new_topic, "w");
        while (mqttClient.available()) 
        {
          image.write(mqttClient.read());
        }
        image.close();
        Card receivedCard(new_topic, "/b/back.jpg");
        cardPaths.append(receivedCard);
        Serial.print(new_topic + " Image saved:\n");
      }
    }
    else if (new_topic.startsWith("back")) // card back saved
    {
      new_topic.replace("back", "");
      new_topic = "/b" + new_topic;
      File image = LittleFS.open(new_topic, "w");
      
      // save the new card back into /b directory
      while (mqttClient.available()) 
      {
        image.write(mqttClient.read());
      }
      image.close();
      Serial.println(new_topic + " Image back saved:");
    }
    else if (new_topic.startsWith("delete")) // card deleted
    {
      Serial.println("Delete card message received");
      new_topic.replace("delete", "");
      // delete card
      for (int i = 0; i < cardPaths.length; i++)
      {
        if (new_topic == cardPaths.getCard(i).frontPath)
        {
          Serial.print("Removing " + new_topic);
          cardPaths.remove(i);
          LittleFS.remove(new_topic); 
          break;  
        }
      }
    }
    else if (new_topic.startsWith("init")) // initialize
    {
      Serial.println("Init message received");
      // delete all cards, receive the card back
      File root = LittleFS.open("/", "r");
      while (File file = root.openNextFile()) 
      {
        String strname = "/" + String(file.name());
        // If it is not a directory and filename ends in .jpg then delete it
        if (!file.isDirectory() && strname.endsWith(".jpg") && !(strname.endsWith("logo.jpg") || strname.endsWith("back.jpg")))
          Serial.println("Deleted :" + strname);
          cardPaths.pop_back();
          LittleFS.remove(strname); 
      }
      root.close();

      // delete all cards in the /b directory, receive the card back
      File backs = LittleFS.open("/", "r");
      while (File file = backs.openNextFile()) 
      {
        String strname = "/" + String(file.name());
        // If it is not a directory and filename ends in .jpg then delete it
        if (!file.isDirectory() && strname.endsWith(".jpg"))
          Serial.println("Deleted :" + strname);
          cardPaths.pop_back();
          LittleFS.remove(strname); 
      }
      backs.close();
      
      File image = LittleFS.open("/b/back.jpg", "w");
      while (mqttClient.available()) 
      {
        image.write(mqttClient.read());
      }
      image.close();
      Serial.println("Default image back saved:");
    }

    Serial.println();
    Serial.println();
  }
}

void checkWifiStatus(bool &isWifiConnected)
{
  if (WiFi.status() != WL_CONNECTED)
  {
    while (WiFi.status() != WL_CONNECTED) 
    {
      WiFi.begin(ssid, pass);
      isWifiConnected = false;
      Serial.print(".");
      delay(1000);
    }
    isWifiConnected = true;
    
    // reconnect to MQTT
    if (!mqttClient.connect(broker, port)) 
    {
      Serial.print("MQTT connection failed! Error code = ");
      Serial.println(mqttClient.connectError());
      while (1);
    }
    // subscribe to a topic
    mqttClient.subscribe(topic);
  }
  isWifiConnected = true;
}
