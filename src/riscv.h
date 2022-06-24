#pragma once
#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include <cstring>
#include <sstream>
#include <sstream>
#include "koopa.h"

using namespace std;

class RegInfo{
  public:
    int addr;
    int reg;
    string glob_addr;
    RegInfo(){reg=-1;addr=-1;glob_addr="";}
    RegInfo(int _reg,int _addr,string _glob_addr){reg=_reg;addr=_addr;glob_addr=_glob_addr;}
};


void Visit(const koopa_raw_program_t &program,stringstream &fout);
void Visit(const koopa_raw_slice_t &slice,stringstream &fout);
void Visit(const koopa_raw_function_t &func,stringstream &fout);
void Visit(const koopa_raw_basic_block_t &block,stringstream &fout);
void Visit(const koopa_raw_return_t &ret,stringstream &fout);

RegInfo Visit(const koopa_raw_integer_t &raw_integer,stringstream &fout);
RegInfo Visit(const koopa_raw_value_t &value,stringstream &fout);
RegInfo Visit(const koopa_raw_binary_t &binary,stringstream &fout);
RegInfo Visit(const koopa_raw_load_t &load,stringstream &fout);

void Visit(const koopa_raw_store_t &store,stringstream &fout);
void Visit(const koopa_raw_branch_t &branch,stringstream &fout);
void Visit(const koopa_raw_jump_t &jump,stringstream &fout);

RegInfo Visit(const koopa_raw_call_t &call,stringstream &fout);
RegInfo Visit(const koopa_raw_global_alloc_t &global,stringstream &fout);
RegInfo Visit(const koopa_raw_get_elem_ptr_t &get_elem_ptr,stringstream &fout);
RegInfo Visit(const koopa_raw_get_ptr_t &get_ptr,stringstream &fout);

void SaveAll(stringstream &fout);
void Visit(const koopa_raw_aggregate_t &aggregate,stringstream &fout);
int getRegister(int stat,stringstream &fout);
int array_size(koopa_raw_type_t ty);

void generation(const char* buf,stringstream &fout);
