// ==========================================
//   RC ROBOT (MEGA): SMART OBSTACLE + AUTO
// ==========================================

#include <Wire.h> // Include Library for I2C communication (IMU)

// --- 1. MOTOR PINS ---
// Left Motor
const int motorLeftSpeed = 9;   // Pin D9: PWM speed control for Left Motor
const int motorLeftBrake = 8;   // Pin D8: Brake/Direction control for Left Motor

// Right Motor
const int motorRightSpeed = 6;  // Pin D6: PWM speed control for Right Motor
const int motorRightBrake = 7;  // Pin D7: Brake/Direction control for Right Motor

// --- 2. SENSOR PINS ---
const int trigPin = 12;         // Pin D12: Ultrasonic Trigger
const int echoPin = 11;         // Pin D11: Ultrasonic Echo

// --- 3. VARIABLES ---
const int safetyDistance = 20;  // RC Mode: Stop distance in cm (original logic)
const int autoStopDist = 50;    // Auto Mode: Stop distance in cm (per requirement 5)
int currentSpeed = 150;         // Default speed variable
char command = '0';             // Current button state storage

// --- 4. NEW VARIABLES (AUTO/MODES) ---
int robotMode = 2;              // 0=RED (Stop), 1=GREEN (Auto), 2=YELLOW (RC - Default)
const int maxPwm = 20;          // Maximum PWM speed allowed (per requirement 4)
float targetHeading = 0.0;      // Stores the desired heading for straight line
float currentHeading = 0.0;     // Stores real-time heading from IMU
const int imuAddress = 0x68;    // I2C Address of IMU (Standard MPU6050/GY-47)

// App Update Timer
long lastTime = 0;              // Variable to store last time App was updated
const int updateInterval = 200; // Interval in milliseconds to update App

void setup() {
  Serial.begin(9600);    // Start USB Serial for Debugging
  Serial1.begin(9600);   // Start Bluetooth Serial (Pins 18 TX, 19 RX)
  Serial1.setTimeout(50); // Set timeout for Bluetooth reading

  Wire.begin();          // Initialize I2C bus for IMU communication
  initIMU();             // Call helper function to wake up the IMU

  pinMode(motorLeftSpeed, OUTPUT);  // Set Left Speed pin as Output
  pinMode(motorLeftBrake, OUTPUT);  // Set Left Brake pin as Output
  pinMode(motorRightSpeed, OUTPUT); // Set Right Speed pin as Output
  pinMode(motorRightBrake, OUTPUT); // Set Right Brake pin as Output

  pinMode(trigPin, OUTPUT); // Set Ultrasonic Trigger as Output
  pinMode(echoPin, INPUT);  // Set Ultrasonic Echo as Input

  // --- APP INTERFACE (UPDATED) ---
  Serial1.println("*.kwl");                              // Clear App configuration
  Serial1.println("clear_panel()");                      // Clear existing panel
  Serial1.println("set_grid_size(16,10)");               // Set grid layout
  Serial1.println("add_4way_pad(11,2,1,2,3,4,0,0,,)");   // Add Direction Pad (RC controls)
  Serial1.println("add_send_box(2,2,8,V,,Type Speed)");  // Add Text Box for speed input
  Serial1.println("add_text_box(2,7,6,L,Dist,255,255,255,)"); // Add Display Box for Distance
  
  // Requirement 2: Add 3 Mode Buttons
  Serial1.println("add_button(2,11,3,G,Auto,0,255,0)");  // Green Button sends 'G' (Auto)
  Serial1.println("add_button(6,11,3,Y,RC,255,255,0)");  // Yellow Button sends 'Y' (RC)
  Serial1.println("add_button(10,11,3,R,Stop,255,0,0)"); // Red Button sends 'R' (Stop)
  
  Serial1.println("run()"); // Run the App Interface
  Serial1.println("*");     // End of App configuration
  
  Serial.println("System Ready."); // Print Ready message to Monitor
}

