#include <Adafruit_ADS1X15.h>
#include "Funciones.h"
void loop() { 
  
String data[ NUM_FIELDS_TO_SEND + 1];  // Podemos enviar hasta 8 datos

    
    data[ 1 ] = String(humedad(channelValue0)); //Escribimos el dato 1. Recuerda actualizar numFields
    #ifdef PRINT_DEBUG_MESSAGES
        Serial.print( "Humedad = " );
        Serial.print( data[ 1 ] );
        Serial.println("%");
    #endif

    data[ 2 ] = String(salinidad(A0)); //Escribimos el dato 2. Recuerda actualizar numFields
    #ifdef PRINT_DEBUG_MESSAGES
        Serial.print( "Salinidad = " );
        Serial.print( data[ 2 ] );
        Serial.println("%");
    #endif

    data[ 3 ] = String(temperatura(channelValue1)); //Escribimos el dato 3. Recuerda actualizar numFields
    #ifdef PRINT_DEBUG_MESSAGES
        Serial.print( "temperatura = " );
        Serial.print( data[ 3 ] );
        Serial.println("ºC");
    #endif

    data[ 4 ] = String(pH(channelValue3)); //Escribimos el dato 4. Recuerda actualizar numFields
    #ifdef PRINT_DEBUG_MESSAGES
        Serial.print( "pH = " );
        Serial.println( data[ 4 ] );
    #endif

    data[ 5 ] = String(luz(channelValue2)); //Escribimos el dato 5. Recuerda actualizar numFields
    #ifdef PRINT_DEBUG_MESSAGES
        Serial.print( "Intensidad luminosa = " );
        Serial.println( data[ 5 ] );
    #endif
    

    //Selecciona si quieres enviar con GET(ThingSpeak o Dweet) o con POST(ThingSpeak)
    //HTTPPost( data, NUM_FIELDS_TO_SEND );
    HTTPGet( data, NUM_FIELDS_TO_SEND );

    //Selecciona si quieres un retardo de 15seg para hacer pruebas o dormir el SparkFun
    delay( 4500 );   
    //Serial.print( "Goodnight" );
    //ESP.deepSleep( sleepTimeSeconds * 1000000 );

}       //utilizado para llamar las  funciones
