/*Software Desenvolvido para o Smart Switch Mini

O Software faz a comunicação do microcontrolador com o modulo da Tuya CB3S.
O funcionamento do software é o seguinte:
Quando o interruptor muda de estado, joga um pulso na saida "Saida_Modulo" de 200ms. Essa saida esta conectada no pino de entrada do modulo.
Quando o modulo recebe esse sinal, ele muda o valor do pino que esta conectado no "Entrada_modulo".
Se o valor do "Entrada_modulo" receber "1", significa que o Rele tem que ser ligado, se receber "0" o Rele precisa desligar.
Se segurar o botão por 5 segundos ou ligar e desligar o interruptor por 10 vezes rapidamente, entrara no modo reset.
Para entrar no modo reset, é necessario colocar a "Saida_Modulo" em "1" por 5 segundos.
Na hora de desligar ou ligar o rele, sera sempre no zero_cross.
O software consegue diferenciar a frequencia, dispara no zero_cross tanto em 50Hz quanto em 60Hz.

Desenvolvedor: João Menarin      6943     28/07/2022


*/
//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <SI_EFM8BB1_Register_Enums.h>                  // SFR declarations
#include "InitDevice.h"
#include "PinsTypedef.h"
// $[Generated Includes]
// [Generated Includes]$

//DEFINIÇÕESv
#define NO_RESULT 0x2000

//VARIAVEIS GLOBAIS
volatile unsigned long int cnt=0; //Variavel esta sendo usada no Interrupts.c para contagem de tempo. Cada unidade equivale a 1ms
unsigned int a = 0;               //Usada na função VerificaBotao para um laço for de 2 segundos
unsigned int contadorint = 0;     //Usada na funçao VerificaInterruptor para verificar se interruptor esta em "1" ou "0"
unsigned int statusint = 0;       //Usada na função VerificaIntettuptor para armezenar o estado do interruptor, "0" = interruptor aberto, "1" = interruptor fechado
unsigned int copiastatusint = 0;  //Usada na função VerificaMudancaEstado para armazenar o estado atual do interruptor
unsigned int contadorreset = 0;   //Usada na função VerificaMudancaEstado para contagem de quantas vezes o interruptor mudou de estado em 500ms
unsigned int contador500ms = 0;   //Usada na função VerificaMudancaEstado para contagem de 500ms dentro de um laço for


//Funções

void Delay_500us(int x);
void Liga(void);
void Desliga(void);
void VerificaInterruptor(void);
void VerificaBotao(void);
void VerificaMudancaEstado(void);

//-----------------------------------------------------------------------------
// SiLabs_Startup() Routine
// ----------------------------------------------------------------------------
// This function is called immediately after reset, before the initialization
// code is run in SILABS_STARTUP.A51 (which runs before main() ). This is a
// useful place to disable the watchdog timer, which is enable by default
// and may trigger before main() in some instances.
//-----------------------------------------------------------------------------
void SiLabs_Startup (void)
{
  // $[SiLabs Startup]
  // [SiLabs Startup]$
}

//-----------------------------------------------------------------------------
// main() Routine
// ----------------------------------------------------------------------------
int main (void){
  // Call hardware initialization routine
  enter_DefaultMode_from_RESET();
  Rele = 0;                                                     //Inicia o produto com o Rele em 0
  Saida_Modulo = 1;                                             //Inicia o produto com o Saida_Modulo em 1
  VerificaInterruptor();                                        //Verifica o estado do interruptor
  copiastatusint = statusint;                                   //Armezana o estado do interruptor, para começar o produto sempre com o Rele desligado

  while (1) {

      VerificaInterruptor();                                    //Verifica o estado do interruptor
      if(statusint != copiastatusint){VerificaMudancaEstado();} //Se "statusint" for diferente de "d", chama a função "VerificaMudanca"
      VerificaBotao();                                          //Verifica o estado do botão e liga ou desliga o rele, ou entra no modo reset
      if(Entrada_Modulo == 1 && Rele == 0){
          Liga();                                               //Liga a carga
      }
      if(Entrada_Modulo == 0 && Rele == 1){
          Desliga();                                            //Desliga a carga
          Delay_500us(100);                                     //Delay de 100ms
      }
   }
}


/*****************************************************************************************************************************
 Função VerificaMudanca
 Essa função verifica se o interruptor mudou de estado e ananalisa se é pra entrar no modo reset ou se é apenas para mudar o estado da carga

******************************************************************************************************************************/

