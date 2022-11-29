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

#include <ESP8266WiFi.h>

// Comentar/Descomentar para ver mensajes de depuracion en monitor serie y/o respuesta del HTTP server
#define PRINT_DEBUG_MESSAGES
//#define PRINT_HTTP_RESPONSE

// Comentar/Descomentar para conexion Fuera/Dentro de UPV
//#define WiFi_CONNECTION_UPV

// Selecciona que servidor REST quieres utilizar entre ThingSpeak y Dweet
#define REST_SERVER_THINGSPEAK //Selecciona tu canal para ver los datos en la web (https://thingspeak.com/channels/360979)
//#define REST_SERVER_DWEET //Selecciona tu canal para ver los datos en la web (https://dweet.io:443/get/dweets/for/cdiocurso2022G12)

///////////////////////////////////////////////////////
/////////////// WiFi Definitions /////////////////////
//////////////////////////////////////////////////////

#ifdef WiFi_CONNECTION_UPV //Conexion UPV
  const char WiFiSSID[] = "GTI1";
  const char WiFiPSK[] = "1PV.arduino.Toledo";
#else //Conexion fuera de la UPV
  const char WiFiSSID[] = "RU-Gandia";
  const char WiFiPSK[] = "104@Gandia-";
#endif



///////////////////////////////////////////////////////
/////////////// SERVER Definitions /////////////////////
//////////////////////////////////////////////////////

#if defined(WiFi_CONNECTION_UPV) //Conexion UPV
  const char Server_Host[] = "proxy.upv.es";
  const int Server_HttpPort = 8080;
#elif defined(REST_SERVER_THINGSPEAK) //Conexion fuera de la UPV
  const char Server_Host[] = "api.thingspeak.com";
  const int Server_HttpPort = 80;
#else
  const char Server_Host[] = "dweet.io";
  const int Server_HttpPort = 80;
#endif

WiFiClient client;

///////////////////////////////////////////////////////
/////////////// HTTP REST Connection ////////////////
//////////////////////////////////////////////////////

#ifdef REST_SERVER_THINGSPEAK 
  const char Rest_Host[] = "api.thingspeak.com";
  String MyWriteAPIKey="OEKNH7UU6UNHURU2"; // Escribe la clave de tu canal ThingSpeak
#else 
  const char Rest_Host[] = "dweet.io";
  String MyWriteAPIKey="cdiocurso2018g12"; // Escribe la clave de tu canal Dweet
#endif

#define NUM_FIELDS_TO_SEND 4 //Numero de medidas a enviar al servidor REST (Entre 1 y 8)

/////////////////////////////////////////////////////
/////////////// Pin Definitions ////////////////
//////////////////////////////////////////////////////

const int LED_PIN = 5; // Thing's onboard, green LED

/////////////////////////////////////////////////////
/////////////// WiFi Connection ////////////////
//////////////////////////////////////////////////////

void connectWiFi()
{
  byte ledStatus = LOW;

  #ifdef PRINT_DEBUG_MESSAGES
    Serial.print("MAC: ");
    Serial.println(WiFi.macAddress());
  #endif
  
  WiFi.begin(WiFiSSID, WiFiPSK);

  while (WiFi.status() != WL_CONNECTED)
  {
    // Blink the LED
    digitalWrite(LED_PIN, ledStatus); // Write LED high/low
    ledStatus = (ledStatus == HIGH) ? LOW : HIGH;
    #ifdef PRINT_DEBUG_MESSAGES
       Serial.println(".");
    #endif
    delay(500);
  }
  #ifdef PRINT_DEBUG_MESSAGES
     Serial.println( "WiFi Connected" );
     Serial.println(WiFi.localIP()); // Print the IP address
  #endif
}

/////////////////////////////////////////////////////
/////////////// HTTP POST  ThingSpeak////////////////
//////////////////////////////////////////////////////

