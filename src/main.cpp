#include <cassert>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <fstream>
#include <memory>
#include "AST.h"
#include "riscv.h"
#include "koopa.h"

using namespace std;

extern FILE *yyin;
extern int yyparse(unique_ptr<BaseAST> &ast);

int main(int argc, const char *argv[]) {
    assert(argc == 5);
    auto mode = argv[1];
    auto input = argv[2];
    auto output = argv[4];

    ofstream fout(output);
    yyin = fopen(input, "r");
    assert(yyin);

    unique_ptr<BaseAST> ast;
    auto ret = yyparse(ast);
    assert(!ret);
    /*
    ast->Dump();
    cout<<endl;
    */
    if(strcmp(mode,"-koopa")==0){
        ast->PrintIR(fout); 
    }
    if(strcmp(mode,"-riscv")==0){
        string tmpFileName=output;
        tmpFileName="_"+tmpFileName+"_tmp";
        ofstream ftmpout(tmpFileName);
        ast->PrintIR(ftmpout);
        ftmpout.close();
        FILE *ftmpin=fopen(tmpFileName.c_str(),"r");
        char *buf=(char*)malloc(100000);
        fread(buf,1,100000,ftmpin);
        generation(buf,fout);
    }
    fout.close();
    return 0;
}