#include <WiFi.h>
#include <WiFiUdp.h>

const char * networkName = "To The Oasis";
const char * networkPswd = "deadbeef";

WiFiUDP Udp;

long scheduledNextTick;

// int resetPin = 0;
int speedoPin = 36;
int resistoPin = 39;
// int heartPin = 34;
int motor1Pin = 23;
int motor2Pin = 22;
int resistKnobPin = 35;

// #define HBUF_SZ 256
#define GME 0.2 //gross mechanical efficiency
#define CAL(joules) ((joules) / 4.184 / GME)
#define KCAL(joules) ((joules) / 4184.0 / GME)

// float heartHist[HBUF_SZ];
// int heartIdx = 0;

float targetResist = 0.4;
float targetResistTol = 0.04;

// float heartBPM = 0.0;

float resisto = 0.0;
float lastSpeedo = 0.0;

// long lastReportMillis = 0;
// long lastPedalCount = -1;

long loopDtMs = 5;//careful changing this - DSP filters below are tuned to this

// long pedalCount = 0;
// long hbCount = 0;
long lastPedalCountedTime = -1;
long now; //current time

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
    // pinMode(heartPin, INPUT);
    // pinMode(resetPin, INPUT);
    Serial.begin(115200);
    connectToWiFi(networkName, networkPswd);
}

// float heart = 0.0;
// long last_hb_millis = 0;
// float hb_bpm = 0.0;
// float last_heart_pulse = 0.0;

void handle_pedal_increment() {
    float power = 0.0;
    float work = 0.0;

    float timeForThisPedal = (now - lastPedalCountedTime) * 0.001;
    lastPedalCountedTime = now;

    // pedalCount++;
    if(timeForThisPedal < 10.0) {
        float pedal_rpm = 60.0 / timeForThisPedal;
        float pedal_rate = pedal_rpm / 60.0 * PI * 2.0;
        float resist_torque_per_vel = 1.8 + 9.0 * resisto;
        power = pedal_rate * pedal_rate * resist_torque_per_vel;
        work = power * timeForThisPedal;
    }

    // lastReportMillis = now;

    String msg = String("1,") +
                 String(resisto) + "," +
                 String(targetResist) + "," + 
                 String(power) + "," + 
                 String(KCAL(work));
    Serial.println(msg);
    msg = msg + "\n";

    Udp.beginPacket("10.1.10.12", 10245); Udp.write((const uint8_t*)msg.c_str(), msg.length()); Udp.endPacket();
    // Udp.beginPacket("10.1.10.12", 10245); Udp.write((const uint8_t*)msg.c_str(), msg.length()); Udp.endPacket();
}


void loop()
{
    now = (long)millis();
    if( scheduledNextTick - now > 0 ) {
        return;
    }

    scheduledNextTick += loopDtMs;

    float speedo = (float)analogRead(speedoPin) / 4096.0;
    resisto = (float)analogRead(resistoPin) / 4096.0;
    // float heart = (float)analogRead(heartPin) / 2048.0 - 1.0;
    float resistKnob = (float)analogRead(resistKnobPin) / 4096.0;
    // int resetButton = digitalRead(resetPin);

    // if(resetButton == 0) {
    //     pedalCount = 0;
    //     // hbCount = 0;
    // }

    if(lastSpeedo > 0.7 && speedo < 0.3) {
        handle_pedal_increment();
    }
    lastSpeedo = speedo;

    ///////////////////////////////////////////////////////////////////////////
    // Heart filtering
    ///////////////////////////////////////////////////////////////////////////
    // heartHist[heartIdx] = heart;
    // heartIdx = (heartIdx + 1) % HBUF_SZ;

    // int heartIdx4 = (heartIdx + HBUF_SZ - 4) % HBUF_SZ;
    // int heartIdx7 = (heartIdx + HBUF_SZ - 7) % HBUF_SZ;

    // float heart_ds4 = heart * 0.7 - heartHist[heartIdx4] + heartHist[heartIdx7] * 0.3;
    // float heart_rct_max = 0.0;
    // for(int i = 0; i < HBUF_SZ; i++) {
    //     if(heartHist[i] > heart_rct_max) {
    //         heart_rct_max = heartHist[i];
    //     }
    // }

    // float heart_pulse_thresh = heart_rct_max * 0.5;

    // float heart_pulse = heart_ds4 > heart_pulse_thresh ? 1.0 : 0.0;
    // float since_last_hb = (float)(now - last_hb_millis) * 0.001;
    // if(heart_pulse > 0.6 && last_heart_pulse < 0.4) {
    //     if(since_last_hb > 0.20) {
    //         hbCount++;
    //         hb_bpm = 60.0 / since_last_hb;
    //     }
    //     last_hb_millis = now;
    // }
    // last_heart_pulse = heart_pulse;
    //float heart_pulse_t


    ///////////////////////////////////////////////////////////////////////////
    // RESISTANCE
    ///////////////////////////////////////////////////////////////////////////
    targetResist = resistKnob;
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
    //if(now - lastReportMillis > 1000) {

    // d['pedal'] = d['pedal_cnt'].cumsum()
    // d['pedal_rpm'] = (d['pedal'] - d['pedal'].rolling(wndsz, min_periods=1).min()) * 60.0 / wndsz
    // d['pedal_rate'] = d['pedal_rpm'] / 60.0 * math.pi * 2.0
    // d['resist_torque_per_vel'] = 1.8 + 9.0 * d['resist_pct'].rolling(wndsz, min_periods=1).mean()
    // d['power'] = d['pedal_rate'] * d['pedal_rate'] * d['resist_torque_per_vel']
    // d['work'] = (d['power'] * (d['t'] - d['t'].shift(1))).cumsum()

    // if(pedalCount != lastPedalCount || now - lastReportMillis > 10000) {
    //     lastReportMillis = now;
    //     lastPedalCount = pedalCount;

    //     String msg = String(pedalCount) + "," +
    //                  String(resisto) + "," +
    //                  String(targetResist) + "," + 
    //                  String(power) + "," + 
    //                  String(totalWork);
    //     Serial.println(msg);
    //     msg = msg + "\n";
    //     //Udp.beginPacket("10.1.10.255", 10245);
    //     Udp.beginPacket("10.1.10.12", 10245);
    //     Udp.write((const uint8_t*)msg.c_str(), msg.length());
    //     Udp.endPacket();

    //     // pedalCount = 0;
    //     // hbCount = 0;
    // }
}

