long scheduledNextTick;

int speedoPin = 36;
int resistoPin = 39;
int heartPin = 34;
int motor1Pin = 23;
int motor2Pin = 22;
int resistKnobPin = 35;

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
    pinMode(resistKnobPin, INPUT);
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
    float resistKnob = (float)analogRead(resistKnobPin) / 4096.0;

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
    if(1) {  // raw heart signal diagnostics
        if(now - lastReportMillis > 0) {
            lastReportMillis = now;
            Serial.println(
                String(10.0*heart) + "," +
                String(10.0*heart_pulse) + "," +
                String(10.0*resisto) + "," +
                String(10.0*speedo));
        }
    }
}
