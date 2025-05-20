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
    //execute_julia(); // julia を実行
    read_jl(ele, jl_source);
    write_jl(ele, jl_source);
    display_element(ele);
    
    return 0;
}