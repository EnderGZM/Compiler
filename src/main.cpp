#include <cassert>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <fstream>
#include <memory>
#include <sstream>
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
    //ast->Dump();
    stringstream buf;
    stringstream buf2;
    if(strcmp(mode,"-koopa")==0){
        ast->PrintIR(buf); 
        fout<<buf.str();
    }
    if(strcmp(mode,"-riscv")==0){
        ast->PrintIR(buf);
        generation(buf.str().c_str(),buf2);
        fout<<buf2.str()<<endl;
    }
    fout.close();
    return 0;
}