// ---------- RIGHT MOTOR ----------
const int ENB = 5;   // PWM
const int IN3 = 7;
const int IN4 = 6;

// ---------- ENCODER ----------
const int C1 = 2;
const int C2 = 3;

volatile long encoderCount = 0;

const int pulsesPerRevolution = 1050;

// ---------- INTEGRAL CONTROL ----------
float targetRPM = 100.0;
float Ki = 50.0;
float integralOutput = 0.0;

const unsigned long sampleTime = 100;
const unsigned long stepTime = 2000;
const unsigned long testDuration = 50000;

unsigned long startTime;
unsigned long lastSampleTime;

void countPulse()
{
  encoderCount++;
}

void setup()
{
  pinMode(ENB, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  pinMode(C1, INPUT_PULLUP);
  pinMode(C2, INPUT_PULLUP);

  // Fixed forward direction
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  analogWrite(ENB, 0);

  attachInterrupt(
    digitalPinToInterrupt(C1),
    countPulse,
    RISING
  );

  Serial.begin(115200);

  Serial.println("TargetRPM\tActualRPM\tPWM");

  startTime = millis();
  lastSampleTime = startTime;
}

void loop()
{
  unsigned long now = millis();
  unsigned long elapsed = now - startTime;

  // Stop after the experiment
  if (elapsed >= testDuration) {
    analogWrite(ENB, 0);
    while (true) {
      // Motor remains stopped
    }
  }

  if (now - lastSampleTime >= sampleTime) {
    float dt = (now - lastSampleTime) / 1000.0;
    lastSampleTime = now;

    // Safely copy and reset encoder count
    noInterrupts();
    long pulses = encoderCount;
    encoderCount = 0;
    interrupts();

    // RPM measurement
    float rpm =
      (pulses * 60.0) /
      (pulsesPerRevolution * dt);

    // Step input: 0 RPM, then 50 RPM
    if (elapsed < stepTime) {
      targetRPM = 0;
    } else {
      targetRPM = 50;
    }

    float error = targetRPM - rpm;

    // Integral-only controller
    integralOutput += Ki * error * dt;
    //Serial.println(integralOutput);

    // Clamp output to valid PWM range
    integralOutput = constrain(integralOutput, 0.0, 255.0);

    int pwm = (int)integralOutput;

    analogWrite(ENB, pwm);

    // Serial Plotter data
    Serial.print(targetRPM);
    Serial.print("\t");
    Serial.print(rpm);
    Serial.print("\t");
    Serial.println(pwm);
  }
}
