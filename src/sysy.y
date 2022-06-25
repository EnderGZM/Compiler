%code requires {
  #include <memory>
  #include <string>
  #include <vector>
  #include <AST.h>
}

%{

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <AST.h>

// 声明 lexer 函数和错误处理函数
int yylex();
void yyerror(unique_ptr<BaseAST> &ast, const char *s);

using namespace std;

%}

%parse-param { unique_ptr<BaseAST> &ast }

%union {
  string *str_val;
  int int_val;
  BaseAST *ast_val;
  BaseExpAST *ast_exp_val;
  char chr_val;
  vector<unique_ptr<BaseAST> > *vec_val;
}

%token INT RETURN OrOP AndOP CONST IF ELSE WHILE BREAK CONTINUE
%token <str_val> IDENT RelOP EqOP
%token <int_val> INT_CONST

%type <ast_val> FuncDef Type Block Stmt CloseStmt OpenStmt Decl BlockItem Def SimpleStmt
%type <ast_exp_val> Exp PrimaryExp UnaryExp AddExp MulExp LorExp InitVal LandExp EqExp RelExp Lval
%type <int_val> Number
%type <vec_val> BlockItemList DefList
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
  : Type IDENT '(' ')' Block {
    auto ast = new FuncDefAST();
    ast->type = unique_ptr<BaseAST>($1);
    ast->ident = *unique_ptr<string>($2);
    ast->block = unique_ptr<BaseAST>($5);
    $$ = ast;
  }
  ;

Type
  : INT {
    auto ast = new TypeAST();
    ast->type=Type_int;
    $$ = ast;
  }
  ;

Block
  : '{' BlockItemList '}' {
    auto ast = new BlockAST();
    ast->blockitem_list=unique_ptr<vector<unique_ptr<BaseAST> > >($2);
    $$ = ast;
  }
  ;

BlockItemList
  : {
    auto vec=new vector<unique_ptr<BaseAST> >;
    $$=vec;
  }
  | BlockItemList BlockItem{
    auto vec=$1;
    vec->push_back(unique_ptr<BaseAST>($2));
    $$=vec;
  }
  ;

BlockItem
  : Decl {
    auto ast=new BlockItemAST();
    ast->type=Blockitem_decl;
    ast->val=unique_ptr<BaseAST>($1);
    $$=ast;
  }
  | Stmt {
    auto ast = new BlockItemAST();
    ast->type=Blockitem_stmt;
    ast->val=unique_ptr<BaseAST>($1);
    $$=ast;
  }
  ;

Decl
  :CONST Type DefList ';'{
    auto ast=new DeclAST();
    ast->type=unique_ptr<BaseAST>($2);
    ast->def_list=unique_ptr<vector<unique_ptr<BaseAST> > >($3);
    $$=ast;
  }
  | Type DefList ';'{
    auto ast=new DeclAST();
    ast->type=unique_ptr<BaseAST>($1);
    ast->def_list=unique_ptr<vector<unique_ptr<BaseAST> > >($2);
    $$=ast;
  }
  ;

DefList
  : Def {
    auto vec=new vector<unique_ptr<BaseAST> >;
    vec->push_back(unique_ptr<BaseAST>($1));
    $$=vec;
  }
  | DefList ',' Def {
    auto vec=$1;
    vec->push_back(unique_ptr<BaseAST>($3));
    $$ = vec;
  }
  ;

Def
  : IDENT{
    auto ast=new DefAST();
    ast->ident=*unique_ptr<string>($1);
    ast->val=NULL;
    $$=ast;
  }
  |IDENT '=' InitVal {
    auto ast=new DefAST();
    ast->ident=*unique_ptr<string>($1);
    ast->val=unique_ptr<BaseExpAST>($3);
    $$=ast;
  }
  ;

InitVal
  : Exp {
    auto ast=new InitValAST();
    ast->exp=unique_ptr<BaseExpAST>($1);
    $$=ast;
  }
  ;

Stmt
  : OpenStmt{
    $$=$1;
  }
  | CloseStmt{
    $$=$1;
  }
  ;

OpenStmt
  : IF '(' Exp ')' Stmt{
    auto ast=new StmtAST();
    ast->type=Stmt_if;
    ast->exp=unique_ptr<BaseExpAST>($3);
    ast->stmt=unique_ptr<BaseAST>($5);
    $$=ast;
  }
  | IF '(' Exp ')' CloseStmt ELSE OpenStmt {
    auto ast=new StmtAST();
    ast->type=Stmt_ifelse;
    ast->exp=unique_ptr<BaseExpAST>($3);
    ast->stmt=unique_ptr<BaseAST>($5);
    ast->else_stmt=unique_ptr<BaseAST>($7);
    $$=ast;
  }
  | WHILE '(' Exp ')' OpenStmt {
    auto ast=new StmtAST();
    ast->type=Stmt_while;
    ast->exp=unique_ptr<BaseExpAST>($3);
    ast->stmt=unique_ptr<BaseAST>($5);
    $$=ast;
  }
  ;

