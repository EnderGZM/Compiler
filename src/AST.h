#pragma once
#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include <cstring>
#include <sstream>
#include <vector>

using namespace std;

class BaseAST {
  public:
    virtual ~BaseAST() = default;
    virtual void Dump()const{}
    virtual void PrintIR(stringstream &fout){}
    virtual void Global_Alloc(stringstream &fout){}
};

class BaseExpAST : public BaseAST{
  public:
    string result;
    virtual int Calc(){return 0;}
};


class CompUnitAST : public BaseAST {
  public:
    unique_ptr<vector<unique_ptr<BaseAST> > >func_def_list;
    unique_ptr<vector<unique_ptr<BaseAST> > >global_decl_list;
    CompUnitAST(){
      func_def_list=unique_ptr<vector<unique_ptr<BaseAST> > >(new vector<unique_ptr<BaseAST> >);
      global_decl_list=unique_ptr<vector<unique_ptr<BaseAST> > >(new vector<unique_ptr<BaseAST> >);
    }
    void PrintIR(stringstream &fout);
};

enum Type_kind{
  Type_int,Type_void
};

class FuncDefAST : public BaseAST {
  public:
    Type_kind type;
    string ident;
    unique_ptr<BaseAST> block;
    unique_ptr<vector<unique_ptr<BaseAST> > >params;
    void PrintIR(stringstream &fout);
};

class FuncFParamAST : public BaseAST {
  public:
    string ident;
    Type_kind type;
    bool is_array;
    unique_ptr<vector<unique_ptr<BaseExpAST> > >len;
    void PrintIR(stringstream &fout);
};

class BlockAST : public BaseAST{
  public:
    unique_ptr<vector<unique_ptr<BaseAST> > >blockitem_list;
    void PrintIR(stringstream &fout);
};

class VarDeclAST : public BaseAST{
  public:
    Type_kind type;
    unique_ptr<vector<unique_ptr<BaseAST> > > def_list;
    void PrintIR(stringstream &fout);
    void Global_Alloc(stringstream &fout);
};

class VarDefAST: public BaseAST{
  public:
    string ident;
    unique_ptr<BaseExpAST>init;
    unique_ptr<vector<unique_ptr<BaseExpAST> > >len;
    void PrintIR(stringstream &fout);
    void Global_Alloc(stringstream &fout);
};

class VarInitAST: public BaseExpAST{
  public:
    unique_ptr<BaseExpAST>exp;
    unique_ptr<vector<unique_ptr<BaseExpAST> > >init;
    void PrintIR(stringstream &fout);
    int Calc();
};

class ConstDeclAST : public BaseAST{
  public:
    Type_kind type;
    unique_ptr<vector<unique_ptr<BaseAST> > > def_list;
    void PrintIR(stringstream &fout);
    void Global_Alloc(stringstream &fout);
};


class ConstDefAST: public BaseAST{
  public:
    string ident;
    unique_ptr<BaseExpAST>init;
    unique_ptr<vector<unique_ptr<BaseExpAST> > >len;
    void PrintIR(stringstream &fout);
    void Global_Alloc(stringstream &fout);
};

class ConstInitAST: public BaseExpAST{
  public:
    unique_ptr<BaseExpAST>exp;
    unique_ptr<vector<unique_ptr<BaseExpAST> > >init;
    void PrintIR(stringstream &fout);
    int Calc();
};

class ConstExpAST:public BaseExpAST {
  public:
    std::unique_ptr<BaseExpAST> exp;
    void PrintIR(stringstream &fout);
    int Calc();
};

enum Stmt_kind{
  Stmt_simple,Stmt_if,Stmt_ifelse,Stmt_while
};

class StmtAST : public BaseAST{
  public:
    Stmt_kind type;
    unique_ptr<BaseExpAST>exp;
    unique_ptr<BaseAST>stmt;
    unique_ptr<BaseAST>else_stmt;
    void PrintIR(stringstream &fout);
};

class LvalAST :public BaseExpAST{
  public:
    string ident;
    unique_ptr<vector<unique_ptr<BaseExpAST> > >idx;
    void PrintIR(stringstream &fout);
};

enum SimpleStmt_kind{
  Simple_ret,Simple_ret_void,Simple_lval,Simple_exp,Simple_void,Simple_block,Simple_break,Simple_continue
};

class SimpleStmtAST : public BaseAST{
  public:
    SimpleStmt_kind type;
    unique_ptr<BaseAST>block;
    unique_ptr<BaseExpAST>exp;
    unique_ptr<LvalAST>lval;
    void PrintIR(stringstream &fout);
};



class ExpAST : public BaseExpAST {
  public:
    unique_ptr<BaseExpAST>exp;
    void PrintIR(stringstream &fout);
    int Calc();
};

class LorExpAST : public BaseExpAST{
  public:
    unique_ptr<BaseExpAST>lor_exp,land_exp;
    char op;
    void PrintIR(stringstream &fout);
    int Calc();
};

class LandExpAST : public BaseExpAST{
  public:
    unique_ptr<BaseExpAST>land_exp,eq_exp;
    char op;
    void PrintIR(stringstream &fout);
    int Calc();
};

class EqExpAST : public BaseExpAST{
  public:
    unique_ptr<BaseExpAST>eq_exp,rel_exp;
    string op;
    void PrintIR(stringstream &fout);
    int Calc();
};

class RelExpAST : public BaseExpAST{
  public:
    unique_ptr<BaseExpAST>rel_exp,add_exp;
    string op;
    void PrintIR(stringstream &fout);
    int Calc();
};

class AddExpAST : public BaseExpAST{
  public:
    unique_ptr<BaseExpAST>add_exp,mul_exp;
    char op;
    void PrintIR(stringstream &fout);
    int Calc();
};

class MulExpAST : public BaseExpAST{
  public:
    unique_ptr<BaseExpAST>mul_exp,unary_exp;
    char op;
    void PrintIR(stringstream &fout);
    int Calc();
};

enum Unary_kind{
  Unary_primary,Unary_call,Unary_op
};

class UnaryExpAST : public BaseExpAST {
public:
    Unary_kind type;
    unique_ptr<BaseExpAST> exp;
    char op;
    string ident;
    unique_ptr<vector<unique_ptr<BaseExpAST> > >params;
    void PrintIR(stringstream &fout);
    int Calc();
};

enum PrimaryExp_kind{
  PrimaryExp_exp,PrimaryExp_number,Primary_lval
};

class PrimaryExpAST : public BaseExpAST {
public:
    PrimaryExp_kind type;
    unique_ptr<BaseExpAST> exp;
    unique_ptr<LvalAST> lval;
    int number;
    void PrintIR(stringstream &fout);
    int Calc();
};

