#include <cassert>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include <sstream>
#include <variant>
#include <map>
#include "AST.h"
using namespace std;

vector<map<string,variant<int,string> > > symbol_tables;
vector<string>cur_args;
map<string, int>func_type;
map<string, int>is_ptr;
map<string, int>arr_dim;

int var_cnt=0;
int if_cnt=0;
int while_cnt=0;
int cut_cnt=0;
int in_call_func=0;
bool is_global=0;
vector<int>cur_array_int;
vector<int>cur_len;
vector<string>cur_array_string;
string cur_while_entry;
string cur_while_end;

variant<int,string> find_symbol(string name){
    int sz=symbol_tables.size();
    for(int i=sz-1;i>=0;--i)
        if(symbol_tables[i].find(name)!=symbol_tables[i].end())
            return symbol_tables[i][name];
    assert(0);
    return "";
}

void Print_Array_Int(stringstream &fout,int pos=0){
    int sz=cur_len.size();
    if(sz==0){
        fout<<to_string(cur_array_int[pos]);
        ++pos;
        return;
    }
    int num=cur_len[sz-1];
    cur_len.pop_back();
    fout<<"{";
    int step=1;
    for(int i=0;i<sz-1;++i)
        step*=cur_len[i];
    for(int i=0;i<num;++i){
        Print_Array_Int(fout,pos);
        pos+=step;
        if(i!=num-1)
            fout<<", ";
    }
    fout<<"}";
    cur_len.push_back(num);
}

void CompUnitAST::PrintIR(stringstream &fout){
    fout<<"decl @getint(): i32\n";
    fout<<"decl @getch(): i32\n";
    fout<<"decl @getarray(*i32): i32\n";
    fout<<"decl @putint(i32)\n";
    fout<<"decl @putch(i32)\n";
    fout<<"decl @putarray(i32, *i32)\n";
    fout<<"decl @starttime()\n";
    fout<<"decl @stoptime()\n";
    fout<<"\n";

    func_type["getint"] = Type_int;
    func_type["getch"] = Type_int;
    func_type["getarray"] = Type_int;
    func_type["putint"] = Type_void;
    func_type["putch"] =Type_void;;
    func_type["putarray"] =Type_void;;
    func_type["starttime"] =Type_void;
    func_type["stoptime"] =Type_void;

    map<string,variant<int,string> >global_map;
    symbol_tables.push_back(global_map);
    is_global=1;
    for(auto it=global_decl_list->begin();it!=global_decl_list->end();++it)
        (*it)->Global_Alloc(fout);
    is_global=0;
    for(auto it=func_def_list->begin();it!=func_def_list->end();++it)
        (*it)->PrintIR(fout);
    symbol_tables.pop_back();
}

void ConstDeclAST::Global_Alloc(stringstream &fout){
    for(auto it=def_list->begin();it!=def_list->end();++it)
        (*it)->Global_Alloc(fout);
}

void VarDeclAST::Global_Alloc(stringstream &fout){
    for(auto it=def_list->begin();it!=def_list->end();++it)
        (*it)->Global_Alloc(fout);
}

void ConstDefAST::Global_Alloc(stringstream &fout){
    int cur_id=symbol_tables.size()-1;
    string name="@"+ident+"_"+to_string(var_cnt++);
    symbol_tables[cur_id][ident]=name;
    if(len->size()==0){
        PrintIR(fout);
        return;
    }
    int sz=len->size();
    arr_dim[name]=sz;
    cur_len.clear();
    string alloc_str;
    for (int i=sz-1;i>=0;--i) {
        int val=(*len)[i]->Calc();
        cur_len.push_back(val);
        if (i==sz-1)
            alloc_str="[i32, "+to_string(val)+"]";
        else
            alloc_str="["+alloc_str+", "+to_string(val)+"]";
    }
    fout<<"global "<<name<<" = alloc "<<alloc_str<<", ";
    cur_array_int.clear();
    init->PrintIR(fout);
    Print_Array_Int(fout);
    fout<<"\n";
}

