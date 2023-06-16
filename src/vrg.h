#define vrg_cnt(vrg1,vrg2,vrg3,vrg4,vrg5,vrg6,vrg7,vrg8,vrgN, ...) vrgN
#define vrg_argn(...)  vrg_cnt(__VA_ARGS__, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#define vrg_cat0(x,y)  x ## y
#define vrg_cat(x,y)   vrg_cat0(x,y)

#define vrg(vrg_f,...) vrg_cat(vrg_f, vrg_argn(__VA_ARGS__))(__VA_ARGS__)

/*

char * msg_default = "How are you?";
#define greet(...)   vrg(greet, __VA_ARGS__)
#define greet1(n)    greetX(n, msg_default)
#define greet2(n, m) greetX(n, m)
void greetX(char *name, char *msg) {
  printf("Hello %s. %s\n", name, msg);
}


*/
