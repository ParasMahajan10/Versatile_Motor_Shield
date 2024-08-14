#include "mbed.h"
#include "VMShield.h"
#include "OLED_Display.h"   // Include your OLED library header

// INITIALIZATIONS

// Buttons
DigitalIn Button1(PC_8); // B1
DigitalIn Button2(PC_6); // B2
DigitalIn Button3(PC_5); // B3
DigitalIn Button4(PA_12); // B4

// Button State Variables
bool B1_State = false;
bool B2_State = false;
bool B3_State = false;
bool B4_State = false;

// BLDC PWM 1
PwmOut pwmPin(PB_1);
AnalogIn pot(PA_0); // A0

// PCA9685 Definitions
#define PCA9685_ADDRESS 0x40
#define SERVO_FREQUENCY 50
#define I2C_SDA PB_9
#define I2C_SCL PB_8

// OLED Definitions (for I2C3)
#define OLED_SDA PC_9
#define OLED_SCL PA_8

// PWM frequency (50Hz) for BLDC
const float pwmFrequency = 50.0f;

// Min and Max pulse widths in seconds (1ms to 2ms)
const float minPulseWidth = 0.0012f;
const float maxPulseWidth = 0.0018f;

// I2C frequency (in Hz)
#define I2C_FREQUENCY 100000

// Bluetooth Serial (TX: PA_9 -> D8, RX: PA_10 -> D2)
BufferedSerial bluetooth(PA_9, PA_10, 9600); // TX, RX (assuming UART pins)

// Motor object creation
Stepper MyStepper(PA_6, PA_5, PB_6, PA_7, PB_13, PC_7, PB_10, PA_8);
DC MyDC(PB_5, PB_4, PC_2, PC_3, PC_12, PC_10);

// Initialize I2C1 for Servos
I2C i2c1(I2C_SDA, I2C_SCL);
Servo MyServo(&i2c1, PCA9685_ADDRESS);

// Music object creation
Music Playit(PA_6, PA_5);

// Initialize I2C3 for OLED
// I2C i2c3(OLED_SDA, OLED_SCL);
OLED_Display oled(OLED_SDA,OLED_SCL);

// I2C Scanner to check if PCA9685 is detected
void i2cScanner(I2C &i2c) {}

// Buffer to hold incoming data
const int BUFFER_SIZE = 100;
char buffer[BUFFER_SIZE];
int bufferIndex = 0;

// Variables to hold the parsed data
int MotTypeCode = 0;
int MotNo = 0;
int Param1 = 0;
int Param2 = 0;
float DTC;
float pulseWidth;
float ParampulseWidth;

// THREADS
Thread thread_stepper1;
Thread thread_stepper2;
Thread thread_dc1;
Thread thread_dc2;
Thread thread_bldc1;
Thread thread_bluetooth;
Thread thread_servos;
Thread thread_for_music;

// Function for palying music
void playNote(int pulseCount, float noteDurationMs, float frequencyHz) {
    float stepDelay = 1000.0f / (frequencyHz * 2.0f);  // Delay for the desired frequency

    Playit.PlayMusic(pulseCount,  noteDurationMs, stepDelay);

}

