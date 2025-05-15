#ifndef FUNCTION_H
#define FUNCTION_H

#include <string>
#include <vector>


struct element{
    double Lj = 6.4e-6;
    double Cg = 25.0e-15;
    double Cc = 18.0e-15;
    double Cn = 6.001e-15;
    double Cr = 5.00e-12;
    double Lr = 65e-12;
    double Ip = 3.6001e-6;
};
//execute.cpp
void execute_julia();
void read_jl(element &ele);

#endif