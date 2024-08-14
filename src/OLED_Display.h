#ifndef OLED_DISPLAY_H 
#define OLED_DISPLAY_H 

#include "mbed.h" 
#include "glcdfont.h" 
//#include "glcdfont_char.h" 


class OLED_Display { 
public: 
    OLED_Display(PinName sda, PinName scl); 

    void begin(); 
    void clearDisplay(); 
    void setCursor(uint8_t x, uint8_t y); 
    void writeText(const char* text); 
        void print_char(char ch, char x_cord, char y_cord); 
        void print_logo(char ch, char x_cord, char y_cord);
         

        void print_string(string string,char x,char y); 
                void print_string_logo(string string,char x,char y); 


    void drawSprite(const char sprite[], int char_h , int char_v);  // New method
    void drawBasicPattern();  // New method

private: 
    I2C i2c; 
    void writeCommand(uint8_t command); 
    void writeData(uint8_t data); 
    void turnON(); 
    void setNormalDisplayMode(); 
    void setInversDisplayMode(); 

    void setPageMode(); 

    static const uint8_t OLED_I2C_ADDRESS = 0x3C; 
    static const uint8_t COMMAND_REG = 0x80; 
    static const uint8_t DATA_REG = 0x40; 
    static const uint8_t ON_CMD = 0xAF; 
    static const uint8_t NORMAL_DISPLAY_CMD = 0xA6; 
        static const uint8_t INVS_DISPLAY_CMD = 0xA7; 
   static const int Char_Verticle_Pages_Required = 2;
   static const int Char_Horizontal_Columns_Required = 10;
   static const int Char_Verticle_Pages_Required_l = 6;
   static const int Char_Horizontal_Columns_Required_l = 60;

    static const uint8_t PAGE_ADDRESSING_MODE = 0x02; 

  static const int  First_char_ascii_code = 32 ;
    static const int  No_of_bytes_Char = 21 ;
    static const int  First_char_ascii_code_l = 32 ;
    static const int  No_of_bytes_Char_l = 360 ;

}; 

#endif 
