#include <Servo.h>        //Библиотека для сервопривода
#include "OneWire.h"      //Библиотека для DS18b20
#include "DHT.h"          //Библиотека для DH22

#define DHTPIN 8          //пин для температурного датчика DH22 (верх)
#define DHTTYPE DHT22     //инициализация датчика DH22
DHT dht(DHTPIN, DHTTYPE); //инициализация датчика DH22

OneWire ds(9);            //пин для температурного датчика DS18b20 (низ)

Servo servo;              //инициализация серво привода

String relayCommand = ""; // Команды от OctoPrint для включения\выключения модуля реле
boolean recievedFlag;     // Флаг для обработки команд relayCommand

String Relay_3dPrinterOn = "RL_PR_ON";    //включить принтер
String Relay_3dPrinterOff = "RL_PR_OFF";  //выключить принтер
String Relay_Fan1On = "RL_FAN1_ON";       //включить выдувной вентилятор  
String Relay_Fan1Off = "RL_FAN1_OFF";     //выключить выдувной вентилятор
String Relay_Fan2On = "RL_FAN2_ON";       //включить вентилятор циркуляции
String Relay_Fan2Off = "RL_FAN2_OFF";     //выключить вентилятор циркуляции
String Relay_LedOn = "RL_LED_ON";         //включить подсветку
String Relay_LedOff = "RL_LED_OFF";       //выключить подстветку
String Relay_HeatOn = "RL_HEAT_ON";       //включить обогрев
String Relay_HeatOff = "RL_HEAT_OFF";     //выключить обогрев
String Relay_PwrOn = "RL_PW_ON";          //включить БП 12В
String Relay_PwrOff =  "RL_PW_OFF";       //выключить БП 12B

String TemperatureButtonGet =  "TMP_BUTTON";  //получить значение нижней температуры
String TemperatureTopGet =  "TMP_TOP";        //получить значение верхней температуры

int pinRL_3dPrint = 2;                    //пин реле принтера - 220 В
int pinRL_Fan1 = 6;                       //пин реле вентилятора выдува - 12 В
int pinRL_Fan2 = 7;                       //пин реле вентилятора циркуляции - 12 В
int pinRL_Led = 5;                        //пин реле подсветки - 12 В
int pinRL_Heat = 3;                       //пин реле обогрева - 220 В
int pinRL_PWR = 4;                        //пин реле БП 12В - 220 В

void setup() {
  
  Serial.begin(9600);                     //включаем UART
  dht.begin();                            //Включаем датчик DH22
  servo.attach(10);                       //устанавливаем пин сервопривода
  
  pinMode(pinRL_3dPrint, OUTPUT);
  pinMode(pinRL_Fan1, OUTPUT);
  pinMode(pinRL_Fan2, OUTPUT);
  pinMode(pinRL_Led, OUTPUT);
  pinMode(pinRL_Heat, OUTPUT);
  pinMode(pinRL_PWR, OUTPUT);
  
  digitalWrite(pinRL_3dPrint, HIGH);
  digitalWrite(pinRL_Fan1, HIGH);
  digitalWrite(pinRL_Fan2, HIGH);
  digitalWrite(pinRL_Led, HIGH);
  digitalWrite(pinRL_Heat, HIGH);
  digitalWrite(pinRL_PWR, HIGH);

}

void loop() {

  if (Serial.available()) { //Слушаем команды от OctoPrint
    ReadUART();             
  }

  climatControl();          //Проверяем температуру, контролируем климат.
     
}

void ReadUART(){
  while (Serial.available() > 0) {         // ПОКА есть что то на вход    
    delay(50);                              // ЗАДЕРЖКА. Без неё работает некорректно!
    relayCommand += (char)Serial.read();   // забиваем строку принятыми данными
    recievedFlag = true;                   // поднять флаг что получили данные
  }

  if (recievedFlag) {                    // Если в буфере что-то есть
    int lenString =  relayCommand.length();    // выясняем длину команды
    relayCommand = relayCommand.substring(0,lenString-2); // отрезаем последние спец символы
    if(relayCommand.substring(0,2) == "RL") { // Если команда начинается на 'RL' запускаем функцию управления реле
        RelayControll(relayCommand); 
    }
    relayCommand = "";                          // очистить
    recievedFlag = false;                  // опустить флаг
  }
}

