#include "OLED_Display.h" 

OLED_Display::OLED_Display(PinName sda, PinName scl) : i2c(sda, scl) {} 

void OLED_Display::begin() { 
    i2c.frequency(400000); 
    turnON(); 
//setInversDisplayMode();
    setNormalDisplayMode();
    setPageMode(); 
    writeCommand(0xA7); // Invert display
    writeCommand(0x8D); // Charge Pump 
    writeCommand(0x14); 
    clearDisplay(); 
} 
void OLED_Display:: print_string(string string,char x,char y)
{
       for(int i=0; i< string.length();i++)
       {
             char ch  = string[i];
             
             char x_cord = x+ i* Char_Horizontal_Columns_Required;
             
             print_char(ch,x_cord,y);
        }
}

void OLED_Display:: print_string_logo(string string,char x,char y)
{
       for(int i=0; i< string.length();i++)
       {
             char ch  = string[i];
             
             char x_cord = x+ i* Char_Horizontal_Columns_Required_l;
             
             print_logo(ch,x_cord,y);
        }
}

void OLED_Display::writeCommand(uint8_t command) { 
    char data[2]; 
    data[0] = COMMAND_REG; 
    data[1] = command; 
    i2c.write(OLED_I2C_ADDRESS << 1, data, 2); 
} 

void OLED_Display::writeData(uint8_t data) { 
    char buffer[2]; 
    buffer[0] = DATA_REG; 
    buffer[1] = data; 
    i2c.write(OLED_I2C_ADDRESS << 1, buffer, 2); 
} 

void OLED_Display::turnON() { 
    writeCommand(ON_CMD); 
} 

void OLED_Display::setNormalDisplayMode() { 
    writeCommand(NORMAL_DISPLAY_CMD); 

} 
void OLED_Display::setInversDisplayMode() { 
    writeCommand(INVS_DISPLAY_CMD); 

} 

void OLED_Display::setPageMode() { 
    writeCommand(0x20); // Set addressing mode 
    writeCommand(PAGE_ADDRESSING_MODE); // Set page addressing mode 
} 

void OLED_Display::clearDisplay() { 
    for (uint8_t page = 0; page < 8; page++) { 
        setCursor(0, page); 
        for (uint8_t column = 0; column < 128; column++) { 
            writeData(0x00); 
        } 
    } 
    setCursor(0, 0); 
} 

void OLED_Display::setCursor(uint8_t x, uint8_t y) { 
    writeCommand(0x00 + (x & 0x0F)); // Set column lower address 
    writeCommand(0x10 + ((x >> 4) & 0x0F)); // Set column higher address 
    writeCommand(0xB0 + y); // Set page address 
} 

void OLED_Display::writeText(const char* text) { 
    while (*text) { 
        char c = *text++; 
        if (c < 0x20 || c > 0x7E) c = 0x20; // Replace non-printable characters with space 
        for (int i = 0; i < 5; i++) { 
            writeData(font[(c - 0x20) * 5 + i]); 
        } 
        writeData(0x00); // Add a column of space between characters 
    } 
} 
void OLED_Display:: print_char(char ch, char x_cord, char y_cord)
{
  char ascii_to_array_index = ch - First_char_ascii_code;
  int char_start_in_array  = ascii_to_array_index * No_of_bytes_Char + 1;
  
  for (char j = x_cord; j < x_cord + Char_Horizontal_Columns_Required; j++)
  {
    for (char i = y_cord; i < y_cord + Char_Verticle_Pages_Required; i++)
    {
      setCursor(j, i);
      int array_index = char_start_in_array + (j - x_cord) * Char_Verticle_Pages_Required + (i - y_cord);
      char char_byte= font_char[array_index];
      writeData(char_byte);
    }
  }
}
void OLED_Display:: print_logo(char ch, char x_cord, char y_cord)
{
  char ascii_to_array_index = ch - First_char_ascii_code_l;
  int char_start_in_array  = ascii_to_array_index * No_of_bytes_Char_l + 1;
  
  for (char j = x_cord; j < x_cord + Char_Horizontal_Columns_Required_l; j++)
  {
    for (char i = y_cord; i < y_cord + Char_Verticle_Pages_Required_l; i++)
    {
      setCursor(j, i);
      int array_index = char_start_in_array + (j - x_cord) * Char_Verticle_Pages_Required_l + (i - y_cord);
      char char_byte= font[array_index];
      writeData(char_byte);
    }
  }
}

void OLED_Display::drawBasicPattern() {
    // Example to draw a simple 8x8 square at the top-left corner
    const int pattern_width = 8;
    const int pattern_height = 8;
    char pattern[64] = {   0x08, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF    }; // 8x8 full of 1s

    for (int y = 0; y < pattern_height; y++) {
        for (int x = 0; x < pattern_width; x++) {
            int index = y * pattern_width + x;
            setCursor(x, y);
            writeData(pattern[index]);
        }
    }
}




void OLED_Display::drawSprite(const char sprite[], int char_h , int char_v) {
    for (int j = 0; j < char_h; j++) {
      
        for ( int  i = 0; i < char_v; i++) {
              setCursor(j,i); // Set cursor to the beginning of the line
            // Write each byte from the sprite array to the display
            writeData(sprite[j* char_v +i]);
        }
    }
}



/*void OLED_Display::drawSprite(const int sprite[24][24], uint8_t spriteWidth, uint8_t spriteHeight) {
    for (uint8_t y = 0; y < spriteHeight; y++) {
        setCursor(0, y); // Set cursor to the beginning of the line
        for (uint8_t x = 0; x < spriteWidth; x++) {
            // Write each byte from the sprite array to the display
            writeData(sprite[y][x]);
        }
    }
}*/