#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#define MAX_TOKENS 8192
#define MAX_VARS 512
#define MAX_FUNCS 128
#define MAX_PARAMS 32
#define MAX_LINES 8192
#define MAX_SRC 262144

typedef enum { T_EOF, T_NEWLINE, T_WORD, T_NUMBER, T_STRING, T_OP, T_ASSIGN, T_LPAREN, T_RPAREN, T_LBRACKET, T_RBRACKET, T_COMMA } TokenType;
typedef struct { TokenType type; char *text; double number; int line; } Token;

typedef enum { V_NOTHING, V_NUMBER, V_STRING, V_BOOL } ValueType;
typedef struct { ValueType type; double number; int boolean; char *string; } Value;

typedef struct { char *name; Value value; } Var;
typedef struct { char *name; char *params[MAX_PARAMS]; int param_count; int start; int end; } Function;

typedef struct { Token *t; int n; int p; } Parser;
typedef struct { int active; int break_loop; int carry_on; int returning; Value return_value; } Flow;

static Var vars[MAX_VARS]; static int var_count=0;
static Function funcs[MAX_FUNCS]; static int func_count=0;
static int runtime_error=0;

static char *dupstr(const char *s){ size_t n=strlen(s)+1; char *p=malloc(n); if(!p){fprintf(stderr,"CAP ALERT [E900]\nOut of memory.\n"); exit(1);} memcpy(p,s,n); return p; }
static Value nothing(void){ return (Value){V_NOTHING,0,0,NULL}; }
static Value num(double x){ return (Value){V_NUMBER,x,0,NULL}; }
static Value boolean(int x){ return (Value){V_BOOL,0,x?1:0,NULL}; }
static Value str(const char *s){ Value v={V_STRING,0,0,dupstr(s?s:" ")}; if(v.string && strcmp(v.string," ")==0 && (!s || !*s)) { free(v.string); v.string=dupstr(""); } return v; }
static void freev(Value v){ if(v.type==V_STRING) free(v.string); }
static Value copyv(Value v){ if(v.type==V_STRING) return str(v.string); return v; }
static int truth(Value v){ if(v.type==V_NOTHING) return 0; if(v.type==V_BOOL) return v.boolean; if(v.type==V_NUMBER) return v.number!=0; if(v.type==V_STRING) return v.string && v.string[0]; return 0; }
static void alert(const char *code,const char *msg){ fprintf(stderr,"CAP ALERT [%s]\n%s\n",code,msg); runtime_error=1; }

static int is_word_start(int c){ return isalpha((unsigned char)c)||c=='_'; }
static int is_word_char(int c){ return isalnum((unsigned char)c)||c=='_'; }

