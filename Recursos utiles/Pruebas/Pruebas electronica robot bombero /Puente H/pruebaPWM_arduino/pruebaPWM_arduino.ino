//motor A  en A01 y A02

//motor B  en B01 y B02

 

int STBY = 9; //activacion

 

//Motor A

int PWMA = 6; //control de vel

int AIN1 = 8; //sentido

int AIN2 = 7; //sentid

 

//Motor B

int PWMB = 11; 

int BIN1 = 10; 

int BIN2 = 12; 

 

void setup(){

pinMode(STBY, OUTPUT);

 

pinMode(PWMA, OUTPUT);

pinMode(AIN1, OUTPUT);

pinMode(AIN2, OUTPUT);

 

pinMode(PWMB, OUTPUT);

pinMode(BIN1, OUTPUT);

pinMode(BIN2, OUTPUT);

}

 

void loop(){

move(1, 255, 1); //motor 1, maxima vel, izq

move(2, 255, 1); //motor 2, full speed, izq

 

delay(1000); 

stop(); 

delay(250); 

 

move(1, 128, 0); //motor 1, maxima vel, der

move(2, 128, 0); //motor 2, maxima vel, der

 

delay(1000);

stop();

delay(250);

}

 

void move(int motor, int speed, int direction){ 

digitalWrite(STBY, HIGH); 
 

boolean inPin1 = LOW;

boolean inPin2 = HIGH;

if(direction == 1){

inPin1 = HIGH;

inPin2 = LOW;

}


if(motor == 1){

digitalWrite(AIN1, inPin1);

digitalWrite(AIN2, inPin2);

analogWrite(PWMA, speed);

}else{

digitalWrite(BIN1, inPin1);

digitalWrite(BIN2, inPin2);

analogWrite(PWMB, speed);

}

}


void stop(){

//enable standby

digitalWrite(STBY, LOW);

}
