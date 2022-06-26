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


int global_cnt=0;
int cur_top=0,cur_size=0;
int ra_addr=-1;
string regs[16]={"t0","t1","t2","t3","t4","t5","t6","a0","a1","a2","a3","a4","a5","a6","a7","x0"};
koopa_raw_value_t reg_value[16];
int reg_vis[16];

koopa_raw_value_t cur_value;
koopa_raw_function_t cur_func;

std::map<const koopa_raw_value_t, RegInfo>values;

void SaveReg(int id,int addr,stringstream &fout){
    if (addr<(1<<11))
        fout<<"\tsw "<<regs[id]<<", "<<addr<<"(sp)\n";
    else {
        fout<<"\tli s3, "<<addr<<"\n";
        fout<<"\tadd s3, s3, sp\n";
        fout<<"\tsw "<<regs[id]<<", 0(s3)\n";
    }
}

void LoadReg(int id,int addr,stringstream &fout){
    if (addr<(1<<11))
        fout<<"\tlw "<<regs[id]<<", "<<addr<<"(sp)\n";
    else {
        fout<<"\tli s1, "<<addr<<"\n";
        fout<<"\tadd s1, s1, sp\n";
        fout<<"\tlw "<<regs[id]<<", 0(s1)\n";
    }
}

void SaveAll(stringstream &fout) {
    for (int i=0;i<15;++i){
        if(reg_vis[i]==1){
            values[reg_value[i]].reg=-1;
            int addr=values[reg_value[i]].addr;
            if (addr==-1) {
                addr=cur_top;
                cur_top+=4;
                values[reg_value[i]].addr=addr;
                SaveReg(i,addr,fout);
            }
        }
        reg_vis[i] = 0;
    }
}
int getRegister(int stat,stringstream &fout){
    for(int i=0;i<15;++i){
        if(!reg_vis[i]){
            reg_value[i]=cur_value;
            reg_vis[i]=stat;
            return i;
        }
    }
    for(int i=0;i<15;++i){
        if(reg_vis[i]==1){
            values[reg_value[i]].reg=-1;
            int addr=values[reg_value[i]].addr;
            if (addr==-1) {
                addr=cur_top;
                cur_top+=4;
                values[reg_value[i]].addr=addr;
                SaveReg(i,addr,fout);
            }
            reg_value[i]=cur_value;
            reg_vis[i]=stat;
            return i;
        }
    }
    assert(0);
    return -1;
}

