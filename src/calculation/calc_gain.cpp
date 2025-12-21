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


double calc_gain(vector<vector<double>> csv_array, double freq_r) {

    if (csv_array.empty()) {
        return 0.0;
    }

    double gain = csv_array[0][1];
    double freq = freq_r - 0.5; // resonator freq kara 0.5GHz hikui tokoro
    //double width = 1.5; // tansaku no haba

    for (auto& column : csv_array) {
        if(column[0] >= freq - WIDTH_F && column[0] <= freq){  //範囲を共振周波数から0.5 GHz低いところから1.5GHzの幅の間のみを探索
            if(column[1] > gain){                            
                gain = column[1];
            } 
        }
    }

    return gain;
}