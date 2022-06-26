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
  CompUnitAST *unit_val;
  BaseAST *ast_val;
  LvalAST *l_val;
  BaseExpAST *ast_exp_val;
  char chr_val;
  vector<unique_ptr<BaseAST> > *vec_val;
  vector<unique_ptr<BaseExpAST> > *vec_exp_val;
}

%token INT RETURN OrOP AndOP CONST IF ELSE WHILE BREAK CONTINUE VOID
%token <str_val> IDENT RelOP EqOP
%token <int_val> INT_CONST

%type <unit_val> CompUnit_
%type <ast_val> FuncDef FuncFParam Block BlockItem
%type <ast_val> Stmt CloseStmt OpenStmt SimpleStmt
%type <ast_val> Decl ConstDecl VarDecl ConstDef VarDef
%type <ast_exp_val> Exp PrimaryExp UnaryExp AddExp MulExp LorExp LandExp EqExp RelExp
%type <ast_exp_val> ConstExp ConstInit VarInit
%type <l_val> Lval;
%type <int_val> Number Type
%type <vec_val> BlockItemList ConstDefList VarDefList FuncFParams
%type <vec_exp_val> DefIndexList ConstInitList VarInitList ExpIndexList FuncRParams
%type <chr_val> UnaryOp AddOp MulOp


%%

CompUnit
  : CompUnit_ {
    ast=unique_ptr<CompUnitAST>($1);
  }
  ;

CompUnit_
  :FuncDef{
    auto ast=new CompUnitAST();
    ast->func_def_list->push_back(unique_ptr<BaseAST>($1));
    $$=ast;
  }
  |Decl{
    auto ast=new CompUnitAST();
    ast->global_decl_list->push_back(unique_ptr<BaseAST>($1));
    $$=ast;
  }
  |CompUnit_ FuncDef{
    auto ast=$1;
    ast->func_def_list->push_back(unique_ptr<BaseAST>($2));
    $$=ast;
  }
  |CompUnit_ Decl{
    auto ast=$1;
    ast->global_decl_list->push_back(unique_ptr<BaseAST>($2));
    $$=ast;
  }
  ;

FuncDef
  : Type IDENT '(' ')' Block {
    auto ast = new FuncDefAST();
    ast->type=(Type_kind)$1;
    ast->ident = *unique_ptr<string>($2);
    ast->block = unique_ptr<BaseAST>($5);
    ast->params=unique_ptr<vector<unique_ptr<BaseAST> > >(new vector<unique_ptr<BaseAST> >);
    $$ = ast;
  }
  | Type IDENT '(' FuncFParams ')' Block {
    auto ast = new FuncDefAST();
    ast->type = (Type_kind)$1;
    ast->ident = *unique_ptr<string>($2);
    ast->params=unique_ptr<vector<unique_ptr<BaseAST> > >($4);
    ast->block = unique_ptr<BaseAST>($6);
    $$ = ast;
  }
  ;

Type
  : INT {
    $$ = Type_int;
  }
  |VOID {
    $$ = Type_void;
  }
  ;

FuncFParams
  : FuncFParam{
    auto vec=new vector<unique_ptr<BaseAST> >;
    vec->push_back(unique_ptr<BaseAST>($1));
    $$=vec;
  }
  | FuncFParams ',' FuncFParam {
    auto vec=$1;
    vec->push_back(unique_ptr<BaseAST>($3));
    $$=vec;
  }
  ;

FuncFParam
  : Type IDENT {
    auto ast=new FuncFParamAST();
    ast->type=(Type_kind)$1;
    ast->ident=*unique_ptr<string>($2);
    ast->is_array=0;
    ast->len=unique_ptr<vector<unique_ptr<BaseExpAST> > >(new vector<unique_ptr<BaseExpAST> >);
    $$=ast;
  }
  | Type IDENT '[' ']' DefIndexList {
    auto ast=new FuncFParamAST();
    ast->type=(Type_kind)$1;
    ast->ident=*unique_ptr<string>($2);
    ast->is_array=1;
    ast->len=unique_ptr<vector<unique_ptr<BaseExpAST> > >($5);
    $$=ast;
  }
  | Type IDENT '[' ']' {
    auto ast=new FuncFParamAST();
    ast->type=(Type_kind)$1;
    ast->ident=*unique_ptr<string>($2);
    ast->is_array=1;
    ast->len=unique_ptr<vector<unique_ptr<BaseExpAST> > >(new vector<unique_ptr<BaseExpAST> >);
    $$=ast;
  }
  ;

