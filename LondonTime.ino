/* ==========================================
 *
 * Use ESP32 WiFi library to get London Time form http request to server: http://worldtimeapi.org/api/timezone/Europe/London.txt
 * Then display to 4 digit 7 segment display module
 *
 * Author: David Zheng
 * ============================================*/
  
#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>

#define DigitNum 4 //Total number of digits
#define TM1650_DISPLAY_BASE 0x34 // Address of the left-most digit 
#define TM1650_DCTRL_BASE   0x24 // Address of the control register of the left-most digit
#define delayTime 30 //Time between each http request(seconds)

//Declear subfunction
void displayToModule (int digit[]);

//Declear global variables
const char* ssid = "HUAWEI-H106GU";
const char* password = "P@ssw0rd";
String serverPath = "http://worldtimeapi.org/api/timezone/Europe/London.txt";

int digitDisplay[10] = {63,6,91,79,102,109,125,7,127,111};//TM1650 data inputs for number 0-9

hw_timer_t * timer = NULL;
int Time, tempTic;

void setup() {
  Serial.begin(115200); 
  // Start WiFi connection
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
  //Reset Display
  Wire.begin();
  for(int i=0; i<DigitNum; i++) {
      // Turn on display 
      Wire.beginTransmission(TM1650_DCTRL_BASE+i);      
      Wire.write((byte) 1);
      Wire.endTransmission();
      // Clear display
      Wire.beginTransmission(TM1650_DISPLAY_BASE+i); 
      Wire.write((byte) 0);
      Wire.endTransmission();
  }
  
  Time=-1;
  tempTic = -delayTime*1000000;
  timer = timerBegin(0, 80, true); 
}

void loop() {
  int digit[DigitNum];
  //Send http requests every 30 seconds after previous update
  if(timerRead(timer) - tempTic >=delayTime*1000000) {
    if(WiFi.status()== WL_CONNECTED){
      HTTPClient http;
      http.begin(serverPath.c_str());
      int httpResponseCode = http.GET();
      //Countinueosly sending http requests if unsuccessful
      while (httpResponseCode!=200) {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
        httpResponseCode = http.GET();
      }
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      String responseData = http.getString();
      //Convert string to int
      digit[0] = responseData[66]-'0';
      digit[1] = responseData[67]-'0';
      digit[2] = responseData[69]-'0';
      digit[3] = responseData[70]-'0';
      Serial.println(responseData);
      Serial.println();
      http.end();
    }  
    tempTic = timerRead(timer);
  }
  displayToModule (digit);
}

void displayToModule (int digit[]) {
  for(int i=0; i<DigitNum; i++) {
      Wire.beginTransmission(TM1650_DISPLAY_BASE+i); 
      
      if(i==1) { //add dot after second digit
        Wire.write((byte)digitDisplay[digit[i]] | 0b10000000);
      } else {
        Wire.write((byte)digitDisplay[digit[i]]);
      }      
      Wire.endTransmission();
  }
}
