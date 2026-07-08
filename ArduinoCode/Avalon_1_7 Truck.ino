///-------Bibliotecas utilizadas------------
#include <ESP32Servo.h>                   //Biblioteca do micro servo motor
#include <QMC5883LCompass.h>              //Biblioteca da bússola
#include <Ultrasonic.h>                   //Biblioteca do sensor ultrassonico 
#include "BluetoothSerial.h"              //Biblioteca do Bluetooth clássico 
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Checa se o bluetooth está disponível 
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif
// Checa o perfil do Serial Port
#if !defined(CONFIG_BT_SPP_ENABLED)
#error Serial Port Profile for Bluetooth is not available or not enabled. It is only available for the ESP32 chip.
#endif   

//----- Definições---pino----
#define IN1          27   //Pino IN1 de controle da direção do motor
#define IN2          26   //Pino IN2 de controle da direção do motor
#define Enable       14   //Pino de enable que habilita a ponte H 

//--------------Leds do sistema------------
#define FarolA        25    //Farol da frente 
#define FarolB        33    //Farol de tras
#define SetaD         12    //Seta da direita
#define SetaE         32    //Seta da Esquerda

//----------Sonoro-----------
#define buzzer        15

//------------Sensores------------------------

//--------SR-HC04 Distância -------
//Sensor da frente
#define Trig1         2
#define Echo1         4

//-----------Sensor de trás---------- 
#define Trig2         5
#define Echo2        18

//---------Servo motor----------
#define servoPin       13

//-------------Variaveis---------
//Variáveis bluetooth
char caractereVindo;
String message =      "";   

//------------Variáveis do display OLED------------
int    WITDH =        128;  //Largura do OLED
int    HEIGHT =       64;   //Altura do OLED
int  SCREEN_ADDRESS = 0x3C; //Endereço do display

//------------Variável do PWM------------ 
int    duty     =     0;    //duty cycle

//---------Variáveis dos sensores de distância------------
int distanciaA;                 //Sensor da frente 
int distanciaB;                 //Sensor atrás 
boolean HCSR04Reading = true;   //Mantém a tela Distancia rodando 
int y = 0;                      //Faz com que o bip do sensor de distância não seja intermitente 

//-------Variáveis Bússola--------
char matriz[3];
byte a;
boolean compassReading = false;   //Manter a tela da bússola atualizando   
char positionAlert;              //Variável usada para guardar a posição da bússola onde há algum perigo ---Sistema Esp Eye's -------
String typeOfAlert;              //Váriavel usada para guardar o tipo de alerta: CLIMA, DESCIDA, ACIDENTE
boolean compassReading2 = false; //Manter a tela Esp Eye atualizando 
int x1;                  //*Variáveis para acessar o plano da bússola x, y e z
int y2;                  //*
int z1;                  //*

//----------------------Variáveis da função Esp Eye--------------------
boolean velocidadeProibida1 = false;    //* Ambas limitam a velocidade do motor quando acionadas
boolean velocidadeProibida2 = false;    // *
int x = 0;                              //Não deixa que o buzzer fique tocando continuamente 

//--------------------Variáveis do sistema------------------
boolean Sistema = true;                 //Mantém o Monitor Atualizando
boolean StartStop = false;              //Liga o StartStop
boolean AvisoSonoro = false;            //Liga o Alerta Sonoro
//--------------Função Navegação------------------------ 
boolean Navegacao = false;              //Permite a Navegação
int navigation_y1;                     //Define um ponto no eixo y
int autorizacao = 0;                   //Autoriza se o processo pode ser iniciado
int velocidade = 0;                    //Seleciona qual a velocidade padrão da navegação 
boolean vel1 = false;                 //Diminui a velocidade por causa do obstáculo
boolean chegada_y0 = false;           //Sinaliza se a primeira linha de nav. foi completa
int local_faixay0 = 0;                //Local da faixa no eixo y
boolean condicao_farol = false;       //Indica se o farol está aberto ou não

//variáveis da função de auxílio a mudança de faixa ou entrada
int TinhaCarro = 0;   //x = 0: Não tem carro;      x = 1: Tem (ou tinha) um carro; 

//A Flag1 é uma flag de controle, ela sinaliza que o sistema foi ligado
boolean Flag1   =     false;
//Caso está flag esteja com true, o motor gira para frente, caso esteja false dará ré
boolean F_T     =     true;   
//variável que indica se o farol foi ligado. farol ligado = true.
boolean FarolLigado = false;

//Criação da Instância a partir de Bluetooth Serial 
BluetoothSerial SerialBlue;  

//Instância do display oled
Adafruit_SSD1306 oled( WITDH,    HEIGHT, &Wire,  -1);

///Instância do HC SR04 da frente
Ultrasonic sonarA(Trig1, Echo1);

//Instância do HC SR04 de trás 
Ultrasonic sonarB(Trig2, Echo2);

