%code requires {
  #include <memory>
  #include <string>
  #include <AST.h>
}

%{

#include <iostream>
#include <memory>
#include <string>
#include <AST.h>

// 声明 lexer 函数和错误处理函数
int yylex();
void yyerror(std::unique_ptr<BaseAST> &ast, const char *s);

using namespace std;

%}

%parse-param { std::unique_ptr<BaseAST> &ast }

%union {
  std::string *str_val;
  int int_val;
  BaseAST *ast_val;
  BaseExpAST *ast_exp_val;
  char chr_val;
}

%token INT RETURN OrOP AndOP
%token <str_val> IDENT RelOP EqOP
%token <int_val> INT_CONST

%type <ast_val> FuncDef FuncType Block Stmt
%type <ast_exp_val> Exp PrimaryExp UnaryExp AddExp MulExp LorExp LandExp EqExp RelExp
%type <int_val> Number
%type <chr_val> UnaryOp AddOp MulOp

%%

CompUnit
  : FuncDef {
    auto comp_unit = make_unique<CompUnitAST>();
    comp_unit->func_def = unique_ptr<BaseAST>($1);
    ast = move(comp_unit);
  }
  ;

FuncDef
  : FuncType IDENT '(' ')' Block {
    auto ast = new FuncDefAST();
    ast->func_type = unique_ptr<BaseAST>($1);
    ast->ident = *unique_ptr<string>($2);
    ast->block = unique_ptr<BaseAST>($5);
    $$ = ast;
  }
  ;

FuncType
  : INT {
    auto ast = new FuncTypeAST();
    ast->type=Type_int;
    $$ = ast;
  }
  ;

Block
  : '{' Stmt '}' {
    auto ast = new BlockAST();
    ast->stmt=unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  ;

Stmt
  : RETURN Exp ';' {
    auto ast = new StmtAST();
    ast->type=Stmt_ret;
    ast->exp = unique_ptr<BaseExpAST>($2);
    $$ = ast;
  }
  ;

Exp
  : LorExp {
    auto ast=new ExpAST();
    ast->exp=unique_ptr<BaseExpAST>($1);
    $$=ast;
  }
  ;

LorExp
  : LandExp{
    auto ast=new LorExpAST();
    ast->op=0;
    ast->land_exp=std::unique_ptr<BaseExpAST>($1);
    $$=ast;
  }
  | LorExp OrOP LandExp{
    auto ast=new LorExpAST();
    ast->lor_exp=std::unique_ptr<BaseExpAST>($1);
    ast->op='|';
    ast->land_exp=std::unique_ptr<BaseExpAST>($3);
    $$=ast;
  }
  ;

LandExp
  : EqExp{
    auto ast=new LandExpAST();
    ast->op=0;
    ast->eq_exp=std::unique_ptr<BaseExpAST>($1);
    $$=ast;
  }
  | LandExp AndOP EqExp{
    auto ast=new LandExpAST();
    ast->land_exp=std::unique_ptr<BaseExpAST>($1);
    ast->op='&';
    ast->eq_exp=std::unique_ptr<BaseExpAST>($3);
    $$=ast;
  }
  ;

EqExp
  : RelExp{
    auto ast=new EqExpAST();
    ast->op="";
    ast->rel_exp=std::unique_ptr<BaseExpAST>($1);
    $$=ast;
  }
  | EqExp EqOP RelExp{
    auto ast=new EqExpAST();
    ast->eq_exp=std::unique_ptr<BaseExpAST>($1);
    ast->op=*($2);
    ast->rel_exp=std::unique_ptr<BaseExpAST>($3);
    $$=ast;
  }
  ;

RelExp
  : AddExp{
    auto ast=new RelExpAST();
    ast->op="";
    ast->add_exp=std::unique_ptr<BaseExpAST>($1);
    $$=ast;
  }
  | RelExp RelOP AddExp{
    auto ast=new RelExpAST();
    ast->rel_exp=std::unique_ptr<BaseExpAST>($1);
    ast->op=*($2);
    ast->add_exp=std::unique_ptr<BaseExpAST>($3);
    $$=ast;
  }
  ;

AddExp
  : MulExp{
    auto ast=new AddExpAST();
    ast->op=0;
    ast->mul_exp=std::unique_ptr<BaseExpAST>($1);
    $$=ast;
  }
  | AddExp AddOp MulExp{
    auto ast=new AddExpAST();
    ast->add_exp=std::unique_ptr<BaseExpAST>($1);
    ast->op=$2;
    ast->mul_exp=std::unique_ptr<BaseExpAST>($3);
    $$=ast;
  }
  ;

MulExp
  : UnaryExp{
    auto ast=new MulExpAST();
    ast->op=0;
    ast->unary_exp=std::unique_ptr<BaseExpAST>($1);
    $$=ast;
  }
  | MulExp MulOp UnaryExp{
    auto ast=new MulExpAST();
    ast->mul_exp=std::unique_ptr<BaseExpAST>($1);
    ast->op=$2;
    ast->unary_exp=std::unique_ptr<BaseExpAST>($3);
    $$=ast;
  }
  ;

UnaryExp
  : PrimaryExp {
    auto ast=new UnaryExpAST();
    ast->op=0;
    ast->exp=unique_ptr<BaseExpAST>($1);
    $$=ast;
  }
  | UnaryOp UnaryExp {
    auto ast=new UnaryExpAST();
    ast->op=$1;
    ast->exp=unique_ptr<BaseExpAST>($2);
    $$=ast;
  }
  ;

AddOp
  : '+' {
    $$ = '+';
  }
  | '-' {
    $$ = '-';
  }
  ;

MulOp
  : '*' {
    $$ = '*';
  }
  | '/' {
    $$ = '/';
  }
  |'%'{
    $$='%';
  }
  ;

UnaryOp
  : '+' {
    $$ = '+';
  }
  | '-' {
    $$ = '-';
  }
  | '!' {
    $$ = '!';
  }
  ;

PrimaryExp
  : '(' Exp ')' {
    auto ast=new PrimaryExpAST();
    ast->type=PrimaryExp_exp;
    ast->exp=unique_ptr<BaseExpAST>($2);
    $$=ast;
  }
  | Number {
    auto ast=new PrimaryExpAST();
    ast->type=PrimaryExp_number;
    ast->number=$1;
    $$=ast;
  }
  ;
Number
  : INT_CONST {
    $$ = $1;
  }
  ;

%%

// 定义错误处理函数, 其中第二个参数是错误信息
// parser 如果发生错误 (例如输入的程序出现了语法错误), 就会调用这个函数
void yyerror(unique_ptr<BaseAST> &ast, const char *s) {
  cerr << "error: " << s << endl;
}