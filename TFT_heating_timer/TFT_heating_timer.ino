#include <SPI.h>
#include <EEPROMex.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <menu.h>//menu macros and objects
#include <genericKeyboard.h>
#include <menuFields.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <menuGFX.h>
//-------KEYS--------
#define keyboard_AnalogInput A6
#define btnUP     menu::downCode
#define btnDOWN   menu::upCode
#define btnLEFT   menu::escCode
#define btnENTER  menu::enterCode
#define btnNONE   -1
//-------TFT---------
#define tftCS 10
#define dc    8
#define rst   7
//-------TEMP SENSOR-
#define ONE_WIRE_BUS A0
//-------PINs--------
// 2-5 Kimenetek
const int motorPin = 5;
const int fanPin = 4;
const int ledPin = 9;
const int inputPin = A1;
boolean ledState = HIGH;
boolean motorState = HIGH;   // unsigned int volt
boolean inputState = HIGH;

//-------VARS--------
unsigned long OnTime = 0;
unsigned long OffTime = 0;
unsigned long previousMillis = 0;
unsigned long prevMenuMillis = 0;
bool reqHeat = true;
const long egyezer = 1000;       //másodperc
const long hatvanezer = 60000;   //perc
int secCounter = 0;
int lastMin = 1;
boolean updMain = true;
boolean updTime = true;
boolean redrawTemp = true;

int old_button;
bool runMenu=false;
bool screenCleared = 0;

float tempC = 0.0;
float lasttempC = 5;
boolean thermostat = true;

byte futesStart = 2;
byte futesStop = 60;
byte tartasStart = 2;
byte tartasStop = 30;
byte temperature = 70;
byte histeresis = 2;