//Instância da bússola 
QMC5883LCompass bussola;

//Objeto do servo motor 
Servo servo1;

void setup() {
  
  //---Inicia o serial, bluetooth classic e o Display OLED
  Serial.begin(115200);
  oled.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);  //Inicializa o display OLED com o endereço 0x3C
  oled.clearDisplay();
  oled.display();

  SerialBlue.begin("Avalon");                       ///Dá nome ao dispositivo bluetooth
  Serial.println("Bluetooth Avalon Pronto");
  bussola.init();                                   //Inicia a bússola 
   servo1.setPeriodHertz(50);
   servo1.attach(servoPin, 500, 2400);               //Configura o pino do PWM do SG90

                           
//-Confg-dos-ports--------I/O-------
    pinMode(IN1,        OUTPUT);
    pinMode(IN2,        OUTPUT);
    pinMode(Enable,     OUTPUT);

    pinMode(FarolA,     OUTPUT);
    pinMode(FarolB,     OUTPUT);

    pinMode(SetaD,      OUTPUT);
    pinMode(SetaE,      OUTPUT);

    pinMode(buzzer,     OUTPUT);

//Configuração do PWM do motor 
analogWriteResolution(Enable, 8);
analogWriteFrequency(Enable, 40000);

//configuração do PWM do farol 
analogWriteResolution(FarolA,   8);
analogWriteFrequency(FarolA,  5000);

analogWriteResolution(FarolB,   8);
analogWriteFrequency(FarolB,  5000);

