#include <TinyGPS.h>
#include <WiFi.h>
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH1106.h>
#include <FirebaseESP32.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

Adafruit_MPU6050 mpu;

#define TX1_pin  14
#define RX1_pin  27

#define TX2_pin  17
#define RX2_pin  16

#define I2C_SDA_2   21
#define I2C_SCL_2   22

#define sim800l Serial

HardwareSerial neogps(2);
HardwareSerial max30100(1);

TinyGPS gps;

float lat = 0.0;
float lon = 0.0;

String datasms = "";

Adafruit_SH1106 display(I2C_SDA_2 , I2C_SCL_2);

/*Put your SSID & Password*/
const char* ssid = "YOUR_WIFI_SSID";  // Enter SSID here
const char* password = "YOUR_WIFI_PASSWORD";  //Enter Password here

#define DS18B20 26
OneWire oneWire(DS18B20);
DallasTemperature sensors(&oneWire);

float bodytemperature = 0.0;

float spo2 = 0.00;
float heartrate = 0.00;
String maxstream;

#define freefallint 23
#define smsbutton 25
#define buzzer 2

int freefallstate = 1;
int smsbuttonstate = 1;

#define FIREBASE_HOST "https://your-project.firebaseio.com/"               // Heere add your Firebase Realtime Database URL 
#define FIREBASE_AUTH "YOUR_FIREBASE_AUTH"                                // Here add your Firebase Database Secret or Authentication Token

FirebaseData firebaseData;
FirebaseJson json;

char buff[10];

String mybodytemperature; 
String myspo2;
String myheartrate;

char Received_SMS;              //Here we store the full received SMS (with phone sending number and date/time) as char
short MLX_OK=-1;

int wt = 0;


void IRAM_ATTR smsstate() {
  smsbuttonstate = 0;
}

void IRAM_ATTR freefall() {
  freefallstate = 0;
}
 
void setup(){
  Serial.begin(9600);
  Serial.print("ESP32 Board MAC Address:  ");
  Serial.println(WiFi.macAddress());
  
  neogps.begin(9600, SERIAL_8N1, RX2_pin, TX2_pin);
  max30100.begin(9600, SERIAL_8N1, RX1_pin, TX1_pin);

  display.begin(SH1106_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)
  // init done
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("IOT Based");
  display.println("Patient");
  display.println("Health");
  display.println("Monitoring");
  display.display();
  delay(3000);

  //connect to your local wi-fi network
  WiFi.begin(ssid, password);
  
  //check wi-fi is connected to wi-fi network
  while(WiFi.status() != WL_CONNECTED && wt <= 10) {
    delay(1000);
    Serial.print(".");
    wt = wt + 1;
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.println("Connecting");
    display.println("To");
    display.println(ssid);
    display.display();
  }

  if(WiFi.status() == WL_CONNECTED){
    Serial.println("");
    Serial.println("WiFi connected..!");
    Serial.print("Got IP: ");
    Serial.println(WiFi.localIP());
  
    Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH); 
    Firebase.reconnectWiFi(true);
   
    //Set database read timeout to 1 minute (max 15 minutes)
    Firebase.setReadTimeout(firebaseData, 1000 * 60);
    //tiny, small, medium, large and unlimited.
    //Size and its write timeout e.g. tiny (1s), small (10s), medium (30s) and large (60s).
    Firebase.setwriteSizeLimit(firebaseData, "tiny"); 
  }

  position();
  delay(1000);
  position();

  updatedisplay();

  pinMode(buzzer, OUTPUT);
  digitalWrite(buzzer, LOW);
  pinMode(smsbutton, INPUT_PULLUP);
  pinMode(freefallint, INPUT);
  attachInterrupt(digitalPinToInterrupt(smsbutton), smsstate, FALLING);
  attachInterrupt(digitalPinToInterrupt(freefallint), freefall, FALLING);

  // Try to initialize!
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");

  //setupt motion detection
  mpu.setHighPassFilter(MPU6050_HIGHPASS_0_63_HZ);
  mpu.setMotionDetectionThreshold(10);
  mpu.setMotionDetectionDuration(200);
  mpu.setInterruptPinLatch(true);  // Keep it latched.  Will turn off when reinitialized.
  mpu.setInterruptPinPolarity(true);
  mpu.setMotionInterrupt(true);
}
 
