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


double out_value(vector<ele_unit> &ele, const string ele_name) {

    double e;
    for (int i = 0; i < ele.size(); i++){
        if(ele[i].name == ele_name){
            e = ele[i].value;
        }
    }

    return e;
}