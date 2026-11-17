// --- PIN DEFINITIONS ---
const int motorPin = 9;       // Connect to Pin 'P' (PWM) on the controller
const int speedSensorPin = 2; // Connect to Pin 'S' (SPEED) on the controller

// --- VARIABLES FOR SPEED MEASUREMENT ---
volatile unsigned long pulseCount = 0; // Counts pulses from the motor
unsigned long previousMillis = 0;      // Stores the last time we printed speed
const long interval = 1000;            // How often to print speed (1000ms = 1 second)

void setup() {
  pinMode(motorPin, OUTPUT);
  
  // Set the speed sensor pin as input. 
  // INPUT_PULLUP is usually safer for these open-drain sensors to avoid noise.
  pinMode(speedSensorPin, INPUT_PULLUP);

  // Attach an interrupt to Pin 2. 
  // Whenever the signal goes from LOW to HIGH (RISING), run the 'countPulses' function.
  attachInterrupt(digitalPinToInterrupt(speedSensorPin), countPulses, RISING);

  Serial.begin(9600); 

  // Start with motor stopped
  analogWrite(motorPin, 0);

  Serial.println("System Ready.");
  Serial.println("Type a speed from 0 to 255 and press Enter:");
}

void loop() {
  // -------------------------
  // 1. MOTOR CONTROL LOGIC (Your original code)
  // -------------------------
  if (Serial.available() > 0) {
    int speed = Serial.parseInt();

    // Ideally, we verify the next char is a newline to confirm a full command
    if (Serial.peek() == '\n') {
       Serial.read(); // Clear the newline
    }
    
    // Safety Constraints
    if (speed < 0) speed = 0;
    if (speed > 255) speed = 255;

    analogWrite(motorPin, speed);

    Serial.print("Command Received. Motor PWM set to: ");
    Serial.println(speed);
  }

  // -------------------------
  // 2. SPEED READING LOGIC
  // -------------------------
  unsigned long currentMillis = millis();

  // Every 1 second (interval), calculate and print the speed
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // Temporarily disable interrupts to read the volatile variable safely
    noInterrupts();
    unsigned long pulses = pulseCount;
    pulseCount = 0; // Reset count for the next second
    interrupts();

    // 'pulses' is now the Frequency in Hertz (pulses per second)
    Serial.print("Feedback Signal: ");
    Serial.print(pulses);
    Serial.println(" Hz (Pulses per second)");
    
    // NOTE: To get real RPM, you need to know your motor's "Pole Pairs".
    // The formula is usually: RPM = (Frequency * 60) / PolePairs
  }
}

// This function runs automatically every time the 'S' pin sends a pulse
void countPulses() {
  pulseCount++;
}