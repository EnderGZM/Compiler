#include <cassert>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include <AST.h>
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
    cout<<"type:"<<type<<",number:"<<number<<"}";
}

void CompUnitAST::PrintIR(ofstream &fout)const{
    func_def->PrintIR(fout);
}

void FuncDefAST::PrintIR(ofstream &fout)const{
    fout<<"fun @"<<ident<<"(): ";
    func_type->PrintIR(fout);
    fout<<"{\n";
    block->PrintIR(fout);
    fout<<"}\n";
}

void FuncTypeAST::PrintIR(ofstream &fout)const{
    if(type=="int")
        fout<<"i32";
}

void BlockAST::PrintIR(ofstream &fout)const{
    fout<<"\%entry: \n";
    stmt->PrintIR(fout);
    fout<<"\n";
}

void StmtAST::PrintIR(ofstream &fout)const{
    if(type=="return"){
        fout<<"\tret ";
        fout<<number;
    }
}
