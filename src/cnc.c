#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

typedef enum {
   chuck_main,
   chuck_back,
   oil
} Relay;

typedef struct {
   double turning_speed;
   double cutoff_speed;
   double drilling_speed;
   double milling_speed;
} Material;

const Material STAINLESS_303 = {
   .turning_speed = 0.001,
   .cutoff_speed = 0.001,
   .drilling_speed = 0.001,
   .milling_speed = 0.001,
};
Material cnc_material;

typedef double ToolSize;

void cnc_toggle(Relay relay, bool state){
   switch(relay) {
      case chuck_main: state ? puts("G30") : puts("G50"); break;
      case chuck_back: state ? puts("G12") : puts("G10"); break; 
      case oil:        state ? puts("G12") : puts("G10"); break; 
   }
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
   if(index == 0){
      fprintf(stderr, "ERROR: cnc_move instruction doesn't move anything.\n");
      exit(1);
   }


   if(index > 2){
      fprintf(stderr, "ERROR: maximum of 2 axis may be moved at a time.\n");
      exit(1);
   }

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
   puts("");
}

void cnc_set_material(Material material){
   cnc_material = material;
}

void cnc_mill_hex(ToolSize size, double width_across_flats, double length){
   printf("(insert code for milling a hex here)\r\n");    
}

#define PICKOFF 1
void cnc_cutoff(ToolSize size, unsigned flags){
    
}

void cnc_reset(){
   printf("M32\r\n");
}

void cnc_standard_setup(){
   printf(
      "(Program Start)\r\n\r\n"
      "M52\r\n"
      "M9\r\n"
      "G50Z0\r\n"
      "M6\r\n\r\n"
   );
}

void cnc_begin_thread(unsigned id){
   printf("$%i\r\n\r\n", id);
}

int main(){
   ToolSize mill = 0.500;
   ToolSize cutoff = 0.050;
   cnc_set_material(STAINLESS_303);

   cnc_begin_thread(1); 
   cnc_standard_setup(); 

   cnc_toggle(oil, true);
   cnc_move(0, 0.001, 0.002, RAPID | RELATIVE);
   cnc_move(0, 0.001, 0.002, DEFAULT);
   cnc_move(0, -0.500, 0, DEFAULT);
   cnc_mill_hex(mill, 0.5, .25);
   cnc_move(0, 0.5, 0, DEFAULT);
   cnc_cutoff(cutoff, PICKOFF);
   cnc_reset();

   cnc_begin_thread(2); 
   cnc_reset();
   
   return 0;
}
