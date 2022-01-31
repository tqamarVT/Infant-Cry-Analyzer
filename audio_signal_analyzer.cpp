// Arrays to save our results in
/* 32 ms, 256 samples*/
/******************************************************************
TAIMOOR QAMAR AND ADAM SCHILLING SENIOR DESIGN PROJECT
******************************************************************/
/******************
OUTPUT AND BINARY CORRESPONDENCE:
HAPPY  00
HUNGER 01
PAIN   10
*******************/

#include <math.h>
#include <LiquidCrystal.h>
// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

float AP_THRESHOLD = 0.180;// Average Power requirement
float ENV_THRESHOLD = 0.600;// Envelope threshold

/* STATISTICAL DECISION */
int A = 0;
int B = 0;
int Hunger = 0;
int Pain = 0;
int Happy = 0; 

float ValPADz[512];
float ValPADs[512];
float AMDF[256]; 
float MinValue;

float values[256];
float envalues[256];

/******************************************************************
 * Network Configuration - customized per network 
 ******************************************************************/

const int PatternCount = 1;
const int InputNodes = 10;
const int HiddenNodes = 8;
const int OutputNodes = 2;
const float LearningRate = 0.3;
const float Momentum = 0.9;
const float InitialWeightMax = 0.5;
const float Success = 0.0004;

/*const byte Target[PatternCount][OutputNodes] = {
  { 0, 0, 0, 0 },  
  { 0, 0, 0, 1 }, 
  { 0, 0, 1, 0 }, 
  { 0, 0, 1, 1 }, 
  { 0, 1, 0, 0 }, 
  { 0, 1, 0, 1 }, 
  { 0, 1, 1, 0 }, 
  { 0, 1, 1, 1 }, 
  { 1, 0, 0, 0 }, 
  { 1, 0, 0, 1 } 
  };*/

/******************************************************************
 * End Network Configuration
 ******************************************************************/
 
int i, j, p, q, r;
int ReportEvery1000;
int RandomizedIndex[PatternCount];
long  TrainingCycle;
float Rando;
float Error;
float Accum;

float Output[OutputNodes];
float Hidden[HiddenNodes];
float HiddenDelta[HiddenNodes];
float OutputDelta[OutputNodes];
float ChangeHiddenWeights[InputNodes+1][HiddenNodes];
float ChangeOutputWeights[HiddenNodes+1][OutputNodes];

 const float HiddenWeights[InputNodes+1][HiddenNodes]= {
   {-2.64,-3.71,3.37,0.60,3.28,7.58,-1.03,3.22}, 
  {-0.93,10.33,9.45,-1.58,-1.79,1.96,-3.03,-0.16}, 
  {-4.77,-2.43,-0.83,7.13,7.62,6.67,0.40,3.77}, 
  {0.63,-5.82,-0.75,0.21,-5.05,-1.76,4.62,3.70}, 
  {9.91,-2.03,7.60,-0.78,-0.45,-0.50,-7.47,6.40}, 
  {-5.01,-2.10,7.82,-1.02,-1.98,-4.09,-7.35,-1.53}, 
  {12.28,2.02,-0.56,-2.63,-1.06,2.84,0.73,1.89}, 
  {-1.91,5.09,10.35,-1.89,4.27,0.40,4.60,0.11}, 
  {-5.33,0.29,-1.85,-1.24,3.27,2.06,-0.55,3.19}, 
  {8.20,6.73,5.93,-1.16,-3.62,1.22,-4.83,-7.19}, 
  {-2.26,-2.09,1.71,-3.36,2.36,-3.09,-3.91,-1.21} };

const float OutputWeights[HiddenNodes+1][OutputNodes]= {  
  {11.90,-19.12}, 
  {20.72,-17.88}, 
  {-20.59,18.56}, 
  {-2.83,-13.60}, 
  {-14.94,0.87}, 
  {6.86,-18.25}, 
  {-15.86,19.66}, 
  {11.16,-2.57}, 
  {-6.14,5.58}  };
  
int capture = 8; // signal capture indicator