void VarDefAST::Global_Alloc(stringstream &fout){
    int cur_id=symbol_tables.size()-1;
    string name="@"+ident+"_"+to_string(var_cnt++);
    symbol_tables[cur_id][ident]=name;
    if(len->size()==0){
        fout<<"global "<<name<<" = alloc i32, ";
        if(init!=NULL){
            int val=((VarInitAST*)(init.get()))->Calc();
            if(val==0)
                fout<<"zeroinit\n";
            else
                fout<<val<<"\n";
        }
        else
            fout<<"zeroinit\n";
    }
    else{
        int sz=len->size();
        arr_dim[name]=sz;
        cur_len.clear();
        string alloc_str;
        for (int i=sz-1;i>=0;--i) {
            int val=(*len)[i]->Calc();
            cur_len.push_back(val);
            if (i==sz-1)
                alloc_str="[i32, "+to_string(val)+"]";
            else
                alloc_str="["+alloc_str+", "+to_string(val)+"]";
        }
        fout<<"global "<<name<<" = alloc "<<alloc_str<<", ";
        if(init!=NULL){
            cur_array_int.clear();
            init->PrintIR(fout);
            Print_Array_Int(fout);
            fout<<"\n";
        }
        else
            fout<<"zeroinit\n";
    }
}

void FuncDefAST::PrintIR(stringstream &fout){
    fout<<"fun @"<<ident<<"(";
    map<string,variant<int,string> >args_map;
    symbol_tables.push_back(args_map);

    int sz=params->size();
    cur_args.clear();
    for(int i=0;i<sz;++i){
        (*params)[i]->PrintIR(fout);
        if(i!=sz-1)
            fout<<", ";
    }
    if(type==Type_int)
        fout<<") : i32 {\n";
    else
        fout<<") {\n";
    func_type[ident]=type;

    fout<<"\%entry:\n";
    for(auto it=cur_args.begin();it!=cur_args.end();++it)
        fout<<*it;
    
    block->PrintIR(fout);
    if(type==Type_int)
        fout<<"\tret 0\n";
    else
        fout<<"\tret\n";
    fout<<"}\n";
    symbol_tables.pop_back();
}

void FuncFParamAST::PrintIR(stringstream &fout){
    int cur_id=symbol_tables.size()-1;
    string name="@param_"+ident,result="%param_"+ident;
    symbol_tables[cur_id][ident]=result;
    if (is_array) {
        int sz=len->size();

        string alloc_str = "i32";
        for (int i=sz-1;i>=0;--i){
            int val=((ConstExpAST*)((*len)[i].get()))->Calc();
            alloc_str="["+alloc_str+","+to_string(val)+"]";
        }
        fout<<name<<": *"<<alloc_str;
        is_ptr[result]=1;
        arr_dim[result]=sz+1;
        cur_args.push_back("\t"+result+" = alloc *"+alloc_str+"\n\tstore "+name+", "+result+"\n");
    }
    else {
        fout<<name<<": i32";
        symbol_tables[cur_id][ident]=result;
        cur_args.push_back("\t"+result+" = alloc i32\n\tstore "+name+", "+result+"\n");
    }
}

void BlockAST::PrintIR(stringstream &fout){
    map<string,variant<int,string> >symbol_table;
    symbol_tables.push_back(symbol_table);
    for(auto it=blockitem_list->begin();it!=blockitem_list->end();it++)
        (*it)->PrintIR(fout);
    symbol_tables.pop_back();
}

void ConstDeclAST::PrintIR(stringstream &fout){
    for(auto it=def_list->begin();it!=def_list->end();++it)
        (*it)->PrintIR(fout);
}

void ConstDefAST::PrintIR(stringstream &fout){
    int cur_id=symbol_tables.size()-1;
    if(len->size()==0){
        int val=init->Calc();
        symbol_tables[cur_id][ident]=val;
    }
    else{
        string name="@"+ident+"_"+to_string(var_cnt++);
        symbol_tables[cur_id][ident]=name;
        int total=1;
        int sz=len->size();
        arr_dim[name]=sz;
        cur_len.clear();
        string alloc_str;
        for (int i=sz-1;i>=0;--i) {
            int val=(*len)[i]->Calc();
            total*=val;
            cur_len.push_back(val);
            if (i==sz-1)
                alloc_str="[i32, "+to_string(val)+"]";
            else
                alloc_str="["+alloc_str+", "+to_string(val)+"]";
        }
        fout<<"\t"<<name<<" = alloc "<<alloc_str<<"\n";
        cur_array_int.clear();
        init->PrintIR(fout);
        for (int i=0;i<total;++i){
            int step=total,p=i;
            string newtmp,tmp=name;
            for (int j=sz-1;j>=0;--j){
                step/=cur_len[j];
                int idx=p/step;
                p-=idx*step;
                newtmp="%"+to_string(var_cnt++);
                fout<<"\t"<<newtmp<<" = getelemptr "<<tmp<<", "<<idx<<"\n";
                tmp=newtmp;
            }
            fout<<"\tstore "<<cur_array_int[i]<<", "<<tmp<<"\n";
        }
    }
}

