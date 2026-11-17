// --- CONFIGURATION SECTION (CHANGE THESE) ---
float wheelDiameter = 0.10;    // Diameter of your wheel in METERS (e.g., 10cm = 0.10)
int pulsesPerRev = 15;         // How many pulses the motor sends for 1 full turn.
                               // (Start with 15. See "How to Calibrate" below to fix this)

// --- PIN DEFINITIONS ---
const int motorPin = 9;         
const int speedSensorPin = 2;   
const int outputToOtherPin = 3; 

// --- VARIABLES ---
volatile unsigned long pulseCount = 0;
unsigned long previousMillis = 0;
const long interval = 1000;
const int MAX_MOTOR_HZ = 500; 

void setup() {
  pinMode(motorPin, OUTPUT);
  pinMode(outputToOtherPin, OUTPUT);
  pinMode(speedSensorPin, INPUT_PULLUP);
  
  attachInterrupt(digitalPinToInterrupt(speedSensorPin), countPulses, RISING);

  Serial.begin(9600); 
  analogWrite(motorPin, 0);
  Serial.println("System Ready.");
}

void loop() {
  // 1. MOTOR CONTROL (Same as before)
  if (Serial.available() > 0) {
    int cmdSpeed = Serial.parseInt();
    if (Serial.peek() == '\n') Serial.read();
    
    if (cmdSpeed < 0) cmdSpeed = 0;
    if (cmdSpeed > 255) cmdSpeed = 255;
    analogWrite(motorPin, cmdSpeed);
  }

  // 2. CALCULATE VELOCITY
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    noInterrupts();
    unsigned long currentHz = pulseCount;
    pulseCount = 0;
    interrupts();

    // --- MATH: Convert Hz to Velocity (m/s) ---
    
    // Step A: Calculate Revolutions Per Second (RPS)
    // We use 'float' to keep decimal points
    float rps = (float)currentHz / pulsesPerRev;

    // Step B: Calculate Circumference (Distance of one turn)
    float circumference = wheelDiameter * 3.14159;

    // Step C: Velocity = RPS * Circumference
    float velocityMS = rps * circumference;

    // --- OUTPUT ---
    Serial.print("Freq: ");
    Serial.print(currentHz);
    Serial.print(" Hz | Velocity: ");
    Serial.print(velocityMS); 
    Serial.println(" m/s");

    // Optional: Send scaled speed to other controller (Method 1 from before)
    int outputSignal = map(currentHz, 0, MAX_MOTOR_HZ, 0, 255);
    outputSignal = constrain(outputSignal, 0, 255);
    analogWrite(outputToOtherPin, outputSignal);
  }
}

void countPulses() {
  pulseCount++;
}