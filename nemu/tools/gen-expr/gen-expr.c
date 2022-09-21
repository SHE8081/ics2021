#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

// this should be enough
static char buf[65536] = {};
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";
static void gen_num()
{
  int r = rand()%UINT8_MAX;
  char *p = buf;
  while ((*p)!='\0')
  {
    p = p + 1;
  }
  snprintf(p,6,"%d",r);
  p = p+6;
  *p = '\0';
}

static void gen(char c)
{
  char *p = buf;
  while ((*p)!='\0')
  {
    p = p + 1;
  }
  *p = c;
  p = p+1;
  *p = '\0';
}

static int choose(int n)
{
  return rand() % n;
}
static void gen_rand_op()
{
  char c;
  switch (choose(4)) {
      case 0:  c='+'; break;
      case 1:  c='-'; break;
      case 2:  c='*'; break;
      default: c='/';  break;
  } 
  gen(c);
}
  //递归深度限制为10，防止最后结果太大越界
static int rdepth = 10;
static void gen_rand_expr() {
if(rdepth>0){
    switch (choose(3)) {
      case 0: gen_num();rdepth--; break;
      case 1: gen('('); gen_rand_expr(); gen(')');rdepth--; break;
      default: gen_rand_expr(); gen_rand_op(); gen_rand_expr();rdepth--; break;
  }
}
else{
  gen_num();
}
  
}

int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i ++) {
    gen_rand_expr();
    sprintf(code_buf, code_format, buf);
    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc /tmp/.code.c -o /tmp/.expr");
    if (ret != 0) continue;

    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    int result;
    fscanf(fp, "%d", &result);
    pclose(fp);

    printf("%u %s\n", result, buf);
     //reset first character of buf array,for next reuse.
    buf[0] = '\0';
    rdepth = 10;
   }
  
  return 0;
}
