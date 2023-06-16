#include "cnc.c"



int main(){
   
   cnc_set_program_number(1051);
   
   cnc_begin_thread(1); 
   cnc_select_tool(4);
   cnc_toggle(oil, true);
   cnc_faceoff_material(STAINLESS_303.cutoff_speed, 0.055);

   cnc_move(RAPID, absolute, X, .5);
   cnc_move(RAPID, absolute, Z, 1.0);
   cnc_cutoff(STAINLESS_303.cutoff_speed, PICKOFF);
   
   cnc_move(RAPID, absolute, X, .5);
   cnc_select_tool(27);
   cnc_move(RAPID, absolute, Z, 0);
   cnc_toggle(chuck_main, false);
   cnc_end_thread();
   
   cnc_begin_thread(2); 
   cnc_end_thread();
   
   cnc_set_standard_machining_data();
   
   return 0;
}
