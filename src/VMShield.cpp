/**
 ******************************************************************************
 * @file    VMShield.cpp
 * @author  [Paras Mahajan]
 * @date    [13/Jun/2024]
 * @brief   Custom C++ library for controlling various types of motors using 
 *          an STM32 Nucleo board and motor shield.
 ******************************************************************************
 * @attention
 * 
 * This library provides an interface for controlling stepper motors, servo 
 * motors, DC motors, and brushless DC (BLDC) motors using Versatile Motor Shield (VMShield). It is designed to be 
 * flexible and easy to integrate with any STM32-based project.
 * 
 * Major motor driving functions provided by this library:
 * -----------------------------------
 * - Stepper    (int Mot_no, int Dir, int steps)
 * - Servo      (int Mot_no, int Degrees)    
 * - DC         (int Mot_no, int Dir, float Duty_Cycle)
 * - BLDC       (int Mot_no, int Dir, float Duty_Cycle) 
 * 
 ******************************************************************************
 */

#include "mbed.h"
#include "VMShield.h"

#define I2C_SDA PB_9
#define I2C_SCL PB_8

// I2C frequency (in Hz)
#define I2C_FREQUENCY 100000

// PCA9685 address (default is 0x40)
#define PCA9685_ADDRESS 0x40

// Servo parameters
#define SERVO_MIN_PULSE_WIDTH 150 // Min pulse length out of 4096
#define SERVO_MAX_PULSE_WIDTH 600 // Max pulse length out of 4096
#define SERVO_FREQUENCY 50 // Analog servos run at ~50 Hz updates

uint16_t pulsewidth;

/* STEPPER MOTOR CLASS IMPLEMEMTATION */
Stepper::Stepper(PinName StepPin_1, PinName DirPin_1, PinName StepPin_2, PinName DirPin_2,
                    PinName StepPin_3, PinName DirPin_3, PinName StepPin_4, PinName DirPin_4)
    : StepPin1(StepPin_1), DirPin1(DirPin_1), StepPin2(StepPin_2), DirPin2(DirPin_2),
        StepPin3(StepPin_3), DirPin3(DirPin_3), StepPin4(StepPin_4), DirPin4(DirPin_4){}

void Stepper::MoveStepper(int Mot_no, int Dir, int steps){

switch(Mot_no){
        case 1 :
            if(Dir == 1){
                DirPin1.write(1);
                for(int x = 0; x < steps; x++) {
                StepPin1.write(1); 
                thread_sleep_for(2);    
                StepPin1.write(0); 
                thread_sleep_for(2); 
                }
            }
            if(Dir == 0){
                DirPin1.write(0);
                for(int y = 0; y < steps; y++) {
                StepPin1.write(1); 
                thread_sleep_for(2);
                StepPin1.write(0); 
                thread_sleep_for(2);
                }
            }
            break;

        case 2 :
            if(Dir == 1){
                DirPin2.write(1);
                for(int x = 0; x < steps; x++) {
                StepPin2.write(1); 
                thread_sleep_for(2);    
                StepPin2.write(0); 
                thread_sleep_for(2); 
                }
            }
            if(Dir == 0){
                DirPin2.write(0);
                for(int y = 0; y < steps; y++) {
                StepPin2.write(1); 
                thread_sleep_for(2);
                StepPin2.write(0); 
                thread_sleep_for(2);
                }
            }
            break;

        case 3 :
            if(Dir == 1){
                DirPin3.write(1);
                for(int x = 0; x < steps; x++) {
                StepPin3.write(1); 
                thread_sleep_for(2);    
                StepPin3.write(0); 
                thread_sleep_for(2); 
                }
            }
            if(Dir == 0){
                DirPin3.write(0);
                for(int y = 0; y < steps; y++) {
                StepPin3.write(1); 
                thread_sleep_for(2);
                StepPin3.write(0); 
                thread_sleep_for(2);
                }
            }
            break;

        case 4 :
            if(Dir == 1){
                DirPin4.write(1);
                for(int x = 0; x < steps; x++) {
                StepPin4.write(1); 
                thread_sleep_for(2);    
                StepPin4.write(0); 
                thread_sleep_for(2); 
                }
            }
            if(Dir == 0){
                DirPin4.write(0);
                for(int y = 0; y < steps; y++) {
                StepPin4.write(1); 
                thread_sleep_for(2);
                StepPin4.write(0); 
                thread_sleep_for(2);
                }
            }
            break;
        
    }

}

