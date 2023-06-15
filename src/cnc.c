#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

typedef enum {
   chuck_main,
   chuck_back,
   knock_out,
   oil,
   check_tool_break,
   part_counter,
   part_chute
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
Material cnc_material = {0};

typedef double ToolSize;

void cnc_toggle(Relay relay, bool state){
   switch(relay) {
      case chuck_main:           state ? printf("M06\r\n") : printf("M07\r\n"); break;
      case chuck_back:           state ? printf("M16\r\n") : printf("M17\r\n"); break;
      case knock_out:            state ? printf("M10\r\n") : printf("M11\r\n"); break;
      case oil:                  state ? printf("M52\r\n") : printf("M53\r\n"); break;
      case part_counter:                 printf("M56\r\n");                     break;
      case check_tool_break:             printf("M51\r\n");                     break;
      case part_chute:           state ? printf("M32\r\n") : printf("M33\r\n"); break;
   }
}


typedef enum {
   spindle_main,
   spindle_back,
   spindle_tool,
} Spindle;

typedef enum {
   forward,
   reverse,
   stop 
} SpindleStatus;

typedef enum {
   per_rotation,
   per_minute
} SpindleFeedType;

void cnc_spindle_set(Spindle spindle, SpindleStatus status, SpindleFeedType feedtype, double feedrate){
   switch(spindle){
   case spindle_main:
      switch(status){
         case forward: printf("M3 S1=%.3f %s\r\n", feedrate, feedtype == per_minute ? "G98" : "G99"); break;
         case reverse: printf("M4 S1=%.3f %s\r\n", feedrate, feedtype == per_minute ? "G98" : "G99"); break;
         case stop: printf("M5\r\n"); break;
      }
      break;
   case spindle_back:
      switch(status){
         case forward: printf("M23 S2=%.3f %s\r\n", feedrate, feedtype == per_minute ? "G98" : "G99"); break;
         case reverse: printf("M24 S2=%.3f %s\r\n", feedrate, feedtype == per_minute ? "G98" : "G99"); break;
         case stop: printf("M25\r\n"); break;
      }
      break;
   case spindle_tool:
      switch(status){
         case forward: printf("M80 S3=%.3f %s\r\n", feedrate, feedtype == per_minute ? "G98" : "G99"); break;
         case reverse: printf("M81 S3=%.3f %s\r\n", feedrate, feedtype == per_minute ? "G98" : "G99"); break;
         case stop: printf("M82\r\n");
      }

   }
}

void cnc_sleep(double seconds){
   printf("G04U%.3f\r\n", seconds);
}

#define DEFAULT 0
#define RAPID 1
#define RELATIVE 2
#define CUT 4
#define SKIP_NEWLINE 8
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

   //main output
   printf("G%i ", flags & RAPID);
   for(int i = 0; i < index; ++i){
      printf("%c%.3f ", axis[i].id, axis[i].value);
   }

   if(!(flags & SKIP_NEWLINE)){
      printf("\r\n");
   }
   
}

void cnc_turn(double x, double y, double z, double feedrate, unsigned flags){ 
   printf("G98 ");
   cnc_move(x, y, z, flags | SKIP_NEWLINE);
   printf("F %.3f", feedrate);
   printf("\r\n");
}


#define OVERSIZED_MILL 1
void cnc_mill_hex(ToolSize size, double width_across_flats, double length, unsigned flags){
   printf("\r\n(TODO: insert code for milling a hex here)\r\n\r\n");    
}

#define PICKOFF 1
void cnc_cutoff(double feedrate, unsigned flags){
   printf(
      "\r\n"
      "(CUTOFF)\r\n"
      "S1=2500\r\n"
      "G814\r\n"
      "T0404Z.993X.475\r\n"
      "M88\r\n"
      "G650\r\n"
      "!2L650\r\n"
      "G1X.197F.010\r\n"
      "G1X-.010F.001\r\n"
      "G813\r\n"
      "G600\r\n"
      "M89\r\n"
      "G1X-.100F.0015\r\n\r\n"
   );
}

void cnc_set_standard_machining_data(){
   printf(
      "\r\n"
      "$0\r\n"
      "D\r\n"
      "#016=0000003750\r\n"
      "#020=0000001000\r\n"
      "#024=0000018000\r\n"
      "#028=0000001000\r\n"
      "#032=0000004000\r\n"
      "#036=0002500000\r\n"
      "#040=0000000010\r\n"
      "#044=-000001000\r\n"
      "#048=0000011000\r\n"
      "#052=0000002000\r\n"
      "#064=0000201000\r\n"
      "#068=0000000000\r\n"
      "%%\r\n"
   );
}

void cnc_faceoff_material(double length){
   printf("(TODO: add code for facing off the material)\r\n\r\n)"); 
}

void cnc_begin_thread(unsigned id){
   printf("$%i\r\n\r\n", id);
}

void cnc_select_tool(unsigned tool_id){
   printf("\r\n(Calling up tool %i)\r\n", tool_id);
   printf("T%02d00\r\n\r\n", tool_id);
}

void cnc_set_program_number(unsigned number){
   if(number < 1 || number > 8999){
      fprintf(stderr, "invalid program number chosen");
      exit(1);
   }
   printf("O%d", number);
}

int main(){
   //setup
   ToolSize MILL = 0.500;
   cnc_set_program_number(1051);

   //machining
   cnc_begin_thread(1); 
   cnc_toggle(oil, true);
   cnc_move(0, 0, -0.055, DEFAULT);
   cnc_sleep(.2);
   cnc_toggle(chuck_main, true);
   cnc_sleep(.2);
   cnc_move(0, 0, -0.050, RELATIVE);

   cnc_move(0, 0.001, 0.002, RAPID | RELATIVE);
   cnc_select_tool(4);
   cnc_spindle_set(spindle_main, forward, per_minute, 0.001);
   cnc_turn(0, 0, .05, STAINLESS_303.turning_speed, RELATIVE);
   cnc_mill_hex(MILL, 0.5, .25, OVERSIZED_MILL);
   cnc_move(0, 0.5, 0, DEFAULT);
   cnc_cutoff(STAINLESS_303.cutoff_speed, PICKOFF);

   cnc_begin_thread(2); 
   
   cnc_set_standard_machining_data();
   return 0;
}
