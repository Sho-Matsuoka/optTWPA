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

    element ele;
    //execute_julia(); // julia を実行
    read_jl(ele);
    display_element(ele);
    
    return 0;
}