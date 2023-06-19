#include "cnc.c"

int main(){
   #define SPINDLE_SPEED 100.0
   cnc_set_program_number(1051);
   
   cnc_begin_thread(1); 
   cnc_select_tool(4);
   cnc_toggle(oil, true);
   cnc_faceoff_material(STAINLESS_303.cutoff_speed, SPINDLE_SPEED, 0.055);

   cnc_move(X_ABS, .5, RAPID);
   cnc_move(Z_ABS, 1.0, RAPID);
   cnc_cutoff(STAINLESS_303.cutoff_speed, SPINDLE_SPEED, true);
   
   cnc_move(X_ABS, .5, RAPID);
   cnc_select_tool(27);
   cnc_move(Z_ABS, 0, RAPID);
   cnc_toggle(chuck_main, false);
   cnc_end_thread();
   
   cnc_begin_thread(2); 
   cnc_sub_spindle_pickoff(.75);
   cnc_end_thread();
   
   cnc_set_standard_machining_data();
   
   return 0;
}