//Inicia todos os pinos com 0 por precaução 
analogWrite(FarolA,      0);
analogWrite(FarolB,      0);
digitalWrite(SetaD,       LOW);
digitalWrite(SetaE,       LOW);
digitalWrite(buzzer,      LOW);

}
void loop() {

//Confere a distância uma vez no loop 
distanciaA = sonarA.read();
distanciaB = sonarB.read();

//Confere a direção pela bússola uma vez no loop
Bussola();

//-----Condicionais do Bluetooth---------
///Configura que a mensagem recebida será mesclada em Message
if(SerialBlue.available()){
  caractereVindo = SerialBlue.read();
    if(caractereVindo != '/n'){
        message += String(caractereVindo); //Converte os caracteres em String
      }
  }
else{
    message = "";
}

//Ligar Sistema
if(message == "Ligar"){
    Flag1 = true;                        //Aciona a Flag que será utilizada para sinalizar que o sistema foi ligado
    Serial.println(message);
    Welcome();                            // * Roda as respectivas funções
    funcaoPWM();                          // *
}
///Desliga o sistema   
if(message == "Desligar"){
    Flag1 = false;                        //Coloca false na Flag 
    Serial.println(message);
    oled.clearDisplay();
    oled.display();
}

///Seleção da direção da rotação do motor (frente ou trás)
//--------F para frente -------------------
if(message == "F" && Flag1 == true){
  //Configura os pinos da ponte
  F_T = true;
  digitalWrite(IN1,       LOW);
  digitalWrite(IN2,       HIGH);
  digitalWrite(Enable,    LOW);   //Enable bloqueia a ponte para que apenas o motor gire quando for selecionado o PWM

  funcaoPWM();

  //Coloca na tela um quadrado selecionando esta opção
  oled.drawRect(5, 20, 15, 15, SSD1306_WHITE);
  oled.display();
}
//--------------T para trás --------------------
if(message == "T" && Flag1 == true){
  //Configura os pinos da ponte
  digitalWrite(IN1,       HIGH);
  digitalWrite(IN2,       LOW);
  digitalWrite(Enable,    LOW);
  F_T = false; //Muda a flag para sinalizar que é ré 

  funcaoPWM();

  //Coloca na tela um quadrado selecionando esta opção
  oled.drawRect(35, 20, 15, 15, SSD1306_WHITE);
  oled.display();

  //Ativa a função sonora caso ela for selecionada 
  if(AvisoSonoro == true){
    digitalWrite(buzzer,      HIGH);
    delay(50);
    digitalWrite(buzzer,      LOW);
  }
}

////--------Seleção do PWM --------------
//--------------Neutro------------------------
//-----------bluetooth---------Flag utilizada---------
if(message == "Neutro" && Flag1 == true){
     duty = 0;                                //Deixa o duty igual a 0
     funcaoPWM();
     //Seleciona na tela 
     ///-----------x---y--width--height--color------------
     oled.drawRect(67, 5,  15,     15,    SSD1306_WHITE);
     oled.display();

    //Mantém escrito na tela o F ou o T
     if(F_T == true){
     oled.drawRect(5, 20, 15, 15, SSD1306_WHITE);
     oled.display();
     }else{
      oled.drawRect(35, 20, 15, 15, SSD1306_WHITE);
      oled.display();
     }
}
//--------------N0---------------------------------
if(message ==  "N0"  && Flag1 == true){
//*******************************************///
      duty = 230;           //Coloca o PWM 
      funcaoPWM();

     //Seleciona na tela 
      oled.drawRect(67, 20, 15, 15, SSD1306_WHITE);
      oled.display();

     //Mantém escrito na tela o F ou o T
      if(F_T == true){
     oled.drawRect(5, 20, 15, 15, SSD1306_WHITE);
     oled.display();
     }else{
      oled.drawRect(35, 20, 15, 15, SSD1306_WHITE);
      oled.display();
     }
}
//--------------N1-------------------------------------
if(message ==  "N1"  && Flag1 == true && velocidadeProibida2 == false){
  //*****************************************//
    duty = 240;             //Coloca o PWM 
    funcaoPWM();

    //Seleciona na tela 
    oled.drawRect(67, 30, 15, 15, SSD1306_WHITE);
    oled.display();

    //Mantém escrito na tela o F ou o T
    if(F_T == true){
     oled.drawRect(5, 20, 15, 15, SSD1306_WHITE);
     oled.display();
     }else{
    oled.drawRect(35, 20, 15, 15, SSD1306_WHITE);
    oled.display();
     }
}
//--------------N2--------------------------
if(message ==  "N2" && Flag1 == true && velocidadeProibida1 == false && velocidadeProibida2 == false){
   //*******************************
    duty = 255;               //Coloca o PWM em 255
    funcaoPWM();

    //Seleciona na tela 
    oled.drawRect(67, 50, 15, 15, SSD1306_WHITE);
    oled.display();

    //Mantém escrito na tela o F ou o T
    if(F_T == true){
     oled.drawRect(5, 20, 15, 15, SSD1306_WHITE);
     oled.display();
     }else{
      oled.drawRect(35, 20, 15, 15, SSD1306_WHITE);
      oled.display();
    }
}  

////----------Fim da seleção -------------------

//Controle de distância de um objeto
if(distanciaA <= 20 && Flag1 == true){
  duty = 0;             //Para o motor para segurança 
  analogWrite(Enable,   duty);
  distanciaA = sonarA.read();

  //Atualiza na tela 
  oled.setCursor(90, 45);
  oled.print("Freio");
  oled.display();

//Aviso sonoro
if(AvisoSonoro == true){
  digitalWrite(buzzer,  HIGH);
  delay(500);
  digitalWrite(buzzer,   LOW);
}
}

//Controle da luminosidade do farol pela distância do carro da frente 
if(distanciaA > 7 && FarolLigado == true && Flag1 == true){
//Diminui o farol pois tem um carro muito perto
analogWrite(FarolA, 5);
analogWrite(FarolB, 5);
}

if(distanciaA > 15 && FarolLigado == true && Flag1 == true){
//Aumenta gradativamente o farol pois o carro está aumentando a distância
analogWrite(FarolA, 90);
analogWrite(FarolB, 90);
}

if(distanciaA > 25 && FarolLigado == true && Flag1 == true){
//Aumenta gradativamente o farol pois o carro está aumentando a distância
analogWrite(FarolA, 255 );
analogWrite(FarolB, 255);
}
if(FarolLigado == false){
analogWrite(FarolA,     0);
analogWrite(FarolB,     0);
}


//Tela com os sensores de distância
if(message == "Distancia" && Flag1 == true){
  HCSR04Reading = true;

//Lê os sensores e roda a função
  while(HCSR04Reading){
  distanciaA = sonarA.read();
  distanciaB = sonarB.read();
  VisaoUltrassom();

//Back, voltar para a página inicial 
  if(SerialBlue.read() == 'B'){
  HCSR04Reading = false;
  funcaoPWM();
  }
  }
}

//Entrar na aba sistemas 
if(message == "Sistema" && Flag1 == true){
  Sistema = true;
  System();

//Espera pela entrada do usuário
 while(Sistema){
  //Variável local que guarda os caracteres bluetooth recebidos
  char caractere = SerialBlue.read();
    //Escolhe ligar o 1. Start Stop 
  if(caractere == '1'){
      oled.fillCircle(60, 15, 3, SSD1306_WHITE);
      oled.display();
      StartStop = true;
  }
  //2. Avisos sonoros 
  if(caractere == '2'){
    oled.fillCircle(65, 25, 3, SSD1306_WHITE);
    oled.display();
    AvisoSonoro = true;
  }
  //3. Navegação 
  if(caractere == '3'){
    oled.fillCircle(80, 36, 3, SSD1306_WHITE);
    oled.display();
    Navegacao = true;
  }
  //Volta para a página inicial, Back
  if(caractere == 'B'){
    Sistema = false;
    funcaoPWM();
  }
 } 
}

//Primeira função 1. Start Stop (S/S) anda e para em congestionamentos 
if(StartStop == true && Flag1 == true){
//Habilita a ponte H no sentido horário para o motor rodar
F_T = true;
digitalWrite(IN1,       LOW);
digitalWrite(IN2,       HIGH);
digitalWrite(Enable,    LOW);  
  
while(StartStop){
//Aciona a direção para frente
//Coloca na tela um quadrado selecionando esta opção
  oled.drawRect(5, 20, 15, 15, SSD1306_WHITE);
  oled.display();

//Se o carro da frente não estiver se movimentando pare 
if(sonarA.read() < 25){
    duty = 0;         //Neutro
    funcaoPWM();
    oled.drawRect(67, 5,  15,     15,    SSD1306_WHITE);
    oled.display();
}

 //Caso se movimente avance
 if(sonarA.read() > 25){
    duty = 235;       //N0
    funcaoPWM();
    oled.drawRect(67, 20, 15, 15, SSD1306_WHITE);
    oled.display();
}

 //Retomar o controle ao motorista
 if(SerialBlue.read() == 'A'){
  StartStop = false;  //Desabilita 
  funcaoPWM();
}
}
}
///---------Fim Start Stop -----------------------------

//Terceira Função: 3. Navegação
if(Navegacao == true && Flag1 == true){
//-----Seleciona a direção F ----------------------
    digitalWrite(IN1,       LOW);
    digitalWrite(IN2,       HIGH);
    digitalWrite(Enable,    LOW);
//---------------------------------------------------
  while(Navegacao){
//Chama a função 
    Navigation();
//Faz a leitura dos sensores
    distanciaA = sonarA.read();
    distanciaB = sonarB.read();
 //Variável local que guarda os caracteres bluetooth recebidos
     char caractere = SerialBlue.read();
//-----Configurar Y0---------------
    if(caractere == 'Y'){
        navigation_y1 = 2000;
        autorizacao = 1;            //Variável de controle
    }
//------Configura a velocidade padrão--------------------------
    if(caractere == '0'){
      velocidade = 0;     //Configura a velocidade como N0
    }
    if(caractere == '1'){
      velocidade = 1;     //Configura a velocidade como N1
    }
    if(caractere == '2'){
      velocidade = 2;     //Configura a velocidade como N2
    }
//---------Configura o local da faixa de pedrestes para o farol-------------
    if(caractere == 'F'){
      local_faixay0 = 1500;
    }
//------Diz se o farol está aberto ou não-------------
    if(caractere == 'C'){
      ///C = Farol ABERTO
      condicao_farol = true;
    }
    if(caractere == 'F'){
      //F = Farol FECHADO
      condicao_farol = false;
    } 
//----------Retomar o controle ao motorista--------------------
    if(caractere == 'A'){
        Navegacao = false; 
        navigation_y1 = 0;
        autorizacao = 0;
        Serial.print(Navegacao);   
        funcaoPWM();
    }

  }
}
//--------Fim Navegação----------------------


//Desativa algumas flags do sistema 
if(message == "A" && Flag1 == true){
  AvisoSonoro = false; //Desativa o alerta sonoro
}


//Chamada da bússola
if(message == "Bussola" && Flag1 == true){
  compassReading = true;

//Repete a função de leitura da bússola
while(compassReading){
     Compass();
  
//Back para voltar a tela inicial 
    if(SerialBlue.read() == 'B'){
      compassReading = false;   //Desabilita 
      funcaoPWM();
    }
  }
}

///Sistema de avisos sobre direções 
if(message == "EspEye" && Flag1 == true){

  compassReading2 = true;
  TelaEspEye1();

  while(compassReading2 == true){
   caractereVindo = SerialBlue.read();
//Norte 
if(caractereVindo == 'N'){
      TelaEspEye1();
      oled.drawRect(3, 13, 10, 10, SSD1306_WHITE);
      positionAlert = 'N';
      Serial.println(positionAlert);

      oled.setCursor(55, 25);
      oled.print("Norte");
      oled.display();
    }
//Sul
if(caractereVindo == 'S'){
      TelaEspEye1();
      oled.drawRect(17, 13, 10, 10, SSD1306_WHITE);
      positionAlert = 'S';
      Serial.println(positionAlert);


      oled.setCursor(55, 25);
      oled.print("Sul");
      oled.display();
    }
//Leste 
if(caractereVindo == 'L'){
      TelaEspEye1();
      oled.drawRect(37, 13, 10, 10, SSD1306_WHITE);
      positionAlert = 'E';
      Serial.println(positionAlert);


      oled.setCursor(55, 25);
      oled.print("Leste");
      oled.display();
    }
//Oeste 
if(caractereVindo == 'O'){
      TelaEspEye1();
      oled.drawRect(57, 13, 10, 10, SSD1306_WHITE);
      positionAlert = 'W';
      Serial.println(positionAlert);

      oled.setCursor(55, 25);
      oled.print("Oeste");
      oled.display();

    }
//-----Informação sobre o tipo de Alerta ----------
//CLIMA
if(caractereVindo == '1'){
  oled.setCursor(90, 35);
  oled.print("On");
  oled.display();
  typeOfAlert = "Neblina";
  Serial.println(typeOfAlert);
}
//ACIDENTE
if(caractereVindo == '2'){
  oled.setCursor(90, 45);
  oled.print("On");
  oled.display();
  typeOfAlert = "Acidente";
  Serial.println(typeOfAlert);
}
//DESCIDA
if(caractereVindo == '3'){
  oled.setCursor(90, 55);
  oled.print("On"); 
  oled.display();
  typeOfAlert = "Descida";
  Serial.println(typeOfAlert);

}
//Apagar os alertas
if(caractereVindo == 'A'){
    TelaEspEye1();
    typeOfAlert = "";       //Apaga o conteúdo da variável type Of Alert
    positionAlert = '-';     //Apaga o conteúdo da variável position Alert
    x = 0;                   //Reseta a variável X
    analogWrite(FarolA,  0);         //Desliga o farol A
    analogWrite(FarolB,  0);         //Desliga o farol B
    Serial.println(typeOfAlert);
    Serial.println(positionAlert);
}
    //Back 
    if(caractereVindo == 'B'){
      funcaoPWM();
      velocidadeProibida2 = false;
      velocidadeProibida1 = false;
      compassReading2 = false;
    }
  }
}///Fechamento do If EspEye1

///Vai tratar os dados passados do comando acima, ou seja positionAlert e typeOfAlert
if(matriz[2] == positionAlert && Flag1 == true){
  //Aciona o alerta sonoro apenas uma vez
     x++;
     AlertaAcao();
     //Ativa o alerta sonoro caso esteja autorizado 
     if(AvisoSonoro == true && x == 1){
      digitalWrite(buzzer,    HIGH);
      delay(120);
      digitalWrite(buzzer,    LOW);
     }
     //Trata do tipo de alerta

     //Descida 
     if(typeOfAlert == "Descida"){
      //Não deixa que o condutor vá na última velocidade
      velocidadeProibida1 = true;
     }
     
     //Clima 
     if(typeOfAlert == "Neblina"){
      //Liga o farol num PWM mais alto
      analogWrite(FarolA, 160);
      analogWrite(FarolB, 160);
     }

     //Acidente 
     if(typeOfAlert == "Acidente"){
      //Mantém a velocidade baixa
        velocidadeProibida2 = true;
     }
}

//Ligar o Farol e a seta 
//------------Ligar Farois -----------------------------
if(message == "OnFarol" && Flag1 == true){
  //Aciona o controle de luminosidade base do farol
    FarolLigado = true;
}
if(message == "OffFarol" && Flag1 == true){
//Desliga os faróis 
  analogWrite(FarolA,      0);
  analogWrite(FarolB,      0);
  FarolLigado = false;
}
//-----------Ligar a seta direita e esquerda
//---------------DIREITA------------------
if(message == "SetaDOn" && Flag1 == true){
  boolean x = true;   //Variável local para manter a seta ligada 

  while(x){
  digitalWrite(SetaD,     HIGH);
  delay(200);
  digitalWrite(SetaD,     LOW);
  delay(200);

  if(SerialBlue.read() == 'D'){
    digitalWrite(SetaD,     LOW);
    x = false;
  }
  PontoCego();
}
}///---------------Fim da Seta Direita------------------

//----------------ESQUERDA--------------------
if(message == "SetaEOn" && Flag1 == true){
  boolean y = true;   //Variável local para manter a seta ligada 

  while(y){
  digitalWrite(SetaE,     HIGH);
  delay(200);
  digitalWrite(SetaE,     LOW);
  delay(200); 

  if(SerialBlue.read() == 'E'){
    digitalWrite(SetaE,     LOW);
    y = false;
  }
  PontoCego();
  }
}


//Buzina
if(message == "OnBuzina" && Flag1 == true){
  digitalWrite(buzzer,      HIGH);
}
if(message == "OffBuzina" && Flag1 == true){
  digitalWrite(buzzer,      LOW);
}

//Movimento do SG90
if(message == "10" && Flag1 == true){
  servo1.write(10);
}

if(message == "20" && Flag1 == true){
  servo1.write(20);
}

if(message == "30" && Flag1 == true){
  servo1.write(30);
}

if(message == "40" && Flag1 == true){
  servo1.write(40);
}

if(message == "50" && Flag1 == true){
  servo1.write(50);
}

if(message == "60" && Flag1 == true){
  servo1.write(60);
}

if(message == "70" && Flag1 == true){
  servo1.write(70);
}

if(message == "80" && Flag1 == true){
  servo1.write(80);
}

if(message == "90" && Flag1 == true){
  servo1.write(90);
}


if(message == "100" && Flag1 == true){
  servo1.write(100);
}

if(message == "110" && Flag1 == true){
  servo1.write(110);
}


if(message == "120" && Flag1 == true){
  servo1.write(120);
}


if(message == "130" && Flag1 == true){
  servo1.write(130);
}


if(message == "140" && Flag1 == true){
  servo1.write(140);
}

if(message == "150" && Flag1 == true){
  servo1.write(150);
}

if(message == "160" && Flag1 == true){
  servo1.write(160);
}

if(message == "170" && Flag1 == true){
  servo1.write(170);
}

if(message == "180" && Flag1 == true){
  servo1.write(180);
}

if(message == "0" && Flag1 == true){
     servo1.write(0);
}
//-----Termina o controle do servo-------------


}
//-----Fim do Loop-------------
//--------Funções-------
void Welcome(){
  oled.clearDisplay();
  oled.setTextSize(2);
  oled.setTextColor(SSD1306_WHITE);
  oled.setCursor(30, 25);  // x = coluna e y = linha 
  oled.println("Avalon");

  oled.drawRect(0, 0, 128, 64, SSD1306_WHITE);
  oled.display();

  delay(2000);
  /*
  oled.clearDisplay();

  oled.drawRect(0, 0, 128, 64, SSD1306_WHITE);
  oled.fillCircle(64, 32, 40, WHITE);
  oled.display();

  delay(4000);
  */
  
  oled.clearDisplay();

  oled.drawRect(0, 0, 128, 64, SSD1306_WHITE);
  oled.setCursor(10, 30);
  oled.println("LITA");
  oled.display();
  delay(4000);
}