void loop() {
  // ==========================================
  // STEP 1: READ SENSOR (FAST)
  // ==========================================
  long duration, distance;        // Declare variables for Ultrasonic calc
  digitalWrite(trigPin, LOW);     // Ensure Trig is Low
  delayMicroseconds(2);           // Wait 2us
  digitalWrite(trigPin, HIGH);    // Set Trig High to send pulse
  delayMicroseconds(10);          // Wait 10us
  digitalWrite(trigPin, LOW);     // Set Trig Low
  
  // Timeout added: Wait max 30ms for echo (prevents lag)
  duration = pulseIn(echoPin, HIGH, 30000); // Measure echo time
  
  if (duration == 0) {            // If no echo received
    distance = 999;               // Assume path is clear (far away)
  } else {
    distance = (duration / 2) / 29.1; // Convert time to cm
  }

  // ==========================================
  // STEP 2: READ BLUETOOTH & UPDATE MODE
  // ==========================================
  if (Serial1.available() > 0) {  // Check if Bluetooth data is available
    char data = Serial1.peek();   // Peek at the first character
    
    // Check for Mode Buttons (Requirement 2)
    if (data == 'G') {            // Green Button (Autonomous)
       Serial1.read();            // Clear buffer
       robotMode = 1;             // Set Mode to 1 (Auto)
       targetHeading = getHeading(); // Lock current heading as target
       Serial.println("Mode: AUTO"); // Debug print
    }
    else if (data == 'Y') {       // Yellow Button (RC)
       Serial1.read();            // Clear buffer
       robotMode = 2;             // Set Mode to 2 (RC)
       Serial.println("Mode: RC");   // Debug print
    }
    else if (data == 'R') {       // Red Button (Stop/PWM 0)
       Serial1.read();            // Clear buffer
       robotMode = 0;             // Set Mode to 0 (Stop)
       stopMotors();              // Immediately stop motors
       Serial.println("Mode: STOP"); // Debug print
    }
    // Check for Speed Update
    else if (data == 'V') { 
       Serial1.read(); 
       int val = Serial1.parseInt(); 
       while(Serial1.available() > 0 && !isAlphaNumeric(Serial1.peek())) { Serial1.read(); }
       currentSpeed = constrain(val, 0, 255); // Keep variable for logic, but limit applied later
       Serial.print("Speed Set: "); Serial.println(currentSpeed);
    }
    // Check for Direction Commands
    else if (data == '1' || data == '2' || data == '3' || data == '4' || data == '0') {
      command = Serial1.read(); 
    }
    else { Serial1.read(); }      // discard junk
  }

  // ==========================================
  // STEP 3: MAIN LOGIC BASED ON MODE
  // ==========================================
  
  // --- MODE 0: RED / STANDBY ---
  if (robotMode == 0) {           // If Red button was pressed
    stopMotors();                 // Ensure PWM is 0
  }

  // --- MODE 1: GREEN / AUTONOMOUS (Requirement 3 & 5) ---
  else if (robotMode == 1) {
    if (distance < autoStopDist) { // Requirement 5: Stop if obj < 50cm
      stopMotors();                // Set PWM to zero
    } else {
      driveStraight(maxPwm);       // Drive straight with IMU correction
    }
  }

  // --- MODE 2: YELLOW / RC (Original Logic) ---
  else if (robotMode == 2) {
    
    // We limit the speed strictly to 20 for this mode too as per Req 4
    int effectiveSpeed = min(currentSpeed, maxPwm); 

    switch (command) {
      
      // --- FORWARD CASE (Has Safety Check) ---
      case '1': 
        if (distance > 0 && distance < safetyDistance) {
          stopMotors(); // Wall detected -> Stop
        } 
        else {
          moveForward(effectiveSpeed); // Go forward (max 20 PWM)
        }
        break;

      // --- TURN LEFT ---
      case '4': 
        turnLeft(effectiveSpeed);
        break;

      // --- TURN RIGHT ---
      case '2': 
        turnRight(effectiveSpeed);
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
  }

  // ==========================================
  // STEP 4: UPDATE APP
  // ==========================================
  if (millis() - lastTime > updateInterval) { // Check if time to update
    lastTime = millis();          // Reset timer
    Serial1.print("*L");          // Text Box ID 'L'
    Serial1.print(distance);      // Send Distance
    Serial1.print(" cm*");        // End command
  }
}

// ==========================================
// HELPER FUNCTIONS
// ==========================================

// --- MOTOR FUNCTIONS ---
void moveForward(int speed) {     // Function to move forward
  speed = min(speed, maxPwm);     // Requirement 4: Cap speed at 20
  analogWrite(motorLeftSpeed, speed);  // Write PWM to Left Motor
  analogWrite(motorRightSpeed, speed); // Write PWM to Right Motor
}

void turnLeft(int speed) {        // Function to turn left
  speed = min(speed, maxPwm);     // Cap speed at 20
  analogWrite(motorLeftSpeed, 0);      // Left Motor Off
  analogWrite(motorRightSpeed, speed); // Right Motor On
}

void turnRight(int speed) {       // Function to turn right
  speed = min(speed, maxPwm);     // Cap speed at 20
  analogWrite(motorLeftSpeed, speed);  // Left Motor On
  analogWrite(motorRightSpeed, 0);     // Right Motor Off
}

void stopMotors() {               // Function to stop
  analogWrite(motorLeftSpeed, 0);      // Write 0 PWM (OFF)
  analogWrite(motorRightSpeed, 0);     // Write 0 PWM (OFF)
} 

// --- IMU / AUTONOMOUS FUNCTIONS (Requirement 3) ---

void initIMU() {                  // Initialize the IMU (GY-47/MPU6050)
  Wire.beginTransmission(imuAddress); // Start I2C to 0x68
  Wire.write(0x6B);               // Power Management Register
  Wire.write(0);                  // Wake up MPU6050
  Wire.endTransmission(true);     // End I2C transmission
}

float getHeading() {              // Function to read IMU Z-axis (Yaw)
  Wire.beginTransmission(imuAddress); // Start I2C
  Wire.write(0x47);               // Register 0x47 (Gyro Z High Byte)
  Wire.endTransmission(false);    // Restart I2C
  Wire.requestFrom(imuAddress, 2, true); // Request 2 bytes (High+Low)
  
  int16_t GyroZ = Wire.read() << 8 | Wire.read(); // Combine bytes
  // NOTE: This is raw Gyro speed. For true angle, you integrate over time.
  // For simplicity here, we use the raw value as an error signal, 
  // or simplistic integration. Ideally, use a library like MPU6050_tockn.
  // Returning a mapped value approx to degrees for the loop.
  return GyroZ / 131.0;           // Convert raw to deg/sec (approx)
}

void driveStraight(int speed) {   // Logic to drive straight with IMU
  float currentGyro = getHeading(); // Get current rotation speed/angle
  
  // Straight line logic: Gyro Z should be 0 (no turning)
  // If > 0, we are turning left -> slow down Right
  // If < 0, we are turning right -> slow down Left
  
  int correction = currentGyro * 5; // Calculate proportional correction
  
  int leftPulse = speed + correction;  // Adjust Left Motor
  int rightPulse = speed - correction; // Adjust Right Motor
  
  // Constrain to Max Speed (Req 4) and Min 0
  leftPulse = constrain(leftPulse, 0, maxPwm); 
  rightPulse = constrain(rightPulse, 0, maxPwm); 

  analogWrite(motorLeftSpeed, leftPulse);   // Apply corrected speed Left
  analogWrite(motorRightSpeed, rightPulse); // Apply corrected speed Right
}