void ConstInitAST::PrintIR(stringstream &fout){
    int sz=cur_len.size();
    int total=1;
    for(int i=0;i<sz;++i)
        total*=cur_len[i];
    int num=cur_len[sz-1];
    cur_len.pop_back();
    int set_num=init->size();
    int p=0;
    for(int i=0;i<set_num;++i){
        ConstInitAST *cur=(ConstInitAST*)((*init)[i].get());
        if(cur->exp!=NULL){
            cur_array_int.push_back(cur->Calc());
            ++p;
        }
        else{
            cur->PrintIR(fout);
            p+=total/num;
        }
    }
    for(;p<total;++p)
        cur_array_int.push_back(0);
    cur_len.push_back(num);
}

void VarDeclAST::PrintIR(stringstream &fout){
    for(auto it=def_list->begin();it!=def_list->end();++it)
        (*it)->PrintIR(fout);
}

void VarDefAST::PrintIR(stringstream &fout){
    int cur_id=symbol_tables.size()-1;
    string name="@"+ident+"_"+to_string(var_cnt++);
    symbol_tables[cur_id][ident]=name;
    if(len->size()==0){
        fout<<"\t"<<name<<" = alloc i32\n";
        if(init!=NULL){
            VarInitAST *exp=(VarInitAST*)(init.get());
            exp->PrintIR(fout);
            fout<<"\tstore "<<exp->result<<", "<<name<<"\n";
        }
    }
    else{
        int total=1;
        int sz=len->size();
        arr_dim[name]=sz;
        cur_len.clear();
        string alloc_str;
        for (int i=sz-1;i>=0;--i) {
            int val=(*len)[i]->Calc();
            total*=val;
            cur_len.push_back(val);
            if (i==sz-1)
                alloc_str="[i32, "+to_string(val)+"]";
            else
                alloc_str="["+alloc_str+", "+to_string(val)+"]";
        }
        fout<<"\t"<<name<<" = alloc "<<alloc_str<<"\n";
        if(init!=NULL){
            cur_array_string.clear();
            init->PrintIR(fout);
            for (int i=0;i<total;++i){
                int step=total,p=i;
                string newtmp,tmp=name;
                for (int j=sz-1;j>=0;--j){
                    step/=cur_len[j];
                    int idx=p/step;
                    p-=idx*step;
                    newtmp="%"+to_string(var_cnt++);
                    fout<<"\t"<<newtmp<<" = getelemptr "<<tmp<<", "<<idx<<"\n";
                    tmp=newtmp;
                }
                fout<<"\tstore "<<cur_array_string[i]<<", "<<tmp<<"\n";
            }
        }
    }
}

void VarInitAST::PrintIR(stringstream &fout){
    if(exp!=NULL){
        exp->PrintIR(fout);
        result=exp->result;
        return;
    }
    int sz=cur_len.size();
    int total=1;
    for(int i=0;i<sz;++i)
        total*=cur_len[i];
    int num=cur_len[sz-1];
    cur_len.pop_back();
    int set_num=init->size();
    int p=0;
    for(int i=0;i<set_num;++i){
        VarInitAST *cur=(VarInitAST*)((*init)[i].get());
        if(cur->exp!=NULL){
            if(is_global){
                cur_array_int.push_back(cur->exp->Calc());
            }
            else{
                cur->exp->PrintIR(fout);
                cur_array_string.push_back(cur->exp->result);
            }
            ++p;
        }
        else{
            cur->PrintIR(fout);
            p+=total/num;
        }
    }
    for(;p<total;++p){
        if(is_global)
            cur_array_int.push_back(0);
        else
            cur_array_string.push_back("0");
    }
    cur_len.push_back(num);
}

