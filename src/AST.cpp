#include <cassert>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include "AST.h"
using namespace std;

void CompUnitAST::Dump()const{
    std::cout << "CompUnitAST { ";
    func_def->Dump();
    std::cout << " }";
}

void FuncDefAST::Dump()const{
    std::cout << "FuncDefAST { ";
    func_type->Dump();
    std::cout << ", " << ident << ", ";
    block->Dump();
    std::cout << " }";
}

void FuncTypeAST::Dump()const{
    cout << "FuncTypeAST { ";
    cout<<type<<" }";
}

void BlockAST::Dump()const{
    cout<< "BlockAST{ ";
    stmt->Dump();
    cout<<" }";
}

void StmtAST::Dump()const{
    cout<<"StmtAST{";
    cout<<"type:"<<type;
    exp->Dump();
    cout<<"}";
}

void ExpAST::Dump()const{
    unary_exp->Dump();
}

void UnaryExpAST::Dump()const{
    if(type==UnaryExp_primary){
        exp->Dump();
    }
    else if(type==UnaryExp_Unary){
        cout<<op;
        exp->Dump();
    }
}

void PrimaryExpAST::Dump()const{
    if(type==PrimaryExp_exp){
        cout<<'(';
        exp->Dump();
        cout<<')';
    }
    else if(type==PrimaryExp_number){
        cout<<number;
    }
}

void CompUnitAST::PrintIR(ofstream &fout){
    func_def->PrintIR(fout);
}

void FuncDefAST::PrintIR(ofstream &fout){
    fout<<"fun @"<<ident<<"(): ";
    func_type->PrintIR(fout);
    fout<<"{\n";
    block->PrintIR(fout);
    fout<<"}\n";
}

void FuncTypeAST::PrintIR(ofstream &fout){
    if(type==Type_int)
        fout<<"i32";
}

void BlockAST::PrintIR(ofstream &fout){
    fout<<"\%entry: \n";
    stmt->PrintIR(fout);
    fout<<"\n";
}

int var_cnt=0;

void StmtAST::PrintIR(ofstream &fout){
    if(type==Stmt_ret){
        exp->PrintIR(fout);
        fout <<"\tret \%"<<exp->result<<"\n";
    }
}

void ExpAST::PrintIR(ofstream &fout){
    unary_exp->PrintIR(fout);
    string a,b;
    result=unary_exp->result;
}

void UnaryExpAST::PrintIR(ofstream &fout){
    if(type==UnaryExp_primary){
        exp->PrintIR(fout);
        result=exp->result;
    }
    else if(type==UnaryExp_Unary){
        exp->PrintIR(fout);
        if(op=='+'){
            result=exp->result;
        }
        else if(op=='-'){
            result="%"+to_string(var_cnt++);
            fout<<"\t"<<result<<" = sub 0, "<<exp->result<<"\n";
        }
        else if(op=='!'){
            result="%"+to_string(var_cnt++);
            fout<<"\t"<<result<<" = eq "<<exp->result<<", 0\n";
        }
    }
}

void PrimaryExpAST::PrintIR(ofstream &fout){
    if(type==PrimaryExp_number)
        result=to_string(number);
    else if(type==PrimaryExp_exp){
        exp->PrintIR(fout);
        result=exp->result;
    }
}
