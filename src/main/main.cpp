#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <cstdlib>
#include <vector>
#include <filesystem>
#include <algorithm>
#include <cmath>
#include "function.hpp"


using namespace std;


int main(int argc, const char *argv[]) {

    vector<ele_unit> ele;
    vector<string> jl_source;
    //cout << "exuecuting Julia ..." << endl;
    //execute_julia("TWPA_src.jl"); // julia を実行
    read_jl(ele, jl_source);
    //write_jl(ele, jl_source);
    //write_jl(ele, jl_source);
    //change_param(ele, "Lj", 6.0e-6);
    //display_element(ele);
    //display_element(ele);

    //cout << out_value(ele, "Lj") << endl;
    //calculation(ele, jl_source);
    run_nsga2_pagmo(100, 200, ele, jl_source, out_value(ele, "Lj"), 1e-15, 1e-13, 1e-15, 1e-13);

    return 0;
}