void StmtAST::PrintIR(stringstream &fout){
    string btrue,bfalse,end;
    string lst_entry,lst_end;
    string while_begin;
    switch(type){
        case Stmt_simple:
            stmt->PrintIR(fout);
        break;
        case Stmt_if:
            exp->PrintIR(fout);
            btrue="\%true_"+to_string(if_cnt);
            end="\%end_"+to_string(if_cnt);
            ++if_cnt;

            fout<<"\tbr "<<exp->result<<", "<<btrue<<", "<<end<<"\n";
            fout<<"\n"<<btrue<<":\n";
            stmt->PrintIR(fout);
            fout<<"\tjump "<<end<<"\n";
            fout<<"\n"<<end<<":\n";
        break;
        case Stmt_ifelse:
            exp->PrintIR(fout);
            btrue="\%true_"+to_string(if_cnt);
            bfalse="\%false_"+to_string(if_cnt);
            end="\%end_"+to_string(if_cnt);
            ++if_cnt;

            fout<<"\tbr "<<exp->result<<", "<<btrue<<", "<<bfalse<<"\n";
            fout<<"\n"<<btrue<<":\n";
            stmt->PrintIR(fout);
            fout<<"\tjump "<<end<<"\n";
            fout<<"\n"<<bfalse<<":\n";
            else_stmt->PrintIR(fout);
            fout<<"\tjump "<<end<<"\n";
            fout<<"\n"<<end<<":\n";
        break;
        case Stmt_while:
            lst_entry=cur_while_entry;
            lst_end=cur_while_end;

            cur_while_entry="\%while_entry_"+to_string(while_cnt);
            cur_while_end="\%while_end_"+to_string(while_cnt);
            while_begin="\%while_begin_"+to_string(while_cnt);

            ++while_cnt;
            fout<<"\tjump "<<cur_while_entry<<"\n";
            fout<<"\n"<<cur_while_entry<<":\n";
            exp->PrintIR(fout);
            fout<<"\tbr "<<exp->result<<", "<<while_begin<<", "<<cur_while_end<<"\n";
            fout<<"\n"<<while_begin<<":\n";
            stmt->PrintIR(fout);
            fout<<"\tjump "<<cur_while_entry<<"\n";
            fout<<"\n"<<cur_while_end<<":\n";

            cur_while_entry=lst_entry;
            cur_while_end=lst_end;
        break;
        default:
            assert(0);
    }
}

void SimpleStmtAST::PrintIR(stringstream &fout){
    switch(type){
        case Simple_ret:
            exp->PrintIR(fout);
            fout<<"\tret "<<exp->result<<"\n";
            ++cut_cnt;
            fout<<"\%cut_"<<cut_cnt<<":\n";
        break;
        case Simple_ret_void:
            fout <<"\tret \n";
            ++cut_cnt;
            fout<<"\%cut_"<<cut_cnt<<":\n";
        break;
        case Simple_lval:{
            exp->PrintIR(fout);
            variant<int,string>value=find_symbol(lval->ident);
            assert(value.index()==1);
            string name=get<1>(value);
            vector<unique_ptr<BaseExpAST> >&idx=*(lval->idx);
            int sz=idx.size();
            if(sz==0)
                fout<<"\tstore "<<exp->result<<", "<<name<<"\n";
            else{
                string tmp;
                if (is_ptr[name]){
                    tmp="%"+to_string(var_cnt++);
                    fout<<"\t"<<tmp<<" = load "<<name<<"\n";
                    for (int i=0;i<sz;++i){
                        idx[i]->PrintIR(fout);
                        string newtmp="%"+to_string(var_cnt++);
                        if(i==0)
                            fout<<"\t"<<newtmp<<" = getptr "<<tmp<<", "<<idx[i]->result<<"\n";
                        else
                            fout<<"\t"<<newtmp<<" = getelemptr "<<tmp<<", "<<idx[i]->result<<"\n";
                        tmp=newtmp;
                    }
                }
                else {
                    tmp=name;
                    for (int i=0;i<sz;++i) {
                        idx[i]->PrintIR(fout);
                        string newtmp="%"+to_string(var_cnt++);
                        fout<<"\t"<<newtmp<<" = getelemptr "<<tmp<<", "<<idx[i]->result<<"\n";
                        tmp=newtmp;
                    }
                }
                fout<<"\tstore "<<exp->result<<", "<<tmp<<"\n";
            }
        }
        break;
        case Simple_exp:
            exp->PrintIR(fout);
        break;
        case Simple_block:
            block->PrintIR(fout);
        break;
        case Simple_break:
            assert(cur_while_end!="");
            fout<<"\tjump "<<cur_while_end<<"\n";
            ++cut_cnt;
            fout<<"\%cut_"<<cut_cnt<<":\n";
        break;
        case Simple_continue:
            assert(cur_while_entry!="");
            fout<<"\tjump "<<cur_while_entry<<"\n";
            ++cut_cnt;
            fout<<"\%cut_"<<cut_cnt<<":\n";
        break;
        case Simple_void:
        break;
        default:
            assert(0);
    }
}

