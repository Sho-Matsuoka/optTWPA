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


double calc_ripple(vector<vector<double>> csv_array, double freq_r){

    double ripple = 0;
    double freq = freq_r - 0.5; // resonator freq kara 0.5GHz hikui tokoro
    double min = 0, max = 0;
    double judge_updown = 0; // -1なら減少、1なら上昇 (一つ前のサイクル)

    for (int i = 0; i < csv_array.size(); i++) {
        if(csv_array[i][0] >= freq - WIDTH_F && csv_array[i][0] <= freq){  //範囲を共振周波数から0.5 GHz低いところから1.5GHzの幅の間のみを探索
            if(csv_array[i][1] >= csv_array[i - 1][1]){ //前サイクルよりも上昇していれば
                if(judge_updown == -1){  //前サイクルが減少していれば (減少 → 上昇 なので負のピーク)
                    min = csv_array[i - 1][1];
                    if(max - min >= ripple && max != 0){   //max - min の値が既存のrippleよりも大きければrippleの最大値を更新 
                        ripple = max - min;                //"max != 0" はmaxが初期値ではない場合であり、(正のピーク) - (負のピーク)を計算する関係上、先に正のピークを迎えてくれないと困るため
                    }
                }
                judge_updown = 1;
            }
            else if(csv_array[i][1] < csv_array[i - 1][1]){ //前サイクルよりも減少していれば
                if(judge_updown == 1){  //前サイクルが上昇していれば (上昇 → 減少 なので正のピーク)
                    max = csv_array[i - 1][1];
                }
                judge_updown = -1;
            }
        }


    // 上昇・減少を繰り返す → 前の値から上昇しているか減少しているかを判別 → 上昇・減少が切り替わっていれば、それがリップルのピーク
    // 正のピークから負のピークまでの変化量がリップルの大きさ

    }
    return ripple;
}