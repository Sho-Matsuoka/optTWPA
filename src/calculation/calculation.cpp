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


result calculation(vector<ele_unit> ele, vector<string> &jl_source) {

    write_jl(ele, jl_source);
    result result;

    cout << " exuecuting Julia ..." << endl;
    execute_julia("TWPA_p.jl"); // julia を実行

    vector<vector<double>> csv_array = read_csv();

    result.gain = calc_gain(csv_array, calc_freq_r(ele));
    result.bandwidth = calc_band(csv_array, result.gain);
    result.ripple = calc_ripple(csv_array, calc_freq_r(ele));

    cout << " Gain     : " << result.gain << " dB" << endl;
    cout << " Bandwidth: " << result.bandwidth<< " GHz" << endl;
    cout << " Ripple   : " << result.ripple << " dB" << endl;
    cout << " freq_r   : " << calc_freq_r(ele) << " GHz" << endl;

    return result;
}