#include <SoftwareSerial.h>
#define SIM800_TXPIN_RXARDUINO 7
#define SIM800_RXPIN_TXARDUINO 8
#define TEMPERATURE_PIN A0 // Se agrega la variable temperatura

SoftwareSerial sim800(SIM800_TXPIN_RXARDUINO, SIM800_RXPIN_TXARDUINO); // Seleccionamos los pines 7 como Rx y 8 como Tx

void setup() {
  Serial.begin(9600);
  sim800.begin(9600);

  delay(1000);
  Serial.println("Connecting to the network...");
  sim800.println("AT");
  delay(1000);

  // Configurar el APN (Nombre del Punto de Acceso) de tu proveedor de servicios móviles
  sim800.println("AT+CSTT=\"internet.movistar.com.co\",\"movistar\",\"movistar\"");
  delay(3000);

  Serial.println("Connecting to GPRS...");
  sim800.println("AT+CIICR");
  delay(6000);

  Serial.println("Getting local IP address...");
  sim800.println("AT+CIFSR");
  delay(2000);
}

void loop() {
  // Lectura de temperatura
  float temperature = readTemperature();

  // Configurar la solicitud POST
  String postData = "&value=" + String(temperature) + "&type=4" + "&id_device=3"; // Parámetros de ejemplo
  Serial.print ("Temperatura enviada:");
  Serial.println (temperature);
  String postRequest = "POST /subirdatos.php HTTP/1.1\r\nHost: telemsebas-0a07559143b3.herokuapp.com\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: ";
  postRequest += String(postData.length());
  postRequest += "\r\n\r\n";
  postRequest += postData;

  // Condicional para enviar SMS si la temperatura sobrepasa 40 grados Celsius
  if (temperature > 40) {
    sendSMS(); // Llamada a la función para enviar SMS
  }

  // Enviar la solicitud POST al servidor
  Serial.println("Sending POST request...");
  sim800.println("AT+CIPSTART=\"TCP\",\"telemsebas-0a07559143b3.herokuapp.com\",80");
  delay(6000);
  sim800.print("AT+CIPSEND=");
  sim800.println(postRequest.length());
  delay(500);
  sim800.print(postRequest);
  delay(500);
  sim800.println("AT+CIPCLOSE");

  // Esperar y mostrar la respuesta del servidor
  delay(3000);
  obtenerRespuesta();

  delay(5000); // Espera 5 segundos antes de realizar otra petición
}

void obtenerRespuesta() {
  while(sim800.available()) Serial.write(sim800.read());
}

float readTemperature() {
  int sensorValue = analogRead(TEMPERATURE_PIN);
  float temperature = (sensorValue / 1023.0) * 5000; // Convertir a voltaje (mV)
  temperature = temperature / 10; // Convertir a temperatura en grados Celsius
  return temperature;
}

// Función para enviar SMS
void sendSMS() {
  sim800.println("AT+CMGF=1"); // Configura el módulo para enviar SMS
  delay(1000);
  
  // Comando AT para establecer el número de teléfono destino
  sim800.println("AT+CMGS=\"+573185222962\""); // Reemplaza +XXXXXXXXXXX con el número de teléfono al que deseas enviar el mensaje
  delay(1000);

  // El texto del mensaje
  sim800.println("Alerta: La temperatura ha sobrepasado los 50 grados Celsius.");
  delay(1000);

  // Ctrl+Z para enviar el SMS
  sim800.write(26);
  delay(1000);

  Serial.println("SMS enviado con éxito");
}
