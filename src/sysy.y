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

%token INT RETURN
%token <str_val> IDENT
%token <int_val> INT_CONST

%type <ast_val> FuncDef FuncType Block Stmt
%type <ast_exp_val> Exp PrimaryExp UnaryExp
%type <int_val> Number
%type <chr_val> UnaryOp

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
  : UnaryExp {
    auto ast=new ExpAST();
    ast->unary_exp=unique_ptr<BaseExpAST>($1);
    $$=ast;
  }
  ;

UnaryExp
  : PrimaryExp {
    auto ast=new UnaryExpAST();
    ast->type=UnaryExp_primary;
    ast->exp=unique_ptr<BaseExpAST>($1);
    $$=ast;
  }
  | UnaryOp UnaryExp {
    auto ast=new UnaryExpAST();
    ast->type=UnaryExp_Unary;
    ast->op=$1;
    ast->exp=unique_ptr<BaseExpAST>($2);
    $$=ast;
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