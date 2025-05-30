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


double change_param(vector<ele_unit>& ele, string ele_name) {
    for (auto &e : ele) {
        if (e.name == ele_name) {
            return e.value;
        }
    }
    throw std::runtime_error("value_param: parameter not found: " + ele_name);
}