#include <16f877a.h>
#include <def_877a.h>

#device *=16 ADC=8
#FUSES NOWDT, HS, NOPUT, NODEBUG, NOBROWNOUT, NOLVP, NOCPD, NOWRT
#use delay(clock = 20MHz)

#use i2c(MASTER, SDA=PIN_C4, SCL=PIN_C3, stream=I2C_LCD, FAST)
#include <I2C_LCD.c>

void main()
{
   delay_ms(300);          // ch? ngu?n ?n d?nh

   LCD_Begin(0x3E);        // 0x27 << 1
   Backlight();

   LCD_Cmd(LCD_CLEAR);
   delay_ms(10);

   LCD_Cmd(LCD_CURSOR_OFF);

   LCD_Goto(1,1);
   printf(LCD_Out, "I2C LCD TEST");

   LCD_Goto(1,2);
   printf(LCD_Out, "PIC16F877A");

   while(TRUE);            // d?ng yên
}