//Página da função do PWM
void funcaoPWM(){
  oled.clearDisplay();
  delay(200);
  oled.invertDisplay(false);
  oled.setTextSize(1);
  oled.setTextColor(SSD1306_WHITE);
  oled.setCursor(10, 10);
  oled.println("Direcao");

  oled.setCursor(10, 25);
  oled.print("F");
  oled.setCursor(40, 25);
  oled.print("T");
 
   oled.setCursor(10, 50);
   oled.print("PWM:");
//               x  y
  oled.setCursor(70, 10);
  oled.print("N");
  oled.setCursor(70, 25);
  oled.print("N0");
  oled.setCursor(70, 35);
  oled.print("N1");
  oled.setCursor(70, 55);
  oled.print("N2");

  oled.drawLine(65, 0, 65, 120, SSD1306_WHITE);
  oled.drawLine(85, 0, 85, 120, SSD1306_WHITE);
  oled.drawRect(0, 0, 128, 64, SSD1306_WHITE);

  oled.setCursor(90, 10);
  oled.print("Painel");

  oled.display();
  //-----------Código PWM--------------
  analogWrite(Enable,   duty);

  //Conta para saber a porcentagem do pulso do PWM em relação ao 255
  int pulso = (duty * 100)/255;
  oled.setTextColor(SSD1306_WHITE);
  oled.setTextSize(1);
  oled.setCursor(40, 50);
  oled.println(pulso);
  oled.setCursor(55, 50);
  oled.println("%");
  oled.display();
  }

