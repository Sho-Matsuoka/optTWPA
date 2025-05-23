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


double calc_ripple(vector<vector<double>> csv_array, double gain) {

    double gain_half = gain / 2;
    double freq_l = 0, freq_h = 0;
    double bandwidth = 0;

    for (int i = 0; i < csv_array.size() - 1; i++) {
        if(csv_array[i][1] <= gain_half){
            if(csv_array[i + 1][1] >= gain_half){
                freq_l = (csv_array[i][0] + csv_array[i + 1][0]) / 2;
                break;
            }
        }
    }

    for (int i = csv_array.size() - 1; i >= 0; i--) {
        if(csv_array[i][1] <= gain_half){
            if(csv_array[i - 1][1] >= gain_half){
                freq_h = (csv_array[i][0] + csv_array[i - 1][0]) / 2;
                break;
            }
        }
    }
    //cout << freq_h << ", " << freq_l << endl;
    bandwidth = freq_h - freq_l;

    return bandwidth;
}