DefIndexList
  : '[' ConstExp ']' {
    auto vec=new vector<unique_ptr<BaseExpAST> >;
    vec->push_back(unique_ptr<BaseExpAST>($2));
    $$=vec;
  }
  | DefIndexList '[' ConstExp ']' {
    auto vec=$1;
    vec->push_back(unique_ptr<BaseExpAST>($3));
    $$=vec;
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
    $$=$1;
  }
  | Stmt {
    $$=$1;
  }
  ;

Decl
  : ConstDecl{
    $$=$1;
  }
  |VarDecl{
    $$=$1;
  };

ConstDecl
  :CONST Type ConstDefList ';'{
    auto ast=new ConstDeclAST();
    ast->type=(Type_kind)$2;
    ast->def_list=unique_ptr<vector<unique_ptr<BaseAST> > >($3);
    $$=ast;
  }
  ;

ConstDefList
  : ConstDef {
    auto vec=new vector<unique_ptr<BaseAST> >;
    vec->push_back(unique_ptr<BaseAST>($1));
    $$=vec;
  }
  | ConstDefList ',' ConstDef {
    auto vec=$1;
    vec->push_back(unique_ptr<BaseAST>($3));
    $$ = vec;
  }
  ;

ConstDef
  :IDENT '=' ConstInit{
    auto ast=new ConstDefAST();
    ast->ident=*unique_ptr<string>($1);
    ast->init=unique_ptr<BaseExpAST>($3);
    ast->len=unique_ptr<vector<unique_ptr<BaseExpAST> > >(new vector<unique_ptr<BaseExpAST> >);
    $$=ast;
  }
  | IDENT DefIndexList '=' ConstInit {
    auto ast=new ConstDefAST();
    ast->ident=*unique_ptr<string>($1);
    ast->len=unique_ptr<vector<unique_ptr<BaseExpAST> > >($2);
    ast->init=unique_ptr<BaseExpAST>($4);
    $$=ast;
  }
  ;

ConstInit
  : ConstExp {
    auto ast=new ConstInitAST();
    ast->exp=unique_ptr<BaseExpAST>($1);
    $$=ast;
  }
  |'{' '}' {
    auto ast=new ConstInitAST();
    ast->exp=NULL;
    ast->init=unique_ptr<vector<unique_ptr<BaseExpAST> > >(new vector<unique_ptr<BaseExpAST> >);
    $$=ast;
  }
  | '{' ConstInitList '}' {
    auto ast=new ConstInitAST();
    ast->exp=NULL;
    ast->init=unique_ptr<vector<unique_ptr<BaseExpAST> > >($2);
    $$=ast;
  }
  ;

ConstInitList
  :ConstInit {
    auto vec=new vector<unique_ptr<BaseExpAST> >;
    vec->push_back(unique_ptr<BaseExpAST>($1));
    $$ = vec;
  }
  | ConstInitList ',' ConstInit{
    auto vec=$1;
    vec->push_back(unique_ptr<BaseExpAST>($3));
    $$=vec;
  }
  ;

ConstExp
  : Exp {
    auto ast=new ConstExpAST();
    ast->exp=unique_ptr<BaseExpAST>($1);
    $$=ast;
  }
  ;

VarDecl
  : Type VarDefList ';' {
    auto ast=new VarDeclAST();
    ast->type=(Type_kind)($1);
    ast->def_list=unique_ptr<vector<unique_ptr<BaseAST> > >($2);
    $$=ast;
  }
  ;

VarDefList
  : VarDef {
    auto vec=new vector<unique_ptr<BaseAST> >;
    vec->push_back(unique_ptr<BaseAST>($1));
    $$=vec;
  }
  | VarDefList ',' VarDef {
    auto vec=$1;
    vec->push_back(unique_ptr<BaseAST>($3));
    $$ = vec;
  }
  ;

