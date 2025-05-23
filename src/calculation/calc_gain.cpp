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


double calc_gain(vector<vector<double>> csv_array) {

    double gain = csv_array[0][1];

    for (auto& column : csv_array) {
        //cout << column[1] << endl;
        if(column[1] > gain){
            gain = column[1];
        } 
    }

    return gain;
}