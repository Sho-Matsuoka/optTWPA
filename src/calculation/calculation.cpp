#include <stdio.h>
#include <unistd.h>
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


result calculation(const vector<ele_unit> &ele, const vector<string> &jl_source, std::size_t tid) {
    stringstream jl_name, liness;
    jl_name << "TWPA_" << getpid();
    if(tid != 0) jl_name << '_' << tid;
    jl_name << ".jl";

    write_jl(ele, jl_source, tid);
    result result;
    display_element(ele);
    cout << " exuecuting Julia ..." << endl;
    execute_julia(jl_name.str()); // julia を実行

    vector<vector<double>> csv_array = read_csv(tid);
    if (csv_array.empty()) {
        // Julia execution failed or returned no data; penalize solution
        return {0.0, 0.0, 1.0};
    }

    result.gain = calc_gain(csv_array, calc_freq_r(ele));
    result.bandwidth = calc_band(csv_array, result.gain);
    result.ripple = calc_ripple(csv_array, calc_freq_r(ele));
    if(calc_max(csv_array, calc_freq_r(ele)) == 1){
        result.ripple = 100;
    }

    cout << " Gain     : " << result.gain << " dB" << endl;
    cout << " Bandwidth: " << result.bandwidth<< " GHz" << endl;
    cout << " Ripple   : " << result.ripple << " dB" << endl;
    cout << " freq_r   : " << calc_freq_r(ele) << " GHz" << endl << endl;
    cout << " impedance: " << calc_imp(ele) << " ohm" << endl << endl;


    return result;
}