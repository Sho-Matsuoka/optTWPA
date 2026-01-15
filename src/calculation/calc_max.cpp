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


double calc_max(vector<vector<double>> csv_array, double freq_r) {

    if (csv_array.empty()) {
        return 0.0;
    }

    double gain_max = csv_array[0][1];
    double freq_gain_max = 0;
    double freq_min = freq_r - 1.5; 
    double freq_max = freq_r + 1.5; 
    //double width = 1.5; // tansaku no haba

    for (auto& column : csv_array) {
        if(column[1] > gain_max){                            
            gain_max = column[1];
            freq_gain_max = column[0];
        } 

    }
    if(freq_gain_max >= freq_min && freq_gain_max <= freq_max){  //範囲を共振周波数から0.5 GHz低いところから1.5GHzの幅の間のみを探索
        return 0;    //OK
    }

    return 1;   //dame
}
