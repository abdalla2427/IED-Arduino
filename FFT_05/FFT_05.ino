#include <SD.h>
#include "arduinoFFT.h"
#include <LiquidCrystal.h>
#include <math.h>

#define SCL_INDEX 0x00
#define SCL_TIME 0x01
#define SCL_FREQUENCY 0x02
#define SCL_PLOT 0x03

const int rs = 8, en = 9, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

arduinoFFT FFT = arduinoFFT(); /* Create FFT object */
/*
These values can be changed in order to evaluate the functions
*/
const uint16_t samples = 64; //This value MUST ALWAYS be a power of 2
const double samplingFrequency = 1000;
/*
These are the input and output vectors
Input vectors receive computed results from FFT
*/
double vReal[samples];
double vImag[samples];
int contTeste = 0;

float calcularTHD(double picos[], int tamanho_picos, double pico)
{
  double resposta = 0;
  for (int i = 0; i < tamanho_picos; i++)
  {
    resposta = resposta + (picos[i]*picos[i]);
  }
  resposta = sqrt(resposta - (pico*pico)) / pico;
  return resposta;
}

void escreverNoSD()
{
  File myFile;

  Serial.print("Inicializando SD card...");

  if (!SD.begin(10)) {
    Serial.println("Inicializacao falhou!");
    while (0.1);
  }
  Serial.println("Inicializacao feita.");
  
  myFile = SD.open("oscilo.txt", FILE_WRITE);

  if (myFile) 
  {
    Serial.print("Fazendo registro no arquivo...");
    for (int i = 0; i < samples; i++) 
    {
      myFile.println(vReal[i]); 
    }
    myFile.close();
    Serial.println("Registro Feito.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("Erro ao abrir oscilo.txt");
  }
}


void setup()
{
  Serial.begin(9600);
  while (!Serial);
  lcd.begin(16, 2);
  lcd.print("THD: ");
  for (uint16_t i = 0; i < samples; i++)
  {
    vImag[i] = 0.0;
  }


}

void loop()
{
  lcd.setCursor(0,1);
  lcd.print("          ");
  lcd.setCursor(0, 5);
  /* Build raw data */
  for (uint16_t i = 0; i < samples; i++)
  {
    vReal[i] = float(analogRead(0)) * 5 / 1024;
    delayMicroseconds(260);
  }
  if (contTeste > 4)
  {
    escreverNoSD();
  }
  FFT.Windowing(vReal, samples, FFT_WIN_TYP_HAMMING, FFT_FORWARD);	/* Weigh data */

  FFT.Compute(vReal, vImag, samples, FFT_FORWARD); /* Compute FFT */

  FFT.ComplexToMagnitude(vReal, vImag, samples); /* Compute magnitudes */
  double x;
  double v;
  FFT.MajorPeak(vReal, samples, samplingFrequency, &x, &v);

  double thd = calcularTHD(vReal, samples, v); 
  lcd.print(thd);
  lcd.print(" %");
  
  delay(1000); /* Repete o calculo de THD a cada 2 segundos */
  contTeste++;
}