static Token *lex(const char *src,int *out_n){
 Token *a=calloc(MAX_TOKENS,sizeof(Token)); int n=0,line=1; const char *p=src;
 while(*p){
  if(n>=MAX_TOKENS-2){ alert("E002","Too many tokens."); break; }
  if(*p==' '||*p=='\t'||*p=='\r'){p++;continue;}
  if(*p=='#'){ while(*p && *p!='\n') p++; continue; }
  if(*p=='\n'){ a[n++]=(Token){T_NEWLINE,dupstr("\\n"),0,line++}; p++; continue; }
  if((unsigned char)*p==0xE2 && (unsigned char)p[1]==0x82 && (unsigned char)p[2]==0xA6){ while(*p && *p!='\n') p++; continue; }
  if(*p=='"'){
   p++; size_t cap=64,len=0; char *buf=malloc(cap);
   while(*p && *p!='"'){
    char c=*p++;
    if(c=='\\' && *p){ char e=*p++; if(e=='n') c='\n'; else if(e=='t') c='\t'; else if(e=='"') c='"'; else { if(len+2>=cap){cap*=2;buf=realloc(buf,cap);} buf[len++]=c; buf[len++]=e; continue; } }
    if(len+2>=cap){cap*=2;buf=realloc(buf,cap);} buf[len++]=c;
   }
   if(*p!='"'){ free(buf); alert("E001","Unterminated string."); break; }
   p++; buf[len]=0; a[n++]=(Token){T_STRING,buf,0,line}; continue;
  }
  if(is_word_start((unsigned char)*p)){
   const char *s=p; while(is_word_char((unsigned char)*p)) p++; size_t l=p-s; char *w=malloc(l+1); memcpy(w,s,l);w[l]=0; a[n++]=(Token){T_WORD,w,0,line}; continue;
  }
  if(isdigit((unsigned char)*p) || (*p=='.' && isdigit((unsigned char)p[1]))){
   char *end; double x=strtod(p,&end); if(end==p){alert("E003","Invalid number.");break;} size_t l=end-p; char *w=malloc(l+1);memcpy(w,p,l);w[l]=0;a[n++]=(Token){T_NUMBER,w,x,line};p=end;continue;
  }
  if(*p=='='){ if(p[1]=='='){a[n++]=(Token){T_OP,dupstr("=="),0,line};p+=2;} else {a[n++]=(Token){T_ASSIGN,dupstr("="),0,line};p++;}continue; }
  if(strchr("+-*/><!",*p)){
   char op[3]={*p,0,0}; if(((*p=='>'||*p=='<'||*p=='!')&&p[1]=='=') ){op[1]='=';p+=2;} else p++; a[n++]=(Token){T_OP,dupstr(op),0,line};continue;
  }
  if(*p=='('){a[n++]=(Token){T_LPAREN,dupstr("("),0,line};p++;continue;} if(*p==')'){a[n++]=(Token){T_RPAREN,dupstr(")"),0,line};p++;continue;}
  if(*p=='['){a[n++]=(Token){T_LBRACKET,dupstr("["),0,line};p++;continue;} if(*p==']'){a[n++]=(Token){T_RBRACKET,dupstr("]"),0,line};p++;continue;}
  if(*p==','){a[n++]=(Token){T_COMMA,dupstr(","),0,line};p++;continue;}
  char msg[128]; snprintf(msg,sizeof(msg),"Unexpected character on line %d.",line); alert("E004",msg); p++;
 }
 a[n++]=(Token){T_EOF,dupstr("EOF"),0,line}; *out_n=n; return a;
}
static int ieq(const char *a,const char *b){ while(*a&&*b){ if(tolower((unsigned char)*a)!=tolower((unsigned char)*b))return 0;a++;b++;}return *a==0&&*b==0; }
static int kw(const char *s,const char *k){return ieq(s,k);}
static Token *cur(Parser *p){return &p->t[p->p];}
static int accept_type(Parser *p,TokenType t){if(cur(p)->type==t){p->p++;return 1;}return 0;}
static void skip_nl(Parser *p){while(cur(p)->type==T_NEWLINE)p->p++;}
static void parser_error(Parser *p,const char *msg){char b[256];snprintf(b,sizeof(b),"Line %d: %s",cur(p)->line,msg);alert("E100",b);}

static int find_var(const char *name){for(int i=var_count-1;i>=0;i--)if(strcmp(vars[i].name,name)==0)return i;return -1;}
static void set_var(const char *name,Value v){int i=find_var(name);if(i<0){if(var_count>=MAX_VARS){alert("E201","Too many variables.");return;}i=var_count++;vars[i].name=dupstr(name);vars[i].value=nothing();}freev(vars[i].value);vars[i].value=copyv(v);}
static int find_func(const char *name){for(int i=0;i<func_count;i++)if(ieq(funcs[i].name,name))return i;return -1;}

static Value parse_expr(Parser *p); static Value execute_range(Parser *p,int start,int end,Flow *flow);