// Function to process received string
void processString(char *str) {
    // Parse the string
    char temp[3];

    // Extract first two digits
    temp[0] = str[0];
    temp[1] = str[1];
    temp[2] = '\0';
    MotTypeCode = atoi(temp);

    switch (MotTypeCode)
    {
    case 14: // 14 for Stepper Motor
        // Extract Stepper Motor N0
        MotNo = str[2] - '0';

        // Extract the Direction
        Param1 = str[3] - '0';

        // Extract the Steps
        Param2 = atoi(&str[4]);

        // Execute the function
        MyStepper.MoveStepper(MotNo, Param1, Param2);
        break;

    case 24: // 24 for Servo Motor
        {
        // Extract Servo Motor
        char temp1[3];

        // Extract Mot no
        temp1[0] = str[2];
        temp1[1] = str[3];
        temp1[2] = '\0';
        MotNo = atoi(temp1);

        // Extract the Degree
        Param1 = atoi(&str[4]);

        i2cScanner(i2c1);
        Servo MyServo(&i2c1, PCA9685_ADDRESS);

        MyServo.begin();
        MyServo.setPWMFreq(SERVO_FREQUENCY);

        // Execute the function
        for (int i = 0; i < Param1; i++) {
            MyServo.setPWM(MotNo, 0, i);
            ThisThread::sleep_for(3ms);
        }
        }
        break;

    case 34: // 34 for DC Motor
        {
        // Extract Motor No
        MotNo = str[2] - '0';

        // Extract the Direction
        Param1 = str[3] - '0';

        // Extract the Duty Cycle
        Param2 = atoi(&str[4]);

        DTC = Param2 / 100.0f;
        // Execute the function
        MyDC.MoveDC(MotNo, Param1, DTC);
        }
        break;

    case 44: // 44 for BLDC Motor
        // Extract Motor No
        MotNo = str[2] - '0';

        // Extract the Throttle
        Param1 = atoi(&str[3]);

        float temp2 = Param1 / 1000.0f;

        // Calculate the pulse width based on the potentiometer value
        ParampulseWidth = minPulseWidth + (temp2 * (maxPulseWidth - minPulseWidth));

        // Set the PWM duty cycle based on the calculated pulse width
        pwmPin.pulsewidth(ParampulseWidth);

        break;
    }
}

// Thread for receiving Bluetooth data and controlling LED
void bluetoothThread() {
    // Initialize I2C1 for Servos
    I2C i2c1(I2C_SDA, I2C_SCL);
    i2c1.frequency(I2C_FREQUENCY);

    // Scan for I2C devices
    i2cScanner(i2c1);
    Servo MyServo(&i2c1, PCA9685_ADDRESS);

    MyServo.begin();
    MyServo.setPWMFreq(SERVO_FREQUENCY);

    while (true) {
        // Read data from Bluetooth module
        if (bluetooth.readable()) {
            char recv;
            bluetooth.read(&recv, 1);

            if (recv == '\n' || recv == '\r') { // End of string
                buffer[bufferIndex] = '\0'; // Null-terminate the string
                processString(buffer); // Process the received string
                bufferIndex = 0; // Reset buffer index for next message
            } else if (bufferIndex < BUFFER_SIZE - 1) { // Add character to buffer
                buffer[bufferIndex++] = recv;
            }
        }
        ThisThread::sleep_for(10ms); // Add a small delay to avoid CPU hogging
    }
}

// Function to stop all threads
void All_stop() {
    thread_stepper1.terminate();
    thread_stepper2.terminate();
    thread_dc1.terminate();
    thread_dc2.terminate();
    thread_bldc1.terminate();
    thread_servos.terminate();

    ThisThread::sleep_for(200ms);

    float temp2 = 200 / 1000.0f;
    // Calculate the pulse width based on the potentiometer value
    ParampulseWidth = minPulseWidth + (temp2 * (maxPulseWidth - minPulseWidth));
    // Set the PWM duty cycle based on the calculated pulse width
    pwmPin.pulsewidth(ParampulseWidth);

    MyDC.MoveDC(1, 0, 0.00f);
    ThisThread::sleep_for(200ms);
    MyDC.MoveDC(2, 0, 0.00f);
    ThisThread::sleep_for(200ms);
}

// Thread for Stepper 1
void thread_stepper_1() {
    while (1) {
        MyStepper.MoveStepper(1, 1, 200);
        ThisThread::sleep_for(1000ms);
        MyStepper.MoveStepper(1, 1, 800);
        ThisThread::sleep_for(1000ms);
        MyStepper.MoveStepper(1, 0, 50);
        ThisThread::sleep_for(1000ms);
        MyStepper.MoveStepper(1, 0, 100);
        ThisThread::sleep_for(1000ms);
    }
}

// Thread for Stepper 2
void thread_stepper_2() {
    while (1) {
        MyStepper.MoveStepper(2, 1, 200);
        ThisThread::sleep_for(1000ms);
        MyStepper.MoveStepper(2, 1, 800);
        ThisThread::sleep_for(1000ms);
        MyStepper.MoveStepper(2, 0, 50);
        ThisThread::sleep_for(1000ms);
        MyStepper.MoveStepper(2, 0, 100);
        ThisThread::sleep_for(1000ms);
    }
}

