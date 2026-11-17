// ==========================================
//   RC ROBOT (MEGA): SMART OBSTACLE LOGIC
// ==========================================

// --- 1. MOTOR PINS ---
// Left Motor
const int motorLeftSpeed = 9;   
const int motorLeftBrake = 8;   

// Right Motor
const int motorRightSpeed = 6;  
const int motorRightBrake = 7;  

// --- 2. SENSOR PINS ---
const int trigPin = 12;
const int echoPin = 11;

// --- 3. VARIABLES ---
const int safetyDistance = 20;  // Stop distance in cm
int currentSpeed = 150;         // Default speed
char command = '0';             // Current button state

// App Update Timer
long lastTime = 0;
const int updateInterval = 200; 

void setup() {
  Serial.begin(9600);    // USB Debugging
  Serial1.begin(9600);   // Bluetooth (Pins 18 TX, 19 RX)
  Serial1.setTimeout(50); 

  pinMode(motorLeftSpeed, OUTPUT);
  pinMode(motorLeftBrake, OUTPUT);
  pinMode(motorRightSpeed, OUTPUT);
  pinMode(motorRightBrake, OUTPUT);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // --- APP INTERFACE ---
  Serial1.println("*.kwl");
  Serial1.println("clear_panel()");
  Serial1.println("set_grid_size(16,10)");
  Serial1.println("add_4way_pad(11,2,1,2,3,4,0,0,,)");
  Serial1.println("add_send_box(2,2,8,V,,Type Speed (0-255))");
  Serial1.println("add_text_box(2,7,6,L,Distance,255,255,255,)");
  Serial1.println("run()");
  Serial1.println("*");
  
  Serial.println("System Ready.");
}

void loop() {
  // ==========================================
  // STEP 1: READ SENSOR (FAST)
  // ==========================================
  long duration, distance;
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Timeout added: Wait max 30ms for echo (prevents lag)
  duration = pulseIn(echoPin, HIGH, 30000); 
  
  if (duration == 0) {
    distance = 999; // No echo? Assume path is clear (far away)
  } else {
    distance = (duration / 2) / 29.1; 
  }

  // ==========================================
  // STEP 2: READ BLUETOOTH
  // ==========================================
  if (Serial1.available() > 0) {
    char data = Serial1.peek(); 
    if (data == 'V') { // Speed Update
       Serial1.read(); 
       int val = Serial1.parseInt(); 
       while(Serial1.available() > 0 && !isAlphaNumeric(Serial1.peek())) { Serial1.read(); }
       currentSpeed = constrain(val, 0, 255);
       Serial.print("Speed Set: "); Serial.println(currentSpeed);
    }
    else if (data == '1' || data == '2' || data == '3' || data == '4' || data == '0') {
      command = Serial1.read(); 
    }
    else { Serial1.read(); }
  }

  // ==========================================
  // STEP 3: SMART CONTROL LOGIC
  // ==========================================
  
  // We check the specific command FIRST
  switch (command) {
    
    // --- FORWARD CASE (Has Safety Check) ---
    case '1': 
      if (distance > 0 && distance < safetyDistance) {
        // WALL DETECTED!
        stopMotors(); // Force Stop immediately
      } 
      else {
        // PATH CLEAR -> Go!
        moveForward(currentSpeed); 
      }
      break;

    // --- TURN LEFT (No Safety Check - Always Allowed) ---
    case '4': 
      turnLeft(currentSpeed);
      break;

    // --- TURN RIGHT (No Safety Check - Always Allowed) ---
    case '2': 
      turnRight(currentSpeed);
      break;

    // --- MANUAL STOP ---
    case '3': 
      stopMotors(); 
      break;

    // --- RELEASED ---
    case '0': 
      stopMotors(); 
      break;
  }

  // ==========================================
  // STEP 4: UPDATE APP
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
  //digitalWrite(motorLeftBrake, LOW);
  //digitalWrite(motorRightBrake, LOW);
  analogWrite(motorLeftSpeed, speed);
  analogWrite(motorRightSpeed, speed);
}

void turnLeft(int speed) {
  // Pivot Left (Left Brake ON, Right Motor ON)
  //digitalWrite(motorLeftBrake, HIGH); 
  analogWrite(motorLeftSpeed, 0);
  
  //digitalWrite(motorRightBrake, LOW); 
  analogWrite(motorRightSpeed, speed);
}

void turnRight(int speed) {
  // Pivot Right (Left Motor ON, Right Brake ON)
  digitalWrite(motorLeftBrake, LOW);  
  analogWrite(motorLeftSpeed, speed);
  
  digitalWrite(motorRightBrake, HIGH); 
  analogWrite(motorRightSpeed, 0);
}

void stopMotors() {
  analogWrite(motorLeftSpeed, 0);
  analogWrite(motorRightSpeed, 0);
 // digitalWrite(motorLeftBrake, HIGH);
  //digitalWrite(motorRightBrake, HIGH);
}