static Value apply_op(const char *op,Value a,Value b){
 if(strcmp(op,"+")==0){ if(a.type==V_STRING||b.type==V_STRING){char x[512];const char *sa=a.type==V_STRING?a.string:"";const char *sb=b.type==V_STRING?b.string:"";if(a.type==V_NUMBER){snprintf(x,sizeof(x),"%.15g",a.number);sa=x;} char y[512]; if(b.type==V_NUMBER){snprintf(y,sizeof(y),"%.15g",b.number);sb=y;} char *z=malloc(strlen(sa)+strlen(sb)+1);strcpy(z,sa);strcat(z,sb);Value v={V_STRING,0,0,z};return v;} if(a.type==V_NUMBER&&b.type==V_NUMBER)return num(a.number+b.number);}
 if(a.type==V_NUMBER&&b.type==V_NUMBER){ if(strcmp(op,"-")==0)return num(a.number-b.number);if(strcmp(op,"*")==0)return num(a.number*b.number);if(strcmp(op,"/")==0){if(b.number==0){alert("E202","Division by zero.");return nothing();}return num(a.number/b.number);}if(strcmp(op,">")==0)return boolean(a.number>b.number);if(strcmp(op,"<")==0)return boolean(a.number<b.number);if(strcmp(op,">=")==0)return boolean(a.number>=b.number);if(strcmp(op,"<=")==0)return boolean(a.number<=b.number);if(strcmp(op,"==")==0)return boolean(a.number==b.number);if(strcmp(op,"!=")==0)return boolean(a.number!=b.number); }
 if(strcmp(op,"==")==0||strcmp(op,"!=")==0){int eq=0;if(a.type==V_STRING&&b.type==V_STRING)eq=strcmp(a.string,b.string)==0;else if(a.type==V_BOOL&&b.type==V_BOOL)eq=a.boolean==b.boolean;else if(a.type==V_NOTHING&&b.type==V_NOTHING)eq=1;return boolean(strcmp(op,"==")==0?eq:!eq);}
 alert("E203","Invalid operation for these values.");return nothing();
}

