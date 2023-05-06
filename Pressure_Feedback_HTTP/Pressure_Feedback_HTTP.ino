#include <WiFi.h>
#include <Wire.h>
#include <HTTPClient.h>
#include "SparkFun_BNO080_Arduino_Library.h" // Click here to get the library: http://librarymanager/All#SparkFun_BNO080
#include <Arduino.h>
#include "HX711.h"

#define WIFI_SSID   "It Burns When IP"
#define WIFI_PASSWD "NotAnEasyPassword"

#if defined(SOFTAP_MODE)
#endif
String macAddress = "";
String ipAddress = "ip";
String mac_address;

// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = 26;
const int LOADCELL_SCK_PIN = 25;
HX711 scale;

BNO080 myIMU;

WiFiClient client;
HTTPClient http;

void setupNetwork()
{
    macAddress = "";
    
    #ifdef SOFTAP_MODE
        WiFi.softAP("martinysoftap");  
    #else
        WiFi.begin(WIFI_SSID, WIFI_PASSWD);
        int retriesWifi = 0;
        while (WiFi.status() != WL_CONNECTED) 
        {
          delay(500);
          Serial.print(".");
          retriesWifi++;
          
          if(retriesWifi >= 100)
            esp_restart();
        }
        Serial.println("");
        Serial.println("WiFi connected");
        ipAddress = WiFi.localIP().toString();
        macAddress += WiFi.macAddress().substring(0, 5);
    #endif
}

void setupScale()
{
  Serial.println("HX711 Demo");

  Serial.println("Initializing the scale");

  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);

  Serial.println("Before setting up the scale:");
  Serial.print("read: \t\t");
  Serial.println(scale.read());      // print a raw reading from the ADC

  Serial.print("read average: \t\t");
  Serial.println(scale.read_average(20));   // print the average of 20 readings from the ADC

  Serial.print("get value: \t\t");
  Serial.println(scale.get_value(5));   // print the average of 5 readings from the ADC minus the tare weight (not set yet)

  Serial.print("get units: \t\t");
  Serial.println(scale.get_units(5), 1);  // print the average of 5 readings from the ADC minus tare weight (not set) divided
            // by the SCALE parameter (not set yet)
            
  //scale.set_scale(INSERT YOUR CALIBRATION FACTOR);
  scale.set_scale(-89.6984);                      // this value is obtained by calibrating the scale with known weights; see the README for details
  scale.tare();               // reset the scale to 0

  Serial.println("After setting up the scale:");

  Serial.print("read: \t\t");
  Serial.println(scale.read());                 // print a raw reading from the ADC

  Serial.print("read average: \t\t");
  Serial.println(scale.read_average(20));       // print the average of 20 readings from the ADC

  Serial.print("get value: \t\t");
  Serial.println(scale.get_value(5));   // print the average of 5 readings from the ADC minus the tare weight, set with tare()

  Serial.print("get units: \t\t");
  Serial.println(scale.get_units(5), 1);        // print the average of 5 readings from the ADC minus tare weight, divided
            // by the SCALE parameter set with set_scale
}

void setup()
{
    Serial.begin(115200);

    Wire.begin(22, 21);

    myIMU.begin();

    Wire.setClock(400000); //Increase I2C data rate to 400kHz

    myIMU.enableAccelerometer(50);    //We must enable the accel in order to get MEMS readings even if we don't read the reports.
    myIMU.enableRawAccelerometer(50); //Send data update every 50ms
    myIMU.enableGyro(50);
    myIMU.enableRawGyro(50);
    myIMU.enableMagnetometer(50);
    myIMU.enableRawMagnetometer(50);

    setupNetwork();
    mac_address = WiFi.macAddress();

    setupScale();

    Serial.println(F("Raw MEMS readings enabled"));
    Serial.println(F("Output is: (accel) x y z (gyro) x y z (mag) x y z"));
}

void sendSensorData(String s)
{

  String domain = "192.168.0.49";
  String serverName = "http://"+domain+":7071/updateData";
  String serverPath = serverName + "?data="+urlencode(s) + "&mac="+urlencode(mac_address);
  Serial.println(serverPath);
      
  // Your Domain name with URL path or IP address with path
  http.begin(serverPath.c_str());
  
  // Send HTTP GET request
  int httpResponseCode = http.GET();
  
  if (httpResponseCode>0) 
  {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    String payload = http.getString();
    Serial.println(payload);
  }
  else 
  {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();
}

String urlencode(String str)
{
    String encodedString="";
    char c;
    char code0;
    char code1;
    char code2;
    for (int i =0; i < str.length(); i++)
    {
      c=str.charAt(i);
      if (c == ' ')
        encodedString+= '+';
 
      else if (isalnum(c))
        encodedString+=c;

      else
      {
        code1=(c & 0xf)+'0';
        
        if ((c & 0xf) >9)
            code1=(c & 0xf) - 10 + 'A';

        c=(c>>4)&0xf;
        code0=c+'0';
        
        if (c > 9)
            code0=c - 10 + 'A';

        code2='\0';
        encodedString+='%';
        encodedString+=code0;
        encodedString+=code1;
        //encodedString+=code2;
      }
      yield();
    }
    return encodedString;
    
}

unsigned char h2int(char c)
{
    if (c >= '0' && c <='9')
        return((unsigned char)c - '0');
        
    if (c >= 'a' && c <='f')
        return((unsigned char)c - 'a' + 10);

    if (c >= 'A' && c <='F')
        return((unsigned char)c - 'A' + 10);

    return(0);
}

String msg = "";

void loop()
{
    //Look for reports from the IMU
    if (myIMU.dataAvailable() == true)
    {
        int x = myIMU.getRawAccelX();
        int y = myIMU.getRawAccelY();
        int z = myIMU.getRawAccelZ();

        Serial.print(x);
        Serial.print("\t");
        Serial.print(y);
        Serial.print("\t");
        Serial.print(z);
        Serial.print("\t");

        int gx = myIMU.getRawGyroX();
        int gy = myIMU.getRawGyroY();
        int gz = myIMU.getRawGyroZ();

        Serial.print(gx);
        Serial.print("\t");
        Serial.print(gy);
        Serial.print("\t");
        Serial.print(gz);
        Serial.print("\t");

        Serial.println();
        
        String weight = String(scale.get_units(10), 5);
        Serial.print("one reading:\t");
        Serial.print(scale.get_units(), 1);
        Serial.print("\t| average:\t");
        Serial.println(scale.get_units(10), 5);

        Serial.println();

        msg = "{\"Acceleration X\"   : " + String(x) + ",\"Acceleration Y\"  : " + String(y) + ",\"Acceleration Z\" : " + String(z) + ",\"Gyro X\" : " +
         String(gx) + ",\"Gyro Y\" : " + String(gy) + ",\"Gyro Z\" : " + String(gz) + ",\"Weight Applied\" : " + weight + "}";

        sendSensorData(msg);
           
        Serial.println();
        Serial.println();    
    }

    scale.power_down();             // put the ADC in sleep mode
    delay(1000);
    scale.power_up();
}
