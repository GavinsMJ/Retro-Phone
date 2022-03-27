//Rotary Phone By Gavins Maragia
/**
 * get the software serial library from library manager
  get the TinyGsm library from library manager
  get the StreamDebugger library from library manager
 * 
 * Read numbers from a rotary dial (or otherwise using pulse dialing; see
 * http://en.wikipedia.org/wiki/Rotary_dial ). 
 *
 */

//Edit code to suit your display of choice
//change the gsm modem to whatever you're using 

//I chose to use the HT1621 display because of its symbols, it has :
// 1 MUTE,, 2 SOUND,, 3 MUSIC,, 4  CLOCK,, 5 -HASH-
// 6 Arrow(->) ,, 7 CIRCLES,, 8  KEY,, 9 BAT LOW
//The display was salvaged from an old office telephone

#define DEBUG 0      //set 1 for serial read statements

#define TINY_GSM_MODEM_SIM800
#define MaxCall_Length  60     //MAXIMUM CALL DURATION IN SECONDS_ _ phone will auto hangup after this time 

#include <Arduino.h>
#include <HT1621.h>
#include<stdio.h>
#include<string.h>
#include <SoftwareSerial.h>
#include <TinyGsmClient.h>
#include <RotaryDialer.h>

void(* resetFunc) (void) = 0;       // creating pointer at memory address 0

#if DEBUG == 1
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)

#else
#define debug(x)
#define debugln(x)
#endif

SoftwareSerial SerialAT(3, 2);  // RX, TX
//SIM800L Tx & Rx is connected to Arduino #3 & #2


// See all AT commands, if wanted
#define DUMP_AT_COMMANDS

#define SerialMon Serial
#define TINY_GSM_DEBUG SerialMon
// communication, set a fixed baud rate using modem.setBaud(#).
#define GSM_AUTOBAUD_MIN 9600
#define GSM_AUTOBAUD_MAX 115200

#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger(SerialAT, SerialMon);
TinyGsm        modem(debugger);
#else
TinyGsm        modem(SerialAT);
#endif


//Rotary dial
HT1621    LCD;

//Rotary Dial pins
#define PIN_READY  7  // blue
#define PIN_PULSE  8  // orange

RotaryDialer dialer = RotaryDialer(PIN_READY, PIN_PULSE);

String num;
String last_called;
String msg;
bool lastcalled = false;
bool back_ = false;
bool in_call = false;
bool res;
int x = 0 ;
int index = 4;
unsigned long previousMillis = 0;        // will store last time number was read
const long interval = 90000;           // clears the screen after interval time if no activity


void clr() {

  LCD.clrScr();
  LCD.symbol(9, true);
  LCD.print("Input PHONE no.");
  LCD.update();
  index = 4;
  num = "";
  x = 0;
}

void counter()
{

  LCD.clrScr();
  LCD.symbol(2, true);
  LCD.print("OnCall_ 0");
  LCD.update();

  int sec = 0;
  unsigned long previousMillis_ = 0;
  const long Second_ = 1000;
  bool onCall = true;

  while (onCall) {
    unsigned long currentMillis_ = millis();
    if (currentMillis_ - previousMillis_ >= Second_) {
      sec++;
      debugln(sec);

      LCD.clrScr();
      LCD.print("OnCall_ ");
      char buf_[3];
      itoa(sec, buf_, 10);

      LCD.print(buf_, 9); //call seconds counter
      LCD.update();

      previousMillis_ = currentMillis_;
    }



    if (sec == MaxCall_Length) {

      LCD.symbol(2, false);
      onCall = false;
    }

    if ((digitalRead(6) == LOW))  // using backspace as end call button
    {

      LCD.symbol(2, false);
      res = modem.callHangup();  //hangs up if backspace button is pressed
      DBG("Hang up:", res ? "OK" : "fail");
      delay(100);

      return;
    }

  }

}