static Value parse_primary(Parser *p){
 Token *t=cur(p);
 if(t->type==T_NUMBER){p->p++;return num(t->number);} if(t->type==T_STRING){p->p++;return str(t->text);} if(t->type==T_LPAREN){p->p++;Value v=parse_expr(p);if(!accept_type(p,T_RPAREN))parser_error(p,"Expected ')' .");return v;}
 if(t->type==T_WORD){char *name=t->text;p->p++;if(kw(name,"true"))return boolean(1);if(kw(name,"false"))return boolean(0);if(kw(name,"nothing"))return nothing();
  if(accept_type(p,T_LPAREN)){ Value args[MAX_PARAMS];int ac=0;skip_nl(p);if(!accept_type(p,T_RPAREN)){do{if(ac>=MAX_PARAMS){parser_error(p,"Too many function arguments.");return nothing();}args[ac++]=parse_expr(p);}while(accept_type(p,T_COMMA));if(!accept_type(p,T_RPAREN)){parser_error(p,"Expected ')' after arguments.");return nothing();}}
   int fi=find_func(name); if(fi<0){char b[200];snprintf(b,sizeof(b),"Function not found: %s",name);alert("E204",b);return nothing();}
   Function *f=&funcs[fi]; int old_count=var_count;
   Var *backup=calloc(MAX_VARS,sizeof(Var));
   if(!backup){alert("E900","Out of memory.");return nothing();}
   for(int i=0;i<old_count;i++){backup[i].name=dupstr(vars[i].name);backup[i].value=copyv(vars[i].value);}
   for(int i=0;i<f->param_count;i++) set_var(f->params[i],i<ac?args[i]:nothing());
   Parser q=*p; Flow fl={1,0,0,0,nothing()}; execute_range(&q,f->start,f->end,&fl);
   Value ret=fl.returning?copyv(fl.return_value):nothing();
   for(int i=0;i<var_count;i++){free(vars[i].name);freev(vars[i].value);}
   for(int i=0;i<old_count;i++){vars[i].name=backup[i].name;vars[i].value=backup[i].value;backup[i].name=NULL;}
   var_count=old_count; free(backup);
   for(int i=0;i<ac;i++) freev(args[i]);
   return ret;
  }
  int idx=find_var(name);if(idx<0){char b[200];snprintf(b,sizeof(b),"Undefined variable: %s",name);alert("E205",b);return nothing();}return copyv(vars[idx].value);
 }
 parser_error(p,"Expected expression.");return nothing();
}
static Value parse_unary(Parser *p){if(cur(p)->type==T_OP&&strcmp(cur(p)->text,"-")==0){p->p++;Value v=parse_unary(p);if(v.type!=V_NUMBER){alert("E206","Unary '-' requires a number.");return nothing();}return num(-v.number);}if(cur(p)->type==T_WORD&&kw(cur(p)->text,"NOT")){p->p++;return boolean(!truth(parse_unary(p)));}return parse_primary(p);}
static Value parse_mul(Parser *p){Value a=parse_unary(p);while(cur(p)->type==T_OP&&(strcmp(cur(p)->text,"*")==0||strcmp(cur(p)->text,"/")==0)){char op[3];strcpy(op,cur(p)->text);p->p++;Value b=parse_unary(p);Value r=apply_op(op,a,b);freev(a);freev(b);a=r;}return a;}
static Value parse_add(Parser *p){Value a=parse_mul(p);while(cur(p)->type==T_OP&&(strcmp(cur(p)->text,"+")==0||strcmp(cur(p)->text,"-")==0)){char op[3];strcpy(op,cur(p)->text);p->p++;Value b=parse_mul(p);Value r=apply_op(op,a,b);freev(a);freev(b);a=r;}return a;}
static Value parse_cmp(Parser *p){Value a=parse_add(p);while(cur(p)->type==T_OP&&(strcmp(cur(p)->text,">")==0||strcmp(cur(p)->text,"<")==0||strcmp(cur(p)->text,">=")==0||strcmp(cur(p)->text,"<=")==0||strcmp(cur(p)->text,"==")==0||strcmp(cur(p)->text,"!=")==0)){char op[3];strcpy(op,cur(p)->text);p->p++;Value b=parse_add(p);Value r=apply_op(op,a,b);freev(a);freev(b);a=r;}return a;}
static Value parse_and(Parser *p){Value a=parse_cmp(p);while(cur(p)->type==T_WORD&&kw(cur(p)->text,"AND")){p->p++;Value b=parse_cmp(p);a=boolean(truth(a)&&truth(b));freev(b);}return a;}
static Value parse_expr(Parser *p){Value a=parse_and(p);while(cur(p)->type==T_WORD&&kw(cur(p)->text,"OR")){p->p++;Value b=parse_and(p);a=boolean(truth(a)||truth(b));freev(b);}return a;}

static int line_end(Parser *p,int pos){while(pos< p->n && p->t[pos].type!=T_NEWLINE && p->t[pos].type!=T_EOF)pos++;return pos;}
static void register_functions(Parser *p){
 for(int i=0;i<p->n;i++) if(p->t[i].type==T_WORD&&kw(p->t[i].text,"function")){if(i+1>=p->n||p->t[i+1].type!=T_WORD){parser_error(p,"Expected function name.");return;}Function f={0};f.name=dupstr(p->t[i+1].text);int j=i+2;if(j<p->n&&p->t[j].type==T_LPAREN){j++;while(j<p->n&&p->t[j].type!=T_RPAREN){if(p->t[j].type==T_WORD){if(f.param_count<MAX_PARAMS)f.params[f.param_count++]=dupstr(p->t[j].text);j++;}else if(p->t[j].type==T_COMMA)j++;else {parser_error(p,"Invalid function parameter list.");return;}}if(j>=p->n){parser_error(p,"Unclosed function parameter list.");return;}j++;}int body=j;while(body<p->n&&p->t[body].type!=T_NEWLINE)body++;if(body<p->n)body++;
 int depth=1,end=-1;
 for(int k=body;k<p->n;k++){
  if(p->t[k].type==T_WORD&&kw(p->t[k].text,"function")) depth++;
  else if(p->t[k].type==T_WORD&&kw(p->t[k].text,"end")){depth--;if(depth==0){end=k;break;}}
 }
 if(end<0){parser_error(p,"Function missing 'end'.");return;}f.start=body;f.end=end;if(func_count<MAX_FUNCS)funcs[func_count++]=f;i=end;}
}