/* DC MOTOR CLASS IMPLEMEMTATION */
DC::DC(PinName EN_1, PinName EN_2, PinName IN_1, PinName IN_2, PinName IN_3, PinName IN_4)
    : EN1(EN_1), EN2(EN_2), IN1(IN_1), IN2(IN_2),IN3(IN_3), IN4(IN_4){}

void DC::MoveDC(int Mot_no, int Dir, float Duty_Cycle){

    switch(Mot_no){
        case 1:
        if(Dir == 1){
            IN1.write(1);
            IN2.write(0);
            EN1.period(0.01f);      
            EN1.write(Duty_Cycle);      
        }
        if(Dir == 0){
            IN1.write(0);
            IN2.write(1);
            EN1.period(0.01f);      
            EN1.write(Duty_Cycle);
        }
        break;

        case 2:
        if(Dir == 1){
            IN3.write(1);
            IN4.write(0);
            EN2.period(0.01f);      
            EN2.write(Duty_Cycle);
        }
        if(Dir == 0){
            IN3.write(0);
            IN4.write(1);
            EN2.period(0.01f);      
            EN2.write(Duty_Cycle);  
        }
        break;
    }
}

/* SERVO MOTOR CLASS IMPLEMEMTATION */

/* SERVO MOTOR CLASS IMPLEMEMTATION */
Servo::Servo(I2C *i2c, uint8_t addr) {
  _i2c = i2c;
  _i2caddr = addr << 1;

}

void Servo::begin(void) {
    reset();
}

void Servo::reset(void) {
 write8(PCA9685_MODE1, 0x1);

}

void Servo::setPWMFreq(float freq) {
  
  float prescaleval = 25000000;
  prescaleval /= 4096;
  prescaleval /= freq;
  prescaleval -= 1;

  uint8_t prescale = floor(prescaleval + 0.5);
  
  uint8_t oldmode = read8(PCA9685_MODE1);
  uint8_t newmode = (oldmode&0x7F) | 0x10; // sleep
  write8(PCA9685_MODE1, newmode); // go to sleep
  write8(PCA9685_PRESCALE, prescale); // set the prescaler
  write8(PCA9685_MODE1, oldmode);
  thread_sleep_for(5);
  write8(PCA9685_MODE1, oldmode | 0xa1);  

}

void Servo::setPWM(uint8_t servonum, uint16_t on, uint16_t degree) {
  
  pulsewidth = ((degree / 180.0) * (SERVO_MAX_PULSE_WIDTH - SERVO_MIN_PULSE_WIDTH) + SERVO_MIN_PULSE_WIDTH);

  uint8_t data[] = { LED0_ON_L+4*servonum, on, on >> 8, pulsewidth, pulsewidth >> 8 };

  _i2c->write(_i2caddr, (const char *)data, 5);
  
}

uint8_t Servo::read8(uint8_t addr) {
    char data;
    if(_i2c->write(_i2caddr, (char *)&addr, 1, true))
        // printf("I2C ERR: no ack on write before read.\n");
    if(_i2c->read(_i2caddr, &data, 1))
        // printf("I2C ERR: no ack on read\n");
    return (uint8_t)data;
}

void Servo::write8(uint8_t addr, uint8_t d) {
    char data[] = { addr, d };
    if(_i2c->write(_i2caddr, data, 2))
    {    
        // printf("I2C ERR: No ACK on i2c write!");
    }
}

/* Class for Music */
Music::Music(PinName StepPin_1, PinName DirPin_1)
    : StepPin1music(StepPin_1), DirPin1music(DirPin_1){}

void Music::PlayMusic(int pulseCount, float noteDurationMs, float stepDelay){

    DirPin1music.write(1);  // Set the direction

    for(int i = 0; i < pulseCount; i++) {
        StepPin1music.write(1); 
        _wait_us_inline(stepDelay * 1000);  // Wait for the calculated delay
        StepPin1music.write(0); 
        wait_us(stepDelay * 1000);  // Wait for the calculated delay
    }

    thread_sleep_for(noteDurationMs);  // Wait between notes

}