void HTTPPost(String fieldData[], int numFields){

// Esta funcion construye el string de datos a enviar a ThingSpeak mediante el metodo HTTP POST
// La funcion envia "numFields" datos, del array fieldData.
// Asegurate de ajustar numFields al número adecuado de datos que necesitas enviar y activa los campos en tu canal web
  
    if (client.connect( Server_Host , Server_HttpPort )){
       
        // Construimos el string de datos. Si tienes multiples campos asegurate de no pasarte de 1440 caracteres
   
        String PostData= "api_key=" + MyWriteAPIKey ;
        for ( int field = 1; field < (numFields + 1); field++ ){
            PostData += "&field" + String( field ) + "=" + fieldData[ field ];
        }     
        
        // POST data via HTTP
        #ifdef PRINT_DEBUG_MESSAGES
            Serial.println( "Connecting to ThingSpeak for update..." );
        #endif
        client.println( "POST http://" + String(Rest_Host) + "/update HTTP/1.1" );
        client.println( "Host: " + String(Rest_Host) );
        client.println( "Connection: close" );
        client.println( "Content-Type: application/x-www-form-urlencoded" );
        client.println( "Content-Length: " + String( PostData.length() ) );
        client.println();
        client.println( PostData );
        #ifdef PRINT_DEBUG_MESSAGES
            Serial.println( PostData );
            Serial.println();
            //Para ver la respuesta del servidor
            #ifdef PRINT_HTTP_RESPONSE
              delay(500);
              Serial.println();
              while(client.available()){String line = client.readStringUntil('\r');Serial.print(line); }
              Serial.println();
              Serial.println();
            #endif
        #endif
    }
}

////////////////////////////////////////////////////
/////////////// HTTP GET  ////////////////
//////////////////////////////////////////////////////

void HTTPGet(String fieldData[], int numFields){
  
// Esta funcion construye el string de datos a enviar a ThingSpeak o Dweet mediante el metodo HTTP GET
// La funcion envia "numFields" datos, del array fieldData.
// Asegurate de ajustar "numFields" al número adecuado de datos que necesitas enviar y activa los campos en tu canal web
  
    if (client.connect( Server_Host , Server_HttpPort )){
           #ifdef REST_SERVER_THINGSPEAK 
              String PostData= "GET https://api.thingspeak.com/update?api_key=";
              PostData= PostData + MyWriteAPIKey ;
           #else 
              String PostData= "GET http://dweet.io/dweet/for/";
              PostData= PostData + MyWriteAPIKey +"?" ;
           #endif
           
           for ( int field = 1; field < (numFields + 1); field++ ){
              PostData += "&field" + String( field ) + "=" + fieldData[ field ];
           }
          
           
           #ifdef PRINT_DEBUG_MESSAGES
              Serial.println( "Connecting to Server for update..." );
           #endif
           client.print(PostData);         
           client.println(" HTTP/1.1");
           client.println("Host: " + String(Rest_Host)); 
           client.println("Connection: close");
           client.println();
           #ifdef PRINT_DEBUG_MESSAGES
              Serial.println( PostData );
              Serial.println();
              //Para ver la respuesta del servidor
              #ifdef PRINT_HTTP_RESPONSE
                delay(500);
                Serial.println();
                while(client.available()){String line = client.readStringUntil('\r');Serial.print(line); }
                Serial.println();
                Serial.println();
              #endif
           #endif  
    }
}



void setup() {
 
  #ifdef PRINT_DEBUG_MESSAGES
    Serial.begin(9600);
  #endif
  
  connectWiFi();
  digitalWrite(LED_PIN, HIGH);

  #ifdef PRINT_DEBUG_MESSAGES
      Serial.print("Server_Host: ");
      Serial.println(Server_Host);
      Serial.print("Port: ");
      Serial.println(String( Server_HttpPort ));
      Serial.print("Server_Rest: ");
      Serial.println(Rest_Host);
  #endif

  pinMode(power_pin,OUTPUT); // Configurar power_pin como pin de salida         
  Serial.println("Inicializando...");
  ads1115.begin(); //Inicializando ads1115 
  Serial.println("Ajustando la ganancia...");
  ads1115.setGain(GAIN_ONE);                //Ajusto la ganancia


  Serial.println("Tomando medidas");

  Serial.println("Rango del ADC: +/- 4.096V (1 bit=2mV)");
}

