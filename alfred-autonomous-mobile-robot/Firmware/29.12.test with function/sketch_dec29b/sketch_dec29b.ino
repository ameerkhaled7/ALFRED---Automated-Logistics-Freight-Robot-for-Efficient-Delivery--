// --- PIN DEFINITIONS ---
const int pwmPin = 9;   // Speed (White Header)
const int dirPin = 8;   // Direction (Green Terminal)
const int brakePin = 7; // Brake (Green Terminal)
const int stopPin = 6;  // Stop (Green Terminal)

// Variables
int currentSpeed = 0;

void setup() {
  pinMode(pwmPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  pinMode(brakePin, OUTPUT);
  pinMode(stopPin, OUTPUT);

  // --- INITIAL STATE (Safe Start) ---
  // CRITICAL CHANGE: We start with STOP active (LOW) to prevent startup spins
  digitalWrite(stopPin, LOW);   // System Locked (Motor Disabled)
  digitalWrite(brakePin, LOW);  // Brake RELEASED
  digitalWrite(dirPin, HIGH);   // Default Direction
  analogWrite(pwmPin, 0);       // Speed 0

  Serial.begin(9600);
  Serial.println("--- BLDC Motor Command Center ---");
  Serial.println("STATUS: SYSTEM LOCKED (Safe Mode)");
  Serial.println("---------------------------------");
  Serial.println("TYPE 'g' AND PRESS ENTER TO START");
  Serial.println("---------------------------------");
  Serial.println("COMMAND LIST:");
  Serial.println(" [0-255] : Set Speed");
  Serial.println(" 'f'     : Forward");
  Serial.println(" 'r'     : Reverse");
  Serial.println(" 'b'     : Brake ON (Hard Stop)");
  Serial.println(" 'n'     : Brake OFF (Normal)");
  Serial.println(" 's'     : Emergency Stop (Coast + Lock)");
  Serial.println(" 'g'     : Go (Unlock System)");
}

void loop() {
  
  if (Serial.available() > 0) {
    // Check if the input is a number (Speed) or a letter (Command)
    if (isDigit(Serial.peek())) {
      
      int newSpeed = Serial.parseInt();
      // Clear the newline character
      Serial.read(); 
      
      // Safety limit
      if (newSpeed > 255) newSpeed = 255;
      if (newSpeed < 0) newSpeed = 0;
      
      currentSpeed = newSpeed;
      analogWrite(pwmPin, currentSpeed);
      Serial.print("Speed set to: ");
      Serial.println(currentSpeed);
      
    } else {
      // It's a letter command
      char command = Serial.read();
      
      // Ignore newline characters
      if(command == '\n' || command == '\r') return;

      switch (command) {
        case 'f': // Forward
          Serial.println("Command: FORWARD");
          // SAFETY: Briefly stop motor before switching direction
          analogWrite(pwmPin, 0);
          delay(200); 
          digitalWrite(dirPin, HIGH);
          delay(200);
          analogWrite(pwmPin, currentSpeed);
          break;

        case 'r': // Reverse
          Serial.println("Command: REVERSE");
          // SAFETY: Briefly stop motor before switching direction
          analogWrite(pwmPin, 0);
          delay(200); 
          digitalWrite(dirPin, LOW);
          delay(200);
          analogWrite(pwmPin, currentSpeed);
          break;

        case 'b': // Brake
          Serial.println("!!! BRAKE ENGAGED !!!");
          digitalWrite(brakePin, HIGH); 
          // Set speed variable to 0 so it doesn't jump when released
          analogWrite(pwmPin, 0); 
          currentSpeed = 0;
          break;
          
        case 'n': // Normal (Brake Off)
          Serial.println("Brake Released.");
          digitalWrite(brakePin, LOW);
          break;

        case 's': // Stop (Coast)
          Serial.println("!!! SYSTEM LOCKED (Coast) !!!");
          digitalWrite(stopPin, LOW); // Lock the motor
          // SAFETY ADDITION: Reset speed to 0
          analogWrite(pwmPin, 0);
          currentSpeed = 0;
          break;

        case 'g': // Go (Enable)
          Serial.println("System Enabled (Go).");
          digitalWrite(stopPin, HIGH); // Unlock the motor
          break;
      }
    }
  }
}