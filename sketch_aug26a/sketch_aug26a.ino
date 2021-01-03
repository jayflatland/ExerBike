#include <WiFi.h>
#include <WiFiUdp.h>

const char * networkName = "To The Oasis";
const char * networkPswd = "deadbeef";

WiFiUDP Udp;

long scheduledNextTick;

int resetPin = 0;
int speedoPin = 36;
int resistoPin = 39;
int heartPin = 34;
int motor1Pin = 23;
int motor2Pin = 22;
int resistKnobPin = 35;

#define HBUF_SZ 256
#define GME 0.2 //gross mechanical efficiency

#define CAL(joules) ((joules) / 4.184 / GME)
#define KCAL(joules) ((joules) / 4184.0 / GME)

float heartHist[HBUF_SZ];
int heartIdx = 0;

int blinkyPinValue = 0;

float targetResist = 0.4;
float targetResistTol = 0.04;

float heartBPM = 0.0;

float lastSpeedo = 0.0;

long lastReportMillis = 0;
long lastPedalCount = -1;

long loopDtMs = 5;//careful changing this - DSP filters below are tuned to this

long pedalCount = 0;
long hbCount = 0;
long lastPedalCountedTime = -1;


void connectToWiFi(const char * ssid, const char * pwd)
{
    //connect to wifi
    Serial.println("Connecting to WiFi network: " + String(ssid));
    WiFi.begin(ssid, pwd);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println();
    Serial.println("WiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}



void setup()
{
    scheduledNextTick = (long)millis() + loopDtMs;
    pinMode(motor1Pin, OUTPUT);
    pinMode(motor2Pin, OUTPUT);
    pinMode(resistKnobPin, INPUT);
    digitalWrite(motor1Pin, LOW);
    digitalWrite(motor2Pin, LOW);
    pinMode(speedoPin, INPUT);
    pinMode(resistoPin, INPUT);
    pinMode(heartPin, INPUT);
    pinMode(resetPin, INPUT);
    Serial.begin(115200);
    connectToWiFi(networkName, networkPswd);
}

float heart = 0.0;
long last_hb_millis = 0;
float hb_bpm = 0.0;
float last_heart_pulse = 0.0;
float totalWork = 0.0;
float power = 0.0;

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
    float resistKnob = (float)analogRead(resistKnobPin) / 4096.0;
    int resetButton = digitalRead(resetPin);

    if(resetButton == 0) {
        pedalCount = 0;
        hbCount = 0;
    }

    float timeForThisPedal = (now - lastPedalCountedTime) * 0.001;
    if(lastSpeedo > 0.7 && speedo < 0.3) {
        pedalCount++;
        if(timeForThisPedal < 10.0) {
            float pedal_rpm = 60.0 / timeForThisPedal;
            float pedal_rate = pedal_rpm / 60.0 * PI * 2.0;
            float resist_torque_per_vel = 1.8 + 9.0 * resisto;
            power = pedal_rate * pedal_rate * resist_torque_per_vel;
            float work = power * timeForThisPedal;
            totalWork += work;
        }

        lastPedalCountedTime = now;
    }
    lastSpeedo = speedo;

    //if idle for 10 minutes, restart
    if(timeForThisPedal > 60.0) {
        totalWork = 0.0;
        pedalCount = 0.0;
    }

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
    float since_last_hb = (float)(now - last_hb_millis) * 0.001;
    if(heart_pulse > 0.6 && last_heart_pulse < 0.4) {
        if(since_last_hb > 0.20) {
            hbCount++;
            hb_bpm = 60.0 / since_last_hb;
        }
        last_hb_millis = now;
    }
    last_heart_pulse = heart_pulse;

    //float heart_pulse_t
    targetResist = resistKnob;

    ///////////////////////////////////////////////////////////////////////////
    // RESISTANCE
    ///////////////////////////////////////////////////////////////////////////
    int resistoDir = 999;
    if(resisto < targetResist - targetResistTol) {
        // Serial.println("Harder... "
        //  + String(resistKnob) + ", "
        //  + String(targetResist) + ", "
        //  + String(resisto));
        digitalWrite(motor1Pin, HIGH);
        digitalWrite(motor2Pin, LOW);
        resistoDir = 1;
    }
    else if(resisto > targetResist + targetResistTol){
        // Serial.println("Easier..."
        //  + String(resistKnob) + ", "
        //  + String(targetResist) + ", "
        //  + String(resisto));
        digitalWrite(motor1Pin, LOW);
        digitalWrite(motor2Pin, HIGH);
        resistoDir = -1;
    }
    else {
        // Serial.println("Hold..."
        //  + String(resistKnob) + ", "
        //  + String(targetResist) + ", "
        //  + String(resisto));
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
            Serial.println(
                //String(10.0 * heart)
                String(10.0 * heart) + "," +
                String(10.0 * heart_pulse)
                // String(10.0 * since_last_hb) + "," + 
                // String(0.1 * hb_bpm)
            );
        }
    }

    else {
        //if(now - lastReportMillis > 1000) {

        // d['pedal'] = d['pedal_cnt'].cumsum()
        // d['pedal_rpm'] = (d['pedal'] - d['pedal'].rolling(wndsz, min_periods=1).min()) * 60.0 / wndsz
        // d['pedal_rate'] = d['pedal_rpm'] / 60.0 * math.pi * 2.0
        // d['resist_torque_per_vel'] = 1.8 + 9.0 * d['resist_pct'].rolling(wndsz, min_periods=1).mean()
        // d['power'] = d['pedal_rate'] * d['pedal_rate'] * d['resist_torque_per_vel']
        // d['work'] = (d['power'] * (d['t'] - d['t'].shift(1))).cumsum()

        if(pedalCount != lastPedalCount || now - lastReportMillis > 10000) {
            lastReportMillis = now;
            lastPedalCount = pedalCount;

            String msg = String(pedalCount) + "," +
                         String(resisto) + "," +
                         String(targetResist) + "," + 
                         String(power) + "," + 
                         String(totalWork);
            Serial.println(msg);
            msg = msg + "\n";
            //Udp.beginPacket("10.1.10.255", 10245);
            Udp.beginPacket("10.1.10.12", 10245);
            Udp.write((const uint8_t*)msg.c_str(), msg.length());
            Udp.endPacket();

            // pedalCount = 0;
            hbCount = 0;
        }
    }

}





