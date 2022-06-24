#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include <cstring>
#include <fstream>
#include <map>
#include <sstream>
#include "riscv.h"
#include "koopa.h"

using namespace std;

void Visit(const koopa_raw_program_t &program,stringstream &fout){
    fout<<"  .text\n";
    Visit(program.values,fout);
    Visit(program.funcs,fout);
}

void Visit(const koopa_raw_slice_t &slice,stringstream &fout) {
  for (size_t i = 0; i < slice.len; ++i) {
    auto ptr=slice.buffer[i];
    switch (slice.kind) {
      case KOOPA_RSIK_FUNCTION:
        Visit(reinterpret_cast<koopa_raw_function_t>(ptr),fout);
        break;
      case KOOPA_RSIK_BASIC_BLOCK:
        Visit(reinterpret_cast<koopa_raw_basic_block_t>(ptr),fout);
        break;
      case KOOPA_RSIK_VALUE:
        Visit(reinterpret_cast<koopa_raw_value_t>(ptr),fout);
        break;
      default:
        assert(false);
    }
  }
}

void Visit(const koopa_raw_function_t &func,stringstream &fout) {
  fout<<"  .globl "<<func->name+1<<"\n";
  fout<<func->name+1<<":\n";
  Visit(func->bbs,fout);
}

void Visit(const koopa_raw_basic_block_t &bb,stringstream &fout) {
  Visit(bb->insts,fout);
}

int register_cnt=0,cur_time=0;
map<koopa_raw_value_t,string> valuemap;
koopa_raw_value_t stringmap[15];
int lst[15];

string getnewreg(){
    string ret;
    if(register_cnt<15){
        ret=register_cnt<7?"t"+to_string(register_cnt):"a"+to_string(register_cnt-7);
        ++register_cnt;
    }
    else{
        int id=0;
        for(int i=0;i<15;++i)
            if(lst[i]<lst[id])
                id=i;
        ret=id<7?"t"+to_string(id):"a"+to_string(id-7);
        valuemap.erase(stringmap[id]);
    }
    return ret;
}

int reg2id(string a){
    if(a[0]=='t')
        return a[1]-'0';
    else if(a[0]=='a')
        return a[1]-'0'+7;
    cout<<"!"<<a<<endl;
    assert(0);
    return 0;
}

string Visit(const koopa_raw_value_t &value,stringstream &fout) {
    if(valuemap.find(value)!=valuemap.end())
        return valuemap[value];
    const auto &kind = value->kind;
    string result;
    switch (kind.tag) {
        case KOOPA_RVT_RETURN:
            result=Visit(kind.data.ret,fout);
            break;
        case KOOPA_RVT_INTEGER:
            result=Visit(kind.data.integer,fout);
            break;
        case KOOPA_RVT_BINARY:
            result=Visit(kind.data.binary,fout);
            break;
        default:
            assert(false);
    }
    ++cur_time;
    valuemap[value]=result;
    if(result!="x0"){
        stringmap[reg2id(result)]=value;
        lst[reg2id(result)]=cur_time;
    }
    return result;
}

string Visit(const koopa_raw_return_t &ret,stringstream &fout) {
    koopa_raw_value_t ret_value=ret.value;
    string result=Visit(ret_value,fout);
    fout<<"\tmv a0, "<<result<<"\n";
    fout<<"\tret\n";
    return result;
}

string Visit(const koopa_raw_integer_t &raw_integer,stringstream &fout){
    int val=raw_integer.value;
    if(val==0)
        return "x0";
    string result=getnewreg();
    fout<<"\tli "<<result<<", "<<val<<"\n";
    return result;
}

string Visit(const koopa_raw_binary_t &binary,stringstream &fout){
    koopa_raw_binary_op_t op=binary.op;
    string Lval=Visit(binary.lhs,fout);
    string Rval=Visit(binary.rhs,fout);
    string result=getnewreg();
    switch(op){
        case KOOPA_RBO_EQ:
            fout<<"\txor "<<result<<", "<<Lval<<", "<<Rval<<"\n";
            fout<<"\tseqz "<<result<<", "<<result<<"\n";
        break;
        case KOOPA_RBO_SUB:
            fout<<"\tsub "<<result<<", "<<Lval<<", "<<Rval<<"\n";
        break;
        case KOOPA_RBO_ADD:
            fout<<"\tadd "<<result<<", "<<Lval<<", "<<Rval<<"\n";
        break;
        case KOOPA_RBO_MUL:
            fout<<"\tmul "<<result<<", "<<Lval<<", "<<Rval<<"\n";
        break;
        case KOOPA_RBO_DIV:
            fout<<"\tdiv "<<result<<", "<<Lval<<", "<<Rval<<"\n";
        break;
        case KOOPA_RBO_MOD:
            fout<<"\trem "<<result<<", "<<Lval<<", "<<Rval<<"\n";
        break;
        case KOOPA_RBO_OR:
            fout<<"\tor "<<result<<", "<<Lval<<", "<<Rval<<"\n";
        break;
        case KOOPA_RBO_AND:
            fout<<"\tand "<<result<<", "<<Lval<<", "<<Rval<<"\n";
        break;
        case KOOPA_RBO_LT:
            fout<<"\tslt "<<result<<", "<<Lval<<", "<<Rval<<"\n";
        break;
        case KOOPA_RBO_GT:
            fout<<"\tsgt "<<result<<", "<<Lval<<", "<<Rval<<"\n";
        break;
        case KOOPA_RBO_LE:
            fout<<"\tsgt "<<result<<", "<<Lval<<", "<<Rval<<"\n";
            fout<<"\txori "<<result<<", "<<result<<", "<<1<<"\n";
        break;
        case KOOPA_RBO_GE:
            fout<<"\tslt "<<result<<", "<<Lval<<", "<<Rval<<"\n";
            fout<<"\txori "<<result<<", "<<result<<", "<<1<<"\n";
        break;
        case KOOPA_RBO_NOT_EQ:
            fout<<"\txor "<<result<<", "<<Lval<<", "<<Rval<<"\n";
            fout<<"\tsnez "<<result<<", "<<result<<"\n";
        break;
        default:
            assert(0);
    }
    return result;
}
void generation(const char* buf,stringstream &fout){
    koopa_program_t program;
    koopa_error_code_t ret = koopa_parse_from_string(buf, &program);
    assert(ret == KOOPA_EC_SUCCESS);
    koopa_raw_program_builder_t builder = koopa_new_raw_program_builder();
    koopa_raw_program_t raw = koopa_build_raw_program(builder, program);
    koopa_delete_program(program);
    Visit(raw,fout);
    koopa_delete_raw_program_builder(builder);
}