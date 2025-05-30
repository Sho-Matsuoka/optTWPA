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


double change_param(vector<ele_unit> ele, string ele_name) {

    for(auto unit : ele){
        if(unit.name == ele_name){
            return unit.value;
        }
    }
}