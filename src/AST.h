#pragma once
#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include <cstring>

using namespace std;

class BaseAST {
  public:
    virtual ~BaseAST() = default;
    virtual void Dump() const = 0;
    virtual void PrintIR(ofstream &fout);
};

class CompUnitAST : public BaseAST {
  public:
    unique_ptr<BaseAST> func_def;
    void Dump() const override;
    void PrintIR(ofstream &fout)  override;
};

class FuncDefAST : public BaseAST {
  public:
    unique_ptr<BaseAST> func_type;
    string ident;
    unique_ptr<BaseAST> block;
    void Dump() const override;
    void PrintIR(ofstream &fout)  override;
};

enum Type_kind{
  Type_int,Type_void
};

class FuncTypeAST : public BaseAST{
  public:
    Type_kind type;
    void Dump() const override;
    void PrintIR(ofstream &fout)  override;
};

class BlockAST : public BaseAST{
  public:
    unique_ptr<BaseAST> stmt;
    void Dump() const override;
    void PrintIR(ofstream &fout)  override;
};

class BaseExpAST : public BaseAST{
  public:
    string result;
    void Dump() const override{}
    void PrintIR(ofstream &fout) override{}
};

enum Stmt_kind{
  Stmt_ret
};

class StmtAST : public BaseAST{
  public:
    Stmt_kind type;
    std::unique_ptr<BaseExpAST> exp;
    void Dump() const override;
    void PrintIR(ofstream &fout)  override;
};

class ExpAST : public BaseExpAST {
  public:
    std::unique_ptr<BaseExpAST>unary_exp;
    string result;
    void Dump() const override;
    void PrintIR(ofstream &fout)  override;
};

enum UnaryExp_kind{
  UnaryExp_primary,UnaryExp_Unary
};

class UnaryExpAST : public BaseExpAST {
public:
    UnaryExp_kind type;
    std::unique_ptr<BaseExpAST> exp;
    string result;
    char op;
    void Dump()const override;
    void PrintIR(ofstream &fout)  override;
};

enum PrimaryExp_kind{
  PrimaryExp_exp,PrimaryExp_number
};

class PrimaryExpAST : public BaseExpAST {
public:
    PrimaryExp_kind type;
    std::unique_ptr<BaseExpAST> exp;
    string result;
    int number;
    void Dump()const override;
    void PrintIR(ofstream &fout)  override;
};
