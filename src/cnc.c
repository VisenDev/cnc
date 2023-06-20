#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h> 
#include <stddef.h>
#include <float.h>

//newline delimiter
#define NL "\r\n"

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

//toggle a machine function on or off
void cnc_toggle(Relay relay, bool state){
   printf("\r\n(%s %s)" NL, RelayNames[relay], state ? "on" : "off");
   switch(relay) {
      case chuck_main:           state ? printf("M06" ) : printf("M07");  break;
      case chuck_back:           state ? printf("M16" ) : printf("M17");  break;
      case knock_out:            state ? printf("M10" ) : printf("M11");  break;
      case oil:                  state ? printf("M52" ) : printf("M53");  break;
      case part_counter:                 printf("M56" ) ;                 break;
      case check_tool_break:             printf("M51" ) ;                 break;
      case part_chute:           state ? printf("M32" ) : printf("M33");  break;
      case interference_check:   state ? printf("M89" ) : printf("M88");  break;
      case sync_spindles:        state ? printf("G814") : printf("G813"); break;
      case pickoff_support:      state ? printf("G650") : printf("G600"); break;
   }
   printf(NL);
}

//update rotation of spindle
void cnc_spindle_set(Spindle spindle, SpindleStatus status, SpindleFeedType feedtype, double feedrate){
   
   switch(spindle){
   case spindle_main:
      switch(status){
         case forward: printf("M3 S1=%.3f %s"NL, feedrate, feedtype == per_minute ? "G98" : "G99"); break;
         case reverse: printf("M4 S1=%.3f %s"NL, feedrate, feedtype == per_minute ? "G98" : "G99"); break;
         case stop: printf("M5"NL); break;
      }
      break;
   case spindle_back:
      switch(status){
         case forward: printf("M23 S2=%.3f %s"NL, feedrate, feedtype == per_minute ? "G98" : "G99"); break;
         case reverse: printf("M24 S2=%.3f %s"NL, feedrate, feedtype == per_minute ? "G98" : "G99"); break;
         case stop: printf("M25"NL); break;
      }
      break;
   case spindle_tool:
      switch(status){
         case forward: printf("%s\r\n M80 S%.3f" NL, feedtype == per_minute ? "G98" : "G99", feedrate); break;
         case reverse: printf("%s\r\nM81 S%.3f" NL, feedtype == per_minute ? "G98" : "G99", feedrate); break;
         case stop: printf("M82" NL);
      }
   }
}

//pause program execution for the specified amount of seconds
void cnc_sleep(double seconds){
   printf("G04U%.3g" NL, seconds);
}

//set maximum z travel at the beginning of program
void cnc_max_z_travel(double distance){
   printf("(MAXIMUM Z TRAVEL)" NL);
   printf("G50 Z%.4g" NL, distance);
}

#define RAPID FLT_MIN_EXP

//move two axis at the same time
void cnc_move_dual(Axis a, double a_movement, Axis b, double b_movement, double feedrate){
   printf(feedrate == RAPID ? "G0" : "G1"); 

   printf(" %c %.4g", a, a_movement);
   if(b != NONE){
      printf("%c %.4g", b, b_movement);
   }
   if(feedrate != RAPID){
      printf(" F%.4g", feedrate);
   }   
   printf("" NL);
}

//move one axis
void cnc_move(Axis axis, double distance, double feedrate){
   cnc_move_dual(axis, distance, NONE, 0, feedrate);
}

//for keeping records of what is currently being indexed
struct {
   bool spindle_main;
   bool spindle_back;
} indexing_status = {false , false};

//stop indexing mode for the selected spindle
void cnc_spindle_indexing_stop(Spindle spindle){
   switch(spindle){
      case spindle_main:
         indexing_status.spindle_main = false;
         printf("M20" NL);
      case spindle_back:
         indexing_status.spindle_back = false;
         printf("M79" NL);
      default:
         fprintf(stderr, "Tool spindle indexing cannot be stopped, exiting..." NL);
         exit(1);
   }
}

//begin indexing the chosen spindle at given angle
void cnc_spindle_indexing_begin_angle(Spindle spindle, unsigned starting_rotation){
   switch(spindle){
      case spindle_main:
         cnc_spindle_set(spindle_main, stop, 0, 0);
         indexing_status.spindle_main = true;
         printf("M28 S%d" NL, starting_rotation);
         break;
      case spindle_back:
         cnc_spindle_set(spindle_back, stop, 0, 0);
         indexing_status.spindle_back = true;
         printf("M78 S%d" NL, starting_rotation);
         break;
      default:
         fprintf(stderr, "Tool spindle cannot be indexed, exiting");
         exit(1);
   }
}

//begin spindle indexing without specifying angle (defaults to 0)
void cnc_spindle_indexing_begin(Spindle spindle){
   cnc_spindle_indexing_begin_angle(spindle, 0);
}

//index the spindle to absolute angle
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

