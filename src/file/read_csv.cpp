#include <stdio.h>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sstream>
#include <thread>
#include <math.h>
#include <cmath>
#include <cstdlib>
#include <unistd.h>
#include "function.hpp"
#include <iomanip> 

using namespace std;


/* Josimの結果を配列に格納 */
vector<vector<double>> read_csv(std::size_t tid) {
    double out;
    stringstream outfile, delete_csv;
    stringstream outline;
    string line;
    vector<vector<double>> csv_array;
    outfile << "freq_gain_" << getpid();
    if(tid != 0) outfile << '_' << tid;
    outfile << ".csv";
    delete_csv << "rm -rf " << outfile.str();
    /* テキストファイルの読み込み */
    ifstream fp_csv(outfile.str());

    if (!fp_csv.is_open()) {
        cerr << "No output of JoSIM."  << endl;
    }

    fp_csv.seekg(0, ios::end);
    if (fp_csv.tellg() == 0) {
        cerr << "ERROR : Output File(.csv) is empty. Julia execution may have some errors." << endl;
        fp_csv.close();
        exit(1);
    }
    fp_csv.seekg(0, ios::beg);



    // シーク位置を先頭の次の行に戻す(０行目がラベルであるため)
    getline(fp_csv, line);

    /* 配列に格納 */
    while(getline(fp_csv, line)){            // 行を上から順に選択
        stringstream outline(line);
        vector<double> column;
        while(outline >> out){
            column.emplace_back(out);
            if(outline.peek() == ','){
                outline.ignore();
            }
        }
        csv_array.emplace_back(column);
    }
    fp_csv.close();

    system(delete_csv.str().c_str());

    return csv_array;
}