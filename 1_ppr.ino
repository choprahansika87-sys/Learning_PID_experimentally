// ppr of left motor is 1065
// Encoder pins of left motor
const int C1 = 2;    // Encoder Channel A connected to pin 2
const int C2 = 3;    // Encoder Channel B connected to pin 3
// Encoder pins of right motor
//const int C1 = 12;   // Encoder Channel A connected to pin 12
//const int C2 = 11;   // Encoder Channel B connected to pin 11

// Stores the number of encoder pulses
// volatile is used because this variable is changed inside an interrupt
volatile long encoderCount = 0;         // Stores encoder position count
// SETUP FUNCTION
// Runs only once when Arduino starts
//
void setup()
{
    // Start Serial Monitor communication
    Serial.begin(9600);

    // Set encoder pins as inputs with internal pull-up resistors
    pinMode(C1, INPUT_PULLUP);
    pinMode(C2, INPUT_PULLUP);

    // Generate an interrupt whenever C1 changes from LOW to HIGH
    attachInterrupt(
        digitalPinToInterrupt(C1),
        readEncoder,
        RISING
    );
}


void loop()
{
    static long oldCount = 0;     // Remembers the previous encoder count

    // Check if the encoder count has changed
    if (encoderCount != oldCount)
    {
        oldCount = encoderCount;  // Update the previous count

        Serial.print("Count: ");  // Print label
        Serial.println(encoderCount); // Print current encoder count
    }
}

// ======================================
// INTERRUPT SERVICE ROUTINE (ISR)
// Executes every rising edge on Channel A
// ======================================

void readEncoder()
{
    // Read Channel B to determine rotation direction

    if (digitalRead(C2) == HIGH)   // If Channel B is HIGH
    {
        encoderCount++;            // Clockwise rotation → increase count
    }
    else                           // Otherwise
    {
        encoderCount--;            // Counter-clockwise rotation → decrease count
    }
}