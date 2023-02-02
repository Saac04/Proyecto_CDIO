#define channelValue0 0      //Defino canal 0 (Para el sensor de humedad)
#define channelValue1 1      //Defino canal 1 (Para el sensor de temperatura)
#define channelValue2 2      //Defino canal 2 (Para el sensor de luz)
#define channelValue3 3      //Defino canal 3 (Para el sensor de pH)

//-----Datos necesarios para el sensor de ph-------------------------
#define Offset 0.37                                                  
#define samplingInterval 20                                          
#define printInterval 800                                            
#define ArrayLength 40
int pHArray[ArrayLength];
int pHArrayIndex=0;                                               
//-------------------------------------------------------------------

//-----Datos necesarios para el sensor de Salinidad------------------
int salinityValue = 0;       //Defino variable humedad               
const int Salado = 108;    //Defino valor en mojado
const int no_Salado = 76;      //Defino valor en seco
//-------------------------------------------------------------------

//-----Datos necesarios para el sensor de Humedad--------------------
int humidityValue = 0;       //Defino variable humedad               
unsigned int Mojado = 17000;    //Defino valor en mojado 4195
unsigned int Seco = 30406;      //Defino valor en seco  7568
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
#define WiFi_CONNECTION_UPV

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

#define NUM_FIELDS_TO_SEND 5 //Numero de medidas a enviar al servidor REST (Entre 1 y 8)

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

int mapeo(int adc){
  int mapValue = map(adc,0,1023,0,255); 
  return mapValue;
}

//-----Sensor de Humedad-----------------------------------------------------
int humedad(int channelValue){
  int16_t adc;
  adc = ads1115.readADC_SingleEnded(channelValue);      //Lectura del adc
  
  int humidityValue1 = adc;   //Convertir valores en % 
  int humidityValue2 = 100*Seco/(Seco-Mojado)-humidityValue1*100/(Seco-Mojado);   //Convertir valores en % 

    /*Serial.print(" Humedad (%): ");
    Serial.print(humidityValue1);         //Muestro el valor en pantalla
    Serial.println("%");*/

  if (humidityValue2 < 100 && humidityValue2 > 0){

    return humidityValue2;
  }
  
  if(humidityValue2 > 100){
    
    return 100;
  }
    
  if(humidityValue2 < 0){
    
    //Serial.println("no hay humedad");
    return 0;
  }
}
//---------------------------------------------------------------------------

//-----Sensor de Salinidad---------------------------------------------------

int salinidad(int channelValue){
  int sol=0;
  int16_t adc;
  digitalWrite(power_pin, HIGH);
  delay(100);

  adc = analogRead(channelValue); //leemos el valor del adc

  int mapeoSalinidad = adc;
  int valorSalinidad = 100*no_Salado/(no_Salado-Salado)-adc*100/(no_Salado-Salado);; 
  
  
  digitalWrite(power_pin, LOW);

  Serial.print("Lectura digital de la sal =");
  Serial.println(channelValue, DEC);

  if (valorSalinidad  < 100 && valorSalinidad  > 0){
    /*Serial.print(" Humedad (%): ");
    Serial.print(humidityValue2,DEC);         //Muestro el valor en pantalla
    Serial.println("%");*/
    return valorSalinidad ;
  }
  
  if(valorSalinidad  > 100){
    
    return 100;
  }
    
  if(valorSalinidad  < 0){
    
    //Serial.println("no hay humedad");
    return 0;
  }
}
//---------------------------------------------------------------------------

//-----Sensor de Temperatura-------------------------------------------------
double temperatura(int channelValue){
  
  int16_t adc=ads1115.readADC_SingleEnded(channelValue);      //Lectura del adc;
  double temperatura;
  double m = 33*pow(10,-3);  
  double b = 0.79;
  double vo = (adc * 4.096 )/32767;
  temperatura = (((vo*0.79  ) - b)/m);

  //Serial.print("Vo: ");
  //Serial.println(vo);
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

float pH(int channelValue){

  static unsigned long samplingTime = millis();
  static unsigned long printTime = millis();
  static float pHValue, voltage, voltage_digital;
  if(millis() - samplingTime > samplingInterval){

     pHArray[pHArrayIndex++]=ads1115.readADC_SingleEnded(channelValue);      //Lectura del adc;

  } 
    
  if (pHArrayIndex == ArrayLength){
    pHArrayIndex=0;
    
    voltage_digital = (averageSample(ArrayLength,& pHArray[0]));
    voltage = voltage_digital*4.096/32767;
    pHValue = 3.5 * voltage + Offset;
    samplingTime = millis();
  }
  if(millis() - printTime > printInterval){
      
     //Serial.print("Voltage: "); 
     //Serial.println(voltage, 2); 
     //Serial.print("pH Value: "); 
     //Serial.println(pHValue, 2); 
      printTime = millis();
     return pHValue;
  }
}
//---------------------------------------------------------------------------

//----------Sensor de luz----------------------------------------------------
double luz(int channelValue){
  int16_t adc=ads1115.readADC_SingleEnded(channelValue);      //Lectura del adc;

  double v_out=(adc*4.096)/32767;
  return v_out;
   
}
//---------------------------------------------------------------------------
