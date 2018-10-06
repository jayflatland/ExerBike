long scheduledNextTick;

int ledPin = 5;
int speedoPin = 36;
int resistoPin = 39;
int heartPin = 34;
int motor1Pin = 23;
int motor2Pin = 22;

float heartHist[64];
int heartIdx = 0;
// float heartVAR = 0.0;

float heartFiltEnveloped = 0.0;
float heart2Hist[64];
int heart2Idx = 0;
float heart2Xave = 0.0;

float targetResist = 0.4;
float targetResistTol = 0.04;

long lastSpeedoTrippedMillis = 0;
float pedalPeriod = 0.0;
float pedalRPM = 0.0;

// int heartIsBeating = 0;
// int ticksSinceLastBeat = 0;
// float heartBPM = 0.0;
// float filtHeartRateBPM = 0.0;

long lastHbMillis = 0;
int hbRateIsLocked = 0;
long hbLockedMillisDelta = 0;
float heartPeriod = 0.0;
float heartBPM = 0.0;


long lastReportMillis = 0;

long loopDtMs = 5;//careful changing this - DSP filters below are tuned to this

/*

FIR filter designed with
http://t-filter.appspot.com

sampling frequency: 200 Hz

* 0 Hz - 15 Hz
  gain = 0
  desired attenuation = -40 dB
  actual attenuation = -43.02125963160662 dB

* 20 Hz - 25 Hz
  gain = 1
  desired ripple = 5 dB
  actual ripple = 2.920816963599269 dB

* 30 Hz - 100 Hz
  gain = 0
  desired attenuation = -40 dB
  actual attenuation = -43.02125963160662 dB

*/

#define FILTER_TAP_NUM 53

static float filter_taps[FILTER_TAP_NUM] = {
    0.004239109487818515,
    0.00041959938934507755,
    -0.0026423805193273742,
    -0.007028887140783344,
    -0.010165835174624148,
    -0.00859915891207442,
    -0.00041026507903428074,
    0.012235346517659503,
    0.02275581668995705,
    0.023265412559111374,
    0.009805937540421514,
    -0.013703470367715067,
    -0.03590683114655655,
    -0.0434450769955968,
    -0.028711156019341717,
    0.004523738539162783,
    0.04131000858153872,
    0.062255366637195865,
    0.0537713658905211,
    0.016449153334507267,
    -0.03343275203589259,
    -0.07105072374285279,
    -0.07580458054536178,
    -0.042894864303808185,
    0.01288431171779728,
    0.06404248882984026,
    0.08461450122416873,
    0.06404248882984026,
    0.01288431171779728,
    -0.042894864303808185,
    -0.07580458054536178,
    -0.07105072374285279,
    -0.03343275203589259,
    0.016449153334507267,
    0.0537713658905211,
    0.062255366637195865,
    0.04131000858153872,
    0.004523738539162783,
    -0.028711156019341717,
    -0.0434450769955968,
    -0.03590683114655655,
    -0.013703470367715067,
    0.009805937540421514,
    0.023265412559111374,
    0.02275581668995705,
    0.012235346517659503,
    -0.00041026507903428074,
    -0.00859915891207442,
    -0.010165835174624148,
    -0.007028887140783344,
    -0.0026423805193273742,
    0.00041959938934507755,
    0.004239109487818515
};


/*

FIR filter designed with
http://t-filter.appspot.com

sampling frequency: 200 Hz

* 0 Hz - 5 Hz
  gain = 1
  desired ripple = 5 dB
  actual ripple = 3.7428345110667873 dB

* 10 Hz - 100 Hz
  gain = 0
  desired attenuation = -40 dB
  actual attenuation = -40.91898560530998 dB

*/

#define FILTER_TAP_NUM2 53

static double filter_taps2[FILTER_TAP_NUM2] = {
  -0.005946901269245122,
  -0.0032211059347523195,
  -0.0038440699902768383,
  -0.004335185893314178,
  -0.004618082667223961,
  -0.004610544806595846,
  -0.004229573289430787,
  -0.0033917453968697483,
  -0.002028051041620645,
  -0.0000779737869300087,
  0.0024926916286312248,
  0.00569982765877049,
  0.009534812572960348,
  0.013961833701309628,
  0.018917781140524174,
  0.024309521855601125,
  0.030016610101736976,
  0.03589781173597167,
  0.04180040186066688,
  0.047555344979786536,
  0.05298415762863876,
  0.05792015092713824,
  0.06221396733171683,
  0.06570516590449127,
  0.06829411291109369,
  0.069881349050518,
  0.07041715978547827,
  0.069881349050518,
  0.06829411291109369,
  0.06570516590449127,
  0.06221396733171683,
  0.05792015092713824,
  0.05298415762863876,
  0.047555344979786536,
  0.04180040186066688,
  0.03589781173597167,
  0.030016610101736976,
  0.024309521855601125,
  0.018917781140524174,
  0.013961833701309628,
  0.009534812572960348,
  0.00569982765877049,
  0.0024926916286312248,
  -0.0000779737869300087,
  -0.002028051041620645,
  -0.0033917453968697483,
  -0.004229573289430787,
  -0.004610544806595846,
  -0.004618082667223961,
  -0.004335185893314178,
  -0.0038440699902768383,
  -0.0032211059347523195,
  -0.005946901269245122
};

