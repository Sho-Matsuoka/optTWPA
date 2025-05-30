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
    display_element(ele);

    //calculation(ele, jl_source);
    //run_nsga2(100, 100, ele, jl_source, value_param(ele, "Lj"), 0, 10, 0, 10);

    return 0;
}