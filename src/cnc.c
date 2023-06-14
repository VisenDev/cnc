#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>

typedef enum {
   chuck_main,
   chuck_back,
   oil
} Relay;

void cnc_toggle(Relay relay, bool state){
   switch(relay) {
      case chuck_main: state ? puts("G30") : puts("G50"); break;
      case chuck_back: state ? puts("G12") : puts("G10"); break; 
      case oil:        state ? puts("G12") : puts("G10"); break; 
   }
}

void cnc_set(int id, int value){
   
}

#define DEFAULT 0
#define RAPID 1
#define RELATIVE 2
void cnc_move(double x, double y, double z, unsigned flags){ 
   
   unsigned index = 0;
   struct {
      char id;
      double value;
   } axis[3]; 
   
   if(x != 0){
      axis[index].id = 'X';
      axis[index].value = x;
      ++index;
   }
   
   if(y != 0){
      axis[index].id = 'Y';
      axis[index].value = y;
      ++index;
   }

   if(z != 0){
      axis[index].id = 'Z';
      axis[index].value = z;
      ++index;
   }

   //Error checking
   if(index == 0)
      fprintf(stderr, "ERROR: cnc_move called with no values other than 0.");

   if(index > 2)
      fprintf(stderr, "ERROR: maximum of 2 axis may be moved at a time.");

   //relative movement
   for(int i = 0; i < index; ++i){
      if(flags & RELATIVE) 
         axis[i].id -= 3;
   }
   
   //output
   printf("G%i ", flags & RAPID);
   for(int i = 0; i < index; ++i){
      printf("%c%.3f ", axis[i].id, axis[i].value);
   }
   puts("\n");
}

void cnc_reset(){
   puts("M32");
}

void cnc_note(const char * str){
   printf("\n(%s)\n", str);
}

int main(){
   cnc_note("Program start");
   cnc_toggle(oil, true);
   cnc_move(0, 0.001, 0.002, RAPID | RELATIVE);
   cnc_move(0, 0.001, 0.002, DEFAULT);
   cnc_move(0, -0.500, 0, DEFAULT);
   cnc_reset();
   
   return 0;
}
