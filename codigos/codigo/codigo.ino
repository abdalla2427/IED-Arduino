#include <Wire.h>
#include <LiquidCrystal.h>
#include <SD.h>
#include "arduinoFFT.h"
#include <math.h>

const int buttonPin = 7;
double thd = 0;
double magnitudePredominante = 1;
double freqPredominante = 60;

const int rs = 8, en = 9, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

const int DS1307 = 0x68; // Address of DS1307 see data sheets
byte second = 0;
byte minute = 0;
byte hour = 0;
byte weekday = 0;
byte monthday = 0;
byte month = 0;
byte year = 0;
String tempo = "";


const uint16_t samples = 64; //This value MUST ALWAYS be a power of 2
const double samplingFrequency = 2173;
/*
  These are the input and output vectors
  Input vectors receive computed results from FFT
*/
double vReal[samples];
double vImag[samples];

void botaoPressionado()
{
  readTime();
  fazerLeitura();
  escreverNoTerminal();
  fazerFFT();
  calcularTHD(vReal, samples, magnitudePredominante);
  escreverNoDisplay();
  delay(5000);
}

byte bcdToDec(byte val)
{
  int msbdec = val / 16 * 10; //gets decimel msb of BCD value(4 bits)
  int lsbdec = val % 16; //gets decimel lsb of BCD value(4 bits)
  int total = msbdec + lsbdec;
  return (total);
  //return ((val/16*10) + (val%16));
}

void readTime()
{
  Wire.beginTransmission(DS1307);
  Wire.write(byte(0));
  Wire.endTransmission();
  Wire.requestFrom(DS1307, 7);
  second = bcdToDec(Wire.read());
  minute = bcdToDec(Wire.read());
  hour = bcdToDec(Wire.read());
  weekday = bcdToDec(Wire.read());
  monthday = bcdToDec(Wire.read());
  month = bcdToDec(Wire.read());
  year = bcdToDec(Wire.read());
}


void calcularTHD(double picos[], uint16_t tamanho_picos, double pico)
{
  double resposta = 0;
  for (uint16_t i = 0; i < tamanho_picos; i++)
  {
    resposta = resposta + (picos[i] * picos[i]);
  }
  thd = sqrt(resposta - (pico * pico)) / pico;
}

void escreverNoDisplay()
{
  lcd.begin(16, 2);
  lcd.print("THD: ");
  lcd.setCursor(0, 1);
  lcd.print("     ");
  lcd.setCursor(0, 5);
  lcd.print(" %");
  lcd.print(thd);
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

    myFile.println(tempo);
    for (uint16_t i = 0; i < samples; i++)
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

void escreverNoTerminal()
{
  Serial.println("Registro feito em: ");
  
  Serial.print(monthday);
  Serial.write('-');
  Serial.print(month);
  Serial.write('-');
  Serial.print(year);
  Serial.write('-');
  Serial.write('-');

  Serial.print(hour);
  Serial.write(':');
  Serial.print(minute);
  Serial.write(':');
  Serial.print(second);
  Serial.write(0xd);
  
  for (uint16_t i = 0; i < samples; i++)
  {
    Serial.println(vReal[i]);
  }
}

void fazerLeitura()
{
  for (uint16_t i = 0; i < samples; i++)
  {
    vImag[i] = 0.0;
  }
  for (uint16_t i = 0; i < samples; i++)
  {
    vReal[i] = (double(analogRead(0) * 5) / (1024.0)) - 2.5;
    delayMicroseconds(460);
  }
}

void fazerFFT()
{
  arduinoFFT FFT = arduinoFFT();
  FFT.Windowing(vReal, samples, FFT_WIN_TYP_HAMMING, FFT_FORWARD);  /* Weigh data */

  FFT.Compute(vReal, vImag, samples, FFT_FORWARD); /* Compute FFT */

  FFT.ComplexToMagnitude(vReal, vImag, samples); /* Compute magnitudes */

  FFT.MajorPeak(vReal, samples, samplingFrequency, &freqPredominante, &magnitudePredominante);
  FFT.~arduinoFFT();
}

void setup()
{
  Serial.begin(9600);
  while (!Serial);
  pinMode(buttonPin, INPUT);
  Wire.begin();
}

void loop()
{
  if (digitalRead(buttonPin) ==  HIGH) botaoPressionado();
}
