#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h> 
#include <stddef.h>
#include <float.h>

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

const Material STAINLESS_303 = { .turning_speed = 0.001, .cutoff_speed = 0.001, .drilling_speed = 0.001, .milling_speed = 0.001, };

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

void cnc_toggle(Relay relay, bool state){
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

void cnc_sleep(double seconds){
   printf("G04U%.3f\r\n", seconds);
}

#define RAPID FLT_MIN_EXP

void cnc_move_dual(Axis a, double a_movement, Axis b, double b_movement, double feedrate){
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

void cnc_move(Axis axis, double distance, double feedrate){
   cnc_move_dual(axis, distance, NONE, 0, feedrate);
}

//for keeping records of what is currently being indexed
struct {
   bool spindle_main;
   bool spindle_back;
} indexing_status = {false , false};

void cnc_spindle_indexing_stop(Spindle spindle){
   switch(spindle){
      case spindle_main:
         indexing_status.spindle_main = false;
         printf("M20\r\n");
      case spindle_back:
         indexing_status.spindle_back = false;
         printf("M79\r\n");
      default:
         fprintf(stderr, "Tool spindle indexing cannot be stopped, exiting...\r\n");
         exit(1);
   }
}

void cnc_spindle_indexing_begin_angle(Spindle spindle, unsigned starting_rotation){
   switch(spindle){
      case spindle_main:
         cnc_spindle_set(spindle_main, stop, 0, 0);
         indexing_status.spindle_main = true;
         printf("M28 S%d\r\n", starting_rotation);
         break;
      case spindle_back:
         cnc_spindle_set(spindle_back, stop, 0, 0);
         indexing_status.spindle_back = true;
         printf("M78 S%d\r\n", starting_rotation);
         break;
      default:
         fprintf(stderr, "Tool spindle cannot be indexed, exiting");
         exit(1);
   }
}

void cnc_spindle_indexing_begin(Spindle spindle){
   cnc_spindle_indexing_begin_angle(spindle, 0);
}

void cnc_spindle_index(Spindle spindle, unsigned degrees){
   switch(spindle){
      case spindle_main:
         if(indexing_status.spindle_main){
            printf("M18 C%d", degrees);
         } else {
            fprintf(stderr, "Main spindle is not in indexing mode, exiting...");
            exit(1);
         }
      case spindle_back:
         if(indexing_status.spindle_back){
            printf("M48 C%d", degrees);
         } else {
            fprintf(stderr, "Back spindle is not in indexing mode, exiting...");
            exit(1);
         }
      default:
         fprintf(stderr, "tool spindle cannot be indexed, exiting");
         exit(1);
   }
}

void cnc_mill_hex(unsigned mill_id, double milling_speed, double mill_diameter, double width_across_flats, double length_of_flats){
   //TODO add error checking to make sure the tool spindle is called up and is currently rotating
   cnc_spindle_set(spindle_main, stop, 0, 0);
   cnc_spindle_indexing_begin_angle(spindle_main, 30);


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
void cnc_cutoff(double cutting_speed, double spindle_speed, bool pickoff){

   cnc_spindle_set(spindle_main, forward, per_minute, spindle_speed);
   cnc_toggle(interference_check, false);
   
   if(pickoff){
      cnc_toggle(sync_spindles, true);
      cnc_toggle(pickoff_support, true);
      printf("!2L650\r\n");
   }

   cnc_move(X_ABS, -0.10, cutting_speed);
   //cnc_spindle_set(spindle_main, stop, 0, 0);
   cnc_toggle(interference_check, true);

   if(pickoff){
      cnc_toggle(sync_spindles, false);
      cnc_toggle(pickoff_support, false);
   }
}

typedef enum {
   left,
   right,
   both
} Direction;

void cnc_chamfer(double size, double cutting_speed, double spindle_speed, Direction side){

   printf("\r\n(Chamfering edge)\r\n");
   cnc_spindle_set(spindle_main, forward, per_minute, spindle_speed);
   
   if(side == left || side == both){
      cnc_move(X_REL, -0.2, RAPID); //move tool away from material
      cnc_move(Z_REL, -size, RAPID);
      cnc_move(X_REL, 0.2, RAPID); //move tool away from material
      cnc_move_dual(X_REL, size, Z_REL, size, cutting_speed);
   }

   if(side == right || side == both){
      cnc_move(X_REL, -0.2, RAPID); //move tool away from material
      cnc_move(Z_REL, size, RAPID);
      cnc_move(X_REL, 0.2, RAPID); //move tool away from material
      cnc_move_dual(X_REL, size, Z_REL, size, cutting_speed);
   }

   cnc_spindle_set(spindle_main, stop, 0, 0);
}

void cnc_sub_spindle_pickoff(double distance_from_zero){
   printf("(pickoff)\r\n");
   cnc_toggle(chuck_back, false);
   cnc_move(Z_ABS, distance_from_zero, RAPID);
   cnc_sleep(0.2);
   cnc_toggle(chuck_back, true);
   cnc_sleep(0.2);
   cnc_move(Z_REL, -1, RAPID);
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

void cnc_faceoff_material(double cutting_speed, double spindle_speed, double length){
   printf("\r\n(faceoff)\r\n"); 
   
   cnc_toggle(oil, true);
   cnc_toggle(chuck_main, false);
   cnc_move(Z_ABS, -length, RAPID);
   cnc_sleep(0.2);
   cnc_toggle(chuck_main, true);
   cnc_sleep(0.2);
   cnc_move(Z_ABS, -0.005, RAPID);
   cnc_move(Z_ABS, 0, 0.001);
   cnc_cutoff(cutting_speed, spindle_speed, false);
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

void cnc_sync_threads(unsigned id){
   //TODO add support for syncing the main and back spindle threads
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
