trace_timestamp.c
global �ܼƥ[
int ld_cnt=0;
int st_cnt=0;

in my_trace_log()

trace_cnt++;�U��
if(rw==2)ld_cnt++;
else if(rw==3)st_cnt++;



my_trace_end()

�o�̴N�i�H��ld/st�`��
ld_cnt, st_cnt