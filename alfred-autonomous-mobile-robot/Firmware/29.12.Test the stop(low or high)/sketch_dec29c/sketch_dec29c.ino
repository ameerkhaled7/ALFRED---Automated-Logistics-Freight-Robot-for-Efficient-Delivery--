// ZS-X11H V2 Speed Tester
// Type '1' for PWM 0, '5' for PWM 255 to find your range.

const int pwmPin = 9;   // White Header P
const int stopPin = 6;  // Green Terminal STOP

void setup() {
  pinMode(pwmPin, OUTPUT);
  pinMode(stopPin, OUTPUT);
  
  // Start with Safety Lock
  digitalWrite(stopPin, LOW); 
  
  // Initialize PWM to 255 (If logic is inverted, this might be STOP)
  analogWrite(pwmPin, 255);

  Serial.begin(9600);
  Serial.println("--- ZS-X11H Speed Tester ---");
  Serial.println("Press 'g' to Enable Motor.");
  Serial.println("Press 's' to Stop/Disable.");
  Serial.println("Type '1' -> PWM 0   (Likely FAST for you)");
  Serial.println("Type '2' -> PWM 60");
  Serial.println("Type '3' -> PWM 127");
  Serial.println("Type '4' -> PWM 200");
  Serial.println("Type '5' -> PWM 255 (Likely SLOW/STOP for you)");
}

void loop() {
  if (Serial.available()) {
    char cmd = Serial.read();
    
    // --- ENABLE/DISABLE ---
    if (cmd == 'g') {
      Serial.println("MOTOR ENABLED");
      digitalWrite(stopPin, HIGH); 
    }
    if (cmd == 's') {
      Serial.println("MOTOR STOPPED (Disabled)");
      digitalWrite(stopPin, LOW); 
    }

    // --- SPEED CONTROL ---
    if (cmd == '1') {
      Serial.println("Speed: PWM 0");
      analogWrite(pwmPin, 0); 
    }
    if (cmd == '2') {
      Serial.println("Speed: PWM 60");
      analogWrite(pwmPin, 60); 
    }
    if (cmd == '3') {
      Serial.println("Speed: PWM 127");
      analogWrite(pwmPin, 127); 
    }
    if (cmd == '4') {
      Serial.println("Speed: PWM 200");
      analogWrite(pwmPin, 200); 
    }
    if (cmd == '5') {
      Serial.println("Speed: PWM 255");
      analogWrite(pwmPin, 255); 
    }
  }
}