//Função de tomada de decisão no sensor de distância 
void VisaoUltrassom(){
  oled.clearDisplay();
  oled.setTextColor(SSD1306_WHITE);
  oled.setTextSize(1);
  oled.setCursor(10, 5);
  oled.print("Visao ultrassom");

  oled.drawRect(0, 0, 128, 64, SSD1306_WHITE);


//Leitura sensor da frente
  oled.setCursor(5, 25);
  oled.print("d1:");

  oled.setCursor(25, 25);
  oled.print(distanciaA);

  oled.setCursor(45, 25);
  oled.print("cm");

//Leitura sensor atrás 
  oled.setCursor(5, 40);
  oled.print("d2:");

  oled.setCursor(25, 40);
  oled.print(distanciaB);

  oled.setCursor(45, 40);
  oled.print("cm");

//Parte gráfica interativa
  oled.setCursor(110, 25);
  oled.print("d1");   

  oled.setCursor(110, 55);
  oled.print("d2");   

//Mostra se o freio automático foi ativado
  oled.setCursor(110, 40);
  oled.print("S");

//Montar os quadrados para simbolizar os sensores 
oled.drawRect(90, 20, 10, 10, SSD1306_WHITE);//d1
oled.drawRect(90, 50, 10, 10, SSD1306_WHITE);//d2
oled.drawCircle(95, 40, 5, SSD1306_WHITE);//  S


//Mexe com os quadrados desenhados dependendo da distância 
if(distanciaA <= 25){
  oled.fillRect(90, 20, 10, 10, SSD1306_WHITE);
}
if(distanciaB <= 20){
  //Y não deixa que o bip seja intermitente 
  y++;
  oled.fillRect(90, 50, 10, 10, SSD1306_WHITE);
  //Liga o alerta sonoro se estiver ativado 
  if(AvisoSonoro == true && y == 1){
  digitalWrite(buzzer,    HIGH);
  delay(90);
  digitalWrite(buzzer,    LOW);
  }
}
//Permite que o bip acontece novamente 
if(distanciaB >= 20){
  y = 0;
}
//Da um freio automático 
if(distanciaA <= 20 && Flag1 == true){
  duty = 0;
  analogWrite(Enable,   duty);
  distanciaA = sonarA.read();
  oled.fillCircle(95, 40, 5, SSD1306_WHITE);

  digitalWrite(buzzer,    HIGH);
  delay(200);
  digitalWrite(buzzer,    LOW);

}
oled.display();
///-----------------------------//////
}

