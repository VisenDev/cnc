#include "cnc.cpp"

int main(){
   cnc_set_program_number(1051);
   
   cnc_begin_thread(1); 
   cnc_select_tool(4);
   cnc_toggle(oil);
   cnc_faceoff_material(STAINLESS_303.cutoff_speed, 0.055);

   cnc_move(RAPID, X_ABS, .5);
   cnc_move(RAPID, Z_ABS, 1.0);
   cnc_cutoff(STAINLESS_303.cutoff_speed, true);
   
   cnc_move(RAPID, X_ABS, .5);
   cnc_select_tool(27);
   cnc_move(RAPID, Z_ABS, 0);
   cnc_toggle(chuck_main, false);
   cnc_end_thread();
   
   cnc_begin_thread(2); 
   cnc_sub_spindle_pickoff(.75);
   cnc_end_thread();
   
   cnc_set_standard_machining_data();
   
   return 0;
}