void LvalAST::PrintIR(stringstream &fout){
}
void ConstExpAST::PrintIR(stringstream &fout){
}

void ExpAST::PrintIR(stringstream &fout){
    exp->PrintIR(fout);
    result=exp->result;
}

void LorExpAST::PrintIR(stringstream &fout){
    string btrue,bfalse,end;
    if(op==0){
        land_exp->PrintIR(fout);
        result=land_exp->result;
    }
    else{
        lor_exp->PrintIR(fout);
        btrue="\%true_"+to_string(if_cnt);
        bfalse="\%false_"+to_string(if_cnt);
        end="\%end_"+to_string(if_cnt);
        ++if_cnt;
        result="%"+to_string(var_cnt++);
        assert(op=='|');
        
        string result_pos="%"+to_string(var_cnt++);
        fout<<"\t"<<result_pos<<" = alloc i32\n";

        fout<<"\tbr "<<lor_exp->result<<", "<<btrue<<", "<<bfalse<<"\n";

        fout<<"\n"<<btrue<<":\n";
        
        fout<<"\tstore 1,"<<result_pos<<"\n";
        fout<<"\tjump "<<end<<"\n";

        fout<<"\n"<<bfalse<<":\n";

        land_exp->PrintIR(fout);
        string tmp="%"+to_string(var_cnt++);
        fout<<"\t"<<tmp<<" = ne "<<land_exp->result<<", 0\n";
        fout<<"\tstore "<<tmp<<", "<<result_pos<<"\n";
        fout<<"\tjump "<<end<<"\n";

        fout<<"\n"<<end<<":\n";
        fout<<"\t"<<result<<" = load "<<result_pos<<"\n";
    }
}

void LandExpAST::PrintIR(stringstream &fout){
    string btrue,bfalse,end;
    if(op==0){
        eq_exp->PrintIR(fout);
        result=eq_exp->result;
    }
    else{
        land_exp->PrintIR(fout);
        btrue="\%true_"+to_string(if_cnt);
        bfalse="\%false_"+to_string(if_cnt);
        end="\%end_"+to_string(if_cnt);
        ++if_cnt;
        result="%"+to_string(var_cnt++);
        assert(op=='&');

        string result_pos="%"+to_string(var_cnt++);
        fout<<"\t"<<result_pos<<" = alloc i32\n";

        fout<<"\tbr "<<land_exp->result<<", "<<btrue<<", "<<bfalse<<"\n";

        fout<<"\n"<<btrue<<":\n";

        eq_exp->PrintIR(fout);
        string tmp="%"+to_string(var_cnt++);
        fout<<"\t"<<tmp<<" = ne "<<eq_exp->result<<", 0\n";
        fout<<"\tstore "<<tmp<<", "<<result_pos<<"\n";
        fout<<"\tjump "<<end<<"\n";

        fout<<"\n"<<bfalse<<":\n";

        fout<<"\tstore 0,"<<result_pos<<"\n";
        fout<<"\tjump "<<end<<"\n";

        fout<<"\n"<<end<<":\n";
        fout<<"\t"<<result<<" = load "<<result_pos<<"\n";
    }
}

void EqExpAST::PrintIR(stringstream &fout){
    if(op==""){
        rel_exp->PrintIR(fout);
        result=rel_exp->result;
    }
    else{
        eq_exp->PrintIR(fout);
        rel_exp->PrintIR(fout);
        result="%"+to_string(var_cnt++);
        if(op=="==")
            fout<<"\t"<<result<<" = eq "<<eq_exp->result<<", "<<rel_exp->result<<"\n";
        else if(op=="!=")
            fout<<"\t"<<result<<" = ne "<<eq_exp->result<<", "<<rel_exp->result<<"\n";
        else
            assert(0);
    }
}