//Função que especificamente lê a bússola
void Bussola(){
  bussola.read();
  a = bussola.getAzimuth();

  bussola.getDirection(matriz, a);

  x1 = bussola.getX();
  y2 = bussola.getY();
  z1 = bussola.getZ();
}

//Função que lê a direção da bússola e constroi a tela  
void Compass(){
  Bussola();
  oled.clearDisplay();

  //Desenha uma divisoria 
  oled.drawLine(75, 0, 75, 64, SSD1306_WHITE);

  oled.drawRect(0, 0, 128, 64, SSD1306_WHITE);
  
  oled.fillCircle(38, 30, 10, SSD1306_WHITE);

//Exibe as direções dos pontos cardeais 
  oled.setCursor(35, 5);
  oled.print("N");

  oled.setCursor(35, 45);
  oled.print("S");

  oled.setCursor(60, 30);
  oled.print("O");

  oled.setCursor(10, 30);
  oled.print("L");

//Exibe a posição no plano x, y, z
  oled.setCursor(85, 20);
  oled.print("x:");

  oled.setCursor(100, 20);
  oled.print(x1);

  oled.setCursor(85, 30);
  oled.print("y:");

  oled.setCursor(100, 30);
  oled.print(y2);

  oled.setCursor(85, 45);
  oled.print("z:");

  oled.setCursor(100, 45);
  oled.print(z1);

  oled.setCursor(78, 5);
  oled.print("Plano");

  oled.display();

//Laços condicionais sobre os pontos cadeais 
//------ Norte --------------
  if(matriz[2] == 'N'){
     oled.drawCircle(35, 10, 7, SSD1306_WHITE);
     oled.display();
  }
//-------- Sul --------------
  if(matriz[2] == 'S'){
     oled.drawCircle(38, 48, 7, SSD1306_WHITE);
     oled.display();
  }
//--------- Leste ------------
  if(matriz[2] == 'W'){
     oled.drawCircle(62, 32, 7,    SSD1306_WHITE);
     oled.display();
  }
// ----------- Oeste ------------------
  if(matriz[2] == 'E'){
    oled.drawCircle(13, 33, 7, SSD1306_WHITE);
    oled.display();
  }
}

