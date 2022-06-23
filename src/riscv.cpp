#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include <cstring>
#include <fstream>
#include <map>
#include "riscv.h"
#include "koopa.h"

using namespace std;

void Visit(const koopa_raw_program_t &program,ofstream &fout){
    fout<<"  .text\n";
    Visit(program.values,fout);
    Visit(program.funcs,fout);
}

void Visit(const koopa_raw_slice_t &slice,ofstream &fout) {
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

void Visit(const koopa_raw_function_t &func,ofstream &fout) {
  fout<<"  .globl "<<func->name+1<<"\n";
  fout<<func->name+1<<":\n";
  Visit(func->bbs,fout);
}

void Visit(const koopa_raw_basic_block_t &bb,ofstream &fout) {
  Visit(bb->insts,fout);
}

int register_cnt=0;
map<koopa_raw_value_t,string> valuemap;
string Visit(const koopa_raw_value_t &value,ofstream &fout) {
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
    valuemap[value]=result;
    return result;
}

string Visit(const koopa_raw_return_t &ret,ofstream &fout) {
    koopa_raw_value_t ret_value=ret.value;
    string result=Visit(ret_value,fout);
    fout<<"\tmv a0, "<<result<<"\n";
    fout<<"\tret\n";
    return result;
}

string Visit(const koopa_raw_integer_t &raw_integer,ofstream &fout){
    int val=raw_integer.value;
    if(val==0)
        return "x0";
    string result=register_cnt<7?("t"+to_string(register_cnt)):("a"+to_string(register_cnt-7));
    ++register_cnt;
    fout<<"\tli "<<result<<", "<<val<<"\n";
    return result;
}

string Visit(const koopa_raw_binary_t &binary,ofstream &fout){
    koopa_raw_binary_op_t op=binary.op;
    string Lval=Visit(binary.lhs,fout);
    string Rval=Visit(binary.rhs,fout);
    string result=register_cnt<7?("t"+to_string(register_cnt)):("a"+to_string(register_cnt-7));
    ++register_cnt;
    switch(op){
        case KOOPA_RBO_EQ:
            fout<<"\txor "<<result<<", "<<Lval<<", "<<Rval<<"\n";
            fout<<"\tseqz "<<result<<", "<<result<<"\n";
        break;
        case KOOPA_RBO_SUB:
            fout<<"\tsub "<<result<<", "<<Lval<<", "<<Rval<<"\n";
        break;
        default:
            assert(false);
    }
    return result;
}
void generation(const char* buf,ofstream &fout){
    koopa_program_t program;
    koopa_error_code_t ret = koopa_parse_from_string(buf, &program);
    assert(ret == KOOPA_EC_SUCCESS);
    koopa_raw_program_builder_t builder = koopa_new_raw_program_builder();
    koopa_raw_program_t raw = koopa_build_raw_program(builder, program);
    koopa_delete_program(program);
    Visit(raw,fout);
    koopa_delete_raw_program_builder(builder);
}