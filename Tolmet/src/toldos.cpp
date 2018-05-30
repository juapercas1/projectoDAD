#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <RestClient.h>
#include <PubSubClient.h>
#include <DHT.h>

//Variables globales de uso común.
WiFiClient espClient;
PubSubClient pubsubClient(espClient);
char msg[50];
const char* serverIP = "192.168.1.132";

ESP8266WebServer http_rest_server(8080);
RestClient client = RestClient(serverIP, 8083);

const char* ssid = "MIWIFI_2G_wTjC";
const char* password = "bxhc6eax6y4w";

String idSensorLluvia = "SA1";
//Cierre de toldos por agua.
unsigned long previousMillis1 = 0;
long OnTime1 = 5000;
int contador = 0;

//Simula el motor. (ON/OFF)
int LED_MOTOR = D0;

//Muestra datos del sensor conectado.
void datosSensorLluvia(){
	//Obtenemos los datos del sensor/sensores conectados al MCU en BD.
	String response = "";
	String aux = "/api/sensores/" + idSensorLluvia;
	int statusCode = client.get(aux.c_str(),&response);
	Serial.println(statusCode);
  const size_t capacity = JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(4);
  DynamicJsonBuffer jsonBuffer(capacity);
  JsonObject& root = jsonBuffer.parseObject(response);
  if (!root.success()) {
    Serial.println("parseObject() failed");
    return;
  }
	//Mostramos los datos obtenidos del parseo.
  String idSensor = root["idSensor"];
  String tipo = root["tipoSensor"];
	String descripcion = root["descripcion"];
	String mcu = root["idWifi"];
  Serial.println(idSensor);
  Serial.println(tipo);
	Serial.println(descripcion);
  Serial.println(mcu);
}

//Añade información a la BD.
void addLluvia(){
	const size_t capacity = JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(2);
	DynamicJsonBuffer jsonBuffer(capacity);
	JsonObject& newJson = jsonBuffer.createObject();
	newJson["idSensor"] = idSensorLluvia;
	//Sensor de agua
	int sensorValue = analogRead(A0);   // Lee el valor analógico A0
	float lluviaValor = sensorValue * (100.0 / 1023.0);   // Convierte 0-1023 a 0-100%.
	String auxLluvia = String(lluviaValor);
	Serial.print("Enviando intensidad de lluvia = ");
	Serial.println(auxLluvia);
	newJson["valor"] = auxLluvia;
	char jsonStr[45];
	newJson.printTo(jsonStr);
	int statusCode = client.put("/api/fechas", jsonStr);

}

//Actualiza el estado de los elementos(TOLDOS)
void actualizarEstado(int estado){
	const size_t capacity = JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(2);
	DynamicJsonBuffer jsonBuffer(capacity);
	JsonObject& newJson = jsonBuffer.createObject();
  newJson["idElemento"] = "TOLDO1";
	String estadoAux = (String) estado;
	newJson["estado"] = estadoAux;

	char jsonStr[45];
	newJson.printTo(jsonStr);
	int statusCode = client.put("/api/elementos/updateEstado", jsonStr);
}

//Abrir o cerrar toldo
void desplegarToldo(){
	Serial.println("Desplegando toldo --> LED = ON");
	digitalWrite(LED_MOTOR, HIGH);
	actualizarEstado(1);
}
void recogerToldo(){
	Serial.println("Recogiendo toldo --> LED = OFF");
	 digitalWrite(LED_MOTOR, LOW);
	 actualizarEstado(0);
}

//Comprobar el estado del toldo.
void comprobarEstado(String elemento){
	String aux = "/api/elementos/estado/"+ elemento;
	int statusCode = client.get(aux.c_str());
}
void cerrarPorLluvia(){
	//Revisa sensor de lluvia, para recoger el toldo en caso de lluvia.
	unsigned long currentMillis = millis();
	//Comprobar sensor de lluvia cada 5 segundos.
	if(currentMillis - previousMillis1 >= OnTime1) {
		int sensorValue = analogRead(A0);   // Lee el valor analógico A0
    float lluviaValor = sensorValue * (100.0 / 1023.0);
    previousMillis1 = currentMillis; // "La ultima vez fue AHORA"
		if(lluviaValor > 10.0){
			contador++;
		//	Serial.println(contador);
		}else{
			contador = 0;
		}
		if(contador == 5){
				comprobarEstado("TOLDO1");
				contador = 0;
		}
  }
}

//Función cuando se recibe un mensaje del servidor MQTT
void callback(char* topic, byte* payload, unsigned int length) {
		Serial.print("Mensaje recibido [");
		String topicAux = (String) topic;
		Serial.print(topic);
		Serial.print("] ");
		String message;
		for (int i = 0; i < length; i++) {
			 message = message + String((char)payload[i]);
 		}
		Serial.print(message);
		Serial.println();
		// Trabajar con el mensaje recibido
		if(topicAux.equals("inMCU1")){
				if(message.equals("desplegarToldo")){
						if(digitalRead(LED_MOTOR) != HIGH){
							desplegarToldo();
						}
				}else if(message.equals("recogerToldo")){
						if(digitalRead(LED_MOTOR) != LOW){
							recogerToldo();
						}
				}else if(message.equals("addDatos")){
						addLluvia();
				}else if(message.startsWith("estado=")){
							String estado = message.substring(7);
								if(estado.equals("1")){
									recogerToldo();
								}
				}
		}
}

//Conexion al broker MQTT
void reconnect() {
	while (!pubsubClient.connected()) {
		Serial.print("Conectando al servidor MQTT --> ");
		if (pubsubClient.connect("MCU1")) {
			Serial.println("Conectado");
			pubsubClient.subscribe("inMCU1");
		} else {
			Serial.print("Error, rc=");
			Serial.print(pubsubClient.state());
			Serial.println(" Reintentando en 5 segundos");
			delay(5000);
		}
	}
}

void setup() {
	//LED que simulará el motor. 1 abierta -- 0 cerrada.
	pinMode(LED_MOTOR, OUTPUT);

	Serial.begin(115200);
	delay(10);

	Serial.println();
	Serial.print("Conectando a ");
	Serial.println(ssid);

	// Modo cliente.
	WiFi.mode(WIFI_STA);
	WiFi.begin(ssid, password);
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}
	Serial.println("");
	Serial.print("Red conectada. Direccion IP: ");
	Serial.println(WiFi.localIP());

	pubsubClient.setCallback(callback);
	pubsubClient.setServer(serverIP, 1883);
}

void loop() {
	// MQTT reconexión
	if (!pubsubClient.connected()) {
		reconnect();
	}
	//Para recoger los mensajes MQTT recividos.
	pubsubClient.loop();

	//Cuando la lluvia es superior a 10% cierre de toldos.
	cerrarPorLluvia();
	}
