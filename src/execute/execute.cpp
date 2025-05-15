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
#include <cmath>
#include "function.hpp"

using namespace std;

void execute_julia(){
    string command_jl = "julia TWPA.jl";
    if(system(command_jl.c_str()) == -1){ //Juliaの実行が失敗した場合
        cout << "error:julia was not executed correctly." << endl;
    }
}

