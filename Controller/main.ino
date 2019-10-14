
#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
#include "Wire.h"
#endif
MPU6050 mpu;


float correct;
int j = 0;

#define OUTPUT_READABLE_YAWPITCHROLL

#define INTERRUPT_PIN 2 //Interrupt

bool blinkState = false;

// MPU control/status vars
bool dmpReady = false;
uint8_t mpuIntStatus;
uint8_t devStatus;
uint16_t packetSize;
uint16_t fifoCount;
uint8_t fifoBuffer[64];

// orientation/motion vars
Quaternion q;          
VectorInt16 aa;        
VectorInt16 aaReal;    
VectorInt16 aaWorld;   
VectorFloat gravity;   
float euler[3];        
float ypr[3];          

// packet structure for InvenSense teapot demo
uint8_t teapotPacket[14] = { '$', 0x02, 0, 0, 0, 0, 0, 0, 0, 0, 0x00, 0x00, '\r', '\n' };


volatile bool mpuInterrupt = false;  
void dmpDataReady() {
  mpuInterrupt = true;
}



void setup() {
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
  Wire.begin();
  Wire.setClock(400000);
#elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
  Fastwire::setup(400, true);
#endif


  Serial.begin(38400);
  while (!Serial);


  mpu.initialize();
  pinMode(INTERRUPT_PIN, INPUT);
  devStatus = mpu.dmpInitialize();
  
  mpu.setXGyroOffset(17);
  mpu.setYGyroOffset(-69);
  mpu.setZGyroOffset(27);
  mpu.setZAccelOffset(1551); // 1688 factory default 

  if (devStatus == 0) {
   
    mpu.setDMPEnabled(true);

    attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), dmpDataReady, RISING);
    mpuIntStatus = mpu.getIntStatus();

    dmpReady = true;

    packetSize = mpu.dmpGetFIFOPacketSize();
  } else {
  }

 
}

void loop() {
  if (!dmpReady) return;
  while (!mpuInterrupt && fifoCount < packetSize) {
    if (mpuInterrupt && fifoCount < packetSize) {
      fifoCount = mpu.getFIFOCount();
    }
  }

  mpuInterrupt = false;
  mpuIntStatus = mpu.getIntStatus();

  fifoCount = mpu.getFIFOCount();

  if ((mpuIntStatus & _BV(MPU6050_INTERRUPT_FIFO_OFLOW_BIT)) || fifoCount >= 1024) {
    mpu.resetFIFO();
    fifoCount = mpu.getFIFOCount();
    Serial.println(F("FIFO overflow!"));

  } else if (mpuIntStatus & _BV(MPU6050_INTERRUPT_DMP_INT_BIT)) {
    while (fifoCount < packetSize) fifoCount = mpu.getFIFOCount();

    mpu.getFIFOBytes(fifoBuffer, packetSize);

    fifoCount -= packetSize;
//Get Yaw, Pitch & Roll
#ifdef OUTPUT_READABLE_YAWPITCHROLL
    mpu.dmpGetQuaternion(&q, fifoBuffer);
    mpu.dmpGetGravity(&gravity, &q);
    mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);

    // Yaw, Pitch, Roll values - Radians to degrees
    ypr[0] = ypr[0] * 180 / M_PI;
    ypr[1] = ypr[1] * 180 / M_PI;
    ypr[2] = ypr[2] * 180 / M_PI;
    
    // Skip 300 readings (self-calibration process)
    if (j <= 300) {
      correct = ypr[0];
      j++;
    }
    // After 300 readings
    else {
      ypr[0] = ypr[0] - correct; // Set the Yaw to 0 deg - subtract  the last random Yaw value from the currrent value to make the Yaw 0 degrees
      // Map the values of the MPU6050 sensor from -90 to 90 to values suatable for the servo control from 0 to 180
      int angle0 = map(ypr[0], -90, 90, 0, 180);
      int angle1 = map(ypr[1], -90, 90, 0, 180);
      int angle2 = map(ypr[2], -90, 90, 180, 0); 
      
    }
#endif
  }
}
