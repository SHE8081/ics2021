#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

enum token_type {
  TK_NOTYPE = 256, TK_EQ,

  /* TODO: Add more token types */
  /* + - x * /*/
  TK_ADD=43,TK_SUB,TK_MUL,TK_DIV,
  /* = ( )*/
  TK_ASSGIN,TK_LP,TK_RP,
  TK_NUM,
} t_type ;


static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus
  {"==", TK_EQ},        // equal
  {"[0-9]",TK_NUM},     // number
  {"\\(",TK_LP},         //(
  {"\\)",TK_RP},         //)  
  {"\\*",TK_MUL},        //*
  {"\\/",TK_DIV},       // chu
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
  int prority;
} Token;

static Token tokens[32] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;



static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

   //record last 
  int pre_t = TK_NOTYPE;
  int rep = 0;
  while (e[position] != '\0') {
    i = 0;
    /* Try all rules one by one. */
    for (; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
          case TK_NOTYPE:break;    
          case TK_ADD:
              tokens[nr_token].type=TK_ADD;tokens[nr_token].prority=1; 
              tokens[nr_token].str[0] = *substr_start;
              pre_t = TK_ADD;
              nr_token = nr_token + 1; break;
          case TK_ASSGIN :
              tokens[nr_token].type=TK_ASSGIN;
              tokens[nr_token].str[0] = *substr_start;
              pre_t = TK_ASSGIN;
              nr_token = nr_token + 1;break;
          case TK_DIV : 
              tokens[nr_token].type=TK_DIV;tokens[nr_token].prority=2;
              tokens[nr_token].str[0] = *substr_start;
              pre_t = TK_DIV;
              nr_token = nr_token + 1;break;
          case TK_EQ : 
              tokens[nr_token].type=TK_EQ;
              tokens[nr_token].str[0] = *substr_start;
              pre_t = TK_EQ;
              nr_token = nr_token + 1;
              break;
          case TK_LP : 
              tokens[nr_token].type=TK_LP;tokens[nr_token].prority=3; 
              tokens[nr_token].str[0] = *substr_start;
              pre_t = TK_LP;
              nr_token = nr_token + 1;
              break;
          case TK_MUL :
              tokens[nr_token].type=TK_MUL;tokens[nr_token].prority=2;
              tokens[nr_token].str[0] = *substr_start;
              pre_t = TK_MUL;
              nr_token = nr_token + 1;
              break;
          case TK_RP :
              tokens[nr_token].type=TK_RP;tokens[nr_token].prority=3;
              tokens[nr_token].str[0] = *substr_start;
              pre_t = TK_RP;
              nr_token = nr_token + 1;
              break;
          case TK_SUB :   
              tokens[nr_token].type=TK_SUB;tokens[nr_token].prority=1;
              tokens[nr_token].str[0] = *substr_start;
              pre_t = TK_SUB;
              nr_token = nr_token + 1;
              break;
          case TK_NUM : 
              if (pre_t != TK_NUM)
              {
                tokens[nr_token].type=TK_NUM;tokens[nr_token].prority=0;
                rep = 0;
                tokens[nr_token].str[rep] = *substr_start;
              }else{
                //上次匹配的结果也是TK_NUM，在上次的token对应的str中的对应位置写入
                rep += 1;
                tokens[nr_token-1].str[rep] = *substr_start; 
              }
              pre_t = TK_NUM;
              nr_token = nr_token + 1;
              break;
          default: 
              printf("Unkown token type!");
        }
        //遇到tokens结束标志，跳出规则匹配
        if(*(e+position) == '\0')
        {
          break;
        }
      }
      if (i == NR_REGEX) {
        printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
        return false;
      }
    }
  }
  return true;
}

uint32_t eval(Token *p ,Token *q);
bool check_parentheses(Token *p, Token *q);

word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }
  Token *t = tokens;
  while (t->str[0]!= '\0' )
  {
    printf("token type is %d and str is %s",t->type,t->str);
  }
  
  return  eval(&tokens[0],&tokens[nr_token]);
}

bool check_parentheses(Token *p, Token *q){
 while(p->type==TK_RP && p->type==TK_RP)
 {
   p = p-1;
   q = q-1;
 } 
 if(p == q){
   return true;
 }else
  return false;
}

Token *  find_main_op(Token * ,Token*);
uint32_t eval(Token *p ,Token *q){ 
  if (p > q) {
    /* Bad expression */
    return -1;
  }
  else if (p == q) {
    /* Single token.
     * For now this token should be a number.
     * Return the value of the number.
     */
    assert(p->type==TK_NUM&&q->type==TK_NUM);
    return atoi(p->str);
  }
  else if (check_parentheses(p, q) == true) {
    /* The expression is surrounded by a matched pair of parentheses.
     * If that is the case, just throw away the parentheses.
     */
    return eval(p + 1, q - 1);
  }
  else {
   Token *pos = find_main_op(p,q);
   uint32_t val1 = eval(p, pos-1);
   uint32_t val2 = eval(pos+1, q);

    switch (pos->type) {
      case TK_ADD: return val1 + val2;break;
      case TK_SUB: return val1 - val2;break;
      case TK_MUL: return val1 * val2;break;
      case TK_DIV: return val1 / val2;break;
      default: assert(0);
    }
  }
}

/**
 * @brief 从左边查找第一个运算符
 * 返回运算符地址
 */
Token * find__from_left(Token *t){
  if (t->type >= TK_ADD && t->type<=TK_DIV)
  {
    return t;
  }
  else
    return t+1;  
}

/**
 * @brief 从右边查找第一个运算符
 * 返回运算符地址
 */
Token * find__from_right(Token *t){
  if (t->type >= TK_ADD && t->type<=TK_DIV)
  {
    return t;
  }
  else
    return t-1;  
}

Token * find_main_op (Token *p, Token *q){
  if(p->prority >= q->prority){
    p = p + 1;
    find__from_left(p);
    if(p != q){
     find_main_op(p,q); 
    }
  }else{
    q = q - 1;
    find__from_right(q);
    if(p != q){
      find_main_op(p,q);
    }
  }
  if(p!=q){
    printf("未寻找到主运算符!");
    assert(p!=q);
  }
  return p;
}
