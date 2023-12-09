#include <Wire.h>
#include "DHT.h"
#include <WiFiMulti.h>  
WiFiMulti wifiMulti;
#define DEVICE "ESP32"
#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>

#define WIFI_SSID "wifi"
#define WIFI_PASSWORD "12341234"
#define INFLUXDB_URL "https://us-east-1-1.aws.cloud2.influxdata.com"
#define INFLUXDB_TOKEN "Nf5MoqWMmH_9DYSfTndBWA2q3xOnFbju8b7vPYIDLGDK_-VQjYFerAKUrSWkn-6m42Fk9I7yUwYKUL78QyEDxA=="
#define INFLUXDB_ORG "ce19576ebb411699"
#define INFLUXDB_BUCKET "Weather"
#define TZ_INFO "UTC-5"

DHT dht (25, DHT22);

float temp = 0;
float humd = 0;

InfluxDBClient client (INFLUXDB_URL,INFLUXDB_ORG,INFLUXDB_BUCKET,INFLUXDB_TOKEN,InfluxDbCloud2CACert);

Point sensor ("weather");// creacion del objeto sensor tipo Point 

void setup()
{
  Serial.begin(9600); // Configuracion de la velocidad de transmision entre la plata y el dispositivo IOT

  dht.begin();// coneccion al dth sensor

  WiFi.mode (WIFI_STA);// configuracion de la conexion wifi
  wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to wifi");
  while (wifiMulti.run() !=WL_CONNECTED)
  {
    Serial.print(".");
    delay(100);
  }
  Serial.println();

  sensor.addTag("device", DEVICE);
  sensor.addTag("SSID", WIFI_SSID);

  timeSync(TZ_INFO, "pool.ntp.org", "tim.nis.gov"); // funcion para validar el tiempo preciso de la toma de datos

  if(client.validateConnection())
  {
    Serial.print("Connected to InfluxDB");
    Serial.println(client.getServerUrl());

  }
  else
  {
     Serial.print("InfluxDB connection failed");
     Serial.println(client.getLastErrorMessage());
  }
}

void loop() 
{
 float temp = dht.readTemperature();// lectura de temperatura
 float humd = dht.readHumidity();// lectura de humedad

  sensor.clearFields(); // limpiar campos para reutilizar el point

  sensor.addField("temperature", temp);// almacenar dato de temperatura en el point
  sensor.addField("humidity", humd);// almacenar dato de humedad en el point

  if(wifiMulti.run() != WL_CONNECTED){
  Serial.println("Wifi connection lost");
  } // chequea la coneccion a wifi y reconectarla si es necesario

  // esribir valores medidos en la base de datos como una sola linea para que los dos valores
  // medidos tengan la misma marca de tiempo
  if(!client.writePoint(sensor))        
  {
    Serial.print("InfluxDB write failed");
    Serial.println(client.getLastErrorMessage());
  }
  Serial.print("Temp: ");
  Serial.println(temp);
  Serial.print("Humidity: ");
  Serial.println(humd);
  delay(60000);   // tiempo de espera para tomar datos (60 seconds) 
}