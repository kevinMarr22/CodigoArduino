#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <TinyGPS++.h>
#include <WiFi.h>
#include <HTTPClient.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

TinyGPSPlus gps;
HardwareSerial mySerial(1); 

// Variables de los botones
const int botonAvanzar = 12;
const int botonSeleccionar = 13;

int opcionSeleccionada = 0; 
int contadorID = 1; 

const char* ssid = "Galaxy S24 Ultra E9ED";
const char* password = "daniel00";

const char* serverURL = "https://0kk5e8xs2l.execute-api.us-east-1.amazonaws.com/coordenadas";

double latitud = 0.0;
double longitud = 0.0;

void setup() {
  Serial.begin(9600);
  mySerial.begin(9600, SERIAL_8N1, 16, 17);
  lcd.init();
  lcd.backlight();

  pinMode(botonAvanzar, INPUT_PULLUP);
  pinMode(botonSeleccionar, INPUT_PULLUP);

  // Conexión a WiFi
  conectarWiFi();

  lcd.setCursor(0, 0);
  lcd.print("Esperando GPS...");
}

void loop() {
  while (mySerial.available() > 0) {
    if (gps.encode(mySerial.read())) {
      if (gps.location.isValid()) {
        latitud = gps.location.lat();
        longitud = gps.location.lng();

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Lat: ");
        lcd.print(latitud, 6);
        lcd.setCursor(0, 1);
        lcd.print("Lon: ");
        lcd.print(longitud, 6);
        delay(5000);

        preguntarEnviarCoordenada();
      }
    }
  }
}

void conectarWiFi() {
  WiFi.begin(ssid, password);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Conectando WiFi...");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    lcd.setCursor(0, 1);
    lcd.print(".");
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi Conectado!");
  delay(1000);
}

void preguntarEnviarCoordenada() {
  bool seleccionado = false;
  lcd.clear();

  while (!seleccionado) {
    lcd.setCursor(0, 0);
    lcd.print("Enviar?");

    if (opcionSeleccionada == 0) {
      lcd.setCursor(0, 1);
      lcd.print("> Si    No");
    } else {
      lcd.setCursor(0, 1);
      lcd.print("  Si  > No");
    }

    if (digitalRead(botonAvanzar) == LOW) {
      opcionSeleccionada = !opcionSeleccionada;
      delay(300);
    }

    if (digitalRead(botonSeleccionar) == LOW) {
      seleccionado = true;
      delay(300); 
    }
  }

  if (opcionSeleccionada == 0) {
    enviarCoordenadaServidor();
  }
}

void enviarCoordenadaServidor() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverURL);

    http.addHeader("Content-Type", "application/json");

    String id = "cord" + String(contadorID);
    String postData = "{\"lat\":" + String(latitud, 6) + 
                      ", \"lon\":" + String(longitud, 6) + 
                      ", \"id\":\"" + id + "\"}";

    int httpResponseCode = http.PUT(postData);

    lcd.clear();
    if (httpResponseCode > 0) {
      lcd.setCursor(0, 0);
      lcd.print("Enviado!");
      contadorID++; 
    } else {
      lcd.setCursor(0, 0);
      lcd.print("Error de envio!");
    }

    http.end();
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Sin conexion");
  }

  delay(2000); 
}

