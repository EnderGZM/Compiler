#include <cassert>
#include <cstdio>
#include <iostream>
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

void CompUnitAST::PrintIR()const{
    func_def->PrintIR();
}

void FuncDefAST::PrintIR()const{
    cout<<"fun @"<<ident<<"(): ";
    func_type->PrintIR();
    cout<<"{"<<endl;
    block->PrintIR();
    cout<<"}"<<endl;
}

void FuncTypeAST::PrintIR()const{
    if(type=="int")
        cout<<"i32 ";
}

void BlockAST::PrintIR()const{
    cout<<"\%entry: "<<endl;
    stmt->PrintIR();
    cout<<endl;
}

void StmtAST::PrintIR()const{
    if(type=="return"){
        cout<<"\tret "<<number;
    }
}