//-----Sensor de Humedad-----------------------------------------------------
int humedadMapeo(){
  int16_t adc0;
  adc0 = ads1115.readADC_SingleEnded(channelValue0);      //Lectura del adc
  
  humidityValue = map(adc0,0,1023,0,255);   //Convertir valores en % 
  int humidityValue2 = 100*Seco/(Seco-Mojado)-humidityValue*100/(Seco-Mojado);   //Convertir valores en % 

  if (humidityValue2 < 0){
    /*Serial.print(" Humedad (%): ");
    Serial.print(humidityValue2,DEC);         //Muestro el valor en pantalla
    Serial.println("%");*/
    return humidityValue2;
  }
  else{
    //Serial.println("no se humedad");
    return 0;
  }
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

int salinidad(){
  int sol=0;
  int16_t adc0;

  digitalWrite(power_pin, HIGH);
  delay(100);

  adc0 = analogRead(A0); //leemos el valor del adc
  digitalWrite(power_pin, LOW);

  //Serial.print("Lectura digital de la sal =");
  //Serial.println(adc0, DEC);

  Data f[] = {{288,0}, {382,5}, {397,10}, {424,15},{436,20}};
  // estos valores son los digitales por gramos, respectivamente
  sol=lagrange(f, adc0, 5); // se llama a la funcion, siendo 5 el numero de muestras
  if (sol < 0){
    /*Serial.print(sol);
    Serial.println(" gramos");*/
    return sol;
  }
  else{
    //Serial.println("no se detecta sal");
    return 0;
  }
}
//---------------------------------------------------------------------------

//-----Sensor de Temperatura-------------------------------------------------
double temperatura(){
  
  int16_t adc=ads1115.readADC_SingleEnded(channelValue1);      //Lectura del adc;
  double temperatura;
  double m = 33*pow(10,-3);  
  double b = 0.79;
  double vo = (adc * 4.096 )/32767;
  temperatura = (((vo*0.644) - b)/m);

  /*Serial.print("Temperatura: ");
  Serial.println(temperatura);*/
  return temperatura;
   
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

float pH(){

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
      
     /*Serial.print("Voltage: "); 
     Serial.println(voltage, 2); 
     Serial.print("pH Value: "); 
     Serial.println(pHValue, 2); 
      printTime = millis();*/
     return pHValue;
  }
}
//---------------------------------------------------------------------------

void loop() { 
  
String data[ NUM_FIELDS_TO_SEND + 1];  // Podemos enviar hasta 8 datos

    
    data[ 1 ] = String(humedadMapeo()); //Escribimos el dato 1. Recuerda actualizar numFields
    #ifdef PRINT_DEBUG_MESSAGES
        Serial.print( "Humedad = " );
        Serial.print( data[ 1 ] );
        Serial.println("%");
    #endif

    data[ 2 ] = String(salinidad()); //Escribimos el dato 2. Recuerda actualizar numFields
    #ifdef PRINT_DEBUG_MESSAGES
        Serial.print( "Salinidad = " );
        Serial.print( data[ 2 ] );
        Serial.println("g");
    #endif

    data[ 3 ] = String(temperatura()); //Escribimos el dato 3. Recuerda actualizar numFields
    #ifdef PRINT_DEBUG_MESSAGES
        Serial.print( "temperatura = " );
        Serial.print( data[ 3 ] );
        Serial.println("ºC");
    #endif

    data[ 4 ] = String(pH()); //Escribimos el dato 4. Recuerda actualizar numFields
    #ifdef PRINT_DEBUG_MESSAGES
        Serial.print( "pH = " );
        Serial.println( data[ 4 ] );
    #endif

    //Selecciona si quieres enviar con GET(ThingSpeak o Dweet) o con POST(ThingSpeak)
    //HTTPPost( data, NUM_FIELDS_TO_SEND );
    HTTPGet( data, NUM_FIELDS_TO_SEND );

    //Selecciona si quieres un retardo de 15seg para hacer pruebas o dormir el SparkFun
    delay( 15000 );   
    //Serial.print( "Goodnight" );
    //ESP.deepSleep( sleepTimeSeconds * 1000000 );

}       //utilizado para llamar las  funciones