static Value execute_range(Parser *p,int start,int end,Flow *flow){
 int i=start; p->p=start;
 while(i<end && !runtime_error){p->p=i;skip_nl(p);i=p->p;if(i>=end)break;
  if(cur(p)->type==T_WORD&&kw(cur(p)->text,"function")){
   int fi=(i+1<p->n && p->t[i+1].type==T_WORD)?find_func(p->t[i+1].text):-1;
   if(fi>=0){i=funcs[fi].end+1;continue;}
   i=line_end(p,i);continue;
  }
  if(cur(p)->type==T_WORD&&kw(cur(p)->text,"give")){p->p++;flow->return_value=parse_expr(p);flow->returning=1;return flow->return_value;}
  if(cur(p)->type==T_WORD&&kw(cur(p)->text,"carry-on")){flow->carry_on=1;return nothing();}
  if(cur(p)->type==T_WORD&&kw(cur(p)->text,"say")){p->p++;Value v=parse_expr(p);if(v.type==V_STRING)printf("%s\n",v.string);else if(v.type==V_NUMBER)printf("%.15g\n",v.number);else if(v.type==V_BOOL)printf("%s\n",v.boolean?"true":"false");else printf("nothing\n");freev(v);i=line_end(p,p->p);continue;}
  if(cur(p)->type==T_WORD&&kw(cur(p)->text,"ask")){p->p++;Value prompt=parse_expr(p);if(prompt.type==V_STRING)printf("%s",prompt.string);char buf[4096];if(!fgets(buf,sizeof(buf),stdin))buf[0]=0;buf[strcspn(buf,"\n")]=0;Value v=str(buf);freev(prompt);/* ask as statement is ignored; assignment handles expression only in future */freev(v);i=line_end(p,p->p);continue;}
  if(cur(p)->type==T_WORD&&kw(cur(p)->text,"if")){
   p->p++;Value cond=parse_expr(p);int body=p->p;while(body<end&&p->t[body].type!=T_NEWLINE)body++;if(body<end)body++;int depth=1,elsepos=-1,j=body;
   for(;j<end;j++){if(p->t[j].type==T_WORD&&kw(p->t[j].text,"if"))depth++;else if(p->t[j].type==T_WORD&&kw(p->t[j].text,"end")){depth--;if(depth==0)break;}else if(depth==1&&p->t[j].type==T_WORD&&kw(p->t[j].text,"ifnot"))elsepos=j;}
   if(j>=end){alert("E101","If statement missing 'end'.");freev(cond);return nothing();}
   int bstart=body,bend=elsepos>=0?elsepos:j;int estart=elsepos>=0?elsepos+1:j;while(estart<j&&p->t[estart].type!=T_NEWLINE)estart++;if(estart<j)estart++;Flow sub={1,0,0,0,nothing()};if(truth(cond))execute_range(p,bstart,bend,&sub);else if(elsepos>=0)execute_range(p,estart,j,&sub);freev(cond);if(sub.returning||sub.carry_on){*flow=sub;return sub.return_value;}i=j+1;continue;
  }
  if(cur(p)->type==T_WORD&&(kw(cur(p)->text,"repeat")||kw(cur(p)->text,"while"))){int iswhile=kw(cur(p)->text,"while");p->p++;int exprpos=p->p;Value nval=parse_expr(p);int body=p->p;while(body<end&&p->t[body].type!=T_NEWLINE)body++;if(body<end)body++;int depth=1,j=body;for(;j<end;j++){if(p->t[j].type==T_WORD&&(kw(p->t[j].text,"repeat")||kw(p->t[j].text,"while")))depth++;else if(p->t[j].type==T_WORD&&kw(p->t[j].text,"end")){depth--;if(depth==0)break;}}if(j>=end){alert("E102","Loop missing 'end'.");freev(nval);return nothing();}int count=iswhile?0:(int)(nval.type==V_NUMBER?nval.number:0);if(!iswhile&&nval.type!=V_NUMBER){alert("E207","repeat requires a number.");freev(nval);return nothing();}for(int k=0;runtime_error&&(0);k++);int k=0;while(!runtime_error && (iswhile ? k<100000 : k<count)){Flow sub={1,0,0,0,nothing()};execute_range(p,body,j,&sub);if(sub.returning){*flow=sub;freev(nval);return sub.return_value;}if(sub.break_loop)break;k++;if(iswhile){p->p=exprpos;Value c=parse_expr(p);int ok=truth(c);freev(c);if(!ok)break;} }freev(nval);i=j+1;continue;}
  if(cur(p)->type==T_WORD&&kw(cur(p)->text,"load")){p->p++;Value m=parse_expr(p);if(m.type!=V_STRING)alert("E208","load requires a module name string.");else {char b[512];snprintf(b,sizeof(b),"Module '%s' is not available yet in this build.",m.string);alert("E209",b);}freev(m);i=line_end(p,p->p);continue;}
  if(cur(p)->type==T_WORD&&kw(cur(p)->text,"the")){p->p++;if(cur(p)->type!=T_WORD){parser_error(p,"Expected variable name after 'the'.");return nothing();}char *name=cur(p)->text;p->p++;if(!accept_type(p,T_ASSIGN)){parser_error(p,"Expected '=' after variable name.");return nothing();}Value v=parse_expr(p);set_var(name,v);freev(v);i=line_end(p,p->p);continue;}
  if(cur(p)->type==T_WORD){int save=p->p;char *name=cur(p)->text;p->p++;if(accept_type(p,T_ASSIGN)){Value v=parse_expr(p);if(find_var(name)<0){char b[200];snprintf(b,sizeof(b),"Cannot assign to undefined variable: %s",name);alert("E210",b);}else set_var(name,v);freev(v);i=line_end(p,p->p);continue;}p->p=save;Value v=parse_expr(p);freev(v);i=line_end(p,p->p);continue;}
  parser_error(p,"Unexpected syntax.");return nothing();
 }
 return nothing();
}

