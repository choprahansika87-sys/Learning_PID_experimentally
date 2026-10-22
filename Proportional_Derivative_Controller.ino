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

const float Kp = 1.5;
const float Kd = 0.05;

float previousError = 0.0;

const unsigned long sampleTime = 100;
const unsigned long stepTime = 2000;
const unsigned long testDuration = 12000;

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
    "Time\tTargetRPM\tActualRPM\tError\tPWM\tdt"
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

    // Read and reset encoder count safely
    noInterrupts();
    long pulses = encoderCount;
    encoderCount = 0;
    interrupts();

    // Calculate RPM; dt is in seconds
    float actualRPM =
      (pulses * 60.0) /
      (pulsesPerRevolution * dt);

    // Step target: 0 RPM, then 50 RPM
    float targetRPM =
      (elapsed < stepTime) ? 0.0 : targetSpeed;

    float error = targetRPM - actualRPM;

    // Proportional contribution
    float P = Kp * error;

    // Derivative contribution
    float D = Kd * (error - previousError) / dt;

    // Combined PD output
    float output = P + D;

    previousError = error;

    // Forward-only PWM, limited to 0–255
    int pwm = (int)constrain(output, 0.0, 255.0);

    analogWrite(ENA, pwm);

    // Serial Plotter data
    Serial.print(elapsed / 1000.0, 2);
    Serial.print('\t');
    Serial.print(targetRPM);
    Serial.print('\t');
    Serial.print(actualRPM);
    Serial.print('\t');
    Serial.print(error);
    Serial.print('\t');
    Serial.print(pwm);
    Serial.print('\t');
    Serial.println(dt);    
  }
}
