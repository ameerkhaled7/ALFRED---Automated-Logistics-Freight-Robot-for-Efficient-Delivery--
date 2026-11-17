// Define the pin connected to the "S" output of the driver
const int speedPulsePin = 18; // Pin 18 is an interrupt pin on Mega

// This variable stores the number of pulses counted
// It must be 'volatile' because it is changed inside an interrupt
volatile long pulseCount = 0;

void setup() {
  // Initialize Serial Monitor to see the data
  Serial.begin(9600);
  
  // Set the pin as Input with internal Pull-Up resistor
  // This prevents the signal from floating when no pulse is sent
  pinMode(speedPulsePin, INPUT_PULLUP);
  
  // Attach an interrupt to the pin. 
  // Whenever the signal goes from LOW to HIGH (RISING), run the 'countPulse' function
  attachInterrupt(digitalPinToInterrupt(speedPulsePin), countPulse, RISING);
  
  Serial.println("--- Pulse Counter Started ---");
  Serial.println("Rotate the wheel one full revolution to see the count.");
}

void loop() {
  // Print the current count every 0.5 seconds
  static long lastCount = 0;
  
  // Only print if the count has changed to keep the screen clean
  if (pulseCount != lastCount) {
    Serial.print("Pulses Detected: ");
    Serial.println(pulseCount);
    lastCount = pulseCount;
  }
  
  delay(100); // Small delay for stability
}

// This function runs automatically every time a pulse is detected
void countPulse() {
  pulseCount++;
}