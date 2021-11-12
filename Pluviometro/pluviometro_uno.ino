#define RAIN_PIN 2 //Pin que recibe la interrupcion
#define CALC_INTERVALO 1000 // Incrementos por medicion
#define DEBOUNCE_TIME 15 // time * 1000 en microsegundos requerido para el ruido

// Resolution .2mm
// Accuracy 500 mm per hour +/- 1%
// Range 700mm per hour
// Average Switch closure Time 135 ms
// Temperatura de operacion 0° a 70° C
// Bounce Settling Time 0.75 ms

unsigned long nextCalc;
unsigned long timer;

/*
Typically global variables are used to pass data between an ISR and the main program. 
To make sure variables shared between an ISR and the main program are updated correctly, 
declare them as volatile.
*/

volatile unsigned int triggerSwitch = 0;
volatile unsigned long last_micros_rg;




void setup() {
  Serial.begin(115200);
  //attachInterrupt(digitalPinToInterrupt(pin), ISR, mode);
  attachInterrupt(digitalPinToInterrupt(RAIN_PIN), countingRain, RISING);
  pinMode(RAIN_PIN, INPUT);
  nextCalc = millis() + CALC_INTERVAL;
  // Pin que recibe la interrupcion -> digitalPinToInterrupt(), 
  
  //Pines digitales para Arduino Uno 2 y 3
  /*
  sólo uno puede ejecutarse a la vez, otras interrupciones se ejecutarán después 
  de que la actual termine en un orden que depende de la prioridad que tienen. 
  
  millis() se basa en las interrupciones para contar, por lo que nunca se incrementará 
  dentro de un ISR. 
  
  Dado que 
    delay() requiere interrupciones para funcionar, no funcionará si se llama dentro de un ISR. 
  
  micros() funciona inicialmente pero empezará a comportarse erráticamente después de 1-2 ms. 
  
  delayMicroseconds() no utiliza ningún contador, por lo que funcionará normalmente.
  */

  /*
  *About Interrupt Service Routines

  ISRs are special kinds of functions that have some unique limitations most other functions 
  do not have. 
  
  An ISR cannot have any parameters, and they shouldn’t return anything.
  */

  /*
  mode

    LOW to trigger the interrupt whenever the pin is low,

    CHANGE to trigger the interrupt whenever the pin changes value

    RISING to trigger when the pin goes from low to high,

    FALLING for when the pin goes from high to low.

The Due, Zero and MKR1000 boards allow also:

    HIGH to trigger the interrupt whenever the pin is high.


  */
}

void loop() {
 
  timer = millis();
  if(timer > nextCalc){
    nextCalc = timer + CALC_INTERVAL;
    Serial.print("Total Tips: ");
    Serial.println((float) rainTrigger);
  }
}

void countingRain(){
  /*
    Comprueba si el tiempo desde la última llamada de interrupción es mayor que 
    el tiempo de rebote. Si es así, entonces la última llamada de interrupción es 
    a través del período ruidoso del rebote del interruptor de láminas, por lo que 
    podemos incrementar en uno.   

    El tiempo que ha transcurrido - el tiempo que ha transcurrido desde el ultimo valor
  */
  if ((long)(micros() - last_micros_rg)>= DEBOUNCE_TIME*1000){
    triggerSwitch += 1;
    last_micros_rg = micros(); 
  }
}