VarDef
  : IDENT {
    auto ast=new VarDefAST();
    ast->ident=*unique_ptr<string>($1);
    ast->init=NULL;
    ast->len=unique_ptr<vector<unique_ptr<BaseExpAST> > >(new vector<unique_ptr<BaseExpAST> >);
    $$=ast;
  }
  | IDENT '=' VarInit {
    auto ast=new VarDefAST();
    ast->ident=*unique_ptr<string>($1);
    ast->init=unique_ptr<BaseExpAST>($3);
    ast->len=unique_ptr<vector<unique_ptr<BaseExpAST> > >(new vector<unique_ptr<BaseExpAST> >);
    $$=ast;
  }
  | IDENT DefIndexList {
    auto ast=new VarDefAST();
    ast->ident=*unique_ptr<string>($1);
    ast->init=NULL;
    ast->len=unique_ptr<vector<unique_ptr<BaseExpAST> > >($2);;
    $$=ast;
  }
  | IDENT DefIndexList '=' VarInit {
    auto ast=new VarDefAST();
    ast->ident=*unique_ptr<string>($1);
    ast->init=unique_ptr<BaseExpAST>($4);
    ast->len=unique_ptr<vector<unique_ptr<BaseExpAST> > >($2);;
    $$=ast;
  }
  ;

VarInit
  : Exp {
    auto ast=new VarInitAST();
    ast->exp=unique_ptr<BaseExpAST>($1);
    $$=ast;
  }
  |'{' '}' {
    auto ast=new VarInitAST();
    ast->exp=NULL;
    ast->init=unique_ptr<vector<unique_ptr<BaseExpAST> > >(new vector<unique_ptr<BaseExpAST> >);
    $$=ast;
  }
  | '{' VarInitList '}' {
    auto ast=new VarInitAST();
    ast->exp=NULL;
    ast->init=unique_ptr<vector<unique_ptr<BaseExpAST> > >($2);
    $$=ast;
  }
  ;

VarInitList
  :VarInit {
    auto vec=new vector<unique_ptr<BaseExpAST> >;
    vec->push_back(unique_ptr<BaseExpAST>($1));
    $$ = vec;
  }
  | VarInitList ',' VarInit{
    auto vec=$1;
    vec->push_back(unique_ptr<BaseExpAST>($3));
    $$=vec;
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
    ast->lval=unique_ptr<LvalAST>($1);
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
    ast->type=Unary_primary;
    ast->exp=unique_ptr<BaseExpAST>($1);
    $$=ast;
  }
  | UnaryOp UnaryExp {
    auto ast=new UnaryExpAST();
    ast->type=Unary_op;
    ast->op=$1;
    ast->exp=unique_ptr<BaseExpAST>($2);
    $$=ast;
  }
  | IDENT '(' ')' {
    auto ast=new UnaryExpAST();
    ast->type=Unary_call;
    ast->ident=*unique_ptr<string>($1);
    ast->params=unique_ptr<vector<unique_ptr<BaseExpAST> > >(new vector<unique_ptr<BaseExpAST> >);
    $$=ast;
  }
  | IDENT '(' FuncRParams ')' {
    auto ast=new UnaryExpAST();
    ast->type=Unary_call;
    ast->ident=*unique_ptr<string>($1);
    ast->params=unique_ptr<vector<unique_ptr<BaseExpAST> > >($3);
    $$=ast;
  }
  ;

FuncRParams
  : Exp {
    auto vec=new vector<unique_ptr<BaseExpAST> >;
    vec->push_back(unique_ptr<BaseExpAST>($1));
    $$=vec;
  }
  | FuncRParams ',' Exp {
    auto vec=$1;
    vec->push_back(unique_ptr<BaseExpAST>($3));
    $$=vec;
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
    ast->lval=unique_ptr<LvalAST>($1);
    $$=ast;
  }
  ;

Lval
  : IDENT{
    auto ast=new LvalAST();
    ast->ident=*unique_ptr<string>($1);
    ast->idx=unique_ptr<vector<unique_ptr<BaseExpAST> > >(new vector<unique_ptr<BaseExpAST> >);
    $$=ast;
  }
  | IDENT ExpIndexList {
    auto ast=new LvalAST();
    ast->ident=*unique_ptr<string>($1);
    ast->idx=unique_ptr<vector<unique_ptr<BaseExpAST> > >($2);
    $$=ast;
  }
  ;
  ;

ExpIndexList
  : '[' Exp ']' {
    auto vec=new vector<unique_ptr<BaseExpAST> >;
    vec->push_back(unique_ptr<BaseExpAST>($2));
    $$=vec;
  }
  | ExpIndexList '[' Exp ']' {
    auto vec=$1;
    vec->push_back(unique_ptr<BaseExpAST>($3));
    $$=vec;
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