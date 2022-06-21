#pragma once
#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>

using namespace std;

class BaseAST {
  public:
    virtual ~BaseAST() = default;
    virtual void Dump() const = 0;
    virtual void PrintIR() const=0;
};

class CompUnitAST : public BaseAST {
  public:
    unique_ptr<BaseAST> func_def;
    void Dump() const override;
    void PrintIR() const override;
};

class FuncDefAST : public BaseAST {
  public:
    unique_ptr<BaseAST> func_type;
    string ident;
    unique_ptr<BaseAST> block;
    void Dump() const override;
    void PrintIR() const override;
};

class FuncTypeAST : public BaseAST{
  public:
    string type;
    void Dump() const override;
    void PrintIR() const override;
};

class BlockAST : public BaseAST{
  public:
    unique_ptr<BaseAST> stmt;
    void Dump() const override;
    void PrintIR() const override;
};

class StmtAST : public BaseAST{
  public:
    string type;
    int number;
    void Dump() const override;
    void PrintIR() const override;
};
