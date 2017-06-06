#include "Servo.h"

/*
TO DO 
  - OPTIMISE CODE  USE milis() instead delay()
  - PID GAINS CALIBRATION
  - TEST SERIAL READ / SERIAL WRITE USING UART REGISTER FLAGS
  - TEST SERIAL READ / SERIAL WRITE USING TDD (TIME DIVISION DUPLEX)
*/


//////////////////////////////////////////////////////////////
///////////////////////////////////////////////////CONSTANTS
/////////////////////////////////////////////////////////


//ANALOGNI PINOVI

//SENZORI
#define LIVO 0  //crveni
#define GORI 1 //kafeno-bili 
#define DESNO 2   //zeleni
#define DOLI 3    //plavi

//SOLARNI PANEL
#define PANEL 5

//POTENCIOMETRI JOYSTICKA
#define joystick_Y 3
#define joystick_X 4

//DIGITALI PINOVI

//interupti moraju bit pin 2 i 3  //0 1 u kodu 2 i 3 na hardwareu
#define menu 2 
#define joystick_rememberPosition 3

//LEDS
#define YELLOW_LED 10
#define BLUE_LED 11

//RELAY
#define RELAY 8

//SERVO PINS
#define PIN_VERTICAL 5
#define PIN_HORIZONTAL 6

//////////////////////////////////////////////////////////////
////////////////////////////////////////////GLOBAL VARIABLES
/////////////////////////////////////////////////////////

typedef struct Angle
{
  int horizontal;
  int vertical;
};

Servo horizontal, vertical;
int poz;
Angle angles;


int hPosition;
int vPosition;

volatile int mode = 1;    //1 je default za automatski mod
volatile bool savePosition = false;
Angle savedPosition;

int timeFromStart = 0; 

//////////////////////////////////////////////////////////////
///////////////////////////////////////FUNCTION DECLARATIONS
/////////////////////////////////////////////////////////

typedef struct Angle;
int pins[4] = {LIVO, GORI, DESNO, DOLI};
float average(int[]);
void ocitavanjeSenzora(int[]);
void ispisVrijednostiSenzora(int[]);
void prominaPolozaja(int *);
void testMotora();
float mirenjeVotazeNaPanelu();
void joystickanje();
void manualMode(int *);
void automaticMode(int *);
Angle pomeranje(int[]);
void relayTest();
void modeChangeHandler();
void positionRememberHandler();
void aMode(int *);
void mMode();
float _PIDCalculation(int, int);
void spCalculation(int[], int *, int *);

//TESTS 
void secondsCounter();
void onRecieved();
//////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////SETUP
/////////////////////////////////////////////////////////

long startUpTime = 0;

void setup() 
{
  delay(100);
  Serial.begin(9600);
 // Serial.begin(57600);

  horizontal.attach(PIN_HORIZONTAL);
  vertical.attach(PIN_VERTICAL);
  horizontal.write(90);
  vertical.write(90);
  pinMode(RELAY,OUTPUT);
  pinMode(YELLOW_LED,OUTPUT);
  pinMode(BLUE_LED, OUTPUT);
  angles.horizontal = 90;
  angles.vertical =  45;
  savedPosition.horizontal = 90;
  savedPosition.vertical = 45;



  //test
  pinMode(13, OUTPUT);
  
  //

  //optimisation
    startUpTime = millis();  
  //

  /*
  interuppts menu button
  interrupt modes: 
   RISING`, which activates an interrupt on a rising edge of the interrupt pin,
   FALLING`, which activates on a falling edge,
   CHANGE`, which responds to any change in the interrupt pin's value, 
   LOW`, which triggers any time the pin is a digital low.
  */


///////////////////////////////////////////////////////INTERRUPTS ATTACH

  attachInterrupt(digitalPinToInterrupt(menu),  modeChangeHandler, RISING);
  //joystick remember position
  attachInterrupt(digitalPinToInterrupt(joystick_rememberPosition), positionRememberHandler, RISING);

}

//////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////LOOP
//////////////////////////////////////////////////////////


