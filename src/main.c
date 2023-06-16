#include "cnc.h"

int main(){
   
   cnc_set_program_number(1051);
   
   cnc_begin_thread(1); 
   cnc_toggle(oil, true);
   cnc_toggle(chuck_main, false);
   cnc_move(RAPID, absolute, Z, -0.055);
   cnc_toggle(chuck_main, true);
   cnc_move(RAPID, absolute, Z, -0.005);
   cnc_select_tool(4);
   cnc_move(0.001, absolute, Z, 0);
   cnc_cutoff(STAINLESS_303.cutoff_speed, DEFAULT);
   cnc_toggle(oil, false);
   cnc_end_thread();
   
   cnc_begin_thread(2); 
   cnc_end_thread();
   
   cnc_set_standard_machining_data();
   
   return 0;
}