// Thread for DC 1
void thread_dc_1() {
    while (1) {
        MyDC.MoveDC(1, 1, 0.40f);
        ThisThread::sleep_for(4000ms);
        MyDC.MoveDC(1, 0, 0.40f);
        ThisThread::sleep_for(4000ms);
        MyDC.MoveDC(1, 0, 0.00f);
        ThisThread::sleep_for(2000ms);
    }
}

// Thread for DC 2
void thread_dc_2() {
    while (1) {
        MyDC.MoveDC(2, 1, 0.40f);
        ThisThread::sleep_for(200ms);
        MyDC.MoveDC(2, 0, 0.40f);
        ThisThread::sleep_for(200ms);
        MyDC.MoveDC(2, 0, 0.00f);
        ThisThread::sleep_for(2000ms);
    }
}

// Thread for servos
void thread_servo_1() {
    while (1) {
        // Positive rotation
        for (int i = 0; i < 180; i++) {
            MyServo.setPWM(0, 0, i);
            MyServo.setPWM(1, 0, i);
            MyServo.setPWM(2, 0, i);
            MyServo.setPWM(3, 0, i);
            MyServo.setPWM(4, 0, i);
            MyServo.setPWM(5, 0, i);
            thread_sleep_for(3);
        }

        thread_sleep_for(500);

        // Negative rotation
        for (int i = 180; i > 0; i--) {
            MyServo.setPWM(0, 0, i);
            MyServo.setPWM(1, 0, i);
            MyServo.setPWM(2, 0, i);
            MyServo.setPWM(3, 0, i);
            MyServo.setPWM(4, 0, i);
            MyServo.setPWM(5, 0, i);
            thread_sleep_for(3);
        }

        thread_sleep_for(500);
    }
}

// Thread for BLDC 1
void thread_bldc_1() {
    // Set the PWM frequency
    pwmPin.period(1.0f / pwmFrequency);

    while (true) {
        // Read the potentiometer value (0.0 to 1.0)
        float potValue = pot.read();

        // Calculate the pulse width based on the potentiometer value
        float pulseWidth = minPulseWidth + (potValue * (maxPulseWidth - minPulseWidth));

        // Set the PWM duty cycle based on the calculated pulse width
        pwmPin.pulsewidth(pulseWidth);

        // Small delay to debounce the potentiometer reading
        thread_sleep_for(10);  // 10ms delay
    }
}