void VerificaMudancaEstado(void){
if(statusint != copiastatusint){                                           //Se "statusint" for diferente de "d", significa que o interruptor mudou de estado
         copiastatusint = statusint;                                       //Armazena o valor de "statusint" na variavel "d", armazenando o novo estado do interruptor
         contadorreset++;                                                  //Incrementa g++
         for(contador500ms = 0; contador500ms < 500; contador500ms++){     //Laço for de 500ms, para verificar se o interruptor vai mudar de estado dentro desses 500ms
             cnt = 1; while(cnt);                                          //Delay de 1ms
             VerificaInterruptor();                                        //Verifica o estado do interruptor
             if(statusint != copiastatusint){                              //Se statusint for diferente de "d", significa que mudou de estado dentro dos 500ms,
                 copiastatusint = statusint;                               //Armazena o valor de "statusint" na variavel "d", armazenando o novo estado do interruptor
                 contadorreset++;                                          //incrementa g
                 contador500ms = 0;                                        //Zera a variavel "h"
             }
             if(contadorreset >= 10){                                      //Se g for maior ou igual a 10, significa que quer entrar no modo reset, para entrar no modo reset, precisa colocar a saida Reset em nivel "0" por 5 segundos
                 Saida_Modulo = 0;                                         //Atribui "0" para a saida Reset
                 Delay_500us(12000);                                       //Delay de 6 segundos para entrar no modo reset
                 Saida_Modulo = 1;                                         //Atribui "1" para a saida Reset
                 contadorreset = 0;                                        //Atribui "0" na variavel "g"
                 contador500ms = 0;                                        //Atribui "0" na variavel "h"
             }
         }
         if(contador500ms >= 499 && contadorreset < 10){                   //Se não houve mudança de estado do interruptor dentro dos 500ms
             contadorreset = 0;                                            //Atribui "0" na variavel "g"
             contador500ms = 0;                                            //Atribui "0" na variavel "h"
             Saida_Modulo = 0;                                             //Atribui "0" a saida Reset para mandar informação para o modulo que o Rele precisa mudar de estado
             Delay_500us(200);                                             //Delay de 200ms
             Saida_Modulo = 1;                                             //Atribui "1" a saida Reset
         }
     }
}

/*******************************************************************************************************************************
Função VerificaBotoes
Essa função verifica se o Botão esta pressionado e depois de passar 10 segundos entra no modo reset.
O botão precisa ficar pressionado por 10 segundos para entrar no modo reset

 *********************************************************************************************************************************/

void VerificaBotao (void){

  if(Botao == 1){Saida_Modulo = 1;}
  if(Botao == 0){
      for( a = 0; a < 5000; a++){
          cnt = 1; while(cnt);
          if(Botao == 1){
              break;
          }
      }
      Saida_Modulo = 0;
  }
}

/*********************************************************************************************************************************
 Função VerificaInterruptor
 Essa função analisa qual estado esta o interruptor. Se o interruptor estiver aberto, o sinal vai ficar sempre em nivel logico "1".
 Se o interruptor estiver fechado, ele ficara alternando entre nivel logico "1" e "0".
 Se a variavel "c" for maior ou igual a 50, significa que o interruptor esta aberto.
*********************************************************************************************************************************/


void VerificaInterruptor(void){

  if(Interruptor == 0){                       //Se interruptor igual a "0".
      contadorint = 0;                        //Zera a variavel c.
      statusint = 1;                          //Atribui "1" a variavel statusint, indicando que o interruptor esta fechado.
  }
  if(Interruptor == 1 && contadorint <= 50){
      cnt = 1; while(cnt);                    //Delay de 1ms
      contadorint++;                          //Incrementa "c"
      if(Interruptor == 0){                   //Se interruptor é igual a "0", significa que o interruptor esta fechado.
          statusint = 1;                      //Atribui "1" a variavel statusint, indicando que o interruptor esta fechado.
          contadorint = 0;                    //Zera a variavel c
      }
      if(contadorint >= 50){                  //Se "c" for igual ou maior que 50, significa que o interruptor esta em estado logico 1, ou seja, interruptor aberto,
          statusint = 0;                      //Atribui "0" a variavel statusint, indicando que o interruptor esta aberto.
          contadorint = 0;                    //Zera a variavel "c"
      }
   }
 }


