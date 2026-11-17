// --- PIN DEFINITIONS ---
const int pwmPin = 9;   
const int dirPin = 8;   
const int brakePin = 7; 
const int stopPin = 6;  

// Variables
int currentSpeed = 0; // We will store this as 0-100% for your sanity

void setup() {
  pinMode(pwmPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  pinMode(brakePin, OUTPUT);
  pinMode(stopPin, OUTPUT);

  // --- INITIAL STATE ---
  digitalWrite(stopPin, LOW);   // System Locked
  digitalWrite(brakePin, LOW);  // Brake Off
  digitalWrite(dirPin, HIGH);   
  
  // INVERTED FIX: Write 255 to start at 0 Volts (Stop)
  analogWrite(pwmPin, 255);       

  Serial.begin(9600);
  Serial.println("--- BLDC INVERTED MODE ---");
  Serial.println("Type 'g' to Go.");
}

void loop() {
  if (Serial.available() > 0) {
    if (isDigit(Serial.peek())) {
      
      int targetPercent = Serial.parseInt();
      Serial.read(); 
      
      if (targetPercent > 100) targetPercent = 100;
      if (targetPercent < 0) targetPercent = 0;
      
      // MATH CONVERSION: 
      // Input 0   -> Output 255 (Stop)
      // Input 100 -> Output 0   (Max)
      int pwmSignal = map(targetPercent, 0, 100, 255, 0);
      
      analogWrite(pwmPin, pwmSignal);
      Serial.print("Speed set to: ");
      Serial.print(targetPercent);
      Serial.println("%");
      
    } else {
      char command = Serial.read();
      if(command == '\n' || command == '\r') return;

      switch (command) {
        case 'b': // Brake
          Serial.println("!!! BRAKE !!!");
          digitalWrite(brakePin, HIGH); 
          analogWrite(pwmPin, 255); // Send 'Stop' signal (255) to PWM
          break;
          
        case 'n': // Normal
          Serial.println("Brake Released.");
          digitalWrite(brakePin, LOW);
          break;

        case 's': // Emergency Stop
          Serial.println("System Locked.");
          digitalWrite(stopPin, LOW); 
          analogWrite(pwmPin, 255); // Ensure PWM is Off
          break;

        case 'g': // Go
          Serial.println("System Enabled.");
          digitalWrite(stopPin, HIGH); 
          break;
          
        case 'f': // Forward
           Serial.println("Forward");
           digitalWrite(dirPin, HIGH);
           break;

        case 'r': // Reverse
           Serial.println("Reverse");
           digitalWrite(dirPin, LOW);
           break;
      }
    }
  }
}