#include <Adafruit_NeoPixel.h>
#include "LiquidCrystal_I2C.h"
#include "DHT20.h"
#include "HCSR04.h"
#include <Arduino.h>
#include <WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

// Define your tasks here
void TaskBlink(void *pvParameters);
void TaskTemperatureHumidity(void *pvParameters);
void TaskSoilMoistureAndRelay(void *pvParameters);
void TaskLightAndLED(void *pvParameters);
void TaskFan(void *pvParameters);
void TaskUltraSonic(void *pvParameters);
void MQTT_Adafruit_Sever(void *pvParameters);

//Define your components here
Adafruit_NeoPixel pixels3(4, D5, NEO_GRB + NEO_KHZ800);
DHT20 dht20;
LiquidCrystal_I2C lcd(0x21,16,2); // địa chỉ màn hình là 0x21 hexa
UltraSonicDistanceSensor ultrasonic(D3, D4); //cảm biến nối với D3-D4

//wifi
#define WLAN_SSID       "10h37"
#define WLAN_PASS       "12345678"
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "Giaqui14032002"
#define AIO_KEY         "key"

WiFiClient client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Subscribe ledControl = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/led");
Adafruit_MQTT_Publish temperature = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/temp");
Adafruit_MQTT_Publish humidity = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/humi");
Adafruit_MQTT_Publish light = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/light");
Adafruit_MQTT_Publish warning = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/warning");

void setup() {

  // Initialize serial communication at 115200 bits per second:
  Serial.begin(115200); 

  //Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  dht20.begin();
  lcd.init();
  lcd.backlight();
  pixels3.begin();
  mqtt.subscribe(&ledControl);
  
  xTaskCreate( TaskBlink, "Task Blink" ,2048  ,NULL  ,2 , NULL);
  xTaskCreate( TaskTemperatureHumidity, "Task Temperature" ,2048  ,NULL  ,2 , NULL);
  xTaskCreate( TaskSoilMoistureAndRelay, "Task Soild Relay" ,2048  ,NULL  ,2 , NULL);
  xTaskCreate( TaskLightAndLED, "Task Light LED" ,2048  ,NULL  ,2 , NULL);
  xTaskCreate( TaskFan, "Task Fan" ,2048  ,NULL  ,2 , NULL);
  xTaskCreate( TaskUltraSonic, "Task UltraSonic" ,2048  ,NULL  ,2 , NULL);
  xTaskCreate( MQTT_Adafruit_Sever, "MQTT Adafruit Sever" ,2048  ,NULL  ,2 , NULL);
  
  //Now the task scheduler is automatically started.
  Serial.printf("Basic Multi Threading Arduino Example\n");

}

void loop() {
}

/*--------------------------------------------------*/
/*---------------------- Tasks ---------------------*/
/*--------------------------------------------------*/



void TaskBlink(void *pvParameters) {  // This is a task.
  //uint32_t blink_delay = *((uint32_t *)pvParameters);

  // initialize digital LED_BUILTIN on pin 13 as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  

  while(1) {                          
    digitalWrite(LED_BUILTIN, HIGH);  // turn the LED ON
    delay(200*5);
    digitalWrite(LED_BUILTIN, LOW);  // turn the LED OFF
    delay(200*5);
  }
}


void TaskTemperatureHumidity(void *pvParameters) {  // This is a task.
  //uint32_t blink_delay = *((uint32_t *)pvParameters);

  while(1) {                          
    Serial.println("Task Temperature and Humidity");
    dht20.read();
    Serial.println(dht20.getTemperature());
    Serial.println(dht20.getHumidity());
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(dht20.getTemperature());
    lcd.setCursor(0, 1);
    lcd.print(dht20.getHumidity());

    float temp = dht20.getTemperature();
    // float humi = dht20.getHumidity();
    if(!temperature.publish(temp)) {
      Serial.println("Fail");
    }
    else {
      Serial.println("OK");
    }

    
    float humi = dht20.getHumidity();
    if(!humidity.publish(humi)) {
      Serial.println("Fail");
    }
    else {
      Serial.println("OK");
    }

    delay(15000);
  }
}

void TaskSoilMoistureAndRelay(void *pvParameters) {  // This is a task.
  //uint32_t blink_delay = *((uint32_t *)pvParameters);

  pinMode(D3, OUTPUT);

  while(1) {                          
    Serial.println("Task Soild and Relay");
    //Serial.println(analogRead(A0));
    
    if(analogRead(A0) > 500){
      digitalWrite(D3, LOW);
    }
    if(analogRead(A0) < 50){
      digitalWrite(D3, HIGH);
    }
    delay(1000);
  }
}


void TaskLightAndLED(void *pvParameters) {  // This is a task.
  //uint32_t blink_delay = *((uint32_t *)pvParameters);

  while(1) {                          
    // Serial.println("Task Light and LED");
    float light_value = analogRead(A1);

    // Serial.println(light_value);

    if(!light.publish(light_value)) {
      Serial.println("Fail");
    }
    else {
      Serial.println("OK");
    }
    if(analogRead(A1) < 350){
      pixels3.setPixelColor(0, pixels3.Color(255,0,0));      
      pixels3.show();
    }
    if(analogRead(A1) > 550){
      pixels3.setPixelColor(0, pixels3.Color(0,0,0));      
      pixels3.show();
    }
    delay(15000);
  }
}



void TaskFan(void *pvParameters) {
  while(1) {
    int temp = dht20.getTemperature();
    if(temp > 32) {
      analogWrite(D7, 250);
    }
    else if(temp > 30) {
      analogWrite(D7, 150);
    }
    else if(temp > 28) {
      analogWrite(D7, 75);
    }
    else {
      analogWrite(D7, 0);
    }
    delay(2000);
  }
}

void TaskUltraSonic(void *pvParameters) {
  while(1) {
    if(ultrasonic.measureDistanceCm() < 10) {
      pixels3.setPixelColor(2, pixels3.Color(255,0,0));      
      pixels3.show();
      if(!warning.publish("\n\nWarning\n")) {
        Serial.println("Fail");
      }
      else {
        Serial.println("OK");
      }
    }
    else {
      pixels3.setPixelColor(2, pixels3.Color(0,0,0));      
      pixels3.show();
      if(!warning.publish("\n\nSecurity\n")) {
        Serial.println("Fail");
      }
      else {
        Serial.println("OK");
      }
    }
    delay(15000);
  }
}

void MQTT_connect()
{
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;

  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();

    delay(5000); // wait 5 seconds

    retries--;
    if (retries == 0) {
      // basically die and wait for WDT to reset me
      while (1);
    }
  }

  Serial.println("MQTT Connected!");
}

void MQTT_Adafruit_Sever(void *pvParameters) {
  while(1) {

    MQTT_connect();
    Adafruit_MQTT_Subscribe *subscription;

    while ((subscription = mqtt.readSubscription(5000))) {
      if (subscription == &ledControl) {
        Serial.print(F("Got: "));
        Serial.println((char *)ledControl.lastread);

        if (!strcmp((char*) ledControl.lastread, "ON")) {
          Serial.println("ON LED");
          pixels3.setPixelColor(1, pixels3.Color(255,0,0));      
          pixels3.setPixelColor(3, pixels3.Color(255,0,0));     
          pixels3.show();
        }
        else {
          Serial.println("OFF LED");
          pixels3.setPixelColor(1, pixels3.Color(0,0,0));      
          pixels3.setPixelColor(3, pixels3.Color(0,0,0));     
          pixels3.show();
        }
      }
    }
    
  }

}