void call_(String& CALL_TARGET)
{
  DBG("Calling:", CALL_TARGET);

  LCD.clrScr();
  LCD.print("Call");
  LCD.update();


  res = modem.callNumber(CALL_TARGET);
  DBG("Call:", res ? "OK" : "fail");

  LCD.clrScr();
  LCD.print("Calling-- ");
  LCD.update();
  delay(100);

  if (res) {
     delay(3000);
    /*
      // Play DTMF A, duration 1000ms
      modem.dtmfSend('A', 1000);

      // Play DTMF 0..4, default duration (100ms)
      for (char tone = '0'; tone <= '4'; tone++) {
      modem.dtmfSend(tone);
      }
    */

    ///this needs to be optimized take the challange ? ;)
    /*
      //identify response
      int activity = 0;
      int8_t status = modem.waitResponse(2000L, GF("OK"), GF("BUSY"), GF("NO ANSWER"), GF("NO CARRIER"));
      switch (status) {
      case 1: counter();break;
      case 2: activity = 1;break;
      case 3: activity = 2;break;
      default: activity = 3;
      }

      if(activity == 1){
      LCD.clrScr();
      LCD.print("BUSY ");
      LCD.update();
      delay(700);
      //clr();
      //return;
      }
      if(activity == 2){
      LCD.clrScr();
      LCD.print("NO ANSWER");
      LCD.update();
      delay(700);
      //clr();
      //return;
      }
      if(activity == 3){
      LCD.clrScr();
      LCD.print("NO CARRIER");
      LCD.update();
      delay(700);
      //clr();
      //return;
      }
    */

    counter();

  }

  res = modem.callHangup();  //hangs up after MaxCall_Length seconds
  DBG("Hang up:", res ? "OK" : "fail");

  LCD.clrScr();
  LCD.print("after__");
  LCD.update();

  clr();
  return;
}

/*
  void sms_(String& SMS_TARGET)
  {
  String imei = "323435323232";
  res = modem.sendSMS(SMS_TARGET, String("Hello from ") + imei);
  DBG("SMS:", res ? "OK" : "fail");

  res = modem.sendSMS_UTF8_begin(SMS_TARGET);
  if (res) {
    auto stream = modem.sendSMS_UTF8_stream();
    stream.print(F(" Print number: "));
    stream.print(595);
    res = modem.sendSMS_UTF8_end();
  }
  DBG("UTF8 SMS:", res ? "OK" : "fail");
  }
*/


char * toArray(int number)
{
  int n = log10(number) + 1;
  int i;
  char *numberArray = calloc(n, sizeof(char));
  for (i = n - 1; i >= 0; --i, number /= 10)
  {
    numberArray[i] = (number % 10) + '0';
  }
  return numberArray;
}


void powerdown_()
{
  modem.poweroff();
  DBG("Poweroff.");
}

void ProcessMsg() {

  if ( msg.indexOf( "RING" ) >= 0 )
  {
    unsigned long currentMillis = millis();
    previousMillis = currentMillis;
    LCD.clrScr();
    LCD.symbol(3, true);
    LCD.print("INCOMING CALL");
    LCD.update();
    in_call = true;
  }
  msg = "";

}

void ReadSerial()
{
  char character;
  while (SerialAT.available())
  {
    character = SerialAT.read();
    msg.concat(character);
    delay (5);
  }

  if (msg != "") {
    ProcessMsg();
    msg.remove(0);     // redundancy

  }
}


void setup_net()
{
  // Set GSM module baud rate
  modem.setBaud(115200);
  SerialAT.begin(115200);

  LCD.clrScr();

  LCD.print("CON__");
  LCD.update();
  delay(100);

  //modem.restart();
  modem.init();

  delay(50);

  String name = modem.getModemName();
  DBG("Modem Name:", name);

  String modemInfo = modem.getModemInfo();
  DBG("Modem Info:", modemInfo);

  LCD.print("____", 7);
  LCD.update();

  /*
    debug("Waiting for network...");
    if (!modem.waitForNetwork()) {
    SerialMon.println(" fail");

    LCD.clrScr();

    LCD.print("fail");
    LCD.update();
    delay(100);
    return;
    }
  */

  delay(100);

  LCD.clrScr();
  LCD.print("sucess");
  LCD.update();

  delay(500);
  debugln(" success");

  if (modem.isNetworkConnected()) {
    debugln("Network connected");

    LCD.clrScr();
    LCD.print("NET GOOD");
    LCD.update();

  }

  /*
    String ccid = modem.getSimCCID();
    DBG("CCID:", ccid);

    String imei = modem.getIMEI();
    DBG("IMEI:", imei);

    String imsi = modem.getIMSI();
    DBG("IMSI:", imsi);

    String cop = modem.getOperator();
    DBG("Operator:", cop);

    int csq = modem.getSignalQuality();
    DBG("Signal quality:", csq);

    String ussd_balance = modem.sendUSSD("*144#");
    DBG("Balance (USSD):", ussd_balance);

    String ussd_phone_num = modem.sendUSSD("*100*4*1*1*#");
    DBG("Phone number (USSD):", ussd_phone_num);
  */

}


