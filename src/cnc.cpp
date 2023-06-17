#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h> 
#include <stddef.h>

#define FOREACH_RELAY(func) \
   func(chuck_main) \
   func(chuck_back) \
   func(knock_out) \
   func(oil) \
   func(check_tool_break) \
   func(part_counter) \
   func(part_chute) \
   func(interference_check) \
   func(sync_spindles) \
   func(pickoff_support) \

#define str(x) #x,
#define val(x) x,

typedef enum { FOREACH_RELAY(val) } Relay;
const char* RelayNames[] = {FOREACH_RELAY(str)};

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

typedef enum {
   spindle_main,
   spindle_back,
   spindle_tool,
} Spindle;

typedef enum {
   forward,
   reverse,
   stop,
} SpindleStatus;

typedef enum {
   per_rotation,
   per_minute,
} SpindleFeedType;

typedef enum {
   absolute,
   relative
} MovementType;

typedef enum {
   X_ABS = 'X', 
   X_REL = 'U',
   Y_ABS = 'Y',
   Y_REL = 'V',
   Z_ABS = 'Z',
   Z_REL = 'W',
   NONE = 0
} Axis;

void cnc_toggle(Relay relay, bool state = true){
   printf("\r\n(%s toggled %s)\r\n", RelayNames[relay], state ? "on" : "off");
   switch(relay) {
      case chuck_main:           state ? printf("M06\r\n" ) : printf("M07\r\n"); break;
      case chuck_back:           state ? printf("M16\r\n" ) : printf("M17\r\n"); break;
      case knock_out:            state ? printf("M10\r\n" ) : printf("M11\r\n"); break;
      case oil:                  state ? printf("M52\r\n" ) : printf("M53\r\n"); break;
      case part_counter:                 printf("M56\r\n" ) ;                    break;
      case check_tool_break:             printf("M51\r\n" ) ;                    break;
      case part_chute:           state ? printf("M32\r\n" ) : printf("M33\r\n"); break;
      case interference_check:   state ? printf("M89\r\n" ) : printf("M88\r\n"); break;
      case sync_spindles:        state ? printf("G814\r\n") : printf("G813\r\n"); break;
      case pickoff_support:      state ? printf("G650\r\n") : printf("G600\r\n"); break;
   }
}

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
         case forward: printf("%s\r\n M80 S%.3f\r\n", feedtype == per_minute ? "G98" : "G99", feedrate); break;
         case reverse: printf("%s\r\nM81 S%.3f\r\n", feedtype == per_minute ? "G98" : "G99", feedrate); break;
         case stop: printf("M82\r\n");
      }
   }
}

void cnc_sleep(double seconds = 0.2){
   printf("G04U%.3f\r\n", seconds);
}

#define RAPID -10000.0
void cnc_move(double feedrate, Axis a, double a_movement, Axis b = NONE, double b_movement = 0){
   printf(feedrate == RAPID ? "G0" : "G1"); 

   printf(" %c %.3f", a, a_movement);
   if(b != NONE){
      printf("%c %.3f", b, b_movement);
   }
   if(feedrate != RAPID){
      printf(" F%.3f", feedrate);
   }   
   printf("\r\n");
}

#define ALLOW_OVERSIZE_MILL 1
void cnc_mill_hex( double mill_speed, double mill_diameter, double width_across_flats, double length, unsigned flags){
//   cnc_spindle_set(spindle_main, stop, 0, 0);
 //  cnc_spindle_set(spindle_tool, forward, per_minute, mill_speed);
  // cnc_spindle_rotate(spindle_main, 30);
/*
   "(MILL HEX)\r\n"
   "M5\r\n"
   "G98\r\n"
   "M80S3000\r\n"
   "M28S30\r\n"
   "T0808X1.0Y1.0\r\n"
   "G50W-.395\r\n"
   "G0X.3095Z1.0\r\n"
   "G1Y-1.0F10.0\r\n"
   "G1Y1.0F90.0\r\n"
   "M28H60\r\n"
   "G1Y-1.0F10.0\r\n"
   "G1Y1.0F90.0\r\n"
   "M28H60\r\n"
   "G1Y-1.0F10.0\r\n"
   "G1Y1.0F90.0\r\n"
   "M28H60\r\n"
   "G1Y-1.0F10.0\r\n"
   "G1Y1.0F90.0\r\n"
   "M28H60\r\n"
   "G1Y-1.0F10.0\r\n"
   "G1Y1.0F90.0\r\n"
   "M28H60\r\n"
   "G1Y-1.0F10.0\r\n"
   "G1Y1.0F90.0\r\n"
   "G0X.347\r\n"
   "G1H360.0F3000\r\n"
   "G50W.395\r\n"
   "G0X1.0\r\n"
   "M82\r\n"
   "G99\r\n"
  */ 
}

//cutoff tool must be called before calling cnc_cutoff
void cnc_cutoff(double feedrate, bool pickoff = false){

   cnc_spindle_set(spindle_main, forward, per_minute, feedrate);
   cnc_toggle(interference_check, false);
   
   if(pickoff){
      cnc_toggle(sync_spindles, true);
      cnc_toggle(pickoff_support, true);
      printf("!2L650\r\n");
   }

   cnc_move(feedrate, X_ABS, -0.10);
   //cnc_spindle_set(spindle_main, stop, 0, 0);
   cnc_toggle(interference_check, true);

   if(pickoff){
      cnc_toggle(sync_spindles, false);
      cnc_toggle(pickoff_support, false);
   }
}

void cnc_sub_spindle_pickoff(double distance_from_zero){
   printf("(pickoff)\r\n");
   cnc_toggle(chuck_back, false);
   cnc_move(RAPID, Z_ABS, distance_from_zero);
   cnc_sleep();
   cnc_toggle(chuck_back, true);
   cnc_sleep();
   cnc_move(RAPID, Z_REL, -1);
}

void cnc_set_standard_machining_data(){
   printf(
      "\r\n"
      "(Setting Machining Data)\r\n"
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

void cnc_select_tool(unsigned tool_id){
   printf("\r\n(Calling up tool %i)\r\n", tool_id);
   printf("T%02d00\r\n", tool_id);
}

void cnc_faceoff_material(double feedrate, double length){
   printf("\r\n(faceoff)\r\n"); 
   
   cnc_toggle(oil, true);
   cnc_toggle(chuck_main, false);
   cnc_move(RAPID, Z_ABS, -length);
   cnc_sleep();
   cnc_toggle(chuck_main, true);
   cnc_sleep();
   cnc_move(RAPID, Z_ABS, -0.005);
   cnc_move(0.001, Z_ABS, 0);
   cnc_cutoff(feedrate);
}

void cnc_begin_thread(unsigned id){
   if(id != 1 && id != 2){
      fprintf(stderr, "invalid id passed to cnc_begin_thread, valid values are: 1, 2\n");
      exit(1);
   }
   
   printf("\r\n(");;
   printf(id == 1 ? "main" : "sub");
   printf(" spindle program begin)\r\n");
   printf("$%i\r\n", id);
}

void cnc_end_thread(){
   printf("(End of Program)\r\n");
   printf("M30\r\n");
}

void cnc_set_program_number(unsigned number){
   if(number < 1 || number > 8999){
      fprintf(stderr, "invalid program number chosen");
      exit(1);
   }
   printf("%%\r\n(Setting program number)\r\nO%d\r\n", number);
}
