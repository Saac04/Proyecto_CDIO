#include <Adafruit_ADS1X15.h>
#define channelValue0 0      //Defino canal 0 (Para el sesor de humedad)
#define channelValue1 1      //Defino canal 1 (Para el sesor de temperatura)
#define channelValue2 2      //Defino canal 2 (Para el sesor de pH)

//-----Datos necesarios para el sensor de ph-------------------------
#define Offset 1.78                                                  
#define samplingInterval 20                                          
#define printInterval 800                                            
#define ArrayLength 40
int pHArray[ArrayLength];
int pHArrayIndex=0;                                               
//-------------------------------------------------------------------

//-----Datos necesarios para el sensor de Humedad--------------------
int humidityValue = 0;       //Defino variable humedad               
const int Mojado = 4195;    //Defino valor en mojado
const int Seco = 7568;      //Defino valor en seco
//-------------------------------------------------------------------

#define power_pin 5      
Adafruit_ADS1115 ads1115;
struct Data{
    int x;
    int y;
};            
void setup() {
  pinMode(power_pin,OUTPUT); // Configurar power_pin como pin de salida
  Serial.begin(9600);         
  Serial.println("Inicializando...");
  ads1115.begin(); //Inicializando ads1115 
  Serial.println("Ajustando la ganancia...");
  ads1115.setGain(GAIN_ONE);                //Ajusto la ganancia


  Serial.println("Tomando medidas");

  Serial.println("Rango del ADC: +/- 4.096V (1 bit=2mV)");
}

//-----Sensor de Humedad-----------------------------------------------------
void humedadMapeo(){
  int16_t adc0;
  adc0 = ads1115.readADC_SingleEnded(channelValue0);      //Lectura del adc
  
  humidityValue = map(adc0,0,1023,0,255);   //Convertir valores en % 
  int humidityValue2 = 100*Seco/(Seco-Mojado)-humidityValue*100/(Seco-Mojado);   //Convertir valores en % 

  Serial.print(" Humedad (%): ");
  Serial.print(humidityValue2,DEC);         //Muestro el valor en pantalla
  Serial.println("%");
}
//---------------------------------------------------------------------------

//-----Sensor de Salinidad---------------------------------------------------
double lagrange(Data f[], int xi, int n){  //funcion del polinomio de Lagrange
    double res = 0; // se crea la variable resultado
 
    for (int i=0; i<n; i++){
        //calcula los elementos individuales de la formula de Lagrange
        double calculo = f[i].y;
        for (int j=0; j<n; j++){
          if (j!=i){
            calculo = calculo*(xi - f[j].x)/double(f[i].x - f[j].x);
          }
        }
        //El calculo hecho se añade al resultado final
        res += calculo;
    }
 
    return res;
}

void salinidad(){
  int sol=0;
  int16_t adc0;

  digitalWrite(power_pin, HIGH);
  delay(100);

  adc0 = analogRead(A0); //leemos el valor del adc
  digitalWrite(power_pin, LOW);

  Serial.print("Lectura digital de la sal =");
  Serial.println(adc0, DEC);

  Data f[] = {{288,0}, {382,5}, {397,10}, {424,15},{436,20}};
  // estos valores son los digitales por gramos, respectivamente
  sol=lagrange(f, adc0, 5); // se llama a la funcion, siendo 5 el numero de muestras
  if (sol < 0){
    Serial.print(sol);
    Serial.println(" gramos");  
  }
  else{
    Serial.println("no se detecta sal");
  }
}
//---------------------------------------------------------------------------

//-----Sensor de Temperatura-------------------------------------------------
void temperatura(){
  
  int16_t adc=ads1115.readADC_SingleEnded(channelValue1);      //Lectura del adc;
  double temperatura;
  double m = 33*pow(10,-3);  
  double b = 0.79;
  double vo = (adc * 4.096 )/32767;
  temperatura = (((vo*0.644) - b)/m);

  Serial.print("Temperatura: ");
  Serial.println(temperatura); 
}
//---------------------------------------------------------------------------

//-----Sensor de pH----------------------------------------------------------
double averageSample(int cuantos, int *p){
  int suma=0;
  for(int i=0;i<cuantos;i++){
    suma=suma+p[i];
  }
  double media=suma/cuantos;
  return media;
}

void pH(){

  static unsigned long samplingTime = millis();
  static unsigned long printTime = millis();
  static float pHValue, voltage;
  if(millis() - samplingTime > samplingInterval){

     pHArray[pHArrayIndex++]=ads1115.readADC_SingleEnded(channelValue2);      //Lectura del adc;

  }
    
  if (pHArrayIndex == ArrayLength){
    pHArrayIndex=0;
    
    voltage = (averageSample(ArrayLength,& pHArray[0]))/10000;
    pHValue = 3.5 * voltage + Offset;
    samplingTime = millis();
  }
  if(millis() - printTime > printInterval){
      
     Serial.print("Voltage: "); 
     Serial.println(voltage, 2); 
     Serial.print("pH Value: "); 
     Serial.println(pHValue, 2); 
      printTime = millis();
  }
}
//---------------------------------------------------------------------------

void loop() { 
  
  salinidad();

  humedadMapeo();

  temperatura();

  pH();

  delay(1000);                       //Retraso 1000ms

}        //utilizado para llamar las  funciones