static char *read_file(const char *path){FILE *f=fopen(path,"rb");if(!f)return NULL;fseek(f,0,SEEK_END);long n=ftell(f);fseek(f,0,SEEK_SET);if(n<0||n>=MAX_SRC){fclose(f);return NULL;}char *s=malloc((size_t)n+1);if(!s){fclose(f);return NULL;}fread(s,1,(size_t)n,f);s[n]=0;fclose(f);return s;}
int main(int argc,char **argv){
 if(argc<2){
  printf("CAP 1.0.0\\nUsage: cap <program.cap>\\n       cap --version\\n       cap --help\\n");
  return 0;
 }
 if(strcmp(argv[1],"--version")==0 || strcmp(argv[1],"-v")==0){ printf("CAP 1.0.0\\n"); return 0; }
 if(strcmp(argv[1],"--help")==0 || strcmp(argv[1],"-h")==0){
  printf("CAP 1.0.0\\n");
  printf("Usage: cap <program.cap>\\n");
  printf("       cap --version\\n");
  printf("       cap --help\\n");
  printf("Run a CAP source file with the native CAP runtime.\\n");
  return 0;
 }
 char *src=read_file(argv[1]);if(!src){fprintf(stderr,"CAP ALERT [E300]\nCould not read file: %s\n",argv[1]);return 1;}
 int n=0;Token *t=lex(src,&n);if(runtime_error){free(src);return 1;}Parser p={t,n,0};register_functions(&p);if(runtime_error){free(src);return 1;}Flow f={1,0,0,0,nothing()};execute_range(&p,0,n-1,&f);
 for(int i=0;i<n;i++) free(t[i].text);
 free(t);
 free(src);
 for(int i=0;i<var_count;i++){ free(vars[i].name); freev(vars[i].value); }
 for(int i=0;i<func_count;i++){ free(funcs[i].name); for(int j=0;j<funcs[i].param_count;j++) free(funcs[i].params[j]); }
 return runtime_error?1:0;
}
