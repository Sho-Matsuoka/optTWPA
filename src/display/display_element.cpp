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

void display_element(vector<ele_unit> &ele){
    for (auto& elem : ele) {
        cout << " name: " << left << setw(2) <<  elem.name << ", value: " << setw(10) << elem.value << ", line: " << setw(4) << elem.line << endl;
    }
}
