
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

*/
