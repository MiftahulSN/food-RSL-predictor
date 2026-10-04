#include <Arduino.h>

#define MH_Z19_RX 18  
#define MH_Z19_TX 17  
#define MH_Z19_PWM 4  

HardwareSerial sensorSerial(1);

const byte readCmd[9] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
const byte zeroCmd[9] = {0xFF, 0x01, 0x87, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78};

void calibrateZero() {
  Serial.println("Sending Zero-Point Calibration command...");
  sensorSerial.write(zeroCmd, 9);
  delay(100);
}

int readCO2UART() {
  while (sensorSerial.available() > 0) {
    sensorSerial.read();
  }
  sensorSerial.write(readCmd, 9);
  delay(100);

  if (sensorSerial.available() >= 9) {
    byte response[9];
    sensorSerial.readBytes(response, 9);
    if (response[0] == 0xFF && response[1] == 0x86) {
      byte checksum = 0;
      for (int i = 1; i < 8; i++) {
        checksum += response[i];
      }
      checksum = 0xFF - checksum + 1;
      if (checksum == response[8]) {
        return (response[2] << 8) | response[3];
      }
    }
  }
  return -1;
}

int readCO2PWM() {
  unsigned long th_us = pulseIn(MH_Z19_PWM, HIGH, 2000000);
  unsigned long tl_us = pulseIn(MH_Z19_PWM, LOW, 2000000);
  if (th_us == 0 || tl_us == 0) return -1; 
  float th = th_us / 1000.0;
  float tl = tl_us / 1000.0;
  // If your sensor is a 0-5000ppm model, change 2000.0 to 5000.0
  float co2 = 2000.0 * (th - 2.0) / (th + tl - 4.0);
  return (int)co2;
}

void setup() {
  Serial.begin(115200);
  sensorSerial.begin(9600, SERIAL_8N1, MH_Z19_RX, MH_Z19_TX);
  pinMode(MH_Z19_PWM, INPUT);

  Serial.println("MH-Z19C Ready.");
  
  // UNCOMMENT THE LINE BELOW ONLY ONCE while the sensor is in stable fresh air (400ppm)
  // calibrateZero();
}

void loop() {
  int co2_uart = readCO2UART();
  int co2_pwm = readCO2PWM();

  Serial.println("----------------------------------------");
  Serial.print("UART CO2 Concentration : ");
  Serial.println(co2_uart > 0 ? String(co2_uart) + " ppm" : "n/a");
  Serial.print("PWM CO2 Concentration  : ");
  Serial.println(co2_pwm > 0 ? String(co2_pwm) + " ppm" : "n/a");

  delay(2000);
}