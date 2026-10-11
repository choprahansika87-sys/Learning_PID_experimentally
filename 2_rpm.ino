// rpm =126.7
// -------- RIGHT MOTOR DRIVER --------
const int ENB = 10;    // PWM
const int IN3 = 9;    // Direction 1
const int IN4 = 8;    // Direction 2

// -------- RIGHT ENCODER --------
const int C1 = 12;        // Encoder Channel A (interrupt pin)
const int C2 = 11;           // Encoder Channel B
//rpm =127.8
// -------- LEFT MOTOR DRIVER --------
//const int ENB = 5;    // PWM
//const int IN3 = 7;    // Direction 1
//const int IN4 = 6;    // Direction 2

// -------- LEFT ENCODER --------
//const int C1 = 2;         // Encoder Channel A (interrupt pin)
//const int C2 = 3;        // Encoder Channel B
// -------- MOTOR --------
const int PPR = 1065;  // Encoder Channel A (interrupt pin)

// -------- USER SETTINGS --------

const int PWM_VALUE = 200;   // Motor will run at PWM value 200
const int SAMPLE_TIME = 1000; // Measure RPM every 1000 ms (1 second)

// ======================================
// GLOBAL VARIABLES
// ======================================

volatile long pulseCount = 0;      // Stores encoder pulses counted by interrupt
unsigned long previousTime = 0;    // Stores previous sampling time

// ======================================
// SETUP FUNCTION
// Runs once when Arduino starts
// ======================================

void setup()
{
    Serial.begin(115200);          // Start serial communication

    pinMode(ENB, OUTPUT);          // PWM pin as output
    pinMode(IN3, OUTPUT);          // Direction pin as output
    pinMode(IN4, OUTPUT);          // Direction pin as output

    pinMode(C1, INPUT_PULLUP);     // Encoder Channel A as input
    pinMode(C2, INPUT_PULLUP);     // Encoder Channel B as input

    // Set motor to rotate in forward direction
    digitalWrite(IN3, HIGH);       // IN3 HIGH
    digitalWrite(IN4, LOW);        // IN4 LOW

    analogWrite(ENB, PWM_VALUE);   // Apply PWM to motor

    // Generate interrupt on every rising edge of Channel A
    attachInterrupt(
        digitalPinToInterrupt(C1), // Convert pin 2 into interrupt number
        countPulse,                // Function to execute
        RISING                     // Trigger on LOW → HIGH transition
    );

    Serial.println("Time(ms)\tRPM"); // Print column headings
}

// ======================================
// LOOP FUNCTION
// Executes continuously
// ======================================

void loop()
{
    unsigned long currentTime = millis(); // Current running time

    // Check if sample time has elapsed
    if (currentTime - previousTime >= SAMPLE_TIME)
    {
        previousTime = currentTime;       // Reset timer

        // Disable interrupts while copying pulse value
        noInterrupts();

        long pulses = pulseCount;         // Copy pulse count

        pulseCount = 0;                  // Reset for next measurement

        interrupts();                   // Enable interrupts again

        // RPM Formula
        // RPM = (Pulses × 60000) / (PPR × Sample Time)
        float rpm = (pulses * 60000.0) / (PPR * SAMPLE_TIME);

        // Print current time
        Serial.print(currentTime);

        Serial.print("\t");             // Print tab space

        // Print calculated RPM
        Serial.println(rpm);
    }
}

// ======================================
// INTERRUPT SERVICE ROUTINE (ISR)
// Runs automatically on every encoder pulse
// ======================================

void countPulse()
{
    pulseCount++;                      // Increase pulse count by 1
}