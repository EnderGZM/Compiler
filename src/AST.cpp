#include <cassert>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include <sstream>
#include <map>
#include "AST.h"
using namespace std;

vector<map<string,string> > symbol_tables;

int var_cnt=0;
int table_cnt=0;
int if_cnt=0;
int ret_cnt=0;

string find_symbol(string name){
    int sz=symbol_tables.size();
    for(int i=sz-1;i>=0;--i)
        if(symbol_tables[i].find(name)!=symbol_tables[i].end())
            return symbol_tables[i][name];
    assert(0);
    return "";
}

void CompUnitAST::Dump()const{
    std::cout << "CompUnitAST { "<<endl;
    func_def->Dump();
    std::cout << " }"<<endl;
}

void FuncDefAST::Dump()const{
    std::cout << "FuncDefAST { "<<endl;
    type->Dump();
    std::cout << ", " << ident << ", "<<endl;
    block->Dump();
    std::cout << " }"<<endl;
}

void TypeAST::Dump()const{
    cout << "FuncTypeAST { "<<endl;
    cout<<type<<" }"<<endl;
}

void BlockAST::Dump()const{
    cout<< "BlockAST{ "<<endl;
    for(auto it=blockitem_list->begin();it!=blockitem_list->end();it++)
        (*it)->Dump();
    cout<<" }"<<endl;
}

/*void StmtAST::Dump()const{
    cout<<"StmtAST{"<<endl;
    cout<<"type:"<<type<<endl;
    exp->Dump();
    cout<<"}"<<endl;
}*/

void ExpAST::Dump()const{
    cout<<"ExpAST{"<<endl;
    exp->Dump();
}

void LorExpAST::Dump()const{
    cout<<"LorExp:"<<endl;
    if(op==0)
        land_exp->Dump();
    else{
        cout<<"{"<<endl;
        lor_exp->Dump();
        cout<<"}||"<<endl;
        cout<<"{"<<endl;
        land_exp->Dump();
        cout<<"}"<<endl;
    }
}

void LandExpAST::Dump()const{
    cout<<"LandExp:"<<endl;
    if(op==0)
        eq_exp->Dump();
    else{
        cout<<"{"<<endl;
        land_exp->Dump();
        cout<<"}&&"<<endl;
        cout<<"{"<<endl;
        eq_exp->Dump();
        cout<<"}"<<endl;
    }
}

void EqExpAST::Dump()const{
    cout<<"EqExp:"<<endl;
    if(op=="")
        rel_exp->Dump();
    else{
        cout<<"{"<<endl;
        eq_exp->Dump();
        cout<<"}"<<op<<endl;
        cout<<"{"<<endl;
        rel_exp->Dump();
        cout<<"}"<<endl;
    }
}

void RelExpAST::Dump()const{
    cout<<"RelExp:";
    if(op=="")
        add_exp->Dump();
    else{
        cout<<"{";
        rel_exp->Dump();
        cout<<"}"<<op;
        cout<<"{";
        add_exp->Dump();
        cout<<"}";
    }
}

void AddExpAST::Dump()const{
    cout<<"AddExp:";
    if(op==0)
        mul_exp->Dump();
    else{
        cout<<"{";
        add_exp->Dump();
        cout<<"}"<<op;
        cout<<"{";
        mul_exp->Dump();
        cout<<"}";
    }
}

void MulExpAST::Dump()const{
    cout<<"MulExp:";
    if(op==0)
        unary_exp->Dump();
    else{
        cout<<"{";
        mul_exp->Dump();
        cout<<"}"<<op;
        cout<<"{";
        unary_exp->Dump();
        cout<<"}";
    }
}

void UnaryExpAST::Dump()const{
    if(op==0){
        exp->Dump();
    }
    else{
        cout<<op;
        exp->Dump();
    }
}

void PrimaryExpAST::Dump()const{
    if(type==PrimaryExp_exp){
        cout<<'(';
        exp->Dump();
        cout<<')';
    }
    else if(type==PrimaryExp_number){
        cout<<"Number:{"<<number<<"}";
    }
}


void CompUnitAST::PrintIR(stringstream &fout){
    func_def->PrintIR(fout);
}

void FuncDefAST::PrintIR(stringstream &fout){
    fout<<"fun @"<<ident<<"(): ";
    type->PrintIR(fout);
    fout<<"{\n";
    fout<<"\%entry: \n";
    stringstream _out;
    block->PrintIR(_out);
    int p;
    for(p=_out.str().size()-2;p>=0&&_out.str()[p]!='\n';--p);
    if(_out.str()[p+1]=='%')
        fout<<_out.str().substr(0,p)<<"\n";
    else
        fout<<_out.str();
    fout<<"}\n";
}

void TypeAST::PrintIR(stringstream &fout){
    if(type==Type_int)
        fout<<"i32";
}

void BlockAST::PrintIR(stringstream &fout){
    map<string,string>symbol_table;
    ++table_cnt;
    symbol_tables.push_back(symbol_table);
    for(auto it=blockitem_list->begin();it!=blockitem_list->end();it++)
        (*it)->PrintIR(fout);
    symbol_tables.pop_back();
}

void BlockItemAST::PrintIR(stringstream &fout){
    val->PrintIR(fout);
}

void DeclAST::PrintIR(stringstream &fout){
    for(auto it=def_list->begin();it!=def_list->end();it++)
        (*it)->PrintIR(fout);
}


void DefAST::PrintIR(stringstream &fout){
    string name="@"+ident+"_"+to_string(table_cnt);
    symbol_tables[symbol_tables.size()-1][ident]=name;
    fout<<"\t"<<name<<" = alloc i32\n";
    if(val!=NULL){
        val->PrintIR(fout);
        fout<<"\tstore "<<val->result<<", "<<name<<"\n";
    }
}

void InitValAST::PrintIR(stringstream &fout){
    exp->PrintIR(fout);
    result=exp->result;
}

void StmtAST::PrintIR(stringstream &fout){
    string btrue,bfalse,end;
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
        default:
            assert(0);
    }
}

void SimpleStmtAST::PrintIR(stringstream &fout){
    switch(type){
        case Simple_ret:
            exp->PrintIR(fout);
            fout<<"\tret "<<exp->result<<"\n";
            ++ret_cnt;
            fout<<"\%after_ret_"<<ret_cnt<<":\n";
        break;
        case Simple_ret_void:
            fout <<"\tret \n";
        break;
        case Simple_lval:
            lval->PrintIR(fout);
            exp->PrintIR(fout);
            fout<<"\tstore "<<exp->result<<", "<<lval->result<<"\n";
        break;
        case Simple_exp:
            exp->PrintIR(fout);
        break;
        case Simple_block:
            block->PrintIR(fout);
        break;
        case Simple_void:
        break;
        default:
            assert(0);
    }
}

void LvalAST::PrintIR(stringstream &fout){
    result=find_symbol(ident);
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
    exp->PrintIR(fout);
    switch(op){
        case 0:
        case '+':
            result=exp->result;
        break;
        case '-':
            result="%"+to_string(var_cnt++);
            fout<<"\t"<<result<<" = sub 0, "<<exp->result<<"\n";
        break;
        case '!':
            result="%"+to_string(var_cnt++);
            fout<<"\t"<<result<<" = eq "<<exp->result<<", 0\n";
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
        lval->PrintIR(fout);
        result="%"+to_string(var_cnt++);
        fout<<"\t"<<result<<" = load "<<lval->result<<"\n";
    }
}
