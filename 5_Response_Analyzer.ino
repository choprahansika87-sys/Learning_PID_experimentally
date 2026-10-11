// ---------- RIGHT MOTOR ----------
const int ENA = 10;
const int IN1 = 9;
const int IN2 = 8;
// ---------- LEFT MOTOR ----------
//const int ENA = 5;
//const int IN1 = 7;
//const int IN2 = 6;

// ---------- RIGHT ENCODER ----------
const int C1 = 12;
const int C2 = 11;
// ---------- LEFT ENCODER ----------
//const int C1 = 2;
//const int C2 = 3;

volatile long encoderCount = 0;

// ---------- CONTROL ----------
float targetRPM = 0;

float Kp = 40.5;

const int pulsesPerRevolution = 1050;

// ---------- TIMING ----------
const unsigned long SAMPLE_TIME = 100;   // ms
const unsigned long STEP_TIME = 2000;    // 2 seconds
const unsigned long TEST_TIME = 10000;   // 10 seconds

unsigned long previousTime = 0;
unsigned long startTime = 0;

void setup()
{
    delay(1000);
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

    // Forward direction of LEFT MOTOR
    //digitalWrite(IN1, LOW);
    //digitalWrite(IN2, HIGH);
    // Forward direction of RIGHT MOTOR
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);

    // Start with motor OFF
    analogWrite(ENA, 0);

    startTime = millis();
    previousTime = startTime;

    // CSV Header
    Serial.println("Time,Target,RPM,Error,PWM");
}

void loop()
{
    unsigned long currentTime = millis();

    // ------------------------------------------------
    // Determine target RPM
    // ------------------------------------------------

    unsigned long elapsed = currentTime - startTime;

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
        // Test finished
        targetRPM = 0;
        analogWrite(ENA, 0);

        return;
    }

    // ------------------------------------------------
    // Run control loop every 100 ms
    // ------------------------------------------------

    if (currentTime - previousTime >= SAMPLE_TIME)
    {
        previousTime = currentTime;

        // Safely copy encoder count
        noInterrupts();

        long count = encoderCount;
        encoderCount = 0;

        interrupts();

        // ------------------------------------------------
        // Calculate RPM
        // ------------------------------------------------

        float rpm =
            (count * 60000.0) /
            (pulsesPerRevolution * SAMPLE_TIME);

        // ------------------------------------------------
        // Calculate error
        // ------------------------------------------------

        float error = targetRPM - rpm;

        // ------------------------------------------------
        // P Controller
        // ------------------------------------------------

        int pwm = Kp * error;

        // Minimum PWM compensation
        if (pwm > 0)
        {
            pwm += 37;
        }

        // Limit PWM
        pwm = constrain(pwm, 0, 255);

        // ------------------------------------------------
        // Apply PWM
        // ------------------------------------------------

        analogWrite(ENA, pwm);

        // ------------------------------------------------
        // Print data
        // ------------------------------------------------

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