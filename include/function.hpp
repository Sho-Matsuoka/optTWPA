#ifndef FUNCTION_H
#define FUNCTION_H

constexpr double WIDTH_F = 1.5;

#include <string>
#include <vector>

struct ele_unit{
    std::string name;
    double value;
    int line;
};

struct result{
    double gain;
    double bandwidth;
    double ripple;
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
void execute_julia(std::string jl_source);

//file
void read_jl(std::vector<ele_unit> &ele, std::vector<std::string> &jl_source);
void write_jl(std::vector<ele_unit> &ele, std::vector<std::string> &jl_source);
std::vector<std::vector<double>> read_csv();

//display
void display_element(std::vector<ele_unit> &ele);

//calculation
result calculation(std::vector<ele_unit> ele, std::vector<std::string> &jl_source);
double calc_gain(std::vector<std::vector<double>> csv_array, double freq_r);
double calc_band(std::vector<std::vector<double>> csv_array, double gain);
double calc_ripple(std::vector<std::vector<double>> csv_array, double freq_r);
double calc_freq_r(std::vector<ele_unit> ele);

//parameter
std::vector<ele_unit> change_param(std::vector<ele_unit> &ele, std::string ele_name, double value);
double value_param(std::vector<ele_unit> ele, std::string ele_name);

//nsga2
void run_nsga2(int pop_size,int generations, std::vector<ele_unit> ele, std::vector<std::string> jl_source, double Lj, double Cg_min, double Cg_max, double Cc_min, double Cc_max);

#endif