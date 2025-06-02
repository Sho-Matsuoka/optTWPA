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


double calc_imp(vector<ele_unit> ele){
    double Lj, Cc, Cn, Cg;

    for (auto e : ele){
        if(e.name == "Lj"){
            Lj = PHI_0 / 2 / M_PI / e.value;
        }
        else if(e.name == "Cg"){
            Cg = e.value;
        }
        else if(e.name == "Cc"){
            Cc = e.value;
        }
        else if(e.name == "Cn"){
            Cn = e.value;
        }
    }

    return sqrt(3 * Lj/ (2 * Cg + Cn + Cc)); 
}