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


double calc_freq_r(vector<ele_unit> ele){
    double freq_r = 0;
    double Cr, Lr;

    for (auto elem : ele){
        if(elem.name == "Cr"){
            Cr = elem.value;
        }
        else if(elem.name == "Lr"){
            Lr = elem.value;
        }
    }

    freq_r = 1 / (2 * M_PI * sqrt(Lr * Cr)) * 1e-9;   

    return freq_r; //(GHz)
}