void TelaEspEye1(){
  oled.clearDisplay();

//---Construção da linha dos pontos cardeais-----
  oled.setTextSize(1);
  oled.setTextColor(SSD1306_WHITE);
  oled.setCursor(5, 5);
  oled.print("ESP Eye's");

  oled.setCursor(5, 15);
  oled.print("N");

  oled.setCursor(20, 15);
  oled.print("S");

  oled.setCursor(40, 15);
  oled.print("L");

  oled.setCursor(60, 15);
  oled.print("O");

//------------------------------------------
  oled.setCursor(5, 25);
  oled.print("Alerta:");


//Qual tipo de problema 
oled.setCursor(20, 35);
oled.print("1. Neblina");

oled.setCursor(20, 45);
oled.print("2. Acidente");

oled.setCursor(20, 55);
oled.print("3. Descida");

oled.drawRect(0, 0, 128, 64, SSD1306_WHITE);

oled.display();
}
//Com os dados coletados, a função EspEye1 conecta a bússola aos dados passsados de localização do alerta e tipo de alerta 
void AlertaAcao(){
  oled.setCursor(90, 20);
  oled.print("1.On");
  oled.display();
}

//Função Start Stop 
void System(){
  oled.clearDisplay();
  oled.drawRect(0, 0, 128, 64, SSD1306_WHITE);

  oled.setCursor(5, 5);
  oled.print("Sistemas:");

  oled.setCursor(5, 15);
  oled.print("1. S/S:");

  oled.setCursor(5, 25);
  oled.print("2. Sonoro");

  oled.setCursor(5, 35);
  oled.print("3. Navegacao");

  oled.drawCircle(60, 15, 3, SSD1306_WHITE);

  oled.drawCircle(65, 25, 3, SSD1306_WHITE);

  oled.drawCircle(80, 36, 3, SSD1306_WHITE);

  oled.display();
}
//Função checar se existe carro em ponto cego 
void PontoCego(){
//Função semi autônoma de verificação de carros atrás em manobras de viragem 
//-------------------Existe carro atrás---------V.C = Veículo no Campo ----------------------------
  if(sonarB.read() <= 20){
    funcaoPWM();
    oled.setCursor(90, 25);
    oled.print("V.C");
    oled.display();
    TinhaCarro = 1;         //Existe um carro atrás
  }

//------Existia um carro porém ele está muito distante ou está num ponto cego----------- P.C = Ponto Cego------------- 
  if(sonarB.read() > 20 && TinhaCarro == 1){
    funcaoPWM();
    oled.setCursor(90, 25);
    oled.print("P.C");
    oled.display();

//Dá um bip 
    digitalWrite(buzzer,      HIGH);
    delay(80);
    digitalWrite(buzzer,      LOW);
  }
//----------Não há carro ou está muito longe--------- L = Livre-------------
  if(sonarB.read() > 25 && TinhaCarro == 0){
    TinhaCarro = 0;       //Não há carro 
    funcaoPWM();
    oled.setCursor(90, 25);
    oled.print("L");    
    oled.display();
  }
//-----------Reseta o valor de TinhaCarro = 0---------------
if(sonarB.read() > 30){
TinhaCarro = 0;          //Não há carro 
}
}

