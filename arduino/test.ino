#include <PZEM004Tv30.h>
#include <Wire.h>
#include <SoftwareSerial.h>




#define capBank_0_83 A0
#define capBank_1_10 A1
#define capBank_2_16 A2
#define capBank_2_50 A3
#define capBank_2_54 A4
#define capBank_2_57 A5
#define capBank_2_70 12
#define capBank_2_71 13

SoftwareSerial mySerial(3, 2);  //SIM800L Tx & Rx is connected to Arduino #3 & #2


#define PzemRxPin 8
#define PzemTxPin 9

PZEM004Tv30 pzem(PzemRxPin, PzemTxPin);

float phi_1 = 0;  //OLD POWER FACTOR ANGLE
float phi_2 = 0;  //NEW POWER FACTOR ANGLE
float Qc = 0;     // REQUIRED REACTIVE POWER
float new_pf = 0.99;
float pi = 22.0000 / 7.0000;
float C = 0;  //req_C
float voltage;
float energy;
float frequency;
float pf;
float power;
float current;
int send_count = 0;

void setup() {
  pinMode(capBank_0_83, OUTPUT);
  pinMode(capBank_1_10, OUTPUT);
  pinMode(capBank_2_16, OUTPUT);
  pinMode(capBank_2_50, OUTPUT);
  pinMode(capBank_2_54, OUTPUT);
  pinMode(capBank_2_57, OUTPUT);
  pinMode(capBank_2_70, OUTPUT);
  pinMode(capBank_2_71, OUTPUT);


  Serial.begin(9600);
  //Begin serial communication with Arduino and SIM800L
  mySerial.begin(9600);

  Serial.println("Initializing...");


  delay(1000);
}

void loop() {


  voltage = pzem.voltage();
  if (voltage != NAN) {
    Serial.print("Voltage: ");
    Serial.print(voltage);
    Serial.println("V");

  } else {
    Serial.println("Error reading voltage");
  }

  current = pzem.current();
  if (current != NAN) {
    Serial.print("Current: ");
    Serial.print(current);
    Serial.println("A");

  } else {
    Serial.println("Error reading current");
  }


  power = pzem.power();
  if (current != NAN) {
    Serial.print("Power: ");
    Serial.print(power);
    Serial.println("W");

  } else {
    Serial.println("Error reading power");
  }

  energy = pzem.energy();
  if (current != NAN) {
    Serial.print("Energy: ");
    Serial.print(energy, 3);
    Serial.println("kWh");


  } else {
    Serial.println("Error reading energy");
  }

  frequency = pzem.frequency();
  if (current != NAN) {
    Serial.print("Frequency: ");
    Serial.print(frequency, 1);
    Serial.println("Hz");

  } else {
    Serial.println("Error reading frequency");
  }

  pf = pzem.pf();
  if (current != NAN) {
    Serial.print("PF: ");
    Serial.println(pf);

  } else {
    Serial.println("Error reading power factor");
  }

  phi_1 = acos(pf) * 180 / pi;
  phi_2 = acos(new_pf) * 180 / pi;
  Qc = power * (tan(phi_1) - tan(phi_2));
  C = Qc / (voltage * voltage * 2 * pi * frequency);



  Serial.print("Phi 1 = ");
  Serial.println(phi_1);
  Serial.print("Phi 2 = ");
  Serial.println(phi_2);
  Serial.print("Required Reactive Power = ");
  Serial.println(Qc);
  Serial.print("Required Capacitor Value = ");
  Serial.println(C, 10);

  if (send_count >= 6) {
    capBankSwitch();
  }

  if (send_count >= 12) {
    goto skip_sms;
  }
  mySerial.println("AT");  //Once the handshake test is successful, it will back to OK
  updateSerial();

  mySerial.println("AT+CMGF=1");  // Configuring TEXT mode
  updateSerial();
  mySerial.println("AT+CNMI=1,2,0,0,0");  // Decides how newly arrived SMS messages should be handled
  updateSerial();
  mySerial.println("AT+CMGS=\"+919902583612\"");  //change ZZ with country code and xxxxxxxxxxx with phone number to sms
  updateSerial();


  if (send_count == 0) {
    mySerial.println(pzem.voltage());
  } else if (send_count == 1) {
    mySerial.println(pzem.current());
  } else if (send_count == 2) {
    mySerial.println(pzem.power());
  } else if (send_count == 3) {
    mySerial.println(pzem.frequency());
  } else if (send_count == 4) {
    mySerial.println(pzem.energy());
  } else if (send_count == 5) {
    mySerial.println(pzem.pf());
  } else if (send_count == 6) {
    mySerial.println(pzem.voltage());
  } else if (send_count == 7) {
    mySerial.println(pzem.current());
  } else if (send_count == 8) {
    mySerial.println(pzem.power());
  } else if (send_count == 9) {
    mySerial.println(pzem.frequency());
  } else if (send_count == 10) {
    mySerial.println(pzem.energy());
  } else if (send_count == 11) {
    mySerial.println(pzem.pf());
  } else if (send_count >= 12) {

skip_sms_01:
    goto skip_sms;

  } else



    updateSerial();
  mySerial.write(26);


skip_sms:
  if (send_count >= 12) {
    goto skip_sms_01;
  }
  send_count++;
  Serial.print(send_count);

  delay(1000);
}
void updateSerial() {
  delay(500);
  while (Serial.available()) {
    mySerial.write(Serial.read());  //Forward what Serial received to Software Serial Port
  }
  while (mySerial.available()) {
    Serial.write(mySerial.read());  //Forward what Software Serial received to Serial Port
  }
}






void capBankSwitch() {

  int C_check = C * 1000000;
  // digitalWrite(capBank_0_83, HIGH);
  int old_pf = pf;
  // int new_pf = old_pf;
  // int pf = new_pf;

  if (old_pf > new_pf) {

    Serial.print(C_check);
    if ((C_check >= 3 && C_check <= 5) || (C_check >= 14 && C_check <= 15) || C_check >= 17) {
      digitalWrite(capBank_0_83, HIGH);
  

    } else if ((C_check >= 0 && C_check <= 2) || (C_check >= 4 && C_check <= 5) || (C_check >= 6 && C <= 7) || (C_check >= 8 && C_check <= 9) || (C_check >= 11 && C_check <= 12) || (C_check >= 13 && C_check <= 15) || C_check >= 16) {

      digitalWrite(capBank_1_10, HIGH);
      
      
    }
    if ((C_check >= 2 && C_check <= 3) || (C_check >= 7 && C_check <= 8) || C_check >= 12) {
      digitalWrite(capBank_2_50, HIGH);

      

    }
    if ((C_check >= 3 && C_check <= 8) | C_check >= 10) {
      digitalWrite(capBank_2_54, HIGH);

      

    }
    if ((C_check >= 5 && C_check <= 8) && C_check >= 10) {
      digitalWrite(capBank_2_57, HIGH);

      
    }
    if (C_check >= 8) {
      digitalWrite(capBank_2_70, HIGH);

     

    }
    if ((C_check >= 8 && C_check <= 10) && C_check >= 15) {
      digitalWrite(capBank_2_71, HIGH);

      

    }
  }
}