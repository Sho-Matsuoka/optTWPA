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
#include <cmath>
#include "function.hpp"

using namespace std;

void read_jl(vector<ele_unit> &ele, vector<string> &jl_source){
 
    //ele.lines.clear();    //reset ele.lines(composed of each line_num)
    string jlfile_name = "TWPA.jl";  //.jl file name
    string line, ignore, buf, ele_name; // for reading name  each element.
    //double ele_value = 0;          // for reading value each element.
    int line_num = 0;
    stringstream liness;   //for stringstream of each .jl line
    ifstream file(jlfile_name);  // open .jl file


    if(!filesystem::is_regular_file(jlfile_name)){ //if  there is not .jl file.
        cout << jlfile_name << " is not found!" << endl;
    }

    if (!file.is_open()) {  // if .jl file can't be opened.
        cerr << "Error opening file: " << jlfile_name;
    }
    while(getline(file, line)) {
        liness.clear();   // reset string stream (liness)
        liness.str("");   // the same as above
        //cout << line << endl;                            
        liness << line; //input line into liness
        liness >> ele_name >> ignore >> buf;   //taking apart liness to name and value.

        jl_source.emplace_back(line); // insert line to jl_source

        if (ele_name == "Lj" || ele_name == "Cg" || ele_name == "Cc" || ele_name == "Cn" || ele_name == "Cr" || ele_name == "Lr" || ele_name == "Ip"){  //read Cg
            //ele_value = stod(buf);   // cast string(buf) => double(ele.Cg)
            if (ele_name == "Lj"){  //read Lj
                if (buf.find("(") != string::npos){  // inser space after "("
                    buf.insert(buf.rfind("(") + 1, " ");
                }            
                if (buf.rfind(")") != string::npos){ // inser space before ")"
                    buf.insert(buf.rfind(")"), " ");
                }            
                liness.clear();   // reset string stream (liness)
                liness.str("");   // the same as above
                liness << buf;    // buf: IctoL( 6.4e-6 ),
                liness >> ignore >> buf >> ignore; 
                //ele.Lj = ele_value;
                //(ele.lines).push_back(line_num);
            }
            ele.push_back({ele_name, stod(buf), line_num});
        }
        /*
        if (ele_name == "Lj"){  //read Lj
            if (buf.find("(") != string::npos){  // inser space after "("
                buf.insert(buf.rfind("(") + 1, " ");
            }            
            if (buf.rfind(")") != string::npos){ // inser space before ")"
                buf.insert(buf.rfind(")"), " ");
            }            
            liness.clear();   // reset string stream (liness)
            liness.str("");   // the same as above
            liness << buf;    // buf: IctoL( 6.4e-6 ),
            liness >> ignore >> ele_value >> ignore; 
            ele.Lj = ele_value;
            (ele.lines).push_back(line_num);
            //cout << line_num << endl;
        }
        else if (ele_name == "Cg"){  //read Cg
            ele.Cg = stod(buf);   // cast string(buf) => double(ele.Cg)
            (ele.lines).push_back(line_num);

        }
        else if (ele_name == "Cc"){  //read Cc 
            ele.Cc = stod(buf);
            (ele.lines).push_back(line_num);

        }
        else if (ele_name == "Cn"){  //read Cn
            ele.Cn = stod(buf);
            (ele.lines).push_back(line_num);

        }
        else if (ele_name == "Cr"){  //read Cr
            ele.Cr = stod(buf);
            (ele.lines).push_back(line_num);

        }
        else if (ele_name == "Lr"){  //read Lr
            ele.Lr = stod(buf);
            (ele.lines).push_back(line_num);
 
        }
        else if (ele_name == "Ip"){  //read Ip
            ele.Ip = stod(buf);
            (ele.lines).push_back(line_num);
        }

*/
        line_num++;
}
}