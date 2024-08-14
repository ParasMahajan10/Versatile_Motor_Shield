/**
 ******************************************************************************
 * @file    VMShield.h
 * @author  [Paras Mahajan]
 * @date    [13/Jun/2024]
 * @brief   Custom C++ library for controlling various types of motors using 
 *          an STM32 Nucleo board and varsatile motor shield.
 ******************************************************************************
 * @attention
 * 
 * This library provides an interface for controlling stepper motors, servo 
 * motors, DC motors, and brushless DC (BLDC) motors. It is designed to be 
 * flexible and easy to integrate with any STM32-based project.
 * 
 * Major motor driving functions provided by this library:
 * -----------------------------------
 * - Stepper    (Mot_no, Dir, steps);
 * - Servo      (Mot_no, Degrees);    
 * - DC         (Mot_no, Dir, Duty_Cycle);
 * -----------------------------------
 * 
 ******************************************************************************
 * Pin Assignments Table
 ******************************************************************************
 *
 * |-----------|----------|-----------|-------------|
 * | Motor     | Function | STM32 Pin | Arduino Pin |
 * |-----------|----------|-----------|-------------|
 * | Stepper 1 | Step_1   | PA_6      | D12         |
 * |           | Dir_1    | PA_5      | D13         |
 * | Stepper 2 | Step_2   | PB_6      | D10         |
 * |           | Dir_2    | PA_7      | D11         |
 * | Stepper 3 | Step_3   | PA_9      | D8          |
 * |           | Dir_3    | PC_7      | D9          |
 * | Stepper 4 | Step_4   | PB_10     | D6          |
 * |           | Dir_4    | PA_8      | D7          |
 * |-----------|----------|-----------|-------------|
 * | DC 1      | EN_1     | PB_5      | D4          |
 * |           | Dir_1A   | PC_2      | N/A         |
 * |           | Dir_2A   | PC_3      | N/A         |
 * | DC 2      | EN_2     | PB_4      | D5          |
 * |           | Dir_3A   | PC_12     | N/A         |
 * |           | Dir_4A   | PC_10     | N/A         |
 * |-----------|----------|-----------|-------------|
 * | BLDC 1    | BLDC_1   | PB_1      | N/A         |
 * | BLDC 2    | BLDC_2   | PB_15     | N/A         |
 * | BLDC 3    | BLDC_3   | PB_14     | N/A         |
 * | BLDC 4    | BLDC_4   | PB_13     | N/A         |
 * |-----------|----------|-----------|-------------|
 * 
 ******************************************************************************
 */

#ifndef VMSHIELD_H
#define VMSHIELD_H

#include "mbed.h"

// Defination for PCA9685 Servo Driver
#define PCA9685_SUBADR1 0x2
#define PCA9685_SUBADR2 0x3
#define PCA9685_SUBADR3 0x4

#define PCA9685_MODE1 0x0
#define PCA9685_PRESCALE 0xFE

#define LED0_ON_L 0x6
#define LED0_ON_H 0x7
#define LED0_OFF_L 0x8
#define LED0_OFF_H 0x9

#define ALLLED_ON_L 0xFA
#define ALLLED_ON_H 0xFB
#define ALLLED_OFF_L 0xFC
#define ALLLED_OFF_H 0xFD

// Stepper Motor Class Defination
class Stepper {
public:
    Stepper(PinName StepPin_1, PinName DirPin_1, 
            PinName StepPin_2, PinName DirPin_2, 
            PinName StepPin_3, PinName DirPin_3, 
            PinName StepPin_4, PinName DirPin_4); // Constructor

    // Method prototyping or member function
    void MoveStepper(int Mot_no, int Dir, int steps);

private:

    // Pin assignment
     DigitalOut StepPin1;   
     DigitalOut DirPin1;
     DigitalOut StepPin2;   
     DigitalOut DirPin2;
     DigitalOut StepPin3;   
     DigitalOut DirPin3;
     DigitalOut StepPin4;   
     DigitalOut DirPin4;     

};

// DC Motor Class Defination
class DC {
public:
    DC(PinName EN_1, PinName EN_2, 
            PinName IN_1, PinName IN_2, 
            PinName IN_3, PinName IN_4); // Constructor

    // Method prototyping or member function
    void MoveDC(int Mot_no, int Dir, float Duty_Cycle);

private:

    // Pin assignment
     PwmOut EN1;   
     PwmOut EN2;
     DigitalOut IN1;   
     DigitalOut IN2;
     DigitalOut IN3;   
     DigitalOut IN4;     

};

// Servo Motor Class Defination

class Servo{

 public:
  Servo(I2C *i2c, uint8_t addr = 0x40);
  void begin(void);
  void reset(void);
  void setPWMFreq(float freq);
  void setPWM(uint8_t num, uint16_t on, uint16_t degree);

 private:
  I2C *_i2c;
  uint8_t _i2caddr;

  uint8_t read8(uint8_t addr);
  void write8(uint8_t addr, uint8_t d);

};

// Music Class Defination
class Music{

public:

    Music(PinName StepPin_1, PinName DirPin_1);
    // Method prototyping or member function
    void PlayMusic(int pulseCount, float noteDurationMs, float stepDelay);

private:

// Pin assignment
     DigitalOut StepPin1music;   
     DigitalOut DirPin1music;

};

#endif