/*********************************************************************************************************************************
 Função EperaZero0
 Espera o zero da senoide da tensão de entrada no momento da descida do nivel alto para baixo ou do nivel baixo para alto.
*********************************************************************************************************************************/
void Liga(void){                  // Definição da função "EsperaZero"
 int e=0,f=0;
 if(Zero==1){                     // Se a senoide estiver em 1, aguarda ela passar por 0
      do { Delay_500us(1); }      // Delay de 500us verificando o sinal da senoide
      while(Zero!= 0);            // Quando for 0, retorna para o disparo da carga
      for(e=0; e<20; e++){        // Conta um tempo de 20ms
        Delay_500us(2);           // Delay de 1ms.
        if (Zero==1){f++;};       // Durante o periodo de 20ms analisa-se o tempo que a senoide fica em nivel alto(1) e incrementa a variavel f
      }
      if(f<=9){Delay_500us(20);}   // Se o tempo que a senoide ficou em nivel alto for menor que 9ms a rede é 60Hz então aguardo 8ms para ligar a carga,
      else{Delay_500us(13);}       // Se for maior que 9ms a rede é 50Hz então aguardo 4,5ms para acionar a carga
 }
 else{                            // Senão, se a senoide ja estiver em 0, espera ela ir para 1
      do {Delay_500us(1);}        // Delay de 500us verificando o sinal da senoide
      while(Zero!= 1);            // Quando for 1, espera passar por 0 novamente
      if(Zero==1){                // Se Zero igual a 1 segue a instrução
        do {Delay_500us(1);}      // Delay de 500us verificando o sinal da senoide
        while(Zero!= 0);          // Quando for 0, retorna para o disparo da carga
        for(e=0; e<20; e++){      // Conta um tempo de 20ms
           Delay_500us(2);        // Delay de 1ms.
           if (Zero==1){f++;};    // Durante o periodo de 20ms analisa-se o tempo que a senoide fica em nivel alto(1) e incrementa a variavel f
        }
        if(f<=9){Delay_500us(20);} // Se o tempo que a senoide ficou em nivel alto for menor que 9ms a rede é 60Hz então aguardo 8ms para ligar a carga,
        else{Delay_500us(13);}     // Se for maior que 9ms a rede é 50Hz então aguardo 4,5ms para acionar a carga
      }
  }
 Rele=1;
 return;
}

/*********************************************************************************************************************************
 Função EperaZero0
 Espera o zero da senoide da tensão de entrada no momento da descida do nivel alto para baixo ou do nivel baixo para alto.
*********************************************************************************************************************************/
void Desliga(void){               // Definição da função "EsperaZero"
 int e=0,f=0;
 if(Zero==1){                     // Se a senoide estiver em 1, aguarda ela passar por 0
      do { Delay_500us(1); }      // Delay de 500us verificando o sinal da senoide
      while(Zero!= 0);            // Quando for 0, retorna para o disparo da carga
      for(e=0; e<20; e++){        // Conta um tempo de 20ms
        Delay_500us(2);           // Delay de 1ms.
        if (Zero==1){f++;};       // Durante o periodo de 20ms analisa-se o tempo que a senoide fica em nivel alto(1) e incrementa a variavel f
      }
      if(f<=9){Delay_500us(19);}   // Se o tempo que a senoide ficou em nivel alto for menor que 9ms a rede é 60Hz então aguardo 8ms para ligar a carga,
      else{Delay_500us(13);}       // Se for maior que 9ms a rede é 50Hz então aguardo 4,5ms para acionar a carga
 }
  else{                           // Senão, se a senoide ja estiver em 0, espera ela ir para 1
      do {Delay_500us(1);}        // Delay de 500us verificando o sinal da senoide
      while(Zero!= 1);            // Quando for 1, espera passar por 0 novamente
      if(Zero==1){                // Se Zero igual a 1 segue a instrução
        do {Delay_500us(1);}      // Delay de 500us verificando o sinal da senoide
        while(Zero!= 0);          // Quando for 0, retorna para o disparo da carga
        for(e=0; e<20; e++){      // Conta um tempo de 20ms
           Delay_500us(2);        // Delay de 1ms.
           if (Zero==1){f++;};    // Durante o periodo de 20ms analisa-se o tempo que a senoide fica em nivel alto(1) e incrementa a variavel f
        }
        if(f<=9){Delay_500us(19);} // Se o tempo que a senoide ficou em nivel alto for menor que 9ms a rede é 60Hz ent�o aguardo 8ms para ligar a carga,
        else{Delay_500us(13);}     // Se for maior que 9ms a rede �é 50Hz então aguardo 4,5ms para acionar a carga
      }
  }
 Rele=0;
 return;
}

/***************************************************************************************
 Função Delay_500us
 Essa função verifica o valor passado para contagem de tempo na variavel cnt se for menor
 que 255 ele atribiu o valor direto para cnt que é decrementado pela interrupção normalmente,
 caso o valor seja maior que 255, essa função verifica em quantos valores de 255 o valor inicial
 passado pode ser dividido, carregando em cnt o valor de 255 tantas vezes quantas forem necessarias
 para atingir o valor inicial passado para função.
****************************************************************************************/

void Delay_500us(int x){ // Função para contagem de tempo
  int y=0,z=0,l=0;

  if(x<256){cnt=x; while(cnt);}
  else{ do{x=x-255;
           z++;
          }while(x>=255);
        for(l=z; l>0; l--){
         cnt=255;while(cnt);
        }
        cnt=x; while(cnt);
  }
 return;
}