// /*********
//   Rui Santos
//   Complete project details at https://RandomNerdTutorials.com/esp32-websocket-server-arduino/
//   The above copyright notice and this permission notice shall be included in all
//   copies or substantial portions of the Software.
// *********/

// // Import required libraries
// #include <WiFi.h>
// #include <AsyncTCP.h>
// #include <ESPAsyncWebServer.h>

// // Replace with your network credentials
// const char* ssid = "To The Oasis";
// const char* password = "deadbeef";

// bool ledState = 0;
// const int ledPin = 2;

// // Create AsyncWebServer object on port 80
// AsyncWebServer server(80);
// AsyncWebSocket ws("/ws");

// const char index_html[] PROGMEM = R"rawliteral(
// <!DOCTYPE HTML><html>
// <head>
//   <title>ESP Web Server</title>
//   <meta name="viewport" content="width=device-width, initial-scale=1">
//   <link rel="icon" href="data:,">
//   <style>
//   html {
//     font-family: Arial, Helvetica, sans-serif;
//     text-align: center;
//   }
//   h1 {
//     font-size: 1.8rem;
//     color: white;
//   }
//   h2{
//     font-size: 1.5rem;
//     font-weight: bold;
//     color: #143642;
//   }
//   .topnav {
//     overflow: hidden;
//     background-color: #143642;
//   }
//   body {
//     margin: 0;
//   }
//   .content {
//     padding: 30px;
//     max-width: 600px;
//     margin: 0 auto;
//   }
//   .card {
//     background-color: #F8F7F9;;
//     box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5);
//     padding-top:10px;
//     padding-bottom:20px;
//   }
//   .button {
//     padding: 15px 50px;
//     font-size: 24px;
//     text-align: center;
//     outline: none;
//     color: #fff;
//     background-color: #0f8b8d;
//     border: none;
//     border-radius: 5px;
//     -webkit-touch-callout: none;
//     -webkit-user-select: none;
//     -khtml-user-select: none;
//     -moz-user-select: none;
//     -ms-user-select: none;
//     user-select: none;
//     -webkit-tap-highlight-color: rgba(0,0,0,0);
//    }
//    /*.button:hover {background-color: #0f8b8d}*/
//    .button:active {
//      background-color: #0f8b8d;
//      box-shadow: 2 2px #CDCDCD;
//      transform: translateY(2px);
//    }
//    .state {
//      font-size: 1.5rem;
//      color:#8c8c8c;
//      font-weight: bold;
//    }
//   </style>
// <title>ESP Web Server</title>
// <meta name="viewport" content="width=device-width, initial-scale=1">
// <link rel="icon" href="data:,">
// </head>
// <body>
//   <div class="topnav">
//     <h1>ESP WebSocket Server</h1>
//   </div>
//   <div class="content">
//     <div class="card">
//       <h2>Output - GPIO 2</h2>
//       <p class="state">state: <span id="state">%STATE%</span></p>
//       <p><button id="button" class="button">Toggle</button></p>
//     </div>
//   </div>
// <script>
//   var gateway = `ws://${window.location.hostname}/ws`;
//   var websocket;
//   window.addEventListener('load', onLoad);
//   function initWebSocket() {
//     console.log('Trying to open a WebSocket connection...');
//     websocket = new WebSocket(gateway);
//     websocket.onopen    = onOpen;
//     websocket.onclose   = onClose;
//     websocket.onmessage = onMessage; // <-- add this line
//   }
//   function onOpen(event) {
//     console.log('Connection opened');
//   }
//   function onClose(event) {
//     console.log('Connection closed');
//     setTimeout(initWebSocket, 2000);
//   }
//   function onMessage(event) {
//     var state;
//     if (event.data == "1"){
//       state = "ON";
//     }
//     else{
//       state = "OFF";
//     }
//     document.getElementById('state').innerHTML = state;
//   }
//   function onLoad(event) {
//     initWebSocket();
//     initButton();
//   }
//   function initButton() {
//     document.getElementById('button').addEventListener('click', toggle);
//   }
//   function toggle(){
//     websocket.send('toggle');
//   }
// </script>
// </body>
// </html>
// )rawliteral";

