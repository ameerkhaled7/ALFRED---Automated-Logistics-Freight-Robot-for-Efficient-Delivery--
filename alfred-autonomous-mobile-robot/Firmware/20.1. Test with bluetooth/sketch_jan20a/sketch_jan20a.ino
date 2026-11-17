// ==========================================
//   RC ROBOT (MEGA): MEMORY SPEED + SAFETY
// ==========================================

// --- 1. MOTOR PINS ---
// Left Motor
const int motorLeftSpeed = 9;   // PWM Pin
const int motorLeftBrake = 8;   // Brake Pin

// Right Motor
const int motorRightSpeed = 6;  // PWM Pin
const int motorRightBrake = 7;  // Brake Pin

// --- 2. SENSOR PINS ---
const int trigPin = 12;
const int echoPin = 11;

// --- 3. VARIABLES ---
const int safetyDistance = 20;  // Stop if closer than 20cm
int currentSpeed = 150;         // Stores your speed (Default 150)
char command = '0';             // Stores the button you are pressing

// App Update Timer
long lastTime = 0;
const int updateInterval = 200; 

void setup() {
  // 1. Start Serial Connections
  Serial.begin(9600);    // For USB Cable (Debugging)
  Serial1.begin(9600);   // For Bluetooth (Pins 18 & 19)
  Serial1.setTimeout(50); // Fast reading for text box

  // 2. Init Motor Pins
  pinMode(motorLeftSpeed, OUTPUT);
  pinMode(motorLeftBrake, OUTPUT);
  pinMode(motorRightSpeed, OUTPUT);
  pinMode(motorRightBrake, OUTPUT);

  // 3. Init Sensor Pins
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // 4. Build App Interface (Keuwlsoft)
  Serial1.println("*.kwl");
  Serial1.println("clear_panel()");
  Serial1.println("set_grid_size(16,10)");
  
  // Direction Pad (ID: 1,2,3,4,0)
  Serial1.println("add_4way_pad(11,2,1,2,3,4,0,0,,)");

  // Speed Input Box (ID: V)
  // When you type a number, it sends "V" followed by the number
  Serial1.println("add_send_box(2,2,8,V,,Type Speed (0-255))");
  
  // Distance Display (ID: L)
  Serial1.println("add_text_box(2,7,6,L,Distance,255,255,255,)");

  Serial1.println("run()");
  Serial1.println("*");
  
  Serial.println("System Ready.");
}

void loop() {
  // ==========================================
  // STEP 1: READ ULTRASONIC SENSOR
  // ==========================================
  long duration, distance;
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  duration = pulseIn(echoPin, HIGH);
  distance = (duration / 2) / 29.1; // Convert to cm

  // ==========================================
  // STEP 2: READ BLUETOOTH COMMANDS
  // ==========================================
  if (Serial1.available() > 0) {
    char data = Serial1.peek(); 

    // --- CASE A: NEW SPEED ENTERED ---
    if (data == 'V') {
       Serial1.read(); // Remove 'V'
       int val = Serial1.parseInt(); // Read the number
       
       // Clean buffer
       while(Serial1.available() > 0 && !isAlphaNumeric(Serial1.peek())) { 
         Serial1.read(); 
       }

       // Update the global speed memory
       currentSpeed = constrain(val, 0, 255);
       
       Serial.print("Speed Memory Updated: ");
       Serial.println(currentSpeed);
    }
    
    // --- CASE B: DIRECTION BUTTONS ---
    else if (data == '1' || data == '2' || data == '3' || data == '4' || data == '0') {
      command = Serial1.read(); 
    }
    else {
      Serial1.read(); // Clear junk
    }
  }

  // ==========================================
  // STEP 3: CONTROL LOGIC (The "Brain")
  // ==========================================
  
  // CHECK: Is the user pressing FORWARD ('1') AND is there an Obstacle?
  if (command == '1' && distance > 0 && distance < safetyDistance) {
    
    // !!! OBSTACLE DETECTED !!!
    // 1. Force PWM to 0
    // 2. Engage Brakes
    // Note: We do NOT change 'currentSpeed', so it remembers it.
    stopMotors(); 
    
    // Optional debug message
    // Serial.println("BLOCKED! PWM forced to 0.");
  }
  else {
    // !!! PATH CLEAR !!!
    // Resume using 'currentSpeed'
    switch (command) {
      case '1': // Forward
        moveForward(currentSpeed); 
        break;
        
      case '4': // Left
        turnLeft(currentSpeed);
        break;
        
      case '2': // Right
        turnRight(currentSpeed);
        break;
        
      case '3': // Down (Manual Stop)
        stopMotors(); 
        break;
        
      case '0': // Released (Auto Stop)
        stopMotors(); 
        break;
    }
  }

  // ==========================================
  // STEP 4: UPDATE APP DISTANCE
  // ==========================================
  if (millis() - lastTime > updateInterval) {
    lastTime = millis();
    Serial1.print("*L");
    Serial1.print(distance);
    Serial1.print(" cm*"); 
  }
}

// ==========================================
// HELPER FUNCTIONS
// ==========================================

void moveForward(int speed) {
  // Release Brakes
  digitalWrite(motorLeftBrake, LOW);
  digitalWrite(motorRightBrake, LOW);
  
  // Apply the Stored Speed
  analogWrite(motorLeftSpeed, speed);
  analogWrite(motorRightSpeed, speed);
}

void turnLeft(int speed) {
  // Pivot Left
  digitalWrite(motorLeftBrake, HIGH); // Brake Left
  analogWrite(motorLeftSpeed, 0);
  
  digitalWrite(motorRightBrake, LOW); // Move Right
  analogWrite(motorRightSpeed, speed);
}

void turnRight(int speed) {
  // Pivot Right
  digitalWrite(motorLeftBrake, LOW);  // Move Left
  analogWrite(motorLeftSpeed, speed);
  
  digitalWrite(motorRightBrake, HIGH); // Brake Right
  analogWrite(motorRightSpeed, 0);
}

void stopMotors() {
  // 1. Set PWM to 0 (Power Cut)
  analogWrite(motorLeftSpeed, 0);
  analogWrite(motorRightSpeed, 0);
  
  // 2. Set Brake to HIGH (Hard Stop)
  digitalWrite(motorLeftBrake, HIGH);
  digitalWrite(motorRightBrake, HIGH);
}