void Visit(const koopa_raw_program_t &program,stringstream &fout){
    fout<<"\t.data\n";
    Visit(program.values,fout);
    fout<<"\n";
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

int array_size(koopa_raw_type_t ty){
    int ret=4;
    while(ty->tag==KOOPA_RTT_ARRAY){
        ret*=ty->data.array.len;
        ty=ty->data.array.base;
    }
    return ret;
}

void Visit(const koopa_raw_function_t &func,stringstream &fout) {
    auto old_values=values;
    if (func->bbs.len==0)
        return;
    cur_func=func;
    cur_top=0;
    cur_size=0;
    ra_addr=-1;
    for (int i=0;i<15;++i)
        reg_vis[i]=0;
    cur_value=0;

    fout<<"\t.text\n";
    fout<<"\t.globl "<<func->name+1<<"\n";
    fout<<func->name+1<<":\n";
    int max_arg_num=8;
    for(int i=0;i<func->bbs.len;++i){
        koopa_raw_basic_block_t block=reinterpret_cast<koopa_raw_basic_block_t>(func->bbs.buffer[i]);
        for (int j=0;j<block->insts.len;++j) {
            koopa_raw_value_t val=reinterpret_cast<koopa_raw_value_t>(block->insts.buffer[j]);
            if (val->kind.tag==KOOPA_RVT_ALLOC)
                cur_size+=array_size(val->ty->data.pointer.base);
            else if (val->ty->tag!=KOOPA_RTT_UNIT)
                cur_size+=4;
            if(val->kind.tag==KOOPA_RVT_CALL)
                max_arg_num=max((int)val->kind.data.call.args.len,max_arg_num);
        }
    }
    cur_size+=4*(max_arg_num-8)+64;
    cur_top+=4*(max_arg_num-8);
    cur_size=(cur_size+15)/16*16;
    if(cur_size<(1<<11))
        fout<<"\taddi sp, sp, "<<-cur_size<<"\n";
    else{
        fout<<"\tli t0, "<<-cur_size<<"\n";
        fout<<"\tadd sp, sp, t0\n";
    }

    ra_addr=cur_size-4;
    if (ra_addr<(1<<11))
        fout<<"\tsw ra, "<<ra_addr<<"(sp)\n";
    else{
        fout<<"\tli s1, "<<ra_addr<<"\n";
        fout<<"\tadd s1, s1, sp\n";
        fout<<"\tsw ra, 0(s1)\n";
    }

    for (int i=0;i<func->params.len;++i){
        koopa_raw_value_t ptr=reinterpret_cast<koopa_raw_value_t>(func->params.buffer[i]);
        if(i<8){
            int id=i+7;
            reg_value[id]=ptr;
            reg_vis[id]=1;
            RegInfo info(id,-1,"");
            values[ptr]=info;
        }
        else{
            int addr=cur_size+(i-8)*4;
            RegInfo info(-1,addr,"");
            values[ptr]=info;
        }
    }
    Visit(func->bbs,fout);
    fout<<"\n";
    values=old_values;
}

void Visit(const koopa_raw_basic_block_t &bb,stringstream &fout){
    string name(bb->name+1);
    if (name!="entry")
        fout<<"_"<<cur_func->name+1<<"_"<<name<<":\n";
    Visit(bb->insts,fout);
}

RegInfo Visit(const koopa_raw_value_t &value,stringstream &fout) {
    koopa_raw_value_t lst_value=cur_value;
    cur_value=value;
    RegInfo result(-1,-1,"");

    if(values.find(value)!=values.end()){
        if(values[value].reg==-1){
            int id=getRegister(1,fout);
            values[value].reg=id;
            if(values[value].local_addr==-1)
                LoadReg(id,values[value].addr,fout);
            else{
                int local_addr=values[value].local_addr;
                if(local_addr<(1<<11))
                    fout<<"\taddi "<<regs[id]<<", sp, "<<local_addr<<"\n";
                else{
                    fout<<"\tli "<<regs[id]<<", "<<local_addr<<"\n";
                    fout<<"\tadd "<<regs[id]<<", sp, "<<regs[id]<<"\n";
                }
            }
        }
        result=values[value];
        cur_value=lst_value;
        return result;
    }

    koopa_raw_value_kind_t kind=value->kind;
    switch (kind.tag) {
        case KOOPA_RVT_RETURN:
            Visit(kind.data.ret,fout);
        break;
        case KOOPA_RVT_INTEGER:
            result=Visit(kind.data.integer,fout);
        break;
        case KOOPA_RVT_BINARY:
            result=Visit(kind.data.binary,fout);
            values[value]=result;
        break;
        case KOOPA_RVT_ALLOC:
            result.local_addr=cur_top;
            cur_top+=array_size(value->ty->data.pointer.base);
            values[value]=result;
        break;
        case KOOPA_RVT_LOAD:
            result=Visit(kind.data.load,fout);
            values[value]=result;
        break;
        case KOOPA_RVT_STORE:
            Visit(kind.data.store,fout);
        break;
        case KOOPA_RVT_BRANCH:
            Visit(kind.data.branch,fout);
        break;
        case KOOPA_RVT_JUMP:
            Visit(kind.data.jump,fout);
        break;
        case KOOPA_RVT_CALL:
            result=Visit(kind.data.call,fout);
            if (result.reg!=-1)
                values[value]=result;
        break;
        case KOOPA_RVT_GLOBAL_ALLOC:
            result=Visit(kind.data.global_alloc,fout);
            values[value]=result;
        break;
        case KOOPA_RVT_GET_ELEM_PTR:
            result=Visit(kind.data.get_elem_ptr,fout);
            values[value]=result;
        break;
        case KOOPA_RVT_AGGREGATE:
            Visit(kind.data.aggregate,fout);
        break;
        case KOOPA_RVT_GET_PTR:
            result=Visit(kind.data.get_ptr,fout);
            values[value]=result;
        break;
        default:
            assert(false);
    }
    cur_value=lst_value;
    return result;
}

void Visit(const koopa_raw_return_t &ret,stringstream &fout) {
    koopa_raw_value_t val=ret.value;
    if(val!=NULL){
        RegInfo result=Visit(val,fout);
        if(result.reg!=7)//a0
            fout<<"\tmv a0, "<<regs[result.reg]<<"\n";
    }
    if (ra_addr<(1<<11))
        fout<<"\tlw ra, "<<ra_addr<<"(sp)\n";
    else{
        fout<<"\tli s1, "<<ra_addr<<"\n";
        fout<<"\tadd s1, s1, sp\n";
        fout<<"\tlw ra, 0(s1)\n";
    }
    if(cur_size<(1<<11))
        fout<<"\taddi sp, sp, "<<cur_size<<"\n";
    else{
        fout<<"\tli t0, "<<cur_size<<"\n";
        fout<<"\tadd sp, sp, t0\n";
    }
    fout<<"\tret\n";
}

RegInfo Visit(const koopa_raw_integer_t &raw_integer,stringstream &fout){
    int val=raw_integer.value;
    RegInfo result(-1,-1,"");
    if(val==0){
        result.reg=15;//x0
        return result;
    }
    result.reg=getRegister(0,fout);
    fout<<"\tli "<<regs[result.reg]<<", "<<val<<"\n";
    return result;
}


RegInfo Visit(const koopa_raw_binary_t &binary,stringstream &fout){
    RegInfo Lval=Visit(binary.lhs,fout);
    int lreg=Lval.reg;
    int lst_lreg_vis=reg_vis[lreg];
    reg_vis[lreg]=2;
    RegInfo Rval=Visit(binary.rhs,fout);
    reg_vis[lreg]=lst_lreg_vis;
    RegInfo result(-1,-1,"");
    int id=getRegister(1,fout);
    string name=regs[id];
    result.reg=id;

    string Lname=regs[Lval.reg],Rname=regs[Rval.reg];
    koopa_raw_binary_op_t op=binary.op;

    switch(op){
        case KOOPA_RBO_EQ:
            fout<<"\txor "<<name<<", "<<Lname<<", "<<Rname<<"\n";
            fout<<"\tseqz "<<name<<", "<<name<<"\n";
        break;
        case KOOPA_RBO_SUB:
            fout<<"\tsub "<<name<<", "<<Lname<<", "<<Rname<<"\n";
        break;
        case KOOPA_RBO_ADD:
            fout<<"\tadd "<<name<<", "<<Lname<<", "<<Rname<<"\n";
        break;
        case KOOPA_RBO_MUL:
            fout<<"\tmul "<<name<<", "<<Lname<<", "<<Rname<<"\n";
        break;
        case KOOPA_RBO_DIV:
            fout<<"\tdiv "<<name<<", "<<Lname<<", "<<Rname<<"\n";
        break;
        case KOOPA_RBO_MOD:
            fout<<"\trem "<<name<<", "<<Lname<<", "<<Rname<<"\n";
        break;
        case KOOPA_RBO_OR:
            fout<<"\tor "<<name<<", "<<Lname<<", "<<Rname<<"\n";
        break;
        case KOOPA_RBO_AND:
            fout<<"\tand "<<name<<", "<<Lname<<", "<<Rname<<"\n";
        break;
        case KOOPA_RBO_LT:
            fout<<"\tslt "<<name<<", "<<Lname<<", "<<Rname<<"\n";
        break;
        case KOOPA_RBO_GT:
            fout<<"\tsgt "<<name<<", "<<Lname<<", "<<Rname<<"\n";
        break;
        case KOOPA_RBO_LE:
            fout<<"\tsgt "<<name<<", "<<Lname<<", "<<Rname<<"\n";
            fout<<"\txori "<<name<<", "<<name<<", "<<1<<"\n";
        break;
        case KOOPA_RBO_GE:
            fout<<"\tslt "<<name<<", "<<Lname<<", "<<Rname<<"\n";
            fout<<"\txori "<<name<<", "<<name<<", "<<1<<"\n";
        break;
        case KOOPA_RBO_NOT_EQ:
            fout<<"\txor "<<name<<", "<<Lname<<", "<<Rname<<"\n";
            fout<<"\tsnez "<<name<<", "<<name<<"\n";
        break;
        default:
            assert(0);
    }
    return result;
}

RegInfo Visit(const koopa_raw_load_t &load,stringstream &fout) {
    RegInfo result(-1,-1,"");
    int id=getRegister(1,fout);
    result.reg=id;
    if(values[load.src].local_addr!=-1)
        LoadReg(id,values[load.src].local_addr,fout);
    else if(values[load.src].glob_addr!=""){
        fout<<"\tla s1, "<<values[load.src].glob_addr<<"\n";
        fout<<"\tlw "<<regs[id]<<", 0(s1)"<<"\n";
    }
    else{
        reg_vis[id]=2;
        RegInfo val=Visit(load.src,fout);
        reg_vis[id]=1;
        fout<<"\tlw "<<regs[id]<<", 0("<<regs[val.reg]<<")\n";
    }
    return result;
}

void Visit(const koopa_raw_store_t &store,stringstream &fout){
    RegInfo val=Visit(store.value,fout);
    if(values[store.dest].local_addr!=-1)
        SaveReg(val.reg,values[store.dest].local_addr,fout);
    else if(values[store.dest].glob_addr!=""){
        fout<<"\tla s1, "<<values[store.dest].glob_addr<<"\n";
        fout<<"\tsw "<<regs[val.reg]<<", 0(s1)"<<"\n";
    }
    else{
        int lst_vis=reg_vis[val.reg];
        reg_vis[val.reg]=2;
        RegInfo dst=Visit(store.dest,fout);
        reg_vis[val.reg]=lst_vis;
        fout<<"\tsw "<<regs[val.reg]<<", 0("<<regs[dst.reg]<<")\n";
    }
}

void Visit(const koopa_raw_branch_t &branch,stringstream &fout){
    string func_name(cur_func->name+1),b_true(branch.true_bb->name+1),b_false(branch.false_bb->name+1);
    b_true="_"+func_name+"_"+b_true;
    b_false="_"+func_name+"_"+b_false;
    RegInfo cond=Visit(branch.cond,fout);
    SaveAll(fout);
    fout<<"\tbnez "<<regs[cond.reg]<<", "<<b_true<<"\n";
    fout<<"\tj "<<b_false<<"\n";
}

void Visit(const koopa_raw_jump_t &jump,stringstream &fout) {
  SaveAll(fout);
  string func_name(cur_func->name+1),target(jump.target->name+1);
  target="_"+func_name+"_"+target;
  fout<<"\tj "<<target<<"\n";
}

RegInfo Visit(const koopa_raw_call_t &call,stringstream &fout) {
    RegInfo result(-1,-1,"");
    for(int i=0;i<call.args.len;++i){
        koopa_raw_value_t arg=reinterpret_cast<koopa_raw_value_t>(call.args.buffer[i]);
        int argreg=Visit(arg,fout).reg;
        if(i<8){
            int id=i+7;
            if(id==argreg)
                continue;
            if(reg_vis[id]==1){
                values[reg_value[id]].reg=-1;
                int addr=values[reg_value[id]].addr;
                if (addr==-1) {
                    addr=cur_top;
                    cur_top+=4;
                    values[reg_value[id]].addr=addr;
                    SaveReg(id,addr,fout);
                }
            }
            fout<<"\tmv "<<regs[id]<<", "<<regs[argreg]<<"\n";
            reg_value[id]=arg;
            reg_vis[id]=2;
        }
        else{
            int addr=(i-8)*4;
            SaveReg(argreg,addr,fout);
        }
    }
    SaveAll(fout);
    fout<<"\tcall "<<call.callee->name+1<<"\n";
    if(cur_value->ty->tag!=KOOPA_RTT_UNIT){
        reg_value[7]=cur_value;
        reg_vis[7]=1;
        result.reg=7;
    }
    return result;
}

RegInfo Visit(const koopa_raw_global_alloc_t &global,stringstream &fout){
    string name="global_var_"+to_string(global_cnt++);
    RegInfo result(-1,-1,name);
    fout<<"\t.globl "<<name<<"\n";
    fout<<name<<":\n";
    koopa_raw_value_kind_t kind=global.init->kind;
    int val,len;
    switch(kind.tag){
        case KOOPA_RVT_INTEGER:
            val=kind.data.integer.value;
            fout<<"\t.word "<<val<<"\n";
        break;
        case KOOPA_RVT_ZERO_INIT:
            len=array_size(cur_value->ty->data.pointer.base)>>2;
            for(int i=0;i<len;++i)
                fout<<"\t.zero 4\n";
        break;
        case KOOPA_RVT_AGGREGATE:
            Visit(global.init,fout);
        break;
        default:
            assert(0);
    }
    return result;
}

RegInfo Visit(const koopa_raw_get_elem_ptr_t &get_elem_ptr,stringstream &fout){
    koopa_raw_value_t src=get_elem_ptr.src;
    RegInfo result(-1,-1,"");
    RegInfo index=Visit(get_elem_ptr.index,fout);
    int len=array_size(src->ty->data.pointer.base->data.array.base);
    //fout<<"#check src addr: "<<values[src].addr<<endl;
    if(values[src].local_addr!=-1){
        int addr=values[src].local_addr;
        if(addr<(1<<11))
            fout<<"\taddi s2, sp, "<<addr<<"\n";
        else{
            fout<<"\tli s1, "<<addr<<"\n";
            fout<<"\tadd s2, sp, s1\n";
        }
        fout<<"\tli s1, "<<len<<"\n";
        fout<<"\tmul s1, "<<regs[index.reg]<<", s1\n";
        int id=getRegister(1,fout);
        result.reg=id;
        fout<<"\tadd "<<regs[id]<<", s2, s1\n";
    }
    else if(values[src].glob_addr!=""){
        int id=getRegister(1,fout);
        result.reg=id;
        fout<<"\tla s2, "<<values[src].glob_addr<<"\n";
        fout<<"\tli s1, "<<len<<"\n";
        fout<<"\tmul s1, "<<regs[index.reg]<<", s1\n";
        fout<<"\tadd "<<regs[id]<<", s2, s1\n";
    }
    else{
        int lst_vis=reg_vis[index.reg];
        reg_vis[index.reg]=2;
        RegInfo srcreg=Visit(src,fout);
        reg_vis[index.reg]=lst_vis;

        fout<<"\tli s1, "<<len<<"\n";
        fout<<"\tmul s1, s1, "<<regs[index.reg]<<"\n";
        
        lst_vis=reg_vis[srcreg.reg];
        reg_vis[srcreg.reg]=2;
        int id=getRegister(1,fout);
        result.reg=id;
        reg_vis[srcreg.reg]=lst_vis;

        fout<<"\tadd "<<regs[id]<<", s1, "<<regs[srcreg.reg]<<"\n";
    }

    return result;
}

RegInfo Visit(const koopa_raw_get_ptr_t  &get_ptr,stringstream &fout){
    koopa_raw_value_t src=get_ptr.src;
    RegInfo result(-1,-1,"");
    RegInfo index=Visit(get_ptr.index,fout);
    int len=array_size(src->ty->data.pointer.base);
    if(values[src].local_addr!=-1){
        int addr=values[src].local_addr;
        if(addr<(1<<11))
            fout<<"\taddi s2, sp, "<<addr<<"\n";
        else{
            fout<<"\tli s1, "<<addr<<"\n";
            fout<<"\tadd s2, sp, s1\n";
        }
        fout<<"\tli s1, "<<len<<"\n";
        fout<<"\tmul s1, "<<regs[index.reg]<<", s1\n";
        int id=getRegister(1,fout);
        result.reg=id;
        fout<<"\tadd "<<regs[id]<<", s2, s1\n";
    }
    else if(values[src].glob_addr!=""){
        int id=getRegister(1,fout);
        result.reg=id;
        fout<<"\tla s2, "<<values[src].glob_addr<<"\n";
        fout<<"\tli s1, "<<len<<"\n";
        fout<<"\tmul s1, "<<regs[index.reg]<<", s1\n";
        fout<<"\tadd "<<regs[id]<<", s2, s1\n";
    }
    else{
        int lst_vis=reg_vis[index.reg];
        reg_vis[index.reg]=2;
        RegInfo srcreg=Visit(src,fout);
        reg_vis[index.reg]=lst_vis;

        fout<<"\tli s1, "<<len<<"\n";
        fout<<"\tmul s1, s1, "<<regs[index.reg]<<"\n";

        lst_vis=reg_vis[srcreg.reg];
        reg_vis[srcreg.reg]=2;
        int id=getRegister(1,fout);
        result.reg=id;
        reg_vis[srcreg.reg]=lst_vis;

        fout<<"\tadd "<<regs[id]<<", s1, "<<regs[srcreg.reg]<<"\n";
    }

    return result;
}

void Visit(const koopa_raw_aggregate_t &aggregate,stringstream &fout) {
    const koopa_raw_slice_t &elems=aggregate.elems;
    for(int i=0;i<elems.len;++i){
        koopa_raw_value_t elem = reinterpret_cast<koopa_raw_value_t>(elems.buffer[i]);
        koopa_raw_value_kind_t kind=elem->kind;
        int val;
        switch(kind.tag){
            case KOOPA_RVT_INTEGER:
                val=kind.data.integer.value;
                fout<<"\t.word "<<val<<"\n";
            break;
            case KOOPA_RVT_AGGREGATE:
                Visit(elem,fout);
            break;
            default:
                assert(0);
        }
    }
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