CloseStmt
  : SimpleStmt {
    auto ast=new StmtAST();
    ast->type=Stmt_simple;
    ast->stmt=unique_ptr<BaseAST>($1);
    $$=ast;
  }
  | IF '(' Exp ')' CloseStmt ELSE CloseStmt {
    auto ast = new StmtAST();
    ast->type=Stmt_ifelse;
    ast->exp=unique_ptr<BaseExpAST>($3);
    ast->stmt=unique_ptr<BaseAST>($5);
    ast->else_stmt=unique_ptr<BaseAST>($7);
    $$=ast;
  }
  | WHILE '(' Exp ')' CloseStmt {
    auto ast=new StmtAST();
    ast->type=Stmt_while;
    ast->exp=unique_ptr<BaseExpAST>($3);
    ast->stmt=unique_ptr<BaseAST>($5);
    $$=ast;
  }
  ;

SimpleStmt
  : RETURN Exp ';' {
    auto ast = new SimpleStmtAST();
    ast->type=Simple_ret;
    ast->exp = unique_ptr<BaseExpAST>($2);
    $$ = ast;
  }
  | RETURN ';'{
    auto ast= new SimpleStmtAST();
    ast->type=Simple_ret_void;
    ast->exp=NULL;
    $$=ast;
  }
  | Lval '=' Exp ';'{
    auto ast=new SimpleStmtAST();
    ast->type=Simple_lval;
    ast->lval=unique_ptr<BaseExpAST>($1);
    ast->exp=unique_ptr<BaseExpAST>($3);
    $$=ast;
  }
  | Exp ';' {
    auto ast=new SimpleStmtAST();
    ast->type=Simple_exp;
    ast->exp=unique_ptr<BaseExpAST>($1);
    $$=ast;
  }
  | ';' {
    auto ast=new SimpleStmtAST();
    ast->type=Simple_void;
    ast->exp=NULL;
    $$=ast;
  }
  | Block {
    auto ast=new SimpleStmtAST();
    ast->type=Simple_block;
    ast->block=unique_ptr<BaseAST>($1);
    $$=ast;
  }
  | BREAK ';' {
    auto ast=new SimpleStmtAST();
    ast->type=Simple_break;
    $$=ast;
  }
  | CONTINUE ';' {
    auto ast=new SimpleStmtAST();
    ast->type=Simple_continue;
    $$=ast;
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
    ast->land_exp=unique_ptr<BaseExpAST>($1);
    $$=ast;
  }
  | LorExp OrOP LandExp{
    auto ast=new LorExpAST();
    ast->lor_exp=unique_ptr<BaseExpAST>($1);
    ast->op='|';
    ast->land_exp=unique_ptr<BaseExpAST>($3);
    $$=ast;
  }
  ;

LandExp
  : EqExp{
    auto ast=new LandExpAST();
    ast->op=0;
    ast->eq_exp=unique_ptr<BaseExpAST>($1);
    $$=ast;
  }
  | LandExp AndOP EqExp{
    auto ast=new LandExpAST();
    ast->land_exp=unique_ptr<BaseExpAST>($1);
    ast->op='&';
    ast->eq_exp=unique_ptr<BaseExpAST>($3);
    $$=ast;
  }
  ;

EqExp
  : RelExp{
    auto ast=new EqExpAST();
    ast->op="";
    ast->rel_exp=unique_ptr<BaseExpAST>($1);
    $$=ast;
  }
  | EqExp EqOP RelExp{
    auto ast=new EqExpAST();
    ast->eq_exp=unique_ptr<BaseExpAST>($1);
    ast->op=*($2);
    ast->rel_exp=unique_ptr<BaseExpAST>($3);
    $$=ast;
  }
  ;

RelExp
  : AddExp{
    auto ast=new RelExpAST();
    ast->op="";
    ast->add_exp=unique_ptr<BaseExpAST>($1);
    $$=ast;
  }
  | RelExp RelOP AddExp{
    auto ast=new RelExpAST();
    ast->rel_exp=unique_ptr<BaseExpAST>($1);
    ast->op=*($2);
    ast->add_exp=unique_ptr<BaseExpAST>($3);
    $$=ast;
  }
  ;

AddExp
  : MulExp{
    auto ast=new AddExpAST();
    ast->op=0;
    ast->mul_exp=unique_ptr<BaseExpAST>($1);
    $$=ast;
  }
  | AddExp AddOp MulExp{
    auto ast=new AddExpAST();
    ast->add_exp=unique_ptr<BaseExpAST>($1);
    ast->op=$2;
    ast->mul_exp=unique_ptr<BaseExpAST>($3);
    $$=ast;
  }
  ;

MulExp
  : UnaryExp{
    auto ast=new MulExpAST();
    ast->op=0;
    ast->unary_exp=unique_ptr<BaseExpAST>($1);
    $$=ast;
  }
  | MulExp MulOp UnaryExp{
    auto ast=new MulExpAST();
    ast->mul_exp=unique_ptr<BaseExpAST>($1);
    ast->op=$2;
    ast->unary_exp=unique_ptr<BaseExpAST>($3);
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
  | Lval{
    auto ast=new PrimaryExpAST();
    ast->type=Primary_lval;
    ast->lval=unique_ptr<BaseExpAST>($1);
    $$=ast;
  }
  ;

Lval
  : IDENT{
    auto ast=new LvalAST();
    ast->ident=*unique_ptr<string>($1);
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