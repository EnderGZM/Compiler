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
void Visit(const koopa_raw_program_t &program,stringstream &fout);
void Visit(const koopa_raw_slice_t &slice,stringstream &fout);
void Visit(const koopa_raw_function_t &func,stringstream &fout);
void Visit(const koopa_raw_basic_block_t &block,stringstream &fout);
void Visit(const koopa_raw_slice_t &slice,stringstream &fout);
string Visit(const koopa_raw_return_t &ret,stringstream &fout);
string Visit(const koopa_raw_integer_t &raw_integer,stringstream &fout);
string Visit(const koopa_raw_value_t &value,stringstream &fout);
string Visit(const koopa_raw_binary_t &binary,stringstream &fout);
void generation(const char* buf,stringstream &fout);
