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


double calc_band(vector<vector<double>> csv_array, double gain) {

    //double gain_half = gain / 2;
    double freq_l = 0, freq_h = 0;
    double bandwidth = 0;

    for (size_t i = 0; i + 1 < csv_array.size(); ++i) {
        if(csv_array[i][1] <= gain_th){
            if(csv_array[i + 1][1] >= gain_th){
                freq_l = (csv_array[i][0] + csv_array[i + 1][0]) / 2;
                break;
            }
        }
    }

    for (size_t i = csv_array.size(); i > 1; --i) {
        size_t idx = i - 1;
        if(csv_array[idx][1] <= gain_th){
            if(csv_array[idx - 1][1] >= gain_th){
                freq_h = (csv_array[idx][0] + csv_array[idx - 1][0]) / 2;
                break;
            }
        }
    }
    //cout << freq_h << ", " << freq_l << endl;
    bandwidth = freq_h - freq_l;

    return bandwidth;
}