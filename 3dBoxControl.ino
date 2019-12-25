#include <Servo.h>
#include "OneWire.h"
#include "DHT.h"

#define DHTPIN 8
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

OneWire ds(9);
Servo servo;

String relayCommand = ""; // Команды от OctoPrint для включения\выключения модуля реле
boolean recievedFlag;     // Флаг для обработки команд relayCommand

String Relay_3dPrinterOn = "RL_PR_ON\n";
String Relay_3dPrinterOff = "RL_PR_OFF\n";
String Relay_Fan1On = "RL_FAN1_ON\n";
String Relay_Fan1Off = "RL_FAN1_OFF\n";
String Relay_Fan2On = "RL_FAN2_ON\n";
String Relay_Fan2Off = "RL_FAN2_OFF\n";
String Relay_LedOn = "RL_LED_ON\n";
String Relay_LedOff = "RL_LED_OFF\n";
String Relay_HeatOn = "RL_HEAT_ON\n";
String Relay_HeatOff = "RL_HEAT_OFF\n";
String Relay_PwrOn = "RL_PW_ON\n";
String Relay_PwrOff =  "RL_PW_OFF\n";

String TemperatureButtonGet =  "TMP_BUTTON\n";
String TemperatureTopGet =  "TMP_TOP\n";

int pinRL_3dPrint = 2;
int pinRL_Fan1 = 3;
int pinRL_Fan2 = 4;
int pinRL_Led = 5;
int pinRL_Heat = 6;
int pinRL_PWR = 7;


void setup() {
  
  Serial.begin(9600);
  dht.begin();
  servo.attach(10);
  
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

  if (Serial.available()) {Relay();}
  
  delay(2000); // 2 секунды задержки
  
  getTemperatureButton();
  getTemperatureTop();
  climatControl();
 
   
}

void Relay(){

  while (Serial.available() > 0) {         // ПОКА есть что то на вход    
    relayCommand += (char)Serial.read();        // забиваем строку принятыми данными
    recievedFlag = true;                   // поднять флаг что получили данные
    delay(2);                              // ЗАДЕРЖКА. Без неё работает некорректно!
  }

  if (recievedFlag) {                      // если данные получены
    if (relayCommand == Relay_3dPrinterOn) {
      digitalWrite(pinRL_3dPrint, LOW);
    }
    if (relayCommand == Relay_3dPrinterOff) {
      digitalWrite(pinRL_3dPrint, HIGH);
    }
    
    if (relayCommand == Relay_Fan1On) {
      digitalWrite(pinRL_Fan1, LOW);
    }
    
    if (relayCommand == Relay_Fan1Off) {
      digitalWrite(pinRL_Fan1, HIGH); 
    }

    if (relayCommand == Relay_Fan2On) {
      digitalWrite(pinRL_Fan2, LOW);
    }
    
    if (relayCommand == Relay_Fan2Off) {
      digitalWrite(pinRL_Fan2, HIGH);
    }

    if (relayCommand == Relay_LedOn) {
      digitalWrite(pinRL_Led, LOW);
    }
    
    if (relayCommand == Relay_LedOff) {
      digitalWrite(pinRL_Led, HIGH);
    }

    if (relayCommand == Relay_HeatOn) {
      digitalWrite(pinRL_Heat, LOW);
    }
    
    if (relayCommand == Relay_HeatOff) {
      digitalWrite(pinRL_Heat, HIGH);
    }

    if (relayCommand == Relay_PwrOn) {
      digitalWrite(pinRL_PWR, LOW);
    }
    
    if (relayCommand == Relay_PwrOff) {
      digitalWrite(pinRL_PWR, HIGH);
    }
    Serial.println(relayCommand);
  }
    relayCommand = "";                          // очистить
    recievedFlag = false;                  // опустить флаг

}

int getTemperatureButton() {

  //float h = dht.readHumidity(); //Измеряем влажность
  float t = dht.readTemperature(); //Измеряем температуру
  if (isnan(t)) {  // Проверка. Если не удается считать показания, выводится «Ошибка считывания», и программа завершает работу
    Serial.println("Ошибка считывания");
    return;
  }
  
  Serial.print("Нижняя Температура: ");
  Serial.print(t);
  Serial.println(" *C ");
  return t;

}

int getTemperatureTop() {
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
  
  // Выводим полученное значение температуры в монитор порта
  Serial.print("Верхняя Температура: ");
  Serial.print(t);
  Serial.println(" *C ");
  return t;

}

void climatControl() {
  int tTop = getTemperatureTop();
  int tButton = getTemperatureButton();

  if (tTop >= tButton+2){digitalWrite(pinRL_Fan2, LOW);  }
  else {digitalWrite(pinRL_Fan2, HIGH);}

  if (tTop > 42){
    digitalWrite(pinRL_Fan1, LOW);
    getServo(1);
  }
  else {
    digitalWrite(pinRL_Fan1, HIGH);
    getServo(0);
  }

  if (tButton < 15) {digitalWrite(pinRL_Heat, LOW);}
  else {digitalWrite(pinRL_Heat, HIGH);}
  
  

}

int getServo(int staus){

  servo.write(0); //ставим вал под 0

  delay(2000); //ждем 2 секунды

  servo.write(180); //ставим вал под 180

  delay(2000); //ждем 2 секунды

}
