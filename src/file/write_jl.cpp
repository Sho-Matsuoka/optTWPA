#include <stdio.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip> 
#include <cstdio>
#include <regex>
#include <algorithm>
#include <cmath>
#include <mutex>
#include "function.hpp"

using namespace std;

void write_jl(const vector<ele_unit> &ele, const vector<string> &jl_source, std::size_t tid){ //for writing .jl file of new parameters
    stringstream jl_name, liness, csv_line;
    jl_name << "TWPA_" << getpid();
    if(tid != 0) jl_name << '_' << tid;
    jl_name << ".jl";
    csv_line << "    open(\"freq_gain_" <<  getpid();
    if(tid != 0) csv_line << '_' << tid;
    csv_line << ".csv\", \"w\") do io";
    ofstream fpin(jl_name.str());
    int y = 0;
    vector<string> jl_source_copy = jl_source;
    if(!fpin.is_open()){
        cerr << "file not error in write_jl.cpp"  << endl;
        return;
    }        
    for (int x = 0; x < ele.size(); x++) {
        liness.clear();
        liness.str("");
        y = ele[x].line;
        if(ele[x].name == "Lj"){
            liness << "        " << ele[x].name << " => " << "IctoLj(" << ele[x].value  << ")" << ",";
        }
        else if(ele[x].name == "Ip"){
            liness << "    " << ele[x].name << "  = " << ele[x].value;
        }
        else{
            liness << "        " << ele[x].name << " => " << ele[x].value << ",";
        }
        jl_source_copy[y] = liness.str();
    }
    for (int x = 0; x < jl_source_copy.size(); x++) {     //内容を書き換えたjl_source_copy(バッファ)を全て書く
        if(jl_source_copy[x].find("open") != string::npos){
            jl_source_copy[x] = csv_line.str();
        }
        fpin << jl_source_copy[x] << endl;  
    }
    fpin.close();
    
    return;
}