// ---------- MOTOR ----------
const int ENA = 10;
const int IN1 = 9;
const int IN2 = 8;

// ---------- ENCODER ----------
const int C1 = 12;
const int C2 = 11;

volatile long encoderCount = 0;

// ---------- CONTROL ----------
float targetRPM = 0;
float Kp = 1.5;

const int pulsesPerRevolution = 1050;

// ---------- TIMING ----------
const unsigned long SAMPLE_TIME = 100;   // ms
const unsigned long STEP_TIME = 2000;    // Target changes after 2 sec
const unsigned long TEST_TIME = 10000;   // Test ends after 10 sec

unsigned long previousTime = 0;
unsigned long startTime = 0;

void setup()
{
    Serial.begin(115200);

    pinMode(ENA, OUTPUT);
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);

    pinMode(C1, INPUT_PULLUP);
    pinMode(C2, INPUT_PULLUP);

    attachInterrupt(
        digitalPinToInterrupt(C1),
        readEncoder,
        RISING
    );

    // Motor forward
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);

    // Start with motor OFF
    analogWrite(ENA, 0);

    startTime = millis();
    previousTime = startTime;

    // CSV header
    Serial.println("Time,Target,RPM,Error,PWM");
}

void loop()
{
    unsigned long currentTime = millis();

    unsigned long elapsed = currentTime - startTime;

    // ------------------------------------
    // STEP INPUT
    // ------------------------------------

    if (elapsed < STEP_TIME)
    {
        targetRPM = 0;
    }
    else if (elapsed < TEST_TIME)
    {
        targetRPM = 100;
    }
    else
    {
        // Stop motor after experiment
        targetRPM = 0;
        analogWrite(ENA, 0);

        return;
    }

    // ------------------------------------
    // CONTROL LOOP
    // ------------------------------------

    if (currentTime - previousTime >= SAMPLE_TIME)
    {
        previousTime = currentTime;

        // Safely read encoder count
        noInterrupts();

        long count = encoderCount;
        encoderCount = 0;

        interrupts();

        // ------------------------------------
        // RPM CALCULATION
        // ------------------------------------

        float rpm =
            (count * 60000.0) /
            (pulsesPerRevolution * SAMPLE_TIME);

        // ------------------------------------
        // ERROR
        // ------------------------------------

        float error = targetRPM - rpm;

        // ------------------------------------
        // P CONTROLLER
        // ------------------------------------

        int pwm = Kp * error;

        // Minimum PWM compensation
        if (pwm > 0)
        {
            pwm += 55;
        }

        // Limit PWM
        pwm = constrain(pwm, 0, 255);

        // ------------------------------------
        // APPLY PWM
        // ------------------------------------

        analogWrite(ENA, pwm);

        // ------------------------------------
        // RESPONSE DATA
        // ------------------------------------

        Serial.print(elapsed);
        Serial.print(",");

        Serial.print(targetRPM);
        Serial.print(",");

        Serial.print(rpm);
        Serial.print(",");

        Serial.print(error);
        Serial.print(",");

        Serial.println(pwm);
    }
}

// ---------- ENCODER INTERRUPT ----------

void readEncoder()
{
    if (digitalRead(C2) == HIGH)
    {
        encoderCount++;
    }
    else
    {
        encoderCount--;
    }
}