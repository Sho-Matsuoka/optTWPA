#ifndef FUNCTION_H
#define FUNCTION_H

#include <string>
#include <vector>


struct element{
    double Lj = 0;
    double Cg = 0;
    double Cc = 0;
    double Cn = 0;
    double Cr = 0;
    double Lr = 0;
    double Ip = 0;
};
//execute
void execute_julia();

//read_file
void read_jl(element &ele);

//display
void display_element(element &ele);

#endif