#ifndef FUNCTION_H
#define FUNCTION_H

#include <string>
#include <vector>

struct ele_unit{
    std::string name;
    double value;
    int line;
};


struct element{
    double Lj = 0;
    double Cg = 0;
    double Cc = 0;
    double Cn = 0;
    double Cr = 0;
    double Lr = 0;
    double Ip = 0;
    std::vector<int> lines;
};
//execute
void execute_julia();

//file
void read_jl(std::vector<ele_unit> &ele, std::vector<std::string> &jl_source);
void write_jl(std::vector<ele_unit> &ele, std::vector<std::string> &jl_source);
std::vector<std::vector<double>> read_csv();

//display
void display_element(std::vector<ele_unit> &ele);

//calculation
double calc_gain(std::vector<std::vector<double>> csv_array);
double calc_band(std::vector<std::vector<double>> csv_array, double gain);
double calc_ripple(std::vector<std::vector<double>> csv_array);
double calc_freq_r(std::vector<ele_unit> ele);

#endif