//////////////////////////////////////
void setup() 
{
  Serial.begin(9600);
  randomSeed(analogRead(3));
  ReportEvery1000 = 1;
  for( p = 0 ; p < PatternCount ; p++ ) {    
    RandomizedIndex[p] = p ;
  }
//pinMode(capture, OUTPUT); //sets pin 8 for envelope threshold indication
// set up the LCD's number of columns and rows:
  lcd.begin(4, 20);
  lcd.setCursor(0,0);
}

void loop() {  
  //unsigned int t = 0;// counting iterations till trigger
/**********************************************/
/***  WAIT FOR TRIGGER  ENVELOPE DETECTION  ***/
/**********************************************/
   begin:// point of return for signals lacking appropriate power

float Normalize = 0;  //maximum value of signal used to normalize 
//float FundFreq = 0;  //Fundamental Freqency of signal

   while((analogRead(A2)*(3.3 / 1100.0)+0.23) < ENV_THRESHOLD)
   {
  //  t++;     
  //  digitalWrite(capture, HIGH); //active low indicate data capture
   }
   
/***************************************/
/***  CAPTURE THE VALUES TO MEMORY   ***/
/***************************************/
   
    for(int i=0;i<256;i++)  {
    values[i] = (analogRead(A0) * (3.3 / 1100.0)-1.50);
  //  envalues[i]=(analogRead(A2) * (3.3 / 1100.0)+0.23);
    delayMicroseconds(77);
    //digitalWrite(capture, LOW);
    lcd.print("Capturing Signal");
    }
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Signal Captured!"); //16 Characters
 /***********************************/
 /*****NORMALIZE accepted signal*****/
 /***********************************/
  for(i=0;i<256;i++)
  {
    if (abs(values[i])>Normalize)
    {
      Normalize = abs(values[i]);
      }
  }
 // Serial.println();Serial.print("Norm = ");Serial.print(Normalize);
for (i=0;i<256;i++)
{
  values[i] = values[i]/Normalize;
}

/*****************************************/
/********AVERAGE POWER OF SAMPLE**********/
/*****************************************/
float AP=0;
    for (int i=0; i<256; i++){
      AP += (values[i]*values[i])/256;
    }
   
   lcd.setCursor(0,1);
   lcd.print("AVG POWER:");    // 10 characters   
   lcd.print(AP);              // 4 characters
   Serial.println("");
   Serial.print("Average Power = "); 
   Serial.print(AP,5);
    
    if (AP < AP_THRESHOLD){   //Qualifying Average Power of signal
      goto begin;
    }
    
 /************************************************/
 /***PRINT OUT THE RESULTS of accepted signal*****/
 /************************************************/
 /*
  Serial.println("\n\n--- Original Results ---"); 
  for(i=0;i<256;i++)
  {
    Serial.print(values[i]);
    Serial.print(',');
  }
    Serial.println(" ");
*/
 /**************************************************/
 /***PRINT the ENVELOPE of the SIGNAL***************/
 /**************************************************/   
 /*  
  // print out the envelope results
  Serial.println("\n\n--- Env_Results ---"); 
  for(i=0;i<256;i++) 
  {
    Serial.print(envalues[i]);
    Serial.print(',');
  }
      Serial.print("iteration count =  ");
      Serial.print(t); // iteration count
*/

/**************************************************/
/***           MFCC CODE STARTS HERE            ***/
/**************************************************/

//HAMMING WINDOW//
float HamWin[256] = {0.08, 0.08014, 0.08056, 0.08126, 0.08223, 0.08349, 0.08502, 0.08683, 0.08891, 0.09126,
     0.09389, 0.09679, 0.09996, 0.10340, 0.10710, 0.11106, 0.11529, 0.11977, 0.12451, 0.12950, 0.13473, 0.14022, 0.14595, 0.15191, 0.15812, 0.16455, 0.17121, 0.17810, 0.18520,
     0.19252, 0.20006, 0.20779, 0.21573, 0.22387, 0.23220, 0.24072, 0.24941, 0.25829, 0.26733, 0.27654, 0.28591, 0.29544, 0.30511, 0.31493, 0.32488, 0.33496, 0.34517, 0.35549,
     0.36593, 0.37647, 0.38712, 0.39785, 0.40867, 0.41958, 0.43055, 0.44159, 0.45269, 0.46385, 0.47505, 0.48628, 0.49756, 0.50885, 0.52017, 0.53150, 0.54283, 0.55417, 0.56549,
     0.57680, 0.58808, 0.59934, 0.61056, 0.62174, 0.63287, 0.64394, 0.65495, 0.66588, 0.67675, 0.68753, 0.69822, 0.70881, 0.71930, 0.72968, 0.73995, 0.75010, 0.76012, 0.77000,
     0.77975, 0.78934, 0.79879, 0.80808, 0.81721, 0.82617, 0.83496, 0.84357, 0.85199, 0.86022, 0.86826, 0.87610, 0.88374, 0.89116, 0.89838, 0.90537, 0.91215, 0.91870, 0.92501,
     0.93110, 0.93695, 0.94255, 0.94792, 0.95303, 0.95789, 0.96250, 0.96686, 0.97095, 0.97478, 0.97835, 0.98166, 0.98469, 0.98746, 0.98995, 0.99217, 0.99411, 0.99578, 0.99718,
     0.99829, 0.99913, 0.99969, 0.99997, 0.99997, 0.99969, 0.99913, 0.99829, 0.99718, 0.99578, 0.99411, 0.99217, 0.98995, 0.98746, 0.98469, 0.98166, 0.97835, 0.97478, 0.97095,
     0.96686, 0.96250, 0.95789, 0.95303, 0.94792, 0.94255, 0.93695, 0.93110, 0.92501, 0.91870, 0.91215, 0.90537, 0.89838, 0.89116, 0.88374, 0.87610, 0.86826, 0.86022, 0.85199,
     0.84357, 0.83496, 0.82617, 0.81721, 0.80808, 0.79879, 0.78934, 0.77975, 0.77000, 0.76012, 0.75010, 0.73995, 0.72968, 0.71930, 0.70881, 0.69822, 0.68753, 0.67675, 0.66588,
     0.65495, 0.64394, 0.63287, 0.62174, 0.61056, 0.59934, 0.58808, 0.57680, 0.56549, 0.55417, 0.54283, 0.53150, 0.52017, 0.50885, 0.49756, 0.48628, 0.47505, 0.46385, 0.45269,
     0.44159, 0.43055, 0.41958, 0.40867, 0.39785, 0.38712, 0.37647, 0.36593, 0.35549, 0.34517, 0.33496, 0.32488, 0.31493, 0.30511, 0.29544, 0.28591, 0.27654, 0.26733, 0.25829,
     0.24941, 0.24072, 0.23220, 0.22387, 0.21573, 0.20779, 0.20006, 0.19252, 0.18520, 0.17810, 0.17121, 0.16455, 0.15812, 0.15191, 0.14595, 0.14022, 0.13473, 0.12950, 0.12451,
     0.11977, 0.11529, 0.11106, 0.10710, 0.10340, 0.09996, 0.09679, 0.09389, 0.09126, 0.08891, 0.08683, 0.08502, 0.08349, 0.08223, 0.08126, 0.08056, 0.08014, 0.08000};
//
float Sw[256]=         {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};  
float FFTssREAL[128]=  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; 
float FFTssIMAG[128]=  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; 
float MagSpectrum[128]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; 

float MFB_0[9]=   {0.2, 0.4, 0.6, 0.8, 1, 0.8, 0.6, 0.4, 0.2};
float MFB_1[11]=  {0.2, 0.4, 0.6, 0.8, 1, 0.85714, 0.71429, 0.57143, 0.42857, 0.28571, 0.14286};
float MFB_2[13]=  {0.14286, 0.28571, 0.42857, 0.57142, 0.71429, 0.85714, 1, 0.85714, 0.71429, 0.57143, 0.42857, 0.28571, 0.14286};
float MFB_3[15]=  {0.14286, 0.28571, 0.42857, 0.57143, 0.71429, 0.85714, 1, 0.88889, 0.77778, 0.66667, 0.55556, 0.44444, 0.33333, 0.22222, 0.11111};
float MFB_4[18]=  {0.11111, 0.22222, 0.33333, 0.44444, 0.55556, 0.66667, 0.77778, 0.88889, 1, 0.9, 0.8, 0.7, 0.6, 0.5, 0.4, 0.3, 0.2, 0.1};
float MFB_5[20]=  {0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1, 0.90909, 0.81818, 0.72727, 0.63636, 0.54545, 0.45455, 0.36364, 0.27273, 0.18182, 0.09091};
float MFB_6[23]=  {0.09091, 0.18182, 0.27273, 0.36364, 0.45455, 0.54545, 0.63636, 0.72727, 0.81818, 0.90909, 1, 0.92308, 0.84615, 0.76923, 0.69231, 0.61538, 0.53846, 0.46154, 0.38462, 0.30769, 0.23077, 0.15385, 0.07692};
float MFB_7[27]=  {0.07692, 0.15385, 0.23077, 0.30769, 0.38462, 0.46154, 0.53846, 0.61538, 0.69231, 0.76923, 0.84615, 0.92308, 1, 0.93333, 0.86667, 0.8, 0.73333, 0.66667, 0.6, 0.53333, 0.46667, 0.4, 0.33333, 0.26667, 0.2, 0.13333, 0.06667};
float MFB_8[31]=  {0.06667,  0.13333, 0.2, 0.26667, 0.33333, 0.4, 0.46667, 0.53333, 0.6, 0.66667, 0.73333, 0.8, 0.86667, 0.93333, 1, 0.94118, 0.88235, 0.82353, 0.76471, 0.70588, 0.64706, 0.58824, 0.52941, 0.47059, 0.41176, 0.35294, 0.29412, 0.23529, 0.17647, 0.11765, 0.05882};
float MFB_9[35]=  {0.05882, 0.11765, 0.17647, 0.23529, 0.29412, 0.35294, 0.41176, 0.47059, 0.52941, 0.58824, 0.64706, 0.70588, 0.76471, 0.82353, 0.88235, 0.94118, 1, 0.94737, 0.89474, 0.84211, 0.78947, 0.73684, 0.68421, 0.63158, 0.57895, 0.52632, 0.47368, 0.42105, 0.36842, 0.31579, 0.26316, 0.21053, 0.15789, 0.10526, 0.05263};

//INITIALIZE FILTER BANK VALUE ARRAY, MFCC VALUE ARRAY, and intermediate arrays

float FBval[10]   =    {0,0,0,0,0,0,0,0,0,0};
float logFBval[10]=    {0,0,0,0,0,0,0,0,0,0};
float Q[10]       =    {0,0,0,0,0,0,0,0,0,0};
float w[10]       =    {0.316228,0.447214,0.447214,0.447214,0.447214,0.447214,0.447214,0.447214,0.447214,0.447214}; 
float MFCC[10]    =    {0,0,0,0,0,0,0,0,0,0};

/***************************************************/
/***           Hamming Window S->Sw              ***/
/***************************************************/
for (int i=0; i<256; i++){
  Sw[i] = HamWin[i] * values[i];// verified window in Matlab
}

/***************************************************/
/***            FOURIER TRANSFORM                ***/
/***************************************************/
//Perform FFT on Sw,keep the first half of ds spectrum
for (int k=0; k<128; k++){
  for (int n=0; n<256; n++) {
      FFTssREAL[k] += 2*Sw[n]*(cos((6.28319*n*k)/256));
      FFTssIMAG[k] += 2*Sw[n]*(sin((6.28319*n*k)/256))*(-1); 
  }
                          // Serial.println(FFTssREAL[k]);
                          // Serial.println(FFTssIMAG[k]);
}
//Determine Single Sided DC component FFT REAL COMPONENT
  FFTssREAL[0] = 0.5*FFTssREAL[0];

//Calculate Magnitude Spectrum
//Serial.print("Spectrum Magnitudes = ");
for (int j=0; j<128; j++){
  MagSpectrum[j] = (0.00390625)*(FFTssREAL[j]*FFTssREAL[j]+FFTssIMAG[j]*FFTssIMAG[j]);
//Serial.print(MagSpectrum[j]);Serial.print(", ");
}  

 /**************************************************/
 /***FUNDAMENTAL FREQUENCY of NORMALIZED signal*****/
 /**************************************************/
/*
 //ValPADs{Signal,Zeros}
   for(int m=0;m<256;m++)
   {
     ValPADs[m] = values[m]; //copying values
   }
      for(int m=256;m<512;m++)
   {
     ValPADs[m] = 0; //copying values
   }
   */
//*******************************
/*
Serial.println("");
Serial.println("ValPADs");
  for(int i=0;i<512;i++)
 {
  Serial.print(ValPADs[i]);Serial.print(", ");
 }
 */
//*********************************
/*
  for(int j=0;j<256;j++)
  {  // initializes AMDF to zero
    AMDF[j] = 0;
  }

for(int k=0;k<256;k++)//begining of loop
{
  for(int i=0;i<512;i++)
  { //initialize ValPADz to 0 each iteration
    ValPADz[i]={0};
  }
  for(int j=0;j<256;j++)
  {  //places signal values
    ValPADz[k+j] = ValPADs[j];
  }
  for(int n=0;n<512;n++)
  {
  AMDF[k] += abs(ValPADs[n]-ValPADz[n]);
  }
}

  for(int i=0;i<256;i++)
 {
  AMDF[i] = (0.003906)*AMDF[i];
 }
*/
//*******************************
/*
Serial.println("");
Serial.println("ValPADz");
  for(int i=0;i<512;i++)
 {
  Serial.print(ValPADz[i]);Serial.print(", ");
 }
 */
//*********************************
//*******************************
/*
Serial.println("");
Serial.println("AMDF");
  for(int i=0;i<256;i++)
 {
  Serial.print(AMDF[i]);Serial.print(", ");
 }
 */
//*********************************
//Infant Fo should be between 300-600hz
/*
MinValue=10.00;
int q=0;
int I=0;
  for(q=16;q<30;q++)  
  {  
    
    if (AMDF[q]<MinValue)
    {
      I=q;
        MinValue = AMDF[I];
        FundFreq = (8000/(I));
    }
  }
Serial.println(""); 
Serial.print("F0 = ");Serial.print(FundFreq,6);
Serial.print(", Index = ");Serial.print(I);
*/

/*********************
*********MFCC*********
********************/

//REDUCED PROCESSING FILTER BANK CALCULATIONS
   for (int k=9; k<18; k++) {                
  FBval[0]+=MagSpectrum[k]*MFB_0[k-9];                         
}
   for (int k=14; k<25; k++) {                
  FBval[1]+=MagSpectrum[k]*MFB_1[k-14];                         
}  
   for (int k=19; k<32; k++) {                
  FBval[2]+=MagSpectrum[k]*MFB_2[k-19];                         
}    
   for (int k=26; k<41; k++) {                
  FBval[3]+=MagSpectrum[k]*MFB_3[k-26];                         
} 
   for (int k=33; k<51; k++) {                
  FBval[4]+=MagSpectrum[k]*MFB_4[k-33];                         
}   
   for (int k=42; k<62; k++) {                
  FBval[5]+=MagSpectrum[k]*MFB_5[k-42];                         
}  
   for (int k=52; k<75; k++) {                
  FBval[6]+=MagSpectrum[k]*MFB_6[k-52];                         
}  
   for (int k=63; k<90; k++) {                
  FBval[7]+=MagSpectrum[k]*MFB_7[k-63];                         
}
   for (int k=76; k<107; k++) {                
  FBval[8]+=MagSpectrum[k]*MFB_8[k-76];                         
}   
   for (int k=91; k<126; k++) {                
  FBval[9]+=MagSpectrum[k]*MFB_9[k-91];                         
}   

//TAKE LOG OF SUMMED FILTER BANK VALUES
for (int m=0; m<10; m++) {
  logFBval[m]= log10(FBval[m]);
}

Serial.println(" ");

//DISCRETE COSINE TRANSFORM
for (int k=0; k<10; k++){
    for (int n=1; n<11; n++){
      Q[k] += logFBval[n-1]*cos((0.15708)*(2*n-1)*(k));
 } 
MFCC[k] = w[k]*Q[k];
//Serial.print("MFCC = "); Serial.print(MFCC[k] );
}

/****************
 MFCC CODE ENDS HERE
 *****************/

 /***********************
  * ANN CODE STARTS HERE
  *********************/

/*  const float Input[PatternCount][InputNodes] = {
{-5.49,0.18,-1.08,-0.47,-0.62,-0.49,-0.51,-0.13,0.17,-0.41},
{1.61,-1.30,-1.18,-0.76,-0.35,0.51,-0.13,0.44,0.32,0.23},
{1.61,-1.30,-1.18,-0.76,-0.35,0.51,-0.13,0.44,0.32,0.23},
{1.61,-1.30,-1.18,-0.76,-0.35,0.51,-0.13,0.44,0.32,0.23},
{1.61,-1.30,-1.18,-0.76,-0.35,0.51,-0.13,0.44,0.32,0.23}
};*/ 


for( p = 0 ; p < PatternCount ; p++ ) { 
    
//    Serial.print ("  Training Pattern: ");
//    Serial.println (p);      
    Serial.print ("MFCC Input = ");
    for( i = 0 ; i < InputNodes ; i++ ) {
      Serial.print (MFCC[i]);
      Serial.print (" ");
    }
    //Serial.print ("  Target ");
    for( i = 0 ; i < OutputNodes ; i++ ) {
     // Serial.print (Target[p][i], DEC);
     // Serial.print (" ");
    }
 
 /*Compute hidden layer activations*/
    for( i = 0 ; i < HiddenNodes ; i++ ) {    
      Accum = HiddenWeights[InputNodes][i] ;
      for( j = 0 ; j < InputNodes ; j++ ) {
        Accum += MFCC[j] * HiddenWeights[j][i] ; //ANN input
      }
      Hidden[i] = 1.0/(1.0 + exp(-Accum)) ;
    }

/******************************************************************
* Compute output layer activations and calculate errors
******************************************************************/
    for( i = 0 ; i < OutputNodes ; i++ ) {    
      Accum = OutputWeights[HiddenNodes][i] ;
      for( j = 0 ; j < HiddenNodes ; j++ ) {
        Accum += Hidden[j] * OutputWeights[j][i] ;
      }
      Output[i] = 1.0/(1.0 + exp(-Accum)) ; 
    }
    Serial.println("");
    Serial.print ("Output = ");
    for( i = 0 ; i < OutputNodes ; i++ ) {       
      Serial.print (Output[i], 5);
      Serial.print (" ");
    }
    Serial.println("");
  }
/********************
 * ANN CODE ENDS HERE
 *********************/

/* STATISTICAL DECISION */
  if (Output[0]<0.5000){
    A = 0;
  }
  if (Output[0]>=0.5000){
    A = 1;
  }
  if (Output[1]<0.5000){
    B = 0;
  }
  if (Output[1]>=0.5000){
    B = 1;
  }
  if (A==0 and B==0){
    Happy++;
    if (Happy > 3){
            lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("HAPPY!");
      Serial.println("HAPPY DECISION");
      Hunger = 0;
      Pain   = 0;
      Happy  = 0;
    }
  }
  if (A==1 and B==0){
    Pain++;
    if (Pain > 3){
            lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("PAIN!");
      Serial.println("PAIN DECISION");
      Hunger = 0;
      Pain   = 0;
      Happy  = 0;
    }
  }
  if (A==0 and B==1){
    Hunger++;
    if (Hunger > 3){
            lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("HUNGRY!");
      Serial.println("HUNGER DECISION");
      Hunger = 0;
      Pain   = 0;
      Happy  = 0;
    }
  }  
  
}