// Thread for music
void thread_music(){
    while(1) {

        // Pirates of Caribbean
        thread_sleep_for(20); // Rest
        playNote(100, 50, 293.66); // D
        playNote(100, 50, 293.66); // D
        playNote(100, 0, 220.00); // A <- Start
        playNote(100, 0, 261.63); // C
        playNote(100, 10, 293.66); // D
        playNote(100, 10, 293.66); // D
        playNote(100, 0, 293.66); // D
        playNote(100, 0, 329.63); // E
        playNote(100, 10, 349.23); // F
        playNote(100, 10, 349.23); // F
        playNote(100, 0, 349.23); // F
        playNote(100, 0, 392.00); // G
        playNote(100, 10, 329.63); // E
        playNote(100, 10, 329.63); // E
        playNote(100, 0, 293.66); // D
        playNote(100, 0, 261.63); // C
        playNote(100, 0, 261.63); // C
        playNote(100, 10, 293.66); // D <End
        thread_sleep_for(30); // Rest<<<<<<<<<<<<<<<<<<<<<<
        playNote(100, 0, 220.00); // A <- Start
        playNote(100, 0, 261.63); // C
        playNote(100, 10, 293.66); // D
        playNote(100, 10, 293.66); // D
        playNote(100, 0, 293.66); // D
        playNote(100, 0, 329.63); // E
        playNote(100, 10, 349.23); // F
        playNote(100, 10, 349.23); // F
        playNote(100, 0, 349.23); // F
        playNote(100, 0, 392.00); // G
        playNote(100, 10, 329.63); // E
        playNote(100, 10, 329.63); // E
        playNote(100, 0, 293.66); // D
        playNote(100, 0, 261.63); // C
        playNote(100, 0, 261.63); // C
        playNote(100, 10, 293.66); // D <End
        thread_sleep_for(30); // Rest<<<<<<<<<<<<<<<<<<<<<<
        playNote(100, 0, 220.00); // A
        playNote(100, 0, 261.63); // C
        playNote(100, 10, 293.66); // D
        playNote(100, 10, 293.66); // D
        playNote(100, 0, 293.66); // D
        playNote(100, 0, 349.23); // F
        playNote(100, 10, 392.00); // G
        playNote(100, 10, 392.00); // G
        playNote(100, 0, 392.00); // G
        playNote(100, 0, 440.00); // A
        playNote(100, 10, 466.16); // A#
        playNote(100, 10, 466.16); // A#
        playNote(100, 0, 440.00); // A
        playNote(100, 0, 392.00); // G
        playNote(100, 0, 440.00); // A
        playNote(100, 10, 293.66); // D
        playNote(100, 0, 293.66); // D
        playNote(100, 0, 329.63); // E
        playNote(100, 0, 349.23); // F
        playNote(100, 0, 392.00); // G
        playNote(100, 0, 293.66); // D
        playNote(100, 0, 293.66); // D
        playNote(100, 0, 349.23); // F
        playNote(100, 0, 329.63); // E
        playNote(100, 0, 349.23); // F
        playNote(100, 0, 293.66); // D
        playNote(100, 0, 329.63); // E
        playNote(100, 0, 440.00); // A
        playNote(100, 0, 261.63); // C
        playNote(100, 0, 293.66); // D
        playNote(100, 0, 293.66); // D
        playNote(100, 0, 293.66); // D
        playNote(100, 0, 329.63); // E
        playNote(100, 0, 349.23); // F
        playNote(100, 0, 349.23); // F
        playNote(100, 0, 349.23); // F
        playNote(100, 0, 392.00); // G
        playNote(100, 0, 329.63); // E
        playNote(100, 0, 329.63); // E
        playNote(100, 0, 293.66); // D
        playNote(100, 0, 261.63); // C
        playNote(100, 0, 293.66); // D <-End

        thread_sleep_for(2000);

       // Beethoven
        // E E♭ E E♭ E
        playNote(80, 0, 659.25 / 2); // E (329.63 Hz)
        playNote(80, 0, 622.25 / 2); // E♭ (311.13 Hz)
        playNote(80, 0, 659.25 / 2); // E (329.63 Hz)
        playNote(80, 0, 622.25 / 2); // E♭ (311.13 Hz)
        playNote(80, 0, 659.25 / 2); // E (329.63 Hz)

        // B D C A
        playNote(80, 0, 493.88 / 2); // B (246.94 Hz)
        playNote(80, 0, 587.33 / 2); // D (293.66 Hz)
        playNote(80, 0, 523.25 / 2); // C (261.63 Hz)
        playNote(130, 60, 440.00 / 2); // A (220.00 Hz)

        // C E A B
        playNote(80, 0, 523.25 / 2); // C (261.63 Hz)
        playNote(80, 0, 659.25 / 2); // E (329.63 Hz)
        playNote(80, 0, 440.00 / 2); // A (220.00 Hz)
        playNote(130, 60, 493.88 / 2); // B (246.94 Hz)

        // E A B C
        playNote(80, 0, 659.25 / 2); // E (329.63 Hz)
        playNote(80, 0, 440.00 / 2); // A (220.00 Hz)
        playNote(80, 0, 493.88 / 2); // B (246.94 Hz)
        playNote(130, 60, 523.25 / 2); // C (261.63 Hz)

        // <- ##############
        playNote(80, 0, 659.25 / 2); // E (329.63 Hz)
        playNote(80, 0, 622.25 / 2); // E♭ (311.13 Hz)
        playNote(80, 0, 659.25 / 2); // E (329.63 Hz)
        playNote(80, 0, 622.25 / 2); // E♭ (311.13 Hz)
        playNote(80, 0, 659.25 / 2); // E (329.63 Hz)

        // B D C A
        playNote(80, 0, 493.88 / 2); // B (246.94 Hz)
        playNote(80, 0, 587.33 / 2); // D (293.66 Hz)
        playNote(80, 0, 523.25 / 2); // C (261.63 Hz)
        playNote(130, 60, 440.00 / 2); // A (220.00 Hz)

        // C E A B
        playNote(80, 0, 523.25 / 2); // C (261.63 Hz)
        playNote(80, 0, 659.25 / 2); // E (329.63 Hz)
        playNote(80, 0, 440.00 / 2); // A (220.00 Hz)
        playNote(130, 60, 493.88 / 2); // B (246.94 Hz)

        // E C B A
        playNote(80, 0, 659.25 / 2); // E (329.63 Hz)
        playNote(80, 0, 523.25 / 2); // C (261.63 Hz)
        playNote(80, 0, 493.88 / 2); // B (246.94 Hz)
        playNote(130, 60, 440.00 / 2); // A (220.00 Hz)

        // <- ##############

        thread_sleep_for(2000);
    
    }
}

