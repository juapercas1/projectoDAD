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
const char* serverIP = "192.168.1.133";

ESP8266WebServer http_rest_server(8080);
RestClient client = RestClient(serverIP, 8083);

const char* ssid = "MIWIFI_2G_wTjC";
const char* password = "bxhc6eax6y4w";

DHT dht; //Sensor de temperatura
String idSensorTemperatura = "ST1";
String idSensorLuminosidad = "SL1";
//Cierre de persianas por luminosidad.
unsigned long previousMillis1 = 0;
long OnTime1 = 5000;
int contador = 0;

//Simula el motor. (ON/OFF)
int LED_MOTOR = D0;

//Muestra datos de los sensores conectados.
void datosSensorTemperatura(){
	//Obtenemos los datos del sensor temperatura conectado al nodeMCU en BD.
	String response = "";
	String aux = "/api/sensores/" + idSensorTemperatura;
	int statusCode = client.get(aux.c_str(), &response);
  const size_t capacity = JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(4);
	Serial.println(response);
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
void datosSensorLuminosidad(){
	//Obtenemos los datos del sensor/sensores conectados al MCU en BD.
	String response = "";
	String aux = "/api/sensores/" + idSensorLuminosidad;
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
void addTemperatura(){
	const size_t capacity = JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(2);
	DynamicJsonBuffer jsonBuffer(capacity);
	//Con esto incluimos en BD las nuevas temperaturas.(RegFechas)
  JsonObject& newJson = jsonBuffer.createObject();
  newJson["idSensor"] = idSensorTemperatura;
	double temperatura = dht.getTemperature();
	String aux = String(temperatura);
	Serial.print("Enviando temperatura = ");
	Serial.println(temperatura);
	//float humedad = dht.getHumidity();
  newJson["valor"] = aux;
  char jsonStr[45];
  newJson.printTo(jsonStr);
  int statusCode = client.put("/api/fechas", jsonStr);
}
void addLuminosidad(){
	const size_t capacity = JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(2);
	DynamicJsonBuffer jsonBuffer(capacity);
	//Con esto incluimos en BD las nuevas luminosidad.(RegFechas)
  JsonObject& newJson = jsonBuffer.createObject();
  newJson["idSensor"] = idSensorLuminosidad;
	//Sensor de luminosidad
	int sensorValue = analogRead(A0);   // Lee el valor analógico A0
	float lumValor = sensorValue * (100.0 / 1023.0);   // Convierte 0-1023 a 0-100%.
	String auxLuminosidad = String(lumValor);
	Serial.print("Enviando luminosidad = ");
	Serial.println(auxLuminosidad);
  newJson["valor"] = auxLuminosidad;
  char jsonStr[45];
  newJson.printTo(jsonStr);
  int statusCode = client.put("/api/fechas", jsonStr);

}

//Actualiza el estado de los elementos(PERSIANA)
void actualizarEstado(int estado){
	const size_t capacity = JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(2);
	DynamicJsonBuffer jsonBuffer(capacity);
	JsonObject& newJson = jsonBuffer.createObject();
  newJson["idElemento"] = "PERSIANA1";
	String estadoAux = (String) estado;
	newJson["estado"] = estadoAux;

	char jsonStr[45];
	newJson.printTo(jsonStr);
	int statusCode = client.put("/api/elementos/updateEstado", jsonStr);
}

//Abrir o cerrar persiana
void abrirPersiana(){
	Serial.println("Abriendo persiana --> LED = ON");
	digitalWrite(LED_MOTOR, HIGH);
	actualizarEstado(1);
}
void cerrarPersiana(){
	Serial.println("Cerrando persiana --> LED = OFF");
	 digitalWrite(LED_MOTOR, LOW);
	 actualizarEstado(0);
}

//Comprobar el estado de la persiana.
void comprobarEstado(String elemento){
	String aux = "/api/elementos/estado/"+ elemento;
	int statusCode = client.get(aux.c_str());
}
void cerraPorLuminosidad(){
	//Revisa sensor de luminosidad, para cerrar persianas cuando oscurezca.
  unsigned long currentMillis = millis();
	//Comprobar sensor de luminosidad cada 5 segundos.
	if(currentMillis - previousMillis1 >= OnTime1) {
		int sensorValue = analogRead(A0);   // Lee el valor analógico A0
    float lumValor = sensorValue * (100.0 / 1023.0);
    previousMillis1 = currentMillis; // "La ultima vez fue AHORA"
		if(lumValor < 20.0){
			contador++;
			//Serial.println(contador);
		}
		if(contador == 5){
				comprobarEstado("PERSIANA1");
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
		if(topicAux.equals("inMCU2")){
				if(message.equals("temperaturaActual")){
					float temperatura = dht.getTemperature();
					snprintf (msg, 75, "Temperatura actual es de: %.2f", temperatura);
					pubsubClient.publish("outMCU2", msg);
				}else if(message.equals("abrirPersiana")){
						if(digitalRead(LED_MOTOR) != HIGH){
							abrirPersiana();
						}
				}else if(message.equals("cerrarPersiana")){
						if(digitalRead(LED_MOTOR) != LOW){
							cerrarPersiana();
						}
				}else if(message.equals("addDatos")){
						addLuminosidad();
						addTemperatura();
				}else if(message.startsWith("estado=")){
							String estado = message.substring(7);
								if(estado.equals("1")){
									cerrarPersiana();
								}
				}
		}
}

//Conexion al broker MQTT
void reconnect() {
	while (!pubsubClient.connected()) {
		Serial.print("Conectando al servidor MQTT --> ");
		if (pubsubClient.connect("MCU2")) {
			Serial.println("Conectado");
			//TODO --> cambiar para publicar sobre el topic correcto.
			//pubsubClient.publish("topic_2", "Hola a todos");
			pubsubClient.subscribe("inMCU2");
		} else {
			Serial.print("Error, rc=");
			Serial.print(pubsubClient.state());
			Serial.println(" Reintentando en 5 segundos");
			delay(5000);
		}
	}
}

void setup() {
	//Iniciamos sensor de temperatura y humedad
	dht.setup(D4);
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

	//Cuando la luminosidad es inferior a 20% cierre de persianas.
	cerraPorLuminosidad();

	}