byte menuTimeout = 60;
byte activeProfil = 1;
//-------CHART VARS-----------------
int newVal = 0;
int tempChart[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
int chartInterval = 600; // 10 perc
int chartTimer = 0;

//-------EEPROM Adresses------------
byte addrfStart1 = 10;
byte addrfStop1 = 20;
byte addrtStart1 = 30;
byte addrtStop1 = 40;
byte addrTemp1 = 50;
byte addrHis1 = 60;
byte addrMenuhide = 70;
byte addrProfil = 80;

byte addrfStart2 = 90;
byte addrfStop2 = 100;
byte addrtStart2 = 110;
byte addrtStop2 = 120;
byte addrTemp2 = 130;
byte addrHis2 = 140;

byte addrfStart3 = 150;
byte addrfStop3 = 160;
byte addrtStart3 = 170;
byte addrtStop3 = 180;
byte addrTemp3 = 190;
byte addrHis3 = 200;

//-------DISPLAY------------
Adafruit_ST7735 tft(tftCS, dc, rst);
menuGFX gfx(tft);
//-------TEMP SENSOR--------
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

/* Colors: BLACK, WHITE, RED, YELLOW, CYAN, MAGENTA, BRIGHT_RED, LIGHT_GREY, 
DARK_GREY */

//aux function
bool nothing() {}

bool pauseMenu() {
  runMenu=false;
  updMain=true;
  updTime=true;
  redrawTemp = true;
  screenCleared = 0;
  return true;
  }

bool loadProfil1() {
 activeProfil = 1;
 EEPROM.write(addrProfil,activeProfil);
 futesStart = EEPROM.read(addrfStart1);
 futesStop = EEPROM.read(addrfStop1);
 tartasStart = EEPROM.read(addrtStart1);
 tartasStop = EEPROM.read(addrtStop1);
 temperature = EEPROM.read(addrTemp1);
 histeresis = EEPROM.read(addrHis1);
 return true;
}
bool loadProfil2() {
 activeProfil = 2;
 EEPROM.write(addrProfil,activeProfil);
 futesStart = EEPROM.read(addrfStart2);
 futesStop = EEPROM.read(addrfStop2);
 tartasStart = EEPROM.read(addrtStart2);
 tartasStop = EEPROM.read(addrtStop2);
 temperature = EEPROM.read(addrTemp2);
 histeresis = EEPROM.read(addrHis2);
 return true;
}
bool loadProfil3() {
 activeProfil = 3;
 EEPROM.write(addrProfil,activeProfil);
 futesStart = EEPROM.read(addrfStart3);
 futesStop = EEPROM.read(addrfStop3);
 tartasStart = EEPROM.read(addrtStart3);
 tartasStop = EEPROM.read(addrtStop3);
 temperature = EEPROM.read(addrTemp3);
 histeresis = EEPROM.read(addrHis3);
 return true;
}
//-------------DISPLAY CHART-----------
bool chart() {
  tft.fillScreen(BLACK); 
  tft.drawLine(5, 120, 5, 20, WHITE);
  tft.drawLine(5, 120, 155, 120, WHITE);

  tft.drawLine(0, 100, 4, 100, WHITE);
  tft.drawLine(0, 80, 4, 80, WHITE);
  tft.drawLine(0, 60, 4, 60, WHITE);
  tft.drawLine(0, 40, 4, 40, WHITE);

  tft.drawLine(25, 121, 25, 125, WHITE);  
  tft.drawLine(45, 121, 45, 125, WHITE);
  tft.drawLine(65, 121, 65, 125, WHITE);  
  tft.drawLine(85, 121, 85, 125, WHITE);
  tft.drawLine(105, 121, 105, 125, WHITE);  
  tft.drawLine(125, 121, 125, 125, WHITE);
  tft.drawLine(145, 121, 145, 125, WHITE);

  tft.fillRect(6, 1, 155, 118, BLACK);
  tft.drawLine(6, temperature, 155, temperature, BLUE);  // Setpoint
  for(int x=1;x<12;x++) {
  tft.drawLine(x*10, tempChart[x-1], (x+1)*10, tempChart[x], GREEN);
  delay(1000);
  return true;
 }
}
//-------------- UPTIME-----------
void displayUptime() {
  int h,m,s;
  s = millis() / 1000;
  m = s / 60;
  h = s / 3600;
  s = s - m * 60;
  m = m - h * 60;
  if (m!=lastMin || updTime==true) {
    lastMin=m;
    tft.setTextColor(MAGENTA);
    tft.fillRect(0,80,128,20,BLACK);
    tft.setCursor(0, 80); 
    tft.print("Run: ");
    tft.print(h);
    printDigits(m);
    tft.println();    
    updTime=false;
  }
}

void printDigits(int digits) {
  tft.print(":");
  if(digits < 10)
    tft.print('0');
  tft.print(digits);
}
//---------Functions---------
void mainScreen() {
 if (reqHeat==true) {
  tft.fillCircle(150,8,5,RED);
 } else if (reqHeat==false) {
  tft.fillCircle(150,8,5,GREEN);
 }
 if (updMain==true) {
  updMain=false;
  runMenu=false;
  tft.setTextColor(WHITE);
  tft.setCursor(0, 0); 
  tft.print("Profil: ");
  tft.println(activeProfil);
  tft.setTextColor(GREEN);
  tft.setCursor(0, 40); 
  tft.print("Set temp: ");
  tft.println(temperature);
  tft.setCursor(0, 60); 
  tft.print("Thermostat:");
  if (thermostat==1) {
    tft.println("BE");
   } else if (thermostat==0) {
    tft.println("KI");
   }
  }
  
  displayUptime(); 
  
  if (redrawTemp) {
  tft.fillRect(60,20,98,18,BLUE);
  tft.setCursor(0, 20); 
  tft.setTextColor(GREEN);
  tft.print("TEMP: ");
  tft.setTextColor(YELLOW);
  tft.print(tempC);tft.println(" C");
  redrawTemp = false;
 }
}

bool saveProfil1() {
  EEPROM.write(addrProfil,activeProfil);
  EEPROM.write(addrMenuhide,menuTimeout);
  EEPROM.write(addrfStart1,futesStart);
  EEPROM.write(addrfStop1,futesStop);
  EEPROM.write(addrtStart1,tartasStart);
  EEPROM.write(addrtStop1,tartasStop);
  EEPROM.write(addrTemp1,temperature);
  EEPROM.write(addrHis1,histeresis);
  return true;
  }

bool saveProfil2() {
  EEPROM.write(addrProfil,activeProfil);
  EEPROM.write(addrMenuhide,menuTimeout);
  EEPROM.write(addrfStart2,futesStart);
  EEPROM.write(addrfStop2,futesStop);
  EEPROM.write(addrtStart2,tartasStart);
  EEPROM.write(addrtStop2,tartasStop);
  EEPROM.write(addrTemp2,temperature);
  EEPROM.write(addrHis2,histeresis);
  return true;
  }

bool saveProfil3() {
  EEPROM.write(addrProfil,activeProfil);
  EEPROM.write(addrMenuhide,menuTimeout);
  EEPROM.write(addrfStart3,futesStart);
  EEPROM.write(addrfStop3,futesStop);
  EEPROM.write(addrtStart3,tartasStart);
  EEPROM.write(addrtStop3,tartasStop);
  EEPROM.write(addrTemp3,temperature);
  EEPROM.write(addrHis3,histeresis);
  return true;
  }

void updVar() {

  if (reqHeat == true) { 
    OnTime = futesStart * egyezer;
    OffTime = futesStop * egyezer;    
  } else {
    OnTime = tartasStart * egyezer;
    OffTime = tartasStop * hatvanezer;   //ez percben van
  }
}

//-----------------------ÜZEMMÓD VÁLTÁS--------------------------
void readInput() {
 if (thermostat == false) {         // Bemenet vezérelt üzemmód
  
    if (digitalRead(inputPin) == LOW) {       // Ha alacsony akkor fűtűnk
    delay(400);
    if (digitalRead(inputPin) == LOW) {
      reqHeat = true;
      digitalWrite(fanPin, LOW);         // Ventillátor bekapcsolása
    } else {
      reqHeat = false;
      digitalWrite(fanPin, HIGH);        // Ventillátor kikapcsolása
    }
  }
 } else if (thermostat == true) {   // Thermostat vezérelt üzemmód
  //valami
   if (tempC < temperature - histeresis && digitalRead(inputPin) == LOW) {
      reqHeat = true;
      digitalWrite(fanPin, LOW);         // Ventillátor bekapcsolása
    } 
   if (tempC >= temperature || digitalRead(inputPin) == HIGH) {
      reqHeat = false;
      digitalWrite(fanPin, HIGH);         // Ventillátor kikapcsolása 
   }
 }  
}

//---------MENU TREE---------

CHOOSE(temperature,temp,"Temp",
    VALUE("60",60,nothing),
    VALUE("65",65,nothing),
    VALUE("70",70,nothing),
    VALUE("75",75,nothing),
    VALUE("80",80,nothing)
);

CHOOSE(histeresis,hister,"Toler.",
    VALUE("2",2,nothing),
    VALUE("4",4,nothing),
    VALUE("6",6,nothing)
);   

CHOOSE(menuTimeout,mtimer,"Menu Timer",
    VALUE("60",60,nothing),
    VALUE("30",30,nothing),
    VALUE("15",15,nothing)
);  

CHOOSE(thermostat,tempCtl,"Thermostat",
    VALUE("BE",1,nothing),
    VALUE("KI",0,nothing)
);

MENU(timerMenu,"Idozites",
    FIELD(futesStart,"fStart","s",1,20,1,1,nothing),
    FIELD(futesStop,"fStop","s",1,200,10,1,nothing),
    FIELD(tartasStart,"tStart","s",1,20,1,1,nothing),
    FIELD(tartasStop,"tStop","p",1,30,10,1,nothing)
);

MENU(loadProfil,"Profilok",
    OP("Profil 1",loadProfil1),
    OP("Profil 2",loadProfil2),
    OP("Profil 3",loadProfil3)
);

MENU(saveProfil,"Mentes",
    OP("Profil 1",saveProfil1),
    OP("Profil 2",saveProfil2),
    OP("Profil 3",saveProfil3)
);

MENU(options,"Beallitasok",
    SUBMENU(loadProfil),
    SUBMENU(mtimer),
    OP("Frissit seb",nothing),
    SUBMENU(saveProfil),
    OP("Firmware",nothing)
);

MENU(subMenu,"Homerseklet",
    SUBMENU(temp),
    SUBMENU(hister),
    SUBMENU(tempCtl)
);

MENU(mainMenu,"Main menu",
    SUBMENU(timerMenu),
    SUBMENU(subMenu),
    SUBMENU(options),
    OP("Exit",pauseMenu)
);

//---------------------------

int readKeyboard() {
  int button, button2, pressed_button;  
  button = getButton();
  if (button != old_button) {
      delay(50);        // debounce
      button2 = getButton();

      if (button == button2) {
         secCounter = 0;
         old_button = button;
         pressed_button = button;
         if(button != 0) {
           if(button == 1) return btnLEFT;
           if(button == 2) return btnUP;
           if(button == 3) return btnDOWN;
           if(button == 4) return btnENTER;
         }
      }
  }else{
    return btnNONE;
  }
}

genericKeyboard mykb(readKeyboard);

int getButton() {
  int i, z, button;
  int sum = 0;

  for (i=0; i < 4; i++) {
     sum += analogRead(keyboard_AnalogInput);
  }
  z = sum / 4;
  if (z > 1000) button = 0;                                           
  else if (z >= 0 && z < 20) button = 1; //LEFT                    
  else if (z > 50 && z < 100) button = 2; //UP                
  else if (z > 210 && z < 250) button = 3; //DOWN                
  else if (z > 150 && z < 190) button = 4; //RIGHT           
  else button = 0;

  return button;
}

void setup() {
  Serial.begin(9600);
  pinMode(motorPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(inputPin, INPUT_PULLUP);
  SPI.begin();
  sensors.begin();
  tft.initR(INITR_BLACKTAB);
  tft.setRotation(1);
  tft.setTextWrap(false);
  tft.setTextColor(ST7735_RED,ST7735_BLACK);
  tft.setTextSize(2);
  gfx.resX*=2;//update resolution after font size change
  gfx.resY*=2;//update resolution after font size change
  gfx.maxX=13;
  gfx.maxY=7;
  gfx.bgColor=RGB565(125,125,125);
  tft.fillScreen(BLACK);
  menuTimeout = EEPROM.read(addrMenuhide);
  activeProfil = EEPROM.read(addrProfil);
  if (activeProfil == 1 ) loadProfil1();
  if (activeProfil == 2 ) loadProfil2();
  if (activeProfil == 3 ) loadProfil3();
}

void loop() {
  readInput();
  updVar();
  if (runMenu) { 
    mainMenu.poll(gfx,mykb);
  }  else if (readKeyboard()==menu::enterCode) {
    runMenu=true;
    tft.setTextColor(ST7735_RED,ST7735_BLACK);
    mainMenu.redraw(gfx,mykb);
    mainMenu.poll(gfx,mykb);
  } else {
    if (screenCleared == 0) {
      tft.fillScreen(BLACK); 
      tft.drawLine(0, 17, 160, 17, RED);
      tft.drawLine(0, 18, 160, 18, RED);
      screenCleared = 1;
    }
    mainScreen();
    secCounter = 0;
  }
//--------------- CSIGA VEZÉRLÉS ------------
  unsigned long currentMillis = millis();

  if ((motorState == LOW) && (currentMillis - previousMillis >= OnTime))
      {
      motorState = HIGH;  // Turn it off
      previousMillis = currentMillis;
      digitalWrite(motorPin, motorState);
      digitalWrite(ledPin, motorState);
      }
  else if ((motorState == HIGH) && (currentMillis - previousMillis >= OffTime))
      {
      motorState = LOW;  // turn it on
      previousMillis = currentMillis;
      digitalWrite(motorPin, motorState);
      digitalWrite(ledPin, motorState);
      }
//---------------MENU TIMER-----------------------
unsigned long currMenuMillis = millis();
if (currMenuMillis - prevMenuMillis >= 1000) {
    chartTimer++;
  if (chartTimer >= chartInterval) {
    chartTimer =0;
      for(int idx=0;idx<11;idx++) {
       tempChart[idx] = tempChart[idx+1];
       }
       tempChart[11] = newVal; 
  }
  if (runMenu) {
    secCounter++;
  }
  prevMenuMillis = currMenuMillis;
}
if (secCounter >= menuTimeout) {
  tft.fillScreen(BLACK); 
  runMenu = false;
  secCounter = 0;
  updMain=true;
  updTime=true;
  redrawTemp = true;
  screenCleared = 0;
}
//--------------INFINITE LOOP---------------------
sensors.setWaitForConversion(false);                                         // makes it async
sensors.requestTemperatures();  
  if (sensors.getTempCByIndex(0) > -100) {
    tempC = sensors.getTempCByIndex(0);
    if (lasttempC != tempC) {
    lasttempC = tempC;
    redrawTemp = true;
     }
   }
}
