#include <16f877a.h>
#include <def_877a.h>
#device *=16 ADC=8
#FUSES NOWDT, HS, NOPUT, NODEBUG, NOBROWNOUT, NOLVP, NOCPD, NOWRT
#use delay(clock = 20MHz)

#define DT1        PIN_C0
#define SCK        PIN_C1
#define RESET_BTN  PIN_A0
#define ONOFF_BTN  PIN_A1
#define LCD_POWER  PIN_A2

#use i2c(MASTER, SDA=PIN_C4, SCL=PIN_C3, stream=I2C_LCD, FAST)
#include <I2C_LCD.c>   // thu vi?n I2C b?n dã cung c?p

float SCALE = 880.0;   
int1 system_on = TRUE;

// -------- HX711 d?c 24-bit ----------
unsigned int32 readCount(void) {
   unsigned int32 data = 0;
   unsigned int8 j;
   output_bit(DT1, 1);
   output_bit(SCK, 0);
   while (input(DT1));        // ch? DRDY
   for (j = 0; j < 24; j++) {
      output_bit(SCK, 1);
      data = data << 1;
      output_bit(SCK, 0);
      if (input(DT1)) data++;
   }
   output_bit(SCK, 1);
   data = data ^ 0x800000;    // chuy?n mã
   output_bit(SCK, 0);
   return data;
}

int32 readAverage(void) {
   unsigned int32 sum = 0;
   int k;
   for (k = 0; k < 10; k++) {
      sum += readCount();
      delay_ms(10);
   }
   return sum / 10;
}
// -------------------------------------

void main() {
   setup_adc_ports(NO_ANALOGS);
   setup_adc(ADC_OFF);
   set_tris_a(0x03);   // RA0, RA1 input; RA2 output (LCD_POWER)
   // B?t ngu?n LCD module I2C (n?u b?n dùng chân di?u khi?n ngu?n)
   output_high(LCD_POWER);
   delay_ms(200);
   // Kh?i t?o LCD I2C: d?a ch? 0x4E (tuong ?ng 0x27 << 1).
   // N?u module c?a b?n là 0x3F thì dùng 0x7E.
   LCD_Begin(0x4E);
   delay_ms(100);
   LCD_Goto(3,1); printf(LCD_Out, "Loading");
   delay_ms(400);
   LCD_Goto(3,1); printf(LCD_Out, "Loading.");
   delay_ms(400);
   LCD_Goto(3,1); printf(LCD_Out, "Loading..");
   delay_ms(400);
   LCD_Goto(3,1); printf(LCD_Out, "Loading...");
   delay_ms(400);

   {
      char name1[] = "Chu Tung Anh";
      int8 i;
      LCD_Goto(3,1);
      for (i = 0; name1[i] != '\0'; i++) {
         LCD_Out(name1[i]);
         delay_ms(120);
      }
      delay_ms(500);

      char name2[] = "Can dien tu";
      LCD_Goto(3,2);
      for (i = 0; name2[i] != '\0'; i++) {
         LCD_Out(name2[i]);
         delay_ms(120);
      }
      delay_ms(1000);
   }

   // d?c offset ban d?u
   unsigned int32 offset = readAverage();

   LCD_Goto(3,1); 
   printf(LCD_Out, "                  "); 
   LCD_Goto(3,2); 
   printf(LCD_Out, "                  "); 
   LCD_Goto(4,1); 
   printf(LCD_Out, "San sang!");
   delay_ms(1000);
   LCD_Goto(4,1);
   printf(LCD_Out,"CAN NANG:");
    float last_grams = -1.0;

   while(TRUE) {
   
       /* ===== NÚT ON/OFF ===== */
       if (!input(ONOFF_BTN)) {
           delay_ms(30);
           if (!input(ONOFF_BTN)) {
               system_on = !system_on;
   
               if (system_on) {
                   output_high(LCD_POWER);
                   delay_ms(150);
                   LCD_Begin(0x4E);
                   offset = readAverage();
   
                   LCD_Goto(4,1);
                   printf(LCD_Out,"CAN NANG:");
} else {
                   output_low(LCD_POWER);
               }
   
               while (!input(ONOFF_BTN));
               delay_ms(50);
           }
       }
   
       if (!system_on) {
           delay_ms(100);
           continue;
       }
   
       /* ===== NÚT RESET (TARE) ===== */
       if (!input(RESET_BTN)) {
           delay_ms(30);
           if (!input(RESET_BTN)) {
               offset = readAverage();   // tare
               last_grams = -1.0;
               while (!input(RESET_BTN));
               delay_ms(50);
           }
       }
   
       /* ===== Ð?C CÂN ===== */
       unsigned int32 raw = readAverage();
       signed int32 delta = (signed int32)raw - (signed int32)offset;
       if (delta < 0) delta = 0;
   
       float grams = (float)delta / SCALE;
   
       /* ===== CH?NG RUNG + NGU?NG ===== */
       if (grams < 0.2) grams = 0.0;          // dead zone
       grams = ((int)(grams * 10 + 0.5)) / 10.0;  // làm tròn 0.1g
   
       float diff = grams - last_grams;
       if (diff < 0) diff = -diff;
   
       /* ===== HI?N TH? ===== */
       if (diff >= 0.1) {
           LCD_Goto(4,2);
           printf(LCD_Out,"      ");
           LCD_Goto(4,2);
           printf(LCD_Out,"%4.1f g", grams);
           last_grams = grams;
       }
   
       delay_ms(300);
   }

}
