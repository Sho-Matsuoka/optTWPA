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

 double gain_th = 20;

int main(int argc, const char *argv[]) {

    vector<ele_unit> ele;
    vector<string> jl_source;
    vector<string> arg_arr(argv, argv+argc);  //コマンドライン引数が格納されている動的配列  コマンドライン引数が格納されている静的配列(argv)の要素を動的配列(arg_arr)に格

   
    cout << " Julia sourve file: " << arg_arr[1] << endl;
    cout << " please input gain threshold: ";
    cin >> gain_th;
    cout << " gain threshold (gain_th): " << gain_th << endl;


    //cout << "exuecuting Julia ..." << endl;
    //execute_julia("TWPA_src.jl"); // julia を実行
    read_jl(ele, jl_source, arg_arr[1]);
    //write_jl(ele, jl_source);
    //write_jl(ele, jl_source);
    //change_param(ele, "Lj", 6.0e-6);
    //display_element(ele);

    //cout << out_value(ele, "Lj") << endl;
    calculation(ele, jl_source);
    //run_nsga2_pagmo(100, 200, ele, jl_source, out_value(ele, "Lj"), 1e-17, 1e-13, 1e-17, 1e-13, 5e-6, 6e-6, 8e9, 9e9);
    //run_nsga2_pagmo_codex(100, 200, ele, jl_source, out_value(ele, "Lj"), 1e-15, 1e-13, 1e-15, 1e-13);

    return 0;
}