// Button read thread
void ButtonThread() {

    bool lastButton1State = Button1.read();
    bool lastButton2State = Button2.read();

    while (1) {
        bool currentButton1State = Button1.read();
        bool currentButton2State = Button2.read();

        if (currentButton1State != lastButton1State && currentButton1State == 1) { // Buetooth task
            B1_State = !B1_State;
            if (B1_State) {
                // thread_bluetooth.start(bluetoothThread);
                thread_bluetooth.terminate();
            } else {
                // thread_bluetooth.terminate();
            }
        }

        if (currentButton2State != lastButton2State && currentButton2State == 1) { // RTOS parallel task
            B2_State = !B2_State;
            if (B2_State) {
                thread_stepper1.start(thread_stepper_1);
                thread_stepper2.start(thread_stepper_2);
                thread_dc1.start(thread_dc_1);
                thread_dc2.start(thread_dc_2);
                thread_bldc1.start(thread_bldc_1);

                Servo MyServo(&i2c1, PCA9685_ADDRESS);
                MyServo.begin();
                MyServo.setPWMFreq(SERVO_FREQUENCY);

                thread_servos.start(thread_servo_1);

                oled.clearDisplay(); 
                oled.setCursor(0,0);
                oled.print_string("RTOS",10,2);

            } else {
                All_stop();
                oled.clearDisplay(); 
                oled.setCursor(0,0);
                oled.print_string("MUSIC",10,2);
                ThisThread::sleep_for(4000ms);

                Servo MyServo(&i2c1, PCA9685_ADDRESS);
                MyServo.begin();
                MyServo.setPWMFreq(SERVO_FREQUENCY);

                thread_for_music.start(thread_music);
            }
        }

        lastButton1State = currentButton1State;
        lastButton2State = currentButton2State;

        ThisThread::sleep_for(50ms);
    }
}

int main() {

        float temp2 = 200 / 1000.0f;

        // Calculate the pulse width based on the potentiometer value
        ParampulseWidth = minPulseWidth + (temp2 * (maxPulseWidth - minPulseWidth));

        // Set the PWM duty cycle based on the calculated pulse width
        pwmPin.pulsewidth(ParampulseWidth);

    // Start Button thread
    Thread Thread_Button;
    Thread_Button.start(ButtonThread);

    thread_bluetooth.start(bluetoothThread);

    // Initialize OLED display
        oled.begin(); 
        ThisThread::sleep_for(100ms);
        oled.clearDisplay(); 
        oled.setCursor(0,0);
        oled.print_string("VMShield",10,2);

        ThisThread::sleep_for(5000ms);

        oled.clearDisplay(); 
        oled.setCursor(0,0);
        oled.print_string("Bluetooth",10,2);

    while (1) {
        ThisThread::sleep_for(1000ms); // Main thread sleeps, letting other threads run
    }
}
