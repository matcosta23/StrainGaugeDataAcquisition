#include "HX711.h"        //Biblioteca do HX711         
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
 
#define DOUT  8       //Pino digitais 8 e 7 do Arduino utilizados para transmissão de dados e clock, respectivamente.                  
#define CLK   7                  
 
HX711 extensometro(DOUT, CLK);                // Define os pinos de clock e data. Por padrão, seleciona o canal A com ganho de 128. 

long int calfactor = 1100;                    // Divide o "dado cru" recebido pelo conversor 
                                                    //de modo a ter algum significado físico.
float amostra = 0;                            // Valor lido no ensaio.
float MovAvgVector[3];                        // Vetor que salva as várias amostras para aplicação da média móvel
float sum, lcdresult;                         // Variáveis finais para exibição ao usuário.
int NSamples = 3;                             // Uma amostra é constituída de uma média simples de N Samples.
int i;
int FullVector = 0;                           // Marcador do vetor para controle de frequência de exibição ao usuário pelo LCD.
int UserRequest = 0;                          // Controla de o usuário solicitou ou não as medições
char UserInput;                               // Char que lê os comandos da interface serial. 
unsigned long time_taken, start, tsample;     // Variáveis para marcar o tempo de enssaio.

// Inicializa o display no endereco 0x27
LiquidCrystal_I2C lcd(0x27,2,1,0,4,5,6,7,3, POSITIVE);

void zerasensor ()
{
  //Serial.println();                                             
  extensometro.tare();                        //Pelo header, essa função faz 10 leituras e admite a média como o valor offset, zerando a medição com base nesse valor.
  //Serial.println("Sensor zerado");
}

float medida(void){
  float data = 0;
  for(i = NSamples; i > 0; i--){
    data += extensometro.get_units();
  }
  return (data/NSamples);
}

void RealizarPrimeiraMedida(void){
  amostra = medida();
  for(i = 0; i < 3; i++){
    MovAvgVector[i] = amostra;
  }
  sum = amostra*3;
}

void setup()
{
  lcd.begin(16,2);                            
  
  Serial.begin(9600);                               // Padrão do arduino                                          
       
  extensometro.set_scale(calfactor);                //Passa o fator de calibração.  
  zerasensor ();                                    // Funçao que zera o sensor: toma o valor offset como referência.  

  RealizarPrimeiraMedida();
}

void loop()
{
  lcd.setBacklight(HIGH);
  
  extensometro.set_scale(calfactor);                // Ajusta o fator de calibração.
  
  sum -= MovAvgVector[2];
  
  for(i = 2; i > 0; i--){
    MovAvgVector[i] = MovAvgVector[i-1];
  }
  
  amostra = medida();
  
  MovAvgVector[0] = amostra;
  sum += amostra;
  
                                                // A função get_value() chama a função read_avarege() que faz a leitura do buffer de recepção 10 vezes e realiza a média
                                                // Após, retira o OFFSET calculado pela função tare()
                                                // Todavia, esse valor são bytes que não estão na escala adequada para aplicação do programa
                                                // Assim, a função get_units() divide o valor lido pelo calfactor selecionado

  FullVector += 1;
  tsample = millis();                           // Marca o instante que a última amostra processada ocorreu;                              

  if (UserRequest){
    lcdresult = sum/3;
    time_taken = tsample - start;                             //Diferença de tempo em milisegundos entre o início do ensaio e a amostra.
    
    //Serial.print("Deformação: ");                                 // Imprime no monitor serial
    Serial.println((lcdresult), 1);                           // O parâmetro 3 na função print ilustra o número de casas decimais utilizadas     
    //Serial.print(" um/m.  Instante de amostragem: ");                                                    
    Serial.println(time_taken); 
    //Serial.print(" milisegundos.  Fator de Calibração: ");            // imprime no monitor serial
    //Serial.println(calfactor);                              // Fator de calibração atual

    if(FullVector == 3){                                      // Exibe a deformação do material em 2 linhas
      lcd.setCursor(0,0);                                     //    do display LCD.  
      lcd.print("Deformacao:");
      lcd.setCursor(0,1);
      lcd.print((lcdresult),0);
      lcd.print(" um/m.   ");
      FullVector = 0;
    }
  }

  if (FullVector == 3) FullVector = 0;
    
  delay(50);                                                  //Função delay tem o parâmetro em milisegundos                                             

  
  if (Serial.available()){                                    // A função serial.available retorna a quantidade de bytes disponíveis para a leitura
                                                              // Se for escrito algum comando, a quantidade será superior à 0 e o if entenderá como True
    UserInput = Serial.read();                                // Essa leitura é realizada pelo que foi digitado pelo usuário na interface serial da porta utilizada

// Comandos responsáveis pelo controle do fator de calibração:
    if (UserInput == '+' || UserInput == 'a')                 // a = aumenta 10
     calfactor += 10;
    else if (UserInput == '-' || UserInput == 'z')            // z = diminui 10
      calfactor -= 10;
    else if (UserInput == 's')                                // s = aumenta 100
      calfactor += 100;
    else if (UserInput == 'x')                                // x = diminui 100
      calfactor -= 100;
    else if (UserInput == 'd')                                // d = aumenta 1000
      calfactor += 1000;
    else if (UserInput == 'c')                                // c = diminui 1000
      calfactor -= 1000;
    else if (UserInput == 'f')                                // f = aumenta 10000
      calfactor += 10000;
    else if (UserInput == 'v')                                // v = dimuni 10000
      calfactor -= 10000;
// Comando para tarar medição:
    else if (UserInput == 't') 
      zerasensor();                                           // t = zera a Balança
// Comando para controle dos momentos de ensaio:
    else if (UserInput == 'g'){
      UserRequest = 1;
      start = millis();                                       // Inicia a contagem de tempo do ensaio.
    }
    else if (UserInput == 'h'){
      UserRequest = 0;
      lcd.clear();
    }
  }
}