void loop() {

  int val[5];   //5

digitalWrite(13,HIGH);
/*
    mode = 2;
    if(mode == 2)
    {
      onRecieved();
    }
*/

    if(mode == 0)
    {
       mMode();
    }
    else if(mode == 1)
    {
      aMode(val); 
      
    }
    else
    {
      mode = 0;
    }

delay(500);

}


///////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////NEW STUFF
//////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////SERIAL DATA RECIEVED HANDLER
//////////////////////////////////////////////////////////////////////

//void serialEvent(){
//
//    /* String podaci = Serial.readString();
//     char *data;
//    podaci.toCharArray(data,3);
//    if(data[0] == 'C')
//    {
//      */
//      if(Serial.read() == 2000)
//      {
//        digitalWrite(13,LOW);
//        mode++;
//                delay(100);
//      }
//
//
//}


/*
void onRecieved()
{



        if (Serial.available() > 0) {
                // read the incoming byte:

                // say what you got:
                Serial.print("I received: ");
                Serial.println(Serial.read());
                digitalWrite(13,HIGH);
                delay(100);
                digitalWrite(13,LOW);
                delay(100);
        }

}
*/

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////INTERRUPT HANDLERS
//////////////////////////////////////////////////////////////////////

//interupt handler za tipku za prominu moda
void modeChangeHandler()
{
    mode++;
}

//interupt handler za tipku joysticka
void positionRememberHandler()
{
  savedPosition.horizontal = horizontal.read();
  savedPosition.vertical = vertical.read();
}

//////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////FUNCTIONS
/////////////////////////////////////////////////////////////

void aMode(int *val)
{
  if(millis() - startUpTime >= 500)
  {
    startUpTime = millis();
    
    digitalWrite(YELLOW_LED, HIGH);
    digitalWrite(BLUE_LED, LOW);

    digitalWrite(RELAY, LOW);

    ocitavanjeSenzora(val);
    ispisVrijednostiSenzora(val);

  }
}

void mMode()
{
  if(millis() - startUpTime >= 100)
  {
    startUpTime = millis();

    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(BLUE_LED, HIGH);

    digitalWrite(RELAY, HIGH);

    Serial.print(analogRead(3));
    Serial.print(" ");
    Serial.println(analogRead(4));
  }
}


void manualMode(int* val)
{
  digitalWrite(BLUE_LED, HIGH);
  digitalWrite(YELLOW_LED, LOW);
  joystickanje();
}

void automaticMode(int* val)
{
  digitalWrite(BLUE_LED, LOW);
  digitalWrite(YELLOW_LED, HIGH);

  ocitavanjeSenzora(val);
  ispisVrijednostiSenzora(val);
}


float mirenjeNaponaNaPanelu()
{
  return analogRead(5) * (5.0 / 1023);
}

void ispisVrijednostiSenzora(int val[])
{
  Serial.print(F("Livo:"));
  Serial.print(val[LIVO]);

  Serial.print("\t");

  Serial.print(F("Gori:"));
  Serial.print( val[GORI]);

  Serial.print("\t");

  Serial.print(F("Desno:"));
  Serial.print(val[DESNO]);

  Serial.print("\t");

  Serial.print(F("Doli:"));
  Serial.print( val[DOLI]);

  Serial.print("\t");

  Serial.print(F("Avg:"));
  Serial.print( average(val));

  Serial.print("\t");
  
  Serial.print(F("Panel:"));
  Serial.print( mirenjeNaponaNaPanelu());

  Serial.println();
}

void joystickanje()
{
  int x = analogRead(4);
  int y = analogRead(5);
  int remember = digitalRead(joystick_rememberPosition);

  int x_position = map(x, 0, 1023, 0, 180);
  int y_position = map(y, 0, 1023, 0, 180);

  horizontal.write(x_position);
  vertical.write(y_position);
}