void RelExpAST::PrintIR(stringstream &fout){
    if(op==""){
        add_exp->PrintIR(fout);
        result=add_exp->result;
    }
    else{
        rel_exp->PrintIR(fout);
        add_exp->PrintIR(fout);
        result="%"+to_string(var_cnt++);
        if(op=="<")
            fout<<"\t"<<result<<" = lt "<<rel_exp->result<<", "<<add_exp->result<<"\n";
        else if(op==">")
            fout<<"\t"<<result<<" = gt "<<rel_exp->result<<", "<<add_exp->result<<"\n";
        else if(op=="<=")
            fout<<"\t"<<result<<" = le "<<rel_exp->result<<", "<<add_exp->result<<"\n";
        else if(op==">=")
            fout<<"\t"<<result<<" = ge "<<rel_exp->result<<", "<<add_exp->result<<"\n";
        else
            assert(0);
    }
}

void AddExpAST::PrintIR(stringstream &fout){
    if(op==0){
        mul_exp->PrintIR(fout);
        result=mul_exp->result;
    }
    else{
        add_exp->PrintIR(fout);
        mul_exp->PrintIR(fout);
        result="%"+to_string(var_cnt++);
        switch(op){
            case '+':
                fout<<"\t"<<result<<" = add "<<add_exp->result<<", "<<mul_exp->result<<"\n";
            break;
            case '-':
                fout<<"\t"<<result<<" = sub "<<add_exp->result<<", "<<mul_exp->result<<"\n";
            break;
            default:
                assert(0);
        }
    }
}

void MulExpAST::PrintIR(stringstream &fout){
    if(op==0){
        unary_exp->PrintIR(fout);
        result=unary_exp->result;
    }
    else{
        mul_exp->PrintIR(fout);
        unary_exp->PrintIR(fout);
        result="%"+to_string(var_cnt++);
        switch(op){
            case '*':
                fout<<"\t"<<result<<" = mul "<<mul_exp->result<<", "<<unary_exp->result<<"\n";
            break;
            case '/':
                fout<<"\t"<<result<<" = div "<<mul_exp->result<<", "<<unary_exp->result<<"\n";
            break;
            case '%':
                fout<<"\t"<<result<<" = mod "<<mul_exp->result<<", "<<unary_exp->result<<"\n";
            break;
            default:
                //cout<<op<<endl;
                assert(0);
        }
    }
}

void UnaryExpAST::PrintIR(stringstream &fout){
    vector<string>param_result;
    int sz;
    switch(type){

        case Unary_primary:
            exp->PrintIR(fout);
            result=exp->result;
        break;

        case Unary_op:
            exp->PrintIR(fout);
            if(op=='+')
                result=exp->result;
            else if(op=='-'){
                result="%"+to_string(var_cnt++);
                fout<<"\t"<<result<<" = sub 0, "<<exp->result<<"\n";
            }
            else if(op=='!'){
                result="%"+to_string(var_cnt++);
                fout<<"\t"<<result<<" = eq "<<exp->result<<", 0\n";
            }
            else
                assert(0);
        break;

        case Unary_call:
            in_call_func = 1;
            for(auto it=params->begin();it!=params->end();++it) {
                (*it)->PrintIR(fout);
                param_result.push_back((*it)->result);
            }
            if (func_type[ident]==Type_int) {
                result="%"+to_string(var_cnt++);
                fout<<"\t"<<result<<" = call @"<<ident<<"(";
            }
            else if (func_type[ident]==Type_void)
                fout<<"\tcall @"<<ident<<"(";

            sz=params->size();
            for(int i=0;i<sz;++i){
                fout<<param_result[i];
                if(i!=sz-1)
                    fout<<", ";
            }
            fout<<")\n";
            in_call_func = 0;
        break;

        default:
            assert(0);
    }
}

