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
    virtual void PrintIR(stringstream &fout)=0;
};

class BaseExpAST : public BaseAST{
  public:
    string result;
};


class CompUnitAST : public BaseAST {
  public:
    unique_ptr<BaseAST> func_def;
    void Dump()const;
    void PrintIR(stringstream &fout);
};

class FuncDefAST : public BaseAST {
  public:
    unique_ptr<BaseAST> type;
    string ident;
    unique_ptr<BaseAST> block;
    void Dump() const;
    void PrintIR(stringstream &fout);
};

enum Type_kind{
  Type_int,Type_void
};

class TypeAST : public BaseAST{
  public:
    Type_kind type;
    void Dump() const;
    void PrintIR(stringstream &fout);
};

class BlockAST : public BaseAST{
  public:
    unique_ptr<vector<unique_ptr<BaseAST> > >blockitem_list;
    void Dump() const;
    void PrintIR(stringstream &fout);
};

enum blockitem_kind{
  Blockitem_decl,Blockitem_stmt
};

class BlockItemAST : public BaseAST{
  public:
    blockitem_kind type;
    unique_ptr<BaseAST>val;
    void PrintIR(stringstream &fout);
};

class DeclAST : public BaseAST{
  public:
    unique_ptr<BaseAST>type;
    unique_ptr<vector<unique_ptr<BaseAST> > > def_list;
    void PrintIR(stringstream &fout);
};


class DefAST: public BaseAST{
  public:
    string ident;
    unique_ptr<BaseExpAST> val;
    void PrintIR(stringstream &fout);
};

class InitValAST: public BaseExpAST{
  public:
    unique_ptr<BaseExpAST>exp;
    void PrintIR(stringstream &fout);
};

enum Stmt_kind{
  Stmt_ret,Stmt_ret_void,Stmt_lval,Stmt_exp,Stmt_void,Stmt_block
};

class StmtAST : public BaseAST{
  public:
    Stmt_kind type;
    unique_ptr<BaseAST>block;
    unique_ptr<BaseExpAST>exp;
    unique_ptr<BaseExpAST>lval;
    //void Dump() const;
    void PrintIR(stringstream &fout);
};

class LvalAST :public BaseExpAST{
  public:
    string ident;
    void PrintIR(stringstream &fout);
};

class ExpAST : public BaseExpAST {
  public:
    unique_ptr<BaseExpAST>exp;
    void Dump() const;
    void PrintIR(stringstream &fout);
};

class LorExpAST : public BaseExpAST{
  public:
    unique_ptr<BaseExpAST>lor_exp,land_exp;
    char op;
    void Dump()const;
    void PrintIR(stringstream &fout);
};

class LandExpAST : public BaseExpAST{
  public:
    unique_ptr<BaseExpAST>land_exp,eq_exp;
    char op;
    void Dump()const;
    void PrintIR(stringstream &fout);
};

class EqExpAST : public BaseExpAST{
  public:
    unique_ptr<BaseExpAST>eq_exp,rel_exp;
    string op;
    void Dump()const;
    void PrintIR(stringstream &fout);
};

class RelExpAST : public BaseExpAST{
  public:
    unique_ptr<BaseExpAST>rel_exp,add_exp;
    string op;
    void Dump()const;
    void PrintIR(stringstream &fout);
};

class AddExpAST : public BaseExpAST{
  public:
    unique_ptr<BaseExpAST>add_exp,mul_exp;
    char op;
    void Dump()const;
    void PrintIR(stringstream &fout);
};

class MulExpAST : public BaseExpAST{
  public:
    unique_ptr<BaseExpAST>mul_exp,unary_exp;
    char op;
    void Dump()const;
    void PrintIR(stringstream &fout);
};

class UnaryExpAST : public BaseExpAST {
public:
    unique_ptr<BaseExpAST> exp;
    char op;
    void Dump()const;
    void PrintIR(stringstream &fout);
};

enum PrimaryExp_kind{
  PrimaryExp_exp,PrimaryExp_number,Primary_lval
};

class PrimaryExpAST : public BaseExpAST {
public:
    PrimaryExp_kind type;
    unique_ptr<BaseExpAST> exp;
    unique_ptr<BaseExpAST> lval;
    int number;
    void Dump()const;
    void PrintIR(stringstream &fout);
};