void prominaPolozaja(int val[])
{

  if (abs(val[LIVO] - val[DESNO]) < 10 )
  {
    return;
  }
  else if (val[LIVO] > val[DESNO])
  {
    angles.horizontal += 10;
  }
  else if (val[LIVO] < val[DESNO])
  {
    angles.horizontal -= 10;
  }

  if (abs(val[GORI] - val[DOLI]) < 10 )
  {
    return;
  }
  else if (val[GORI] > val[DOLI])
  {
    angles.vertical += 10;
  }
  else if (val[GORI] < val[DOLI])
  {
    angles.vertical -= 10;
  }
  horizontal.write(angles.horizontal);
  vertical.write(angles.vertical);

  hPosition = horizontal.read();
  vPosition = vertical.read();

  Serial.print(F("Horizontal position = "));
  Serial.print(hPosition);
  Serial.print("\t");
  Serial.print(F("Vertical position = "));
  Serial.print(vPosition);
  Serial.println();
  Serial.println();
/*
  if (hPosition == 0)
  {
    angles.horizontal = 180;
  }

  if (hPosition == 180)
  {
    angles.horizontal = 0;
  }

  if (vPosition == 0)
  {
    angles.vertical = 180;
  }

  if (vPosition == 180)
  {
    angles.vertical = 0;
  }
*/
}


Angle pomeranje(int val[])
{
  int left = val[0];
  int up = val[1];
  int right = val[2];
  int down = val[3];
  Angle platformAngle;

  //vertical aligment

  if(up == down)
    platformAngle.horizontal = platformAngle.horizontal;
  else if(up > down)
      platformAngle.horizontal += 2;
  else if(down > up)
      platformAngle.horizontal -=2;
  if(left == right)
    platformAngle.vertical = platformAngle.vertical;
  else if(up > down)
      platformAngle.vertical += 2;
  else if(down > up)
      platformAngle.vertical -=2;

    return platformAngle;
}


void ocitavanjeSenzora(int *val)
{
  for (int i = 0; i < 4; i++)
  {
    val[i] = analogRead(pins[i]);
  }
}

float average(int data[])
{
  int sum;

  for (int i = 0; i < 4; i++)
  {
    sum = sum + data[i];
  }
  return sum / 4;
}

//////////////////////////////////////////////////////////////
//////////////////////////////////////////////PID CONTROLLER
//////////////////////////////////////////////////////////

void prominaPolozajaPID(int val[])
{
  int verticalSP;
  int horizontalSP;
  spCalculation(val,&horizontalSP, &verticalSP);

  //vertical
  _PIDCalculation(verticalSP, vertical.read());
  //horizontal
  _PIDCalculation(horizontalSP, horizontal.read());

}

float _PIDCalculation(int sp, int pv)
{
  // constants
  float kI = 0.5;   //integral gain
  float kP = 0.2;   //proportion gain
  float kD = 0.01;  //derivation gain
  float dt = 0.01;  //100ms
  float toleration = 10;

  //other
  float output;
  float preOutput;
  float error = sp - pv;
  
  float integral, derivation, proportion;

  if(error < toleration)
  {
    proportion = error * kP;

    integral = integral * error*dt;

    derivation = (error) / dt;

    preOutput = output;

    output = kP*proportion + kI*integral + kD*derivation;

    return output;
  }

  preOutput = output;
  return output;
}


void spCalculation(int values[], int* hSP, int* vSP)
{
  // LIVO 0
  // GORI 1
  // DESNO 2
  // DOLI 3

  *hSP = ( values[0] + values[2] ) / 2;
  *vSP = ( values[1] + values[3] ) / 2;

}
//////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////TESTS
//////////////////////////////////////////////////////////

void testMotora()
{
  for (poz = 0; poz < 180; poz++) // motor se okrece od 0 do 180 stupnjeva
  { // u koracima od 1 stupnja
    horizontal.write(poz);                // kazemo servo da provjeri varijablu poz za poziciju
    horizontal.write(poz);
    delay(15);                       // cekamo 15ms da motor dode na zadanu poziciju
  }

  for (poz = 180; poz >= 1; poz--)  // vracamo motor na pocetnu poziciju
  {
    vertical.write(poz);
    vertical.write(poz);
    delay(15);
  }
}

void secondsCounter()
{
  static int i = 0;
  if(millis() - startUpTime >= 5)
  {
    startUpTime = millis();
    Serial.println(i++);
  }
}
