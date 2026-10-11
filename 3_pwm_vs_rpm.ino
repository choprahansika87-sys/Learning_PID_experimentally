// -------- RIGHT MOTOR DRIVER --------
const int ENB = 10;          // PWM pin to control
const int IN3 = 9;           // Motor direction pin 1
const int IN4 = 8;           // Motor direction pin 2

// -------- LEFT MOTOR DRIVER --------
//const int ENB = 5;
//const int IN1 = 6;
//const int IN2 = 7;

// -------- RIGHT ENCODER --------
const int C1 = 12;           // Encoder Channel A
const int C2 = 13;           // Encoder Channel B (not used)

// -------- LEFT ENCODER --------
//const int C1 = 2;
//const int C2 = 3;

// -------- MOTOR --------
const int PPR = 1065;        // Pulses Per Revolution

// -------- GLOBAL VARIABLES --------
volatile long pulseCount = 0;

// Timers
unsigned long sampleTimer = 0;
unsigned long pwmTimer = 0;

// Current PWM
int pwm = 0;

// RPM averaging
float rpmSum = 0;
int sampleCount = 0;


// =====================================================
// SETUP
// =====================================================

void setup()
{
    Serial.begin(115200);

    // -------- MOTOR DRIVER --------
    pinMode(ENB, OUTPUT);
    pinMode(IN3, OUTPUT);
    pinMode(IN4, OUTPUT);

    // -------- ENCODER --------
    pinMode(C1, INPUT_PULLUP);
    pinMode(C2, INPUT_PULLUP);

    // -------- MOTOR DIRECTION --------
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);

    // Start motor stopped
    analogWrite(ENB, pwm);

    // -------- ENCODER INTERRUPT --------
    attachInterrupt(
        digitalPinToInterrupt(C1),
        countPulse,
        RISING
    );

    // -------- TIMERS --------
    sampleTimer = millis();
    pwmTimer = millis();

    // -------- CSV HEADER --------
    Serial.println("PWM,RPM");
}


// =====================================================
// LOOP
// =====================================================

void loop()
{
    unsigned long currentTime = millis();


    // =================================================
    // SAMPLE RPM EVERY 100 ms
    // =================================================

    if (currentTime - sampleTimer >= 100)
    {
        sampleTimer = currentTime;

        // Safely read and reset pulse counter
        noInterrupts();

        long pulses = pulseCount;
        pulseCount = 0;

        interrupts();


        // ---------------------------------------------
        // Convert pulses in 100 ms to RPM
        //
        // RPM = (pulses × 60) / (PPR × 0.1)
        //
        // RPM = (pulses × 600) / PPR
        // ---------------------------------------------

        float rpm = (pulses * 600.0) / PPR;


        // Add RPM to average
        rpmSum += rpm;
        sampleCount++;
    }


    // =================================================
    // CHANGE PWM EVERY 7 SECONDS
    // =================================================

    if (currentTime - pwmTimer >= 7000)
    {
        pwmTimer = currentTime;


        // Calculate average RPM
        float averageRPM = 0;

        if (sampleCount > 0)
        {
            averageRPM = rpmSum / sampleCount;
        }


        // ---------------------------------------------
        // PRINT:
        //
        // PWM,RPM
        //
        // Example:
        // 50,123.45
        // ---------------------------------------------

        Serial.print(pwm);
        Serial.print(",");
        Serial.println(averageRPM, 2);


        // Reset RPM averaging
        rpmSum = 0;
        sampleCount = 0;


        // Increase PWM by 5
        pwm += 5;


        // ---------------------------------------------
        // STOP AFTER PWM > 255
        // ---------------------------------------------

        if (pwm > 255)
        {
            analogWrite(ENB, 0);

            Serial.println("Characterization Complete.");

            while (1);
        }


        // Apply new PWM
        analogWrite(ENB, pwm);
    }
}


// =====================================================
// INTERRUPT SERVICE ROUTINE
// =====================================================

void countPulse()
{
    pulseCount++;
}