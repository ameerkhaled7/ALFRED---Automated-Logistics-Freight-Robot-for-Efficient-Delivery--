#include <util/atomic.h> // For the ATOMIC_BLOCK macro

// Connect Riorand "S" pin to Mega Pin 18
#define SPEED_PULSE 18 

volatile int posi = 0; 

void setup() {
  Serial.begin(9600);
  
  // We use INPUT_PULLUP because the S pin is usually an open-collector
  pinMode(SPEED_PULSE, INPUT_PULLUP);
  
  // Attach interrupt to Pin 18
  attachInterrupt(digitalPinToInterrupt(SPEED_PULSE), readPulse, RISING);
  
  Serial.println("Riorand Pulse Counter Ready...");
}

void loop() {
  int pos = 0; 
  
  // ATOMIC_BLOCK ensures the 2-byte 'int' is read without being 
  // interrupted halfway through by the motor pulse
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    pos = posi;
  }

  Serial.println(pos);
  delay(10); // Small delay to prevent Serial buffer overflow
}

// Simplified ISR: Just counts up because the S pin only provides one signal
void readPulse(){
  posi++;
}