//Função da Navegação Semiautônoma 
void Navigation(){
  Bussola();
  oled.clearDisplay();

  oled.drawRect(0 , 0, 128, 64, SSD1306_WHITE);
  
  oled.setCursor(5, 5);
  oled.print("Setaveis");

  oled.setCursor(5, 15);
  oled.print("Vel:");

  oled.setCursor(5, 30);
  oled.print("F:");

  oled.setCursor(5, 45);
  oled.print("E.F");

  oled.setCursor(25, 45);
  oled.print(local_faixay0);

  oled.drawCircle(30, 30, 5, SSD1306_WHITE);
  oled.drawCircle(50, 30, 3, SSD1306_WHITE);

  oled.drawLine(0, 40, 60, 40, SSD1306_WHITE);
  oled.drawLine(60, 0, 60, 64, SSD1306_WHITE);

  oled.setCursor(64, 5);
  oled.print("y");

  oled.setCursor(75, 5);
  oled.print(y2);

  oled.setCursor(64, 20);
  oled.print("Y1");

  oled.setCursor(80, 20);
  oled.print(navigation_y1);


  oled.drawCircle(120, 10, 4, SSD1306_WHITE);
  oled.drawLine(120, 10, 120, 55, SSD1306_WHITE);

//------Seleciona a velocidade certa na tela ----------------------
  if(velocidade == 0 && vel1 == false){
    oled.setCursor(30, 15);
    oled.print("N0");
  }
  if(velocidade == 1 || vel1 == true){
    oled.setCursor(30, 15);
    oled.print("N1");
  }
  if(velocidade == 2 && vel1 == false){
    oled.setCursor(30, 15);
    oled.print("N2");
  }

//---------Faz a primeira navegação em Y------------------
  if(autorizacao == 1){ 
     
//Leitura dos sensores e controle de velocidade
   if(distanciaA < 20){
      vel1 = true;    //Reduz a velocidade
   }
   if(distanciaA > 25){
     vel1 = false;
   }
   if(distanciaA < 10){
    velocidade = 0; //Reduz mais ainda
    vel1 = false;
   }
   if(distanciaA < 7){
    velocidade = 3;
   }

///-----Faz a seleção do PWM levando em consideração o selecionado----------------
    if(velocidade == 0 ){
      analogWrite(Enable, 100);
    }
    if(velocidade == 1 || vel1 == true){
      analogWrite(Enable, 130);
    }
    if(velocidade == 2 && vel1 == false){
      analogWrite(Enable, 200);
    }
    if(velocidade == 3){
      analogWrite(Enable, 0);
      oled.setCursor(30, 15);
      oled.print("N");
    }
//-------Local da Faixa de pedestres--------------
//------Condição os farois------------------
if(condicao_farol == true){
  oled.fillCircle(30, 30, 5, SSD1306_WHITE);
  oled.display();
}
if(condicao_farol == false){
  oled.fillCircle(50, 30, 3, SSD1306_WHITE);
  oled.display();
}
//----------Parar no farol-----------------

if(local_faixay0 <= y2 && condicao_farol == false){
//---Para antes da faixa de pedestres no farol --------------------- 
analogWrite(Enable, 0);
digitalWrite(buzzer,  HIGH);
delay(50);
digitalWrite(buzzer,  LOW);
}


//---------Chega ao destino em Y0-----------------
if(y2 >= navigation_y1 && condicao_farol == true){
  analogWrite(Enable, 0);                   //Para o caminhão 
  Serial.println(y2);
  oled.fillCircle(120, 10, 4, SSD1306_WHITE);   //Preenche o caminho na tela
  chegada_y0 = true;
}

  }
  oled.display();
}