void loop(){
  digitalWrite(buzzer, LOW);
  
  position();
  measurebpm();
  updatedisplay();
  
  Serial.print("Heart rate:");
  Serial.print(heartrate);
  Serial.print("bpm / SpO2:");
  Serial.print(spo2);
  Serial.println("%");

  sensors.requestTemperatures();
  bodytemperature = sensors.getTempFByIndex(0);
  
  Serial.print("Body Temperature: ");
  Serial.print(bodytemperature);
  Serial.println("° F");

  if (heartrate >= 110 || bodytemperature >= 100 || (spo2 <= 92 && spo2 >= 70) || smsbuttonstate == 0) {
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0,50);
    display.print("                  ");
    display.display();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0,50);
    display.print("Sending Alert");
    display.display();
    digitalWrite(buzzer, HIGH);
    datasms = "\nAlert! I Need Help.\nHeartbeat Is: "+String(heartrate)+",\nOxygen Level Is: "+String(spo2)+",\nTemperature Is: "+String(bodytemperature)+",\nhttp://maps.google.com/maps?q=loc:"+String(lat, 6)+","+String(lon, 6);  //This string is sent as SMS
    Send_Data();
    ReceiveMode();
    smsbuttonstate = 1;
  }

  String RSMS;             //We add this new variable String type, and we put it in loop so everytime gets initialized
                           //This is where we put the Received SMS, yes above there's Recevied_SMS variable, we use a trick below
                           //To concatenate the "char Recevied_SMS" to "String RSMS" which makes the "RSMS" contains the SMS received but as a String
                           //The recevied SMS cannot be stored directly as String

  
  while(sim800l.available()>0){   //When SIM800L sends something to the Arduino... problably the SMS received... if something else it's not a problem 
    Received_SMS=sim800l.read();  //"char Received_SMS" is now containing the full SMS received  
    RSMS.concat(Received_SMS);    //concatenate "char received_SMS" to RSMS which is "empty"
    MLX_OK=RSMS.indexOf("LOC");   //"indexOf function looks for the substring "x" within the String (here RSMS) and gives us its index or position
                                  //For example if found at the beginning it will give "0" after 1 character it will be "1"
                                  //If it's not found it will give "-1", so the variables are integers
      
  }

  if (MLX_OK != -1) {
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0,50);
    display.print("                  ");
    display.display();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0,50);
    display.print("Message Received");
    display.display();
    datasms = "\nHeartbeat Is: "+String(heartrate)+",\nOxygen Level Is: "+String(spo2)+",\nTemperature Is: "+String(bodytemperature)+",\nhttp://maps.google.com/maps?q=loc:"+String(lat, 6)+","+String(lon, 6);  //This string is sent as SMS
    Send_Data();
    ReceiveMode();
    MLX_OK=-1;
  }

  if(freefallstate == 0){
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0,50);
    display.print("                  ");
    display.display();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0,50);
    display.print("Sending Fall Alert");
    display.display();
    digitalWrite(buzzer, HIGH);
    datasms = "\nAlert! Person Has Fallen.\nHeartbeat Is: "+String(heartrate)+",\nOxygen Level Is: "+String(spo2)+",\nTemperature Is: "+String(bodytemperature)+",\nhttp://maps.google.com/maps?q=loc:"+String(lat, 6)+","+String(lon, 6);  //This string is sent as SMS
    Send_Data();
    ReceiveMode();
    if (WiFi.status() == WL_CONNECTED) {
      uploaddata();
    }
    freefallstate = 1;
  }

  if(WiFi.status() != WL_CONNECTED){
    wt = 0; 
  
    WiFi.begin(ssid, password);
    //check wi-fi is connected to wi-fi network
    while(WiFi.status() != WL_CONNECTED && wt <= 10) {
      delay(1000);
      Serial.print(".");
      wt = wt + 1;
      display.clearDisplay();
      display.setTextSize(2);
      display.setTextColor(WHITE);
      display.setCursor(0, 0);
      display.println("Connecting");
      display.println("To");
      display.println(ssid);
      display.display();
    }
    if(WiFi.status() == WL_CONNECTED){
      Serial.println("");
      Serial.println("WiFi connected..!");
      Serial.print("Got IP: ");
      Serial.println(WiFi.localIP());
    
      Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH); 
      Firebase.reconnectWiFi(true);
     
      //Set database read timeout to 1 minute (max 15 minutes)
      Firebase.setReadTimeout(firebaseData, 1000 * 60);
      //tiny, small, medium, large and unlimited.
      //Size and its write timeout e.g. tiny (1s), small (10s), medium (30s) and large (60s).
      Firebase.setwriteSizeLimit(firebaseData, "tiny");
    } 
  } else if (WiFi.status() == WL_CONNECTED && smsbuttonstate != 0) {
      uploaddata();
  }

  Serial.println("******---------******---------******");

}

