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

    double gain = csv_array[0][1];
    double freq = freq_r - 0.5; // resonator freq kara 0.5GHz hikui tokoro
    double width = 1.5; // tansaku no haba

    for (auto& column : csv_array) {
        if(column[0] >= freq - width && column[0] <= freq){ // hani wo resonator freq kara 0.5GHz hikui tokoro kara 1.5GHz no haba no aida nomi wo tansaku
            if(column[1] > gain){
                gain = column[1];
            } 
        }
    }

    return gain;
}