//mills a hex
void cnc_mill_hex(unsigned mill_id, double milling_speed, double mill_diameter, double width_across_flats, double length_of_flats){
   //TODO add error checking to make sure the tool spindle is called up and is currently rotating
   cnc_spindle_set(spindle_main, stop, 0, 0);
   cnc_spindle_indexing_begin_angle(spindle_main, 30);


/*
   "(MILL HEX)" NL
   "M5" NL
   "G98" NL
   "M80S3000" NL
   "M28S30" NL
   "T0808X1.0Y1.0" NL
   "G50W-.395" NL
   "G0X.3095Z1.0" NL
   "G1Y-1.0F10.0" NL
   "G1Y1.0F90.0" NL
   "M28H60" NL
   "G1Y-1.0F10.0" NL
   "G1Y1.0F90.0" NL
   "M28H60" NL
   "G1Y-1.0F10.0" NL
   "G1Y1.0F90.0" NL
   "M28H60" NL
   "G1Y-1.0F10.0" NL
   "G1Y1.0F90.0" NL
   "M28H60" NL
   "G1Y-1.0F10.0" NL
   "G1Y1.0F90.0" NL
   "M28H60" NL
   "G1Y-1.0F10.0" NL
   "G1Y1.0F90.0" NL
   "G0X.347" NL
   "G1H360.0F3000" NL
   "G50W.395" NL
   "G0X1.0" NL
   "M82" NL
   "G99" NL
  */ 
}

//cutoff tool must be called before calling cnc_cutoff
void cnc_cutoff(double cutting_speed, double spindle_speed, bool pickoff){

   cnc_spindle_set(spindle_main, forward, per_minute, spindle_speed);
   
   if(pickoff){
      cnc_toggle(sync_spindles, true);
      cnc_toggle(pickoff_support, true);
      cnc_toggle(interference_check, false);
      //TODO Update this sync
      printf("!2L650" NL);
   }

   cnc_move(X_ABS, -0.10, cutting_speed);

   if(pickoff){
      cnc_toggle(sync_spindles, false);
      cnc_toggle(pickoff_support, false);
      cnc_toggle(interference_check, true);
   }
}

typedef enum {
   left,
   right,
   both
} Direction;

//chamfers a corner that the tool is currently adjacent to
void cnc_chamfer(double size, double cutting_speed, double spindle_speed, Direction side){

   printf("\r\n(Chamfering edge)" NL);
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

//TODO finish this function at spindle syncronization
void cnc_sub_spindle_pickoff(double distance_from_zero){
   printf("(pickoff)" NL);
   cnc_toggle(chuck_back, false);
   cnc_move(Z_ABS, distance_from_zero, RAPID);
   cnc_sleep(0.2);
   cnc_toggle(chuck_back, true);
   cnc_sleep(0.2);
   cnc_move(Z_REL, -1, RAPID);
}

//TODO finish this function
void cnc_set_standard_machining_data(){
   printf(
      "" NL
      "(Setting Machining Data)" NL
      "" NL
      "$0" NL
      "D" NL
      "#016=0000003750" NL
      "#020=0000001000" NL
      "#024=0000018000" NL
      "#028=0000001000" NL
      "#032=0000004000" NL
      "#036=0002500000" NL
      "#040=0000000010" NL
      "#044=-000001000" NL
      "#048=0000011000" NL
      "#052=0000002000" NL
      "#064=0000201000" NL
      "#068=0000000000" NL
      "%%" NL
   );
}

//call up a tool
//TODO fingure out how offsets work
void cnc_select_tool(unsigned tool_id){
   printf("\r\n(Calling up tool %i)" NL, tool_id);
   printf("T%02d00" NL, tool_id);
}

//faceoff the material for a chucker A20 at given speed
void cnc_faceoff_material(double cutting_speed, double spindle_speed, double length){
   printf("\r\n(faceoff)" NL); 
   
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

//begin new program
unsigned _current_program;
void cnc_begin_program(unsigned id){
   if(id != 1 && id != 2){
      fprintf(stderr, "invalid id passed to cnc_begin_thread, valid values are: 1, 2\n");
      exit(1);
   }
   
   //update internal records of program number
   _current_program = id;
   printf(NL "(");;
   printf(id == 1 ? "MAIN" : "SUB");
   printf(" PROGRAM)" NL);
   printf("$%i" NL, id);
}

void cnc_sync_programs(unsigned id){
   //TODO add support for syncing the main and back spindle threads
   printf("!L%d%d" NL, _current_program, id);
}

void cnc_end_program(){
   printf("(END OF PROGRAM)" NL);
   printf("M2" NL);
}

//Set the program number at the beginning of the program
void cnc_set_program_number(unsigned number){
   if(number < 1 || number > 8999){
      fprintf(stderr, "invalid program number chosen");
      exit(1);
   }
   printf("%%\r\n(SETTING PROGRAM NUMBER)\r\nO%d" NL, number);
}
