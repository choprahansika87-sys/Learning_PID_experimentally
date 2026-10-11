// ============================================================
//            RIGHT  MOTOR PINS
// ============================================================

// ENA is the PWM pin used to control motor speed
const int ENA = 10;

// IN1 and IN2 are used to control the motor direction
const int IN1 = 9;
const int IN2 = 8;


// ============================================================
//             RIGHT ENCODER
// ============================================================

// Pin connected to the encoder output signal
const int encoderPin = 12;
// ============================================================
//            LEFT MOTOR PINS
// ============================================================

// ENA is the PWM pin used to control motor speed
//const int ENA = 5;

// IN1 and IN2 are used to control the motor direction
//const int IN1 = 7;
//const int IN2 = 6;


// ============================================================
//             LEFTENCODER
// ============================================================

// Pin connected to the encoder output signal
//const int encoderPin = 2;

// pulseCount stores the number of encoder pulses detected
// 'volatile' is used because this variable is changed
// inside an interrupt function
volatile long pulseCount = 0;


// ============================================================
//              CONTROL PARAMETERS
// ============================================================

// Desired motor speed in RPM
float targetRPM = 150;

// Proportional gain of the P controller
// Higher Kp = stronger correction when there is an error
float Kp = 0.45;

// Initial PWM value sent to the motor
// PWM range for Arduino is normally 0 to 255
int pwmValue = 0;


// ============================================================
//              MOTOR SPECIFICATIONS
// ============================================================

// Number of encoder pulses produced for one complete
// revolution of the motor shaft
const int pulsesPerRevolution = 1050;

// IMPORTANT:
// Change this value according to your actual encoder.
// For example, if your encoder produces 1000 pulses
// per revolution, use 1000 here.


// ============================================================
//              TIMER
// ============================================================

// Stores the time at which the previous RPM calculation
// was performed
unsigned long previousTime = 0;


void setup()
{
    // Start serial communication at 9600 bits per second
    // This allows us to see RPM, error and PWM in Serial Monitor
    Serial.begin(9600);


    // ========================================================
    //              MOTOR PIN SETUP
    // ========================================================

    // Set ENA as an OUTPUT because Arduino sends PWM
    // to this pin to control motor speed
    pinMode(ENA, OUTPUT);

    // Set IN1 as an OUTPUT for motor direction control
    pinMode(IN1, OUTPUT);

    // Set IN2 as an OUTPUT for motor direction control
    pinMode(IN2, OUTPUT);


    // ========================================================
    //              ENCODER PIN SETUP
    // ========================================================

    // Configure encoder pin as INPUT_PULLUP
    // The internal pull-up resistor keeps the signal HIGH
    // when there is no encoder pulse
    pinMode(encoderPin, INPUT_PULLUP);


    // ========================================================
    //              INTERRUPT SETUP
    // ========================================================

    // Attach an interrupt to the encoder pin
    //
    // digitalPinToInterrupt(encoderPin)
    // converts the encoder pin number into the corresponding
    // interrupt number.
    //
    // countPulse
    // is the function that will run whenever a pulse occurs.
    //
    // RISING
    // means the interrupt occurs when the encoder signal
    // changes from LOW to HIGH.
    attachInterrupt(
        digitalPinToInterrupt(encoderPin),
        countPulse,
        RISING
    );


    // ========================================================
    //              MOTOR DIRECTION
    // ========================================================

    // Set IN1 HIGH and IN2 LOW
    // This makes the motor rotate in the forward direction
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
}


void loop()
{
    // Get the current time in milliseconds
    // millis() returns the time since Arduino started
    unsigned long currentTime = millis();


    // ========================================================
    //              SAMPLE TIME
    // ========================================================

    // Check whether 1000 ms (1 second) has passed
    // since the previous RPM calculation
    //
    // currentTime - previousTime
    // gives the elapsed time
    //
    // 1000 ms = 1 second
    if (currentTime - previousTime >= 1000)
    {
        // Update previousTime so that the next calculation
        // will happen after another 1 second
        previousTime = currentTime;


        // ====================================================
        //              READ ENCODER PULSES
        // ====================================================

        // Temporarily disable interrupts
        // This prevents pulseCount from changing while
        // we are copying its value
        noInterrupts();

        // Copy the number of pulses detected during
        // the last 1-second period into 'pulses'
        long pulses = pulseCount;

        // Reset pulseCount to zero so that we can start
        // counting pulses for the next 1-second period
        pulseCount = 0;

        // Enable interrupts again
        interrupts();


        // ====================================================
        //              CALCULATE RPM
        // ====================================================

        // Calculate motor speed in RPM
        //
        // pulses = number of encoder pulses in 1 second
        //
        // 60 = seconds in one minute
        //
        // pulsesPerRevolution = encoder pulses per revolution
        //
        // RPM = (pulses × 60) / pulsesPerRevolution
        //
        // 600.0 is used because:
        // 60 seconds × 10 = 600
        // The code originally uses a 100 ms formula,
        // but the current sampling period is 1000 ms.
        float rpm = (pulses * 600.0) / pulsesPerRevolution;


        // ====================================================
        //              CALCULATE ERROR
        // ====================================================

        // Error is the difference between desired RPM
        // and actual measured RPM
        //
        // Example:
        // Target RPM = 150
        // Actual RPM = 120
        // Error = 150 - 120 = 30 RPM
        float error = targetRPM - rpm;


        // ====================================================
        //              P CONTROLLER
        // ====================================================

        // Proportional control calculates a correction
        // based on the current speed error
        //
        // Kp × error = required correction
        //
        // The correction is added to the current PWM value
        pwmValue = pwmValue + (Kp * error);


        // ====================================================
        //              LIMIT PWM
        // ====================================================

        // Arduino PWM value must remain between 0 and 255
        //
        // If pwmValue becomes greater than 255,
        // it is limited to 255.
        //
        // If pwmValue becomes less than 0,
        // it is limited to 0.
        pwmValue = constrain(pwmValue, 0, 255);


        // ====================================================
        //              SEND PWM TO MOTOR
        // ====================================================

        // Send the calculated PWM value to ENA
        // This controls the voltage/power supplied to the motor
        //
        // 0   = motor stopped
        // 255 = maximum PWM
        analogWrite(ENA, pwmValue);


        // ====================================================
        //              SERIAL MONITOR OUTPUT
        // ====================================================

        // Print the measured RPM
        Serial.print("RPM: ");
        Serial.print(rpm);


        // Print the speed error
        Serial.print("   Error: ");
        Serial.print(error);


        // Print the current PWM value
        Serial.print("   PWM: ");
        Serial.println(pwmValue);
    }
}


// ============================================================
//              ENCODER INTERRUPT FUNCTION
// ============================================================

// This function is automatically called every time
// the encoder produces a RISING pulse.
//
// Each pulse represents a small amount of motor rotation.
void countPulse()
{
    // Increase the pulse counter by 1
    pulseCount++;
}