void RelayControll(String command){
  if (command == Relay_3dPrinterOn) {
    digitalWrite(pinRL_3dPrint, LOW);    // Включить принтер
  }
  if (command == Relay_3dPrinterOff) {
    digitalWrite(pinRL_3dPrint, HIGH);   //выключить принтер
  }
  if (command == Relay_Fan1On) {
    digitalWrite(pinRL_Fan1, LOW);      //включить выдув
  }
  if (command == Relay_Fan1Off) {
    digitalWrite(pinRL_Fan1, HIGH); 
  }
  if (command == Relay_Fan2On) {
    digitalWrite(pinRL_Fan2, LOW);
  }
  if (command == Relay_Fan2Off) {
    digitalWrite(pinRL_Fan2, HIGH);
  }
  if (command == Relay_LedOn) {
    digitalWrite(pinRL_Led, LOW);
  }
  if (command == Relay_LedOff) {
    digitalWrite(pinRL_Led, HIGH);
  }
  if (command == Relay_HeatOn) {
    digitalWrite(pinRL_Heat, LOW);
  }
  if (command == Relay_HeatOff) {
      digitalWrite(pinRL_Heat, HIGH);
  }
  if (command == Relay_PwrOn) {
    digitalWrite(pinRL_PWR, LOW);
  }
  if (command == Relay_PwrOff) {
    digitalWrite(pinRL_PWR, HIGH);
  }
}

void climatControl() {
  int tTop = getTemperatureTop();
  int tButton = getTemperatureButton();

  if (tTop >= tButton+2){
    RelayControll(Relay_Fan2On);  
  }
  else {
    RelayControll(Relay_Fan2Off);
  }

  if (tTop > 42){
    RelayControll(Relay_Fan1On);
    getServo(1);
  }
  if (tTop < 39) {
    RelayControll(Relay_Fan1Off);
    getServo(0);
  }

  if (tButton < 12) {
    RelayControll(Relay_HeatOn);
  }
  if (tButton > 15) {
    RelayControll(Relay_HeatOff);
  }
}

int getTemperatureTop() {
  //float h = dht.readHumidity(); //Измеряем влажность
  float t = dht.readTemperature(); //Измеряем температуру
  if (isnan(t)) {  // Проверка. Если не удается считать показания, возращается значение 1000
    return 1000;
  }
  return t;        // Отдаем значение температуры
}

int getTemperatureButton() {
  byte data[2]; // Место для значения температуры
  
  ds.reset(); // Начинаем взаимодействие со сброса всех предыдущих команд и параметров
  ds.write(0xCC); // Даем датчику DS18b20 команду пропустить поиск по адресу. В нашем случае только одно устрйоство 
  ds.write(0x44); // Даем датчику DS18b20 команду измерить температуру. Само значение температуры мы еще не получаем - датчик его положит во внутреннюю память
  
  delay(1000); // Микросхема измеряет температуру, а мы ждем.  
  
  ds.reset(); // Теперь готовимся получить значение измеренной температуры
  ds.write(0xCC); 
  ds.write(0xBE); // Просим передать нам значение регистров со значением температуры

  // Получаем и считываем ответ
  data[0] = ds.read(); // Читаем младший байт значения температуры
  data[1] = ds.read(); // А теперь старший

  // Формируем итоговое значение: 
  //    - сперва "склеиваем" значение, 
  //    - затем умножаем его на коэффициент, соответсвующий разрешающей способности (для 12 бит по умолчанию - это 0,0625)
  float t =  ((data[1] << 8) | data[0]) * 0.0625;
  
  if (isnan(t)) {  // Проверка. Если не удается считать показания, возращается значение 1000
    return 1000;
  }
  return t;
}

int getServo(int staus){

  servo.write(0); //ставим вал под 0

  delay(2000); //ждем 2 секунды

  servo.write(180); //ставим вал под 180

  delay(2000); //ждем 2 секунды

}
