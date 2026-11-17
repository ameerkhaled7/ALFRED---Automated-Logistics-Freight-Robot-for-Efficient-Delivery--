int motorPin = 9; // Connect to Pin P on the controller

void setup() {
  pinMode(motorPin, OUTPUT);
  Serial.begin(9600); // Start the communication with the computer
  
  // Start with motor stopped
  analogWrite(motorPin, 0);
  
  Serial.println("System Ready.");
  Serial.println("Type a speed from 0 to 255 and press Enter:");
}

void loop() {
  // Check if you have typed a number in the Serial Monitor
  if (Serial.available() > 0) {
    
    // Read the number you typed
    int speed = Serial.parseInt();

    // Ignore extra "Enter" key presses
    if (Serial.read() == '\n') {
      
      // Limit the speed to the allowed range (0-255)
      if (speed < 0) speed = 0;
      if (speed > 255) speed = 255;

      // Send the speed to the controller
      analogWrite(motorPin, speed);

      // Print back what we did so you can see it
      Serial.print("Motor speed set to: ");
      Serial.println(speed);
    }
  }
}