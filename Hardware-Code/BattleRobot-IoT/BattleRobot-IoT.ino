#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

void startFirebase();
void resultFirebase(FirebaseData &data);
void diam();
void move(int pin1, int pin2, int pinSpeed, int state1, int state2, int speed);

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
bool signupOK = false;

#define WIFI_SSID "ssid"
#define WIFI_PASSWORD "password"
#define API_KEY "api_key"
#define DATABASE_URL "url"

int pinKanan1 = 16;
int pinKanan2 = 17;
int pinSpeedKanan = 4;
int pinKiri1 = 18;
int pinKiri2 = 19;
int pinSpeedKiri = 5;

unsigned long prev_getData = 0;
String stringState;
String stringValue;
int intValue;
int jalan;
int belok;

int globalSpeed = 120;

void setup() {
  Serial.begin(115200);

  pinMode(LED_BUILTIN, OUTPUT);

  pinMode(pinKanan1, OUTPUT);
  pinMode(pinKanan2, OUTPUT);
  pinMode(pinSpeedKanan, OUTPUT);
  pinMode(pinKiri1, OUTPUT);
  pinMode(pinKiri2, OUTPUT);
  pinMode(pinSpeedKiri, OUTPUT);

  startFirebase();
}

// Memulai koneksi internet dan Firebase
void startFirebase()
{
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");

  while(WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.print(".");
    delay(250);
    digitalWrite(LED_BUILTIN, LOW);
    delay(250);
  }
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  config.api_key = API_KEY;

  if(Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("Firebase success");
    digitalWrite(LED_BUILTIN, LOW);
    signupOK = true;
  }
  else {
    String firebaseErrorMessage = config.signer.signupError.message.c_str();
    Serial.printf("%s\n", firebaseErrorMessage);
  }

  config.database_url = DATABASE_URL;
  config.token_status_callback = tokenStatusCallback;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  fbdo.setBSSLBufferSize(1024, 1024);
  fbdo.setResponseSize(1024);
  if (!Firebase.RTDB.beginStream(&fbdo, "/")){
    Serial.println(fbdo.errorReason());
  }
}

void loop() {
  int stateKanan1;
  int stateKanan2;
  int speedKanan;
  int stateKiri1;
  int stateKiri2;
  int speedKiri;

  // Baca data dari Firebase
  if (!Firebase.RTDB.readStream(&fbdo)){
    Serial.println(fbdo.errorReason());
  }
  if (fbdo.streamTimeout()){
    Serial.println("Stream timeout, resume streaming...");
    Serial.println();
  }
  if (fbdo.streamAvailable()){
    resultFirebase(fbdo);
  }

  // Pisahkan data
  stringState.replace("\\\"", "");

  int separatorIndex = stringState.indexOf('-'); 

  String stringJalan = stringState.substring(0, separatorIndex);
  String stringBelok = stringState.substring(separatorIndex + 1);

  jalan = stringJalan.toInt();
  belok = stringBelok.toInt();

  Serial.print("Jalan : ");
  Serial.println(jalan);
  Serial.print("Belok : ");
  Serial.println(belok);
  Serial.println();

  //Bergerak lurus
  if(jalan==0){       //Diam
    stateKanan1 = 0;
    stateKanan2 = 0;
    speedKanan = 0;
    stateKiri1 = 0;
    stateKiri2 = 0;
    speedKiri = 0;
  }
  else if(jalan==1){  //Maju
    stateKanan1 = 0;
    stateKanan2 = 1;
    speedKanan = globalSpeed;
    stateKiri1 = 0;
    stateKiri2 = 1;
    speedKiri = globalSpeed;
  }
  else if(jalan==2){  //Mundur
    stateKanan1 = 1;
    stateKanan2 = 0;
    speedKanan = globalSpeed - 30;
    stateKiri1 = 1;
    stateKiri2 = 0;
    speedKiri = globalSpeed - 30;
  }

  //Berbelok
  if(belok==0){       //Diam
    speedKanan += 0;
    speedKiri += 0;
  }
  else if(belok==1){  //Belok kiri
    if(jalan==0){
      stateKanan1 = 0;
      stateKanan2 = 1;
      speedKanan = globalSpeed - 20;
      speedKiri = 0;
    }
    else{
      speedKanan += globalSpeed / 2;
      speedKiri += 0;
    }
  }
  else if(belok==2){  //Belok kanan
    if(jalan==0){
      stateKiri1 = 0;
      stateKiri2 = 1;
      speedKanan = 0;
      speedKiri = globalSpeed - 20;
    }
    else{
      speedKanan += 0;
      speedKiri += globalSpeed / 2;
    }
  }

  move(pinKanan1, pinKanan2, pinSpeedKanan, stateKanan1, stateKanan2, speedKanan);
  move(pinKiri1, pinKiri2, pinSpeedKiri, stateKiri1, stateKiri2, speedKiri);
}

void resultFirebase(FirebaseData &data){
  if (fbdo.dataTypeEnum() == firebase_rtdb_data_type_string){
    stringState = fbdo.to<String>();
  }
}

// Perintah untuk bergerak
void move(int pin1, int pin2, int pinSpeed, int state1, int state2, int speed){
  analogWrite(pinSpeed, speed);
  digitalWrite(pin1, state1);
  digitalWrite(pin2, state2);
}

