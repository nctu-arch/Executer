#include "stdio.h"
#include "sim-stat.h"

 

stat_var_rec *stat_var_pool[1024];
int stat_var_cnt;

stat_var_rec *stat_register(stat_var_rec *t, char *expr, char *ctr, char *file, int line)
{
	stat_var_rec *u;
	int i;
	
	if (t->regist!=0)  // we did it before
		return t->regist;
		
	t->counter_name = ctr;
	t->cond_expr = expr;
	t->src_file = file;
	t->src_line = line;
	t->count = t->nonzero_cnt = 0;
	t->sum = t->sum2 = 0.;
	t->next = 0;
	
		// merge nodes for different source points

	for (i=0; i<stat_var_cnt; i++) { // we use the simplest sequential search
		u = stat_var_pool[i];
		if (u->counter_name == ctr) {
			t->next = u->next;
			u->next = t;
			t->regist = u;
			return u;
		}
	}
	stat_var_pool[stat_var_cnt++] = t;
	t->regist = t;
	return t;
}


inline stat_variable_action(stat_var_rec *v, int var)
{
	register stat_var_rec *u;
	u = v->regist;
	
	u->count++;
	if (var) {
		u->nonzero_cnt++;
		u->sum += (float) var;
		u->sum2 += (float) var*var;
	}
}



void stat_summary(int level)
{
	int i;
	stat_var_rec *v, *u;
	float avg,var;
	
	for (i=0; i<stat_var_cnt; i++) {
		v = stat_var_pool[i];
		if (v->sum) {
			printf("%2i) Accumlating %s: %i times (nz: %i)   ", 
				i, v->counter_name, v->count, v->nonzero_cnt);
			avg = v->sum/((float) v->count);
			var = v->sum2/((float) v->count);
			var = sqrt(var-avg*avg);
			printf("avg: %.2f;; variance: %.2f  ", avg, var);
		}
		else {
			printf("%2i) Counting %s: %i    ", 
				i, v->counter_name, v->count);
		}
		if (level >= 1) {
			u = v;
			while (u != 0) {
				printf(" [%s]@%s-%i", u->cond_expr, u->src_file,u->src_line);
				u = u->next;
			}
		}
		printf("\n");
	}
}
