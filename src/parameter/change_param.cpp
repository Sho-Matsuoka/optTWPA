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


vector<ele_unit> change_param(vector<ele_unit> &ele, string ele_name, double value) {

    for (int i = 0; i < ele.size(); i++){
        if(ele[i].name == ele_name){
           ele[i].value = value;
        }
    }

    return ele;
}