// void notifyClients() {
//   ws.textAll(String(ledState));
// }

// void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
//   AwsFrameInfo *info = (AwsFrameInfo*)arg;
//   if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
//     data[len] = 0;
//     if (strcmp((char*)data, "toggle") == 0) {
//       ledState = !ledState;
//       notifyClients();
//     }
//   }
// }

// void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
//              void *arg, uint8_t *data, size_t len) {
//   switch (type) {
//     case WS_EVT_CONNECT:
//       Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
//       break;
//     case WS_EVT_DISCONNECT:
//       Serial.printf("WebSocket client #%u disconnected\n", client->id());
//       break;
//     case WS_EVT_DATA:
//       handleWebSocketMessage(arg, data, len);
//       break;
//     case WS_EVT_PONG:
//     case WS_EVT_ERROR:
//       break;
//   }
// }

// void initWebSocket() {
//   ws.onEvent(onEvent);
//   server.addHandler(&ws);
// }

// String processor(const String& var){
//   Serial.println(var);
//   if(var == "STATE"){
//     if (ledState){
//       return "ON";
//     }
//     else{
//       return "OFF";
//     }
//   }
// }

// void setup(){
//   // Serial port for debugging purposes
//   Serial.begin(115200);

//   pinMode(ledPin, OUTPUT);
//   digitalWrite(ledPin, LOW);
  
//   // Connect to Wi-Fi
//   WiFi.begin(ssid, password);
//   while (WiFi.status() != WL_CONNECTED) {
//     delay(1000);
//     Serial.println("Connecting to WiFi..");
//   }

//   // Print ESP Local IP Address
//   Serial.println(WiFi.localIP());

//   initWebSocket();

//   // Route for root / web page
//   server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
//     request->send_P(200, "text/html", index_html, processor);
//   });

//   // Start server
//   server.begin();
// }

// void loop() {
//   ws.cleanupClients();
//   digitalWrite(ledPin, ledState);
// }