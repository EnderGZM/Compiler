#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include <cstring>
#include <fstream>
#include "riscv.h"
#include "koopa.h"

using namespace std;
void Visit(const koopa_raw_return_t &ret,ofstream &fout) {
    fout<<"  li a0, ";
    Visit(ret.value,fout);
    fout<<"\n  ret\n";
}

void Visit(const koopa_raw_integer_t &ret,ofstream &fout) {
    fout<<ret.value;
}

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

void Visit(const koopa_raw_value_t &value,ofstream &fout) {
  const auto &kind = value->kind;
  switch (kind.tag) {
    case KOOPA_RVT_RETURN:
      Visit(kind.data.ret,fout);
      break;
    case KOOPA_RVT_INTEGER:
      Visit(kind.data.integer,fout);
      break;
    default:
      assert(false);
  }
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