void measurebpm(){
  //max30100.listen();
  while(max30100.available()>0){
        
     maxstream=max30100.readString();
     Serial.println(maxstream);   //Show it on the serial monitor (optional)     
     for (int i = 0; i < maxstream.length(); i++) {
      if (maxstream.substring(i, i+1) == ";") {
        heartrate = maxstream.substring(0, i).toFloat();
        spo2 = maxstream.substring(i+1).toFloat();
        break;
      }
    } 
  }
}

void ReceiveMode(){       //Set the SIM800L Receive mode
  
  sim800l.println("AT"); //If everything is Okay it will show "OK" on the serial monitor
  delay(100);
  sim800l.println("AT+CMGF=1"); // Configuring TEXT mode
  delay(100);
  sim800l.println("AT+CNMI=2,2,0,0,0"); //Configure the SIM800L on how to manage the Received SMS... Check the SIM800L AT commands manual
  delay(100);
}

void Send_Data(){
  
  Serial.println("Sending Data...");     //Displays on the serial monitor...Optional
  sim800l.print("AT+CMGF=1\r");          //Set the module to SMS mode
  delay(100);
  sim800l.print("AT+CMGS=\"[Add phone number here]\"\r");  //Your phone number don't forget to include your country code example +212xxxxxxxxx"
  delay(500);  
  sim800l.print(datasms);  //This string is sent as SMS
  delay(500);
  sim800l.print((char)26);//Required to tell the module that it can send the SMS
  delay(500);
  sim800l.println();
  Serial.println("Data Sent.");
}

void position(){
  //neogps.listen();
  while(neogps.available()){ // check for gps data
    if(gps.encode(neogps.read())){// encode gps data 
      gps.f_get_position(&lat,&lon); // get latitude and longitude
      // display position
    
      Serial.print("Latitude: ");
      Serial.println(lat,6);
      Serial.print("Longitude: ");
      Serial.println(lon,6);
    }
  }
}

void updatedisplay(){
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.print("Temp= ");
  display.print(bodytemperature);
  display.println(" F");
  display.setCursor(0, 10);
  display.print("Heart rate: ");
  display.print(heartrate);
  display.println(" bpm");
  display.setCursor(0, 20);
  display.print("SpO2: ");
  display.print(spo2);
  display.println(" %");
  display.setCursor(0, 30);
  display.print("Latitude: ");
  display.print(lat, 6);
  display.setCursor(0, 40);
  display.print("Longitude: ");
  display.print(lon, 6);
  display.display(); 
}

void uploaddata(){
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,50);
  display.print("                  ");
  display.display();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,50);
  display.print("Uploading Data");
  display.display();
  
  json.set("/heartrate", heartrate);
  json.set("/spo2", spo2);
  json.set("/temperature", bodytemperature);
  json.set("/latitude", lat);
  json.set("/longitude", lon);
  json.set("/freefall", freefallstate);
  Firebase.updateNode(firebaseData,"/data",json);
}
