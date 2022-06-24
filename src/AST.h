#pragma once
#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include <cstring>
#include <sstream>

using namespace std;

class BaseAST {
  public:
    virtual ~BaseAST() = default;
    virtual void Dump()const{}
    virtual void PrintIR(stringstream &fout)=0;
};

class CompUnitAST : public BaseAST {
  public:
    unique_ptr<BaseAST> func_def;
    void Dump()const;
    void PrintIR(stringstream &fout);
};

class FuncDefAST : public BaseAST {
  public:
    unique_ptr<BaseAST> func_type;
    string ident;
    unique_ptr<BaseAST> block;
    void Dump() const;
    void PrintIR(stringstream &fout);
};

enum Type_kind{
  Type_int,Type_void
};

class FuncTypeAST : public BaseAST{
  public:
    Type_kind type;
    void Dump() const;
    void PrintIR(stringstream &fout);
};

class BlockAST : public BaseAST{
  public:
    unique_ptr<BaseAST> stmt;
    void Dump() const;
    void PrintIR(stringstream &fout);
};

class BaseExpAST : public BaseAST{
  public:
    string result;
};

enum Stmt_kind{
  Stmt_ret
};

class StmtAST : public BaseAST{
  public:
    Stmt_kind type;
    std::unique_ptr<BaseExpAST> exp;
    void Dump() const;
    void PrintIR(stringstream &fout);
};

class ExpAST : public BaseExpAST {
  public:
    std::unique_ptr<BaseExpAST>exp;
    void Dump() const;
    void PrintIR(stringstream &fout);
};

class LorExpAST : public BaseExpAST{
  public:
    std::unique_ptr<BaseExpAST>lor_exp,land_exp;
    char op;
    void Dump()const;
    void PrintIR(stringstream &fout);
};

class LandExpAST : public BaseExpAST{
  public:
    std::unique_ptr<BaseExpAST>land_exp,eq_exp;
    char op;
    void Dump()const;
    void PrintIR(stringstream &fout);
};

class EqExpAST : public BaseExpAST{
  public:
    std::unique_ptr<BaseExpAST>eq_exp,rel_exp;
    string op;
    void Dump()const;
    void PrintIR(stringstream &fout);
};

class RelExpAST : public BaseExpAST{
  public:
    std::unique_ptr<BaseExpAST>rel_exp,add_exp;
    string op;
    void Dump()const;
    void PrintIR(stringstream &fout);
};

class AddExpAST : public BaseExpAST{
  public:
    std::unique_ptr<BaseExpAST>add_exp,mul_exp;
    char op;
    void Dump()const;
    void PrintIR(stringstream &fout);
};

class MulExpAST : public BaseExpAST{
  public:
    std::unique_ptr<BaseExpAST>mul_exp,unary_exp;
    char op;
    void Dump()const;
    void PrintIR(stringstream &fout);
};

class UnaryExpAST : public BaseExpAST {
public:
    std::unique_ptr<BaseExpAST> exp;
    char op;
    void Dump()const;
    void PrintIR(stringstream &fout);
};

enum PrimaryExp_kind{
  PrimaryExp_exp,PrimaryExp_number
};

class PrimaryExpAST : public BaseExpAST {
public:
    PrimaryExp_kind type;
    std::unique_ptr<BaseExpAST> exp;
    int number;
    void Dump()const;
    void PrintIR(stringstream &fout);
};