void setup()
{
    scheduledNextTick = (long)millis() + loopDtMs;
    pinMode(motor1Pin, OUTPUT);
    pinMode(motor2Pin, OUTPUT);
    digitalWrite(motor1Pin, LOW);
    digitalWrite(motor2Pin, LOW);
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
    // SPEED
    ///////////////////////////////////////////////////////////////////////////
    long millisSinceLastSpeedoTripped = now - lastSpeedoTrippedMillis;
    if(speedo < 0.4 && millisSinceLastSpeedoTripped > 500) {
        lastSpeedoTrippedMillis = now;
    }

    if(millisSinceLastSpeedoTripped > 3000) {
        millisSinceLastSpeedoTripped = 3000;
    }

    if((float)millisSinceLastSpeedoTripped > pedalPeriod) {
        pedalPeriod = (float)millisSinceLastSpeedoTripped;
    }
    pedalPeriod = pedalPeriod * 0.9995;
    pedalRPM = 60000.0 / pedalPeriod;

    ///////////////////////////////////////////////////////////////////////////
    // HEART
    ///////////////////////////////////////////////////////////////////////////
    //store to buffer
    heartHist[heartIdx] = heart;
    heartIdx = (heartIdx + 1) % 64;

    //apply FIR filter
    float heartFilt = 0.0;
    for(int i = 0; i < FILTER_TAP_NUM; i++ ) {
        heartFilt += heartHist[(heartIdx - i + 64)%64] * filter_taps[i];
    }

    if( heartFilt > heartFiltEnveloped) {
        heartFiltEnveloped = heartFilt;
    }
    heartFiltEnveloped *= 0.95;

    heart2Hist[heart2Idx] = heartFiltEnveloped;
    heart2Idx = (heart2Idx + 1) % 64;

    //apply FIR filter2
    float heart2Filt = 0.0;
    for(int i = 0; i < FILTER_TAP_NUM2; i++ ) {
        heart2Filt += heart2Hist[(heart2Idx - i + 64)%64] * filter_taps2[i];
    }

    //measure variance
    //heartVAR = heartVAR * 0.999 + heartFilt * 0.001;
    //float heartSTD = sqrt(heartVAR);
    heart2Xave = heart2Xave * 0.999 + heart2Filt * 0.001;

    //determine if HB signal is active
    int hbActive = heart2Filt > heart2Xave * 2.0;

    long millisSinceLastHb = now - lastHbMillis;
    if(hbActive && millisSinceLastHb > 400) {
        lastHbMillis = now;
    }

    if(millisSinceLastHb > 3000) {
        millisSinceLastHb = 3000;
    }

    if((float)millisSinceLastHb > heartPeriod) {
        heartPeriod = (float)millisSinceLastHb;
    }
    heartPeriod = heartPeriod * 0.9995;
    heartBPM = 60000.0 / heartPeriod;

    // int validHb = 0;
    // if(hbActive) {
    //     // Serial.println("heartbeat!");
    //     if(hbRateIsLocked) {
    //         if(millisSinceLastHb > hbLockedMillisDelta - 30 && millisSinceLastHb < hbLockedMillisDelta + 30)
    //             validHb = 1;
    //     } else {
    //         if(millisSinceLastHb > 300) {//} && millisSinceLastHb < 1500) {
    //             validHb = 1;
    //         }
    //     }
    // }

    // if(validHb) {
    //     Serial.println("valid heartbeat!");
    //     Serial.print("millisSinceLastHb=");Serial.println(millisSinceLastHb);
    //     lastHbMillis = now;
    //     hbLockedMillisDelta = millisSinceLastHb;
    //     if(millisSinceLastHb < 1500) {
    //         Serial.println("hb locked!");
    //         hbRateIsLocked = 1;
    //     }
    // }

    // if(hbRateIsLocked && millisSinceLastHb > 1500) {
    //     Serial.println("hb unlocked!");
    //     hbRateIsLocked = 0;
    // }

    //determine if HB is likely accurate
    // * if not locked, time since last is between 300 and 1500ms
    // * if locked, time since last is within 10ms of previous interval

    // int prevHeartIsBeating = heartIsBeating;
    // if(!heartIsBeating && abs(heartFilt) > heartSTD * 2.0 && ticksSinceLastBeat > 70) {
    //     heartIsBeating = 1;
    //     float newHeartRateBPM = 60.0 / (ticksSinceLastBeat * (float)loopDtMs * 0.001);
    //     if(newHeartRateBPM > 40.0 && newHeartRateBPM < 190.0) {
    //         heartBPM = newHeartRateBPM;
    //     }
    //     ticksSinceLastBeat = 0;
    // }
    // else {
    //     heartIsBeating = 0;
    // }
    // ticksSinceLastBeat++;
    // filtHeartRateBPM = filtHeartRateBPM * 0.9995 + heartBPM * 0.0005;

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
    if(0) {  // raw heart signal diagnostics
        if(now - lastReportMillis > 0) {
            lastReportMillis = now;
            //Serial.print(10.0*heart);Serial.print(",");
            Serial.print(1000.0*heartFilt);Serial.print(",");
            Serial.print(1000.0*heartFiltEnveloped);Serial.print(",");
            Serial.print(1000.0*heart2Xave);Serial.print(",");
            Serial.print(1000.0*heart2Filt);Serial.println();
        }
    }

    if(1) {  // best final reporting
        if(now - lastReportMillis > 100) {
            lastReportMillis = now;
            Serial.print(pedalRPM);Serial.print(",");
            Serial.print(heartBPM);Serial.println();
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
