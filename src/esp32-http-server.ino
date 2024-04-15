#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <ESP32Servo.h>

//Claves para conectarse a la red WiFi
const char *ssid = "Wokwi-GUEST";
const char *password = "";
const int channel = 11;
//URL de la API especificando el Id de la toma de agua
const String apiUrl = "http://192.168.1.11:8120/toma_agua/TMA_3728";
//Objeto HTTPClient para realizar peticiones HTTP
HTTPClient http;
//Objeto Servo para controlar el servo
Servo myservo;

bool estadoAnterior = false; // Variable para almacenar el estado anterior de la toma

//Configuración inicial
void setup()
{
// Inicializa el puerto serie y la conexión WiFi
 Serial.begin(115200);
 WiFi.begin(ssid, password, channel);
 // Espera a que la conexión WiFi se establezca
 http.begin(apiUrl);
 // Inicializa el servo en el pin 17
 myservo.attach(17);
}
//Función para verificar la conexión WiFi
void checkWiFi() {
 while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Reconectando a WiFi...");
    WiFi.begin(ssid, password, channel);
    delay(5000); // Espera antes de intentar reconectar
 }
}
//Función para mover el servo que recibe la posición inicial, final y el tiempo de retraso
void moveServo(int startPos, int endPos, int delayTime) {
 for (int pos = startPos; pos != endPos; pos += (pos < endPos ? 1 : -1)) {
    myservo.write(pos);
    delay(delayTime);
 }
 myservo.write(endPos); 
}

void loop()
{
 checkWiFi(); // Verifica y reconecta si es necesario


 int statusCode = http.GET(); // Realiza una petición GET a la API
 Serial.println("HTTP request");
 if (statusCode == HTTP_CODE_OK)
 {
    Serial.println("HTTP request OK");
    // Get current system time
    time_t now = time(nullptr);
    struct tm *timeinfo;
    timeinfo = localtime(&now);
    
    // Print current time and day
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%H:%M:%S %A", timeinfo);
    Serial.print("Current time and day: ");
    Serial.println(buffer);
    String response = http.getString();
    JsonDocument doc;
    deserializeJson(doc, response);
    bool estadoActual = doc["estado"]; // Respuesta JSON permitiendo ver el estado de la toma
    if (estadoActual != estadoAnterior)
    { // Verificar si el estado ha cambiado
      if (estadoActual)
      {
        Serial.println("Toma is active");
        moveServo(180, 0, 15); // Usa la función moveServo para mover el servo
        myservo.write(0); // Mantener el servo en la posición abierta
      }
      else
      {
        Serial.println("Toma is inactive");
        moveServo(0, 180, 15); // Usa la función moveServo para mover el servo
        myservo.write(180); // Mantener el servo en la posición cerrada
      }
      estadoAnterior = estadoActual; // Actualizar el estado anterior
    }
    else
    {
      Serial.println("No hay cambios en el estado de la toma");
    }
 }
 else
 {
    Serial.println("Error in HTTP request");
 }
 delay(2000);
}