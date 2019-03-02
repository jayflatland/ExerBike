int motor1Pin = 23;

void setup()
{
    pinMode(motor1Pin, OUTPUT);
    Serial.begin(115200);
}

void loop()
{
    digitalWrite(motor1Pin, HIGH);
    delayMicroseconds(10);
    digitalWrite(motor1Pin, LOW);
    delay(100);
}
