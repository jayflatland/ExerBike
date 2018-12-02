long scheduledNextTick;

int ledPin = 5;
int speedoPin = 36;
int resistoPin = 39;
int heartPin = 34;
int motor1Pin = 23;
int motor2Pin = 22;
int blinkyPin = 21;

#define HBUF_SZ 256
float heartHist[HBUF_SZ];
int heartIdx = 0;

int blinkyPinValue = 0;

float targetResist = 0.4;
float targetResistTol = 0.04;

float heartBPM = 0.0;


long lastReportMillis = 0;

long loopDtMs = 5;//careful changing this - DSP filters below are tuned to this


void setup()
{
    scheduledNextTick = (long)millis() + loopDtMs;
    pinMode(motor1Pin, OUTPUT);
    pinMode(motor2Pin, OUTPUT);
    pinMode(blinkyPin, OUTPUT);
    digitalWrite(motor1Pin, LOW);
    digitalWrite(motor2Pin, LOW);
    digitalWrite(blinkyPin, LOW);
    pinMode(speedoPin, INPUT);
    pinMode(resistoPin, INPUT);
    pinMode(heartPin, INPUT);
    Serial.begin(115200);
}

float heart = 0.0;
void loop()
{
    long now = (long)millis();
    if( scheduledNextTick - now > 0 ) {
        return;
    }

    scheduledNextTick += loopDtMs;

    float speedo = (float)analogRead(speedoPin) / 4096.0;
    float resisto = (float)analogRead(resistoPin) / 4096.0;
    float heart = (float)analogRead(heartPin) / 2048.0 - 1.0;

    ///////////////////////////////////////////////////////////////////////////
    // Heart filtering
    ///////////////////////////////////////////////////////////////////////////
    heartHist[heartIdx] = heart;
    heartIdx = (heartIdx + 1) % HBUF_SZ;

    int heartIdx4 = (heartIdx + HBUF_SZ - 4) % HBUF_SZ;
    int heartIdx7 = (heartIdx + HBUF_SZ - 7) % HBUF_SZ;

    float heart_ds4 = heart * 0.7 - heartHist[heartIdx4] + heartHist[heartIdx7] * 0.3;
    float heart_rct_max = 0.0;
    for(int i = 0; i < HBUF_SZ; i++) {
        if(heartHist[i] > heart_rct_max) {
            heart_rct_max = heartHist[i];
        }
    }

    float heart_pulse_thresh = heart_rct_max * 0.5;

    float heart_pulse = heart_ds4 > heart_pulse_thresh ? 1.0 : 0.0;

    //float heart_pulse_t

    ///////////////////////////////////////////////////////////////////////////
    // BLINKY PIN (200Hz calibration)
    ///////////////////////////////////////////////////////////////////////////
    blinkyPinValue = ~blinkyPinValue;
    digitalWrite(blinkyPin, blinkyPinValue);

    ///////////////////////////////////////////////////////////////////////////
    // RESISTANCE
    ///////////////////////////////////////////////////////////////////////////
    int resistoDir = 999;
    if(resisto < targetResist - targetResistTol) {
        digitalWrite(motor1Pin, HIGH);
        digitalWrite(motor2Pin, LOW);
        resistoDir = 1;
    }
    else if(resisto > targetResist + targetResistTol){
        digitalWrite(motor1Pin, LOW);
        digitalWrite(motor2Pin, HIGH);
        resistoDir = -1;
    }
    else {
        digitalWrite(motor1Pin, LOW);
        digitalWrite(motor2Pin, LOW);
        resistoDir = 0;
    }

    ///////////////////////////////////////////////////////////////////////////
    // REPORTING
    ///////////////////////////////////////////////////////////////////////////
    if(1) {  // raw heart signal diagnostics
        if(now - lastReportMillis > 0) {
            lastReportMillis = now;
            Serial.print(10.0*heart);Serial.print(",");
            Serial.print(10.0*heart_pulse);Serial.print(",");
            Serial.print(10.0*resisto);Serial.print(",");
            Serial.print(10.0*speedo);Serial.println();
        }
    }
}

//#include <WiFi.h>
//
//// WiFi network name and password:
//const char * networkName = "jaysplace2";
//const char * networkPswd = "deadbeef";
//
//// Internet domain to request from:
//const char * hostDomain = "google.com";
//const int hostPort = 80;

//const int BUTTON_PIN = 0;
//const int LED_PIN = 5;

//void setup()
//{
//  // Initilize hardware:
//  Serial.begin(115200);
//  //pinMode(BUTTON_PIN, INPUT);
//  //pinMode(LED_PIN, OUTPUT);
//
//  // Connect to the WiFi network (see function below loop)
//  //connectToWiFi(networkName, networkPswd);
//
//  //digitalWrite(LED_PIN, LOW); // LED off
//  //Serial.print("Press button 0 to connect to ");
//  //Serial.println(hostDomain);
//}
//
//void loop()
//{
//  /*
//  if (digitalRead(BUTTON_PIN) == LOW)
//  { // Check if button has been pressed
//    while (digitalRead(BUTTON_PIN) == LOW)
//      ; // Wait for button to be released
//
//    digitalWrite(LED_PIN, HIGH); // Turn on LED
//    requestURL(hostDomain, hostPort); // Connect to server
//    digitalWrite(LED_PIN, LOW); // Turn off LED
//  }
//  */
//  
//}

//void connectToWiFi(const char * ssid, const char * pwd)
//{
//  int ledState = 0;
//
//  printLine();
//  Serial.println("Connecting to WiFi network: " + String(ssid));
//
//  WiFi.begin(ssid, pwd);
//
//  while (WiFi.status() != WL_CONNECTED) 
//  {
//    // Blink LED while we're connecting:
//    digitalWrite(LED_PIN, ledState);
//    ledState = (ledState + 1) % 2; // Flip ledState
//    delay(500);
//    Serial.print(".");
//  }
//
//  Serial.println();
//  Serial.println("WiFi connected!");
//  Serial.print("IP address: ");
//  Serial.println(WiFi.localIP());
//}
//
//void requestURL(const char * host, uint8_t port)
//{
//  printLine();
//  Serial.println("Connecting to domain: " + String(host));
//
//  // Use WiFiClient class to create TCP connections
//  WiFiClient client;
//  if (!client.connect(host, port))
//  {
//    Serial.println("connection failed");
//    return;
//  }
//  Serial.println("Connected!");
//  printLine();
//
//  // This will send the request to the server
//  client.print((String)"GET / HTTP/1.1\r\n" +
//               "Host: " + String(host) + "\r\n" +
//               "Connection: close\r\n\r\n");
//  unsigned long timeout = millis();
//  while (client.available() == 0) 
//  {
//    if (millis() - timeout > 5000) 
//    {
//      Serial.println(">>> Client Timeout !");
//      client.stop();
//      return;
//    }
//  }
//
//  // Read all the lines of the reply from server and print them to Serial
//  while (client.available()) 
//  {
//    String line = client.readStringUntil('\r');
//    Serial.print(line);
//  }
//
//  Serial.println();
//  Serial.println("closing connection");
//  client.stop();
//}
//
//void printLine()
//{
//  Serial.println();
//  for (int i=0; i<30; i++)
//    Serial.print("-");
//  Serial.println();
//}