void PrimaryExpAST::PrintIR(stringstream &fout){
    if(type==PrimaryExp_number)
        result=to_string(number);
    else if(type==PrimaryExp_exp){
        exp->PrintIR(fout);
        result=exp->result;
    }
    else{
        variant<int,string> value=find_symbol(lval->ident);
        if (value.index()==0)
            result=to_string(get<0>(value));
        else{
            result="%"+to_string(var_cnt++);
            int sz=lval->idx->size();
            string name=get<1>(value);
            vector<unique_ptr<BaseExpAST> >&idx=*(lval->idx);
            if(sz==0){
                if (in_call_func&&arr_dim[name]&&!is_ptr[name])
                    fout<<"\t"<<result<<" = getelemptr "<<name<<", 0\n";
                else
                    fout<<"\t"<<result<<" = load "<<name<<"\n";
            }
            else{
                string tmp;
                if (is_ptr[name]){
                    tmp="%"+to_string(var_cnt++);
                    fout<<"\t"<<tmp<<" = load "<<name<<"\n";
                    for (int i=0;i<sz;++i){
                        idx[i]->PrintIR(fout);
                        string newtmp="%"+to_string(var_cnt++);
                        if(i==0)
                            fout<<"\t"<<newtmp<<" = getptr "<<tmp<<", "<<idx[i]->result<<"\n";
                        else
                            fout<<"\t"<<newtmp<<" = getelemptr "<<tmp<<", "<<idx[i]->result<<"\n";
                        tmp=newtmp;
                    }
                }
                else {
                    tmp=name;
                    for (int i=0;i<sz;++i) {
                        idx[i]->PrintIR(fout);
                        string newtmp="%"+to_string(var_cnt++);
                        fout<<"\t"<<newtmp<<" = getelemptr "<<tmp<<", "<<idx[i]->result<<"\n";
                        tmp=newtmp;
                    }
                }
                if (in_call_func&&(arr_dim[name]>sz))
                    fout<<"\t"<<result<<" = getelemptr "<<tmp<< ", 0\n";
                else
                    fout<<"\t"<<result<<" = load "<<tmp<< "\n";
            }
        }
    }
}

int ConstExpAST::Calc(){
    return exp->Calc();
}

int ExpAST::Calc(){
    return exp->Calc();
}

int LorExpAST::Calc(){
    if(op==0)
        return land_exp->Calc();
    else
        return lor_exp->Calc()||land_exp->Calc();
    return 0;
}

int LandExpAST::Calc(){
    if(op==0)
        return eq_exp->Calc();
    else
        return land_exp->Calc()&&eq_exp->Calc();
    return 0;
}

int EqExpAST::Calc(){
    if(op=="")
        return rel_exp->Calc();
    else if(op=="!=")
        return eq_exp->Calc()!=rel_exp->Calc();
    else if(op=="==")
        return eq_exp->Calc()==rel_exp->Calc();
    return 0;
}

int RelExpAST::Calc(){
    if(op=="")
        return add_exp->Calc();
    else if(op=="<=")
        return rel_exp->Calc()<=add_exp->Calc();
    else if(op==">=")
        return rel_exp->Calc()>=add_exp->Calc();
    else if(op=="<")
        return rel_exp->Calc()<add_exp->Calc();
    else if(op==">")
        return rel_exp->Calc()>add_exp->Calc();
    return 0;
}

int AddExpAST::Calc(){
    if(op==0)
        return mul_exp->Calc();
    else if(op=='+')
        return add_exp->Calc()+mul_exp->Calc();
    else if(op=='-')
        return add_exp->Calc()-mul_exp->Calc();
    return 0;
}

int MulExpAST::Calc(){
    if(op==0)
        return unary_exp->Calc();
    else if(op=='*')
        return mul_exp->Calc()*unary_exp->Calc();
    else if(op=='/')
        return mul_exp->Calc()/unary_exp->Calc();
    else if(op=='%')
        return mul_exp->Calc()%unary_exp->Calc();
    return 0;
}

int UnaryExpAST::Calc(){
    if(type==Unary_primary)
        return exp->Calc();
    else if(type==Unary_op){
        switch(op){
            case '+':
                return exp->Calc();
            case '-':
                return -exp->Calc();
            case '!':
                return !exp->Calc();
        }
    }
    assert(0);
    return 0;
}

int PrimaryExpAST::Calc(){
    if(type==PrimaryExp_exp)
        return exp->Calc();
    else if(type==PrimaryExp_number)
        return number;
    else if(type==Primary_lval){
        variant<int,string>value=find_symbol(lval->ident);
        assert(value.index()==0);
        return get<0>(value);
    }
    return 0;
}

int ConstInitAST::Calc(){
    return exp->Calc();
}

int VarInitAST::Calc(){
    return exp->Calc();
}