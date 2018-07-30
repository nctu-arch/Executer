trace_timestamp.c
global 變數加
int ld_cnt=0;
int st_cnt=0;

in my_trace_log()

trace_cnt++;下面
if(rw==2)ld_cnt++;
else if(rw==3)st_cnt++;



my_trace_end()

這裡就可以有ld/st總數
ld_cnt, st_cnt