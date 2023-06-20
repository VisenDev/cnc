#include "cnc.c"

int main(){
   #define SPINDLE_SPEED 100.0
   cnc_set_program_number(1051);
   
   cnc_begin_program(1); 
   cnc_max_z_travel(1.100);
   cnc_select_tool(4);
   cnc_faceoff_material(STAINLESS_303.cutoff_speed, SPINDLE_SPEED, 0.055);

   //turning
   cnc_select_tool(5);
   cnc_spindle_set(spindle_main, forward, SPINDLE_SPEED, per_rotation);
   cnc_move(X_ABS, 0.333, STAINLESS_303.turning_speed);
   cnc_move(Z_ABS, 1.0, STAINLESS_303.turning_speed);
         
   //call up cutoff and cut off part
   cnc_move(X_ABS, .5, RAPID);
   cnc_select_tool(4);
   cnc_cutoff(STAINLESS_303.cutoff_speed, SPINDLE_SPEED, true);
   
   cnc_move(X_ABS, .5, RAPID);
   cnc_select_tool(27);
   cnc_move(Z_ABS, 0, RAPID);
   cnc_toggle(chuck_main, false);
   cnc_end_program();
   
   cnc_begin_program(2); 
   cnc_sub_spindle_pickoff(.75);
   cnc_end_program();
   
   cnc_set_standard_machining_data();
   
   return 0;
}
