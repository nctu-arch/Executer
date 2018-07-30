#include <math.h>

typedef struct var_rec {
  char *counter_name;
  int reference;
  int count;
  int nonzero_cnt;
  float sum,sum2;
  char *cond_expr;
  char *src_file;
  int	src_line;
  struct var_rec *regist;
  struct var_rec *next;
} stat_var_rec;


#ifdef Arch_stat
#define stat_tick_action(v) v->regist->count++;

#define stat_tick(Condition, Counter)  { \
	static stat_var_rec my##Counter##__LINE__; \
	(my##Counter##__LINE__.regist!=0) ? ((void)0):stat_register(&my##Counter##__LINE__,  # Condition , # Counter , __FILE__, __LINE__); \
	if (Condition) stat_tick_action(my##Counter##__LINE__.regist); \
		}
#define stat_variable(Condition, Counter, var) { \
	static stat_var_rec my##Counter##__LINE__; \
	(my##Counter##__LINE__.regist!=0) ? ((void)0):stat_register(&my##Counter##__LINE__,  # Condition , # Counter , __FILE__, __LINE__); \
	 if (Condition) stat_variable_action(my##Counter##__LINE__.regist,   var); \
	 	}

#else

#define stat_tick(Condition, Counter) ((void)0)
#define stat_variable(Condition, Counter, var)  ((void)0)
#endif


