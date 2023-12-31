#include "cnc.c"

int main(){
   #define X_HOME .675

   cnc_set_program_number(5555);
   cnc_begin_program(1);
   cnc_max_z_travel(1.700);
   cnc_select_tool(4, X_HOME);
   cnc_toggle(oil, true);
   cnc_toggle(chuck_main, false);

   cnc_faceoff_material(999, 999, 0.055);
   cnc_reload();

   printf(NL "(DRILLING HOLE)" NL);
   cnc_move(X_ABS, X_HOME, RAPID);
   cnc_move(Z_ABS, -.25, RAPID);
   cnc_select_tool(21, 0); 
   cnc_move(Z_ABS, 0, RAPID);
   cnc_move(Z_ABS, 0.62, 999);
   cnc_move(Z_ABS, -.25, RAPID);

   printf(NL "TODO: add tap threading code" NL);

   printf(NL "(TURN DOWN STOCK)" NL); 
   cnc_move(Z_ABS, -.25, RAPID);
   cnc_select_tool(5, X_HOME);
   cnc_sync_programs(34);
   cnc_move(X_ABS, .500, RAPID);
   cnc_spindle_set(spindle_main, forward, per_minute, 999);
   cnc_move(Z_ABS, 1.56, 999);

   printf(NL "(TURN GROOVE)" NL);
   cnc_move(X_ABS, .437, 999);
   cnc_move(X_ABS, X_HOME, RAPID);
   cnc_move(Z_REL, .070, RAPID);
   cnc_select_tool(4, X_HOME); 
   cnc_cutoff(999, 999, true);
   cnc_move(X_ABS, X_HOME, RAPID);
   cnc_move(Z_ABS, 0, RAPID);
   cnc_toggle(chuck_main, false);
   cnc_end_program();

   cnc_begin_program(2);
   cnc_sync_programs(34);
   cnc_toggle(part_chute, true);
   cnc_sub_spindle_pickoff(1.00);
   cnc_toggle(part_counter, true);
   cnc_end_program();

   cnc_set_standard_machining_data();
   
   return 0;
}
