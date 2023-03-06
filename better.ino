#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include "DHT.h"
#include "IRLremote.h"
#include <EEPROM.h>

//Display Settings
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//Temperautre and Humidity
#define DHTPIN 5  //Pin für Temperatur und Luftfeuchtigkeit
#define DHTTYPE DHT11
DHT dht(DHTPIN,DHTTYPE);

//IR Remote Control
#define pinIR 2
CNec IRLremote;

//EEPROM BABY
int address = 0;


int pwmpin = 3;
int brakepin = 9;
int directionpin = 12;



bool power = false;
int speed = 50;
float dhtsensor = 0;
int ir = 0;
int before = 0;

float calcSpeed = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(pwmpin, OUTPUT);
  pinMode(brakepin,OUTPUT);
  pinMode(directionpin, OUTPUT);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  pinMode(5, INPUT);
  dht.begin();

  if (!IRLremote.begin(pinIR))
    Serial.println(F("You did not choose a valid pin."));
}

void loop() {
  // put your main code here, to run repeatedly:
  dhtsensor = senseTemp();
  ir = irRemote();
  calcSpeed = speed / 250.0 * 100.0;
  Serial.println(calcSpeed);  
  if(power==true){
    showDisplay(dhtsensor, ir, calcSpeed, speed);
    motor(speed);
  }
  if(power==false){
    display.display();
    display.clearDisplay();
  }
  //eepromTest();
  if(ir!=before){
    switch(ir){
      case 69:  //Einschalt Knopf
        if(power==false && ir!=before){
          power=true;
          Serial.println("POWER");
          byte val;
          speed = EEPROM.read(address);
          before = ir;          
          startMotor();
          Serial.println(speed);
        }else if(power == true && ir!=before){
          power = false;
          stopMotor(speed);
          Serial.println("Aus");
          EEPROM.update(address, speed); 
          before = ir; 
        }
        break;
      case 12:
        Serial.println("Stufe 1");
        speed = 80;
        before = ir;
        break;
      case 24:
        Serial.println("Stufe 2");
        speed = 150;
        before = ir;
        break;
      case 94:
        Serial.println("Stufe 3");
        speed = 250;
        before = ir;
        break;
      case 7:
        Serial.println("Niedriger");
        if(speed>50){
          speed = speed - 5;
        }       
        before = 0; 
        break;
      case 9:
        Serial.println("Höher");
        if(speed<250){
          speed = speed + 5;
        }
        ir = 0;
        break;
    }
  }
  delay(500);
  
}


float senseTemp(){
  //Serial.println(digitalRead(5));
  float temp = dht.readTemperature();  

  //Serial.println("Temperatur ist : ");
  //Serial.println(temp);
  if (isnan(temp)) {
    Serial.println("Failed to read from DHT sensor!");
    delay(7000);
  }
  return temp;
}

void showDisplay(float temperature, int eingabe, int accel, int pwm){
  display.display();
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,0);  
  display.print("Temperatur:");
  display.setCursor(0,20);
  display.setTextSize(1);
  //display.setTextColor(WHITE);
  display.print(temperature);
  display.print(" ");
  display.cp437(true);
  display.write(167);
  display.print("C");
  display.setCursor(100,20);
  display.print(eingabe);
  display.setCursor(0,40);
  display.setTextSize(2);
  display.println("Speed");
  display.setTextSize(1);
  display.print(accel);
  display.print("%");
  display.setCursor(100,40);
  display.print(pwm);  
  display.display();
}

int irRemote(){
  auto data = IRLremote.read();    
  if(data.command!=0){

        // Print the protocol data
    Serial.print(F("Address: "));
    Serial.println(data.address);
    Serial.print(F("Command: "));
    Serial.println(data.command);
    Serial.println();  
    return data.command;

  }
  data.command = 0;
}

void startMotor(){
  byte val;
  val = EEPROM.read(address);
  Serial.println("Start Motor");
  digitalWrite(brakepin,LOW);
  digitalWrite(directionpin,LOW);
  analogWrite(pwmpin, val);
}

void stopMotor(int accel){
  Serial.println("Stop Motor");
  digitalWrite(brakepin,HIGH);
  digitalWrite(directionpin,LOW);
  analogWrite(pwmpin, 0);  
  EEPROM.update(address, accel);  
  //EEPROM.update(address, speed);
}

void motor(int accel){
  Serial.println(accel);
  digitalWrite(brakepin,LOW);
  digitalWrite(directionpin,LOW);
  analogWrite(pwmpin, accel);
}

void eepromTest(){
  byte val;
  val = EEPROM.read(address);
  Serial.println(val);
  delay(2000);
  Serial.println("Safe");
  EEPROM.update(address, 240);
}