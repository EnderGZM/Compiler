#pragma once
#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include <cstring>
#include <sstream>
#include "koopa.h"

using namespace std;
void Visit(const koopa_raw_slice_t &slice,ofstream &fout);
void Visit(const koopa_raw_function_t &func,ofstream &fout);
void Visit(const koopa_raw_basic_block_t &block,ofstream &fout);
void Visit(const koopa_raw_slice_t &slice,ofstream &fout);
void Visit(const koopa_raw_value_t &value,ofstream &fout);
void generation(const char* buf,ofstream &fout);