void setup() {


  Serial.begin(115200);


  debugln("Starting sketch");

  LCD.begin(10, 11, 12); //   PINS (WD,DATA,CS)
  LCD.clrScr();


  /// 1 MUTE,, 2 SOUND,, 3 MUSIC Note,, 4  CLOCK,, 5 -HASH-
  /// 6 -> ,, 7 CIRCLES,, 8  KEY,, 9 BAT LOW
  for (int j = 1 ; j < 10; j++) {
    LCD.symbol(j, false);
  }


  LCD.print("Retro Phone ");    //  create a string (string, [start position])

  LCD.update();
  delay(700);

  LCD.clrScr();

  LCD.print("By Gavins");
  LCD.update();

  delay(700);

  DBG("Wait...");
  setup_net();


  delay(500);

  LCD.clrScr();
  LCD.symbol(9, true);
  LCD.print("Input PHONE no.");
  LCD.update();

  dialer.setup();

  pinMode(5, INPUT_PULLUP);
  pinMode(6, INPUT_PULLUP);
  pinMode(9, INPUT_PULLUP);
  pinMode(A3, INPUT_PULLUP);
}

void loop() {
  /*
    DBG("Initializing modem...");
    if (!modem.restart()) {
      // if (!modem.init()) {
      DBG("Failed to restart modem, delaying 10s and retrying");
      // restart autobaud in case GSM just rebooted
      TinyGsmAutoBaud(SerialAT, GSM_AUTOBAUD_MIN, GSM_AUTOBAUD_MAX);
      return;
    }
  */
  if ((digitalRead(5) == LOW)) { // clear

    clr();
    delay(100);
  }

  if ((digitalRead(6) == LOW &&  in_call == false)) { // backspace

    if (num.length() > 0)
    {
      back_ = true;
      delay(500);
    }

  }

  if ((digitalRead(A3) == LOW)) { // INCOMING CALL
    ReadSerial();
  } else {
    LCD.symbol(3, false);
    LCD.update();
    in_call = false;
  }

  //ATA – Accepts incoming call.
  //ATH – Hangs up the call
  if ((digitalRead(9) == LOW) /*&& (num.length() == 0*/ &&  (in_call == true)) {
    SerialAT.println("ATA");
    counter();
    clr();
  }
  if ((digitalRead(6) == LOW &&  in_call == true)) { // backspace
    SerialAT.println("ATH");
    LCD.clrScr();
    LCD.print("Hanging up");
    LCD.update();
    delay(500);
    in_call = false;
    clr();
  }

  if ((digitalRead(9) == LOW) && (num.length() == 0 &&  in_call == false)) {

    delay(200);
    LCD.clrScr();
    LCD.print("calling last call");
    LCD.update();

    if (lastcalled == true) {
      call_(last_called);
    }
    delay(200);
    clr();

  }


  if ((num.length() == 10) && (digitalRead(9) == LOW))
  {
    int i = 1;
    String mob;

    for (int j = 0; j < 9; j++, i++) {
      mob += num[i];
    }

    String cmd = ("+254" + mob);
    last_called = "";
    last_called = cmd;
    //cmd += mob;
    //cmd += (F("\""));
    //cmd += (F("\r"));

    LCD.clrScr();
    LCD.print("__calling__");
    LCD.update();

    delay(500);

    call_(cmd);

    delay(500);

    lastcalled = true;

    LCD.clrScr();
    LCD.print("_END call__ _ _ _");
    LCD.update();

    delay(200);
    clr();

  }

  bool change = false;
  int num_;
  if (dialer.update()) {
    num_ = dialer.getNextNumber();
    Serial.println(num_);
    change = true;
  }

  unsigned long currentMillis = millis();
  if (change == true) {

    if (x == 0) {
      LCD.clrScr();
      LCD.update();
    }
    num += (String)num_;
    Serial.println(num);

    LCD.print("no_ ");

    char buf[2];
    itoa(num_, buf, 10);
    Serial.print ("Char   ");
    Serial.println(buf);


    LCD.print(buf, index);
    LCD.update();

    previousMillis = currentMillis;


    index = index + 1;
    x = x + 1;
  }
  if (back_ == true) {

    index = index - 1;
    x = x - 1;

    int cx = (num.length() - 1);
    String buffer_ = num;
    num = "";
    for (int j = 0; j < cx; j++) {
      num += buffer_[j];
    }

    LCD.print(" ", index);
    LCD.update();
    back_ = false;
  }

  if (currentMillis - previousMillis >= interval) {
    // clear screen after interval time if no activity takes place
    in_call = false;
    clr();
  }

}