#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "WiFi.h"
#include <AsyncUDP.h>

#define ch1_pin 47
#define ch2_pin 9
#define led_pin 10
#define pulse_width 2  // ms
#define period 20      // ms
#define interval_time 500 //leg rest time(ms)
#define pulse_out_time 100 //leg active time(ms)

const char *ssid = "nomusan";
const char *password = "nomurabayado";

bool enable_flag = false;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(3, led_pin, NEO_GRB + NEO_KHZ800);
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
AsyncUDP udp;
AsyncUDPMessage udpmsg;

TaskHandle_t taskHandle[2];//task handle forLR pulse_out

void Task_udp_listen(void *pvParameters)
{
  while (1)
  {
    udp.onPacket([](AsyncUDPPacket packet)
    {
      //Serial.print("UDP packet received: ");
      //Serial.println((char*)packet.data());

      if (strncmp((char*)packet.data(), "LEFT",4) == 0) {
        // START
        Serial.println("LEFT");
        xTaskNotifyGive(taskHandle[1]);
        

      } else if (strncmp((char*)packet.data(),"RIGHT", 5) == 0) {
        // STOP
        Serial.println("RIGHT");
        xTaskNotifyGive(taskHandle[0]);
      }
    });
    delay(1);
  }
}

void Task_pulse_out_L(void *pvParameters)
{
  while (1)
  {
    if (xTaskNotifyWait(0, 0, NULL, pdMS_TO_TICKS(0)) == pdTRUE) {
      //portENTER_CRITICAL(&timerMux);
      //Serial.println("pulse_out");
      uint32_t t_0 = millis();
      uint32_t t = 0;//Time elapsed since the start of muscle contraction
      strip.setPixelColor(2, strip.Color(255, 0, 0));
      strip.show();
      while(t < pulse_out_time){
        digitalWrite(ch1_pin,HIGH); //on
        delay(pulse_width);  // Standby for a specified time
        digitalWrite(ch1_pin,LOW); //off
        delay(period - pulse_width);
        t = millis() - t_0;//Update elapsed time
      }
      
      //portEXIT_CRITICAL(&timerMux);
     // Serial.println(t);
     // digitalWrite(ch1_pin,LOW); //off
      strip.setPixelColor(2, strip.Color(0, 0, 0));
      strip.show();
      delay(interval_time);
    }
    delay(1);
  }
}

void Task_pulse_out_R(void *pvParameters)
{
  while (1)
  {
    if (xTaskNotifyWait(0, 0, NULL, pdMS_TO_TICKS(0)) == pdTRUE) {
      //portENTER_CRITICAL(&timerMux);
      //Serial.println("pulse_out");
      uint32_t t_0 = millis();
      uint32_t t = 0;//Time elapsed since the start of muscle contraction
      strip.setPixelColor(0, strip.Color(0, 0, 255));
      strip.show();
      while(t < pulse_out_time){
        digitalWrite(ch2_pin,HIGH); //on
        delay(pulse_width);  // Standby for a specified time
        digitalWrite(ch2_pin,LOW); //off
        delay(period - pulse_width);
        t = millis() - t_0;//Update elapsed time
      }
      
      //portEXIT_CRITICAL(&timerMux);
     // Serial.println(t);
     // digitalWrite(ch1_pin,LOW); //off
      strip.setPixelColor(0, strip.Color(0, 0, 0));
      strip.show();
      delay(interval_time);
    }
    delay(1);
  }
}


void setup() {
  pinMode(ch1_pin, OUTPUT);
  pinMode(ch2_pin, OUTPUT);
  digitalWrite(ch2_pin,HIGH);
  digitalWrite(ch1_pin,HIGH);
  strip.begin();
  strip.setBrightness(125);
  strip.show(); //initialization
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);        // Set WiFi mode
  WiFi.begin(ssid, password); // Start WiFi connection
  Serial.println("WiFi connecting...");
  strip.setPixelColor(0, strip.Color(255, 0, 0));
  strip.setPixelColor(1, strip.Color(255, 0, 0));
  strip.setPixelColor(2, strip.Color(255, 0, 0));
  strip.show();
  if (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.println("WiFi Failed");
    while (1)
    {
      delay(1000);
      Serial.println("WiFi Failed");
    }
  }
  strip.setPixelColor(0, strip.Color(0, 0, 255));
  strip.setPixelColor(1, strip.Color(0, 0,255));
  strip.setPixelColor(2, strip.Color(0, 0, 255));
  strip.show();
  delay(500);
  strip.setPixelColor(0, strip.Color(0, 0, 0));
  strip.setPixelColor(1, strip.Color(0, 0, 0));
  strip.setPixelColor(2, strip.Color(0, 0, 0));
  strip.show();
  Serial.print("IP address:");
  Serial.println(WiFi.localIP());
  udp.connect(IPAddress(192, 168, 11, 202), 1234);
  udp.listen(1234);
  Serial.print("UDP port: ");
  Serial.println("1234");



  //Create a task for UDP communication
  xTaskCreateUniversal(
    Task_udp_listen, 
    "udp_task",
    3192,
    NULL,
    10,
    NULL,
    0
  );
  xTaskCreateUniversal(
    Task_pulse_out_L,
    "pulse_out_L",
    3192,
    NULL,
    3,
    &taskHandle[1],
    0
  );
  xTaskCreateUniversal(
    Task_pulse_out_R,
    "pulse_out_R",
    3192,
    NULL,
    3,
    &taskHandle[0],
    0
  );
}


void loop() {
  //Serial.println("loop");

  delay(1);
}