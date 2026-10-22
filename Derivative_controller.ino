//RIGHT_MOTOR
//const int ENB = 5;
//const int IN3 = 7;
//const int IN4 = 6;

//const int C1 = 2;
//const int C2 = 3;

//LEFT_MOTOR

const int ENA = 10;
const int IN1 = 9;
const int IN2 = 8;

const int C1 = 11;
const int C2 = 12;
volatile long encoderCount = 0;

const int pulsesPerRevolution = 1050;

// ---------- CONTROLLER ----------
const float targetSpeed = 50.0;
const float Kd = 0.05;

float previousError = 0.0;
float derivativeOutput = 0.0;

const unsigned long sampleTime = 100;
const unsigned long stepTime = 2000;
const unsigned long testDuration = 50000;

unsigned long startTime;
unsigned long lastSampleTime;

void countPulse() {
  encoderCount++;
}

void setup() {
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  pinMode(C1, INPUT_PULLUP);
  pinMode(C2, INPUT_PULLUP);

  // Fixed forward direction
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, 0);

  attachInterrupt(
    digitalPinToInterrupt(C1),
    countPulse,
    RISING
  );

  Serial.begin(115200);
  Serial.println(
    "Time\tTargetRPM\tActualRPM\tError\tD_Output\tPWM"
  );

  startTime = millis();
  lastSampleTime = startTime;
}

void loop() {
  unsigned long now = millis();
  unsigned long elapsed = now - startTime;

  // Stop the motor when the test ends
  if (elapsed >= testDuration) {
    analogWrite(ENA, 0);

    while (true) {
      // Motor remains stopped
    }
  }

  if (now - lastSampleTime >= sampleTime) {

    float dt = (now - lastSampleTime) / 1000.0;
    lastSampleTime = now;

    // Safely read and reset encoder count
    noInterrupts();
    long pulses = encoderCount;
    encoderCount = 0;
    interrupts();

    // RPM: dt is in seconds
    float actualRPM = (pulses * 60.0) / (pulsesPerRevolution * dt);

    // Step target: 0 RPM, then 50 RPM
    float targetRPM = 0.0;
    if (elapsed >= stepTime)
    {
      targetRPM = targetSpeed;
    }

    float error = targetRPM - actualRPM;

    // Derivative of error
    derivativeOutput = Kd * (error - previousError) / dt;

    previousError = error;

    // Convert D output to a safe forward-only PWM
    int pwm = (int)constrain(derivativeOutput, 0.0, 255.0);

    analogWrite(ENA, pwm);

    // Log response
    Serial.print(elapsed / 1000.0, 2);
    Serial.print('\t');
    Serial.print(targetRPM);
    Serial.print('\t');
    Serial.print(actualRPM);
    Serial.print('\t');
    Serial.print(error);
    Serial.print('\t');
    Serial.print(derivativeOutput);
    Serial.print('\t');
    Serial.println(pwm);
  }
}
