#include "function.hpp"    // result calculation(double Cg, double Cc, double Cn)
#include <random>
#include <vector>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <pagmo/pagmo.hpp>
#include <pagmo/algorithms/nsga2.hpp>
#include <pagmo/population.hpp>
#include <vector>
#include <cmath>
#include <iostream>
using namespace pagmo;


// UDP: JosephsonCircuits を呼び出して (gain, bandwidth, ripple) を得る問題定義
struct josephson_problem {
    // コンストラクタで Lj を受け取る
    double Lj;
    josephson_problem(double Lj_) : Lj(Lj_) {}

    // ========== fitness() ==========
    // x は長さ 2 のベクトル: x[0] = Cg, x[1] = Cc
    // 戻り値は 目的関数値 2 個に続いて不等式制約 2 個を追加して返す:
    //   obj[0] = -gain （NSGA-II は最小化するため符号反転）
    //   obj[1] = -bandwidth
    //   cons[0] = ripple - 0.1    <= 0 が成り立つこと（ripple ≤ 0.1）
    //   cons[1] = -Cn              <= 0 が成り立つこと（Cn > 0）
    std::vector<double> fitness(const std::vector<double> &x) const {
        const double Cg = x[0];
        const double Cc = x[1];
        // Cn を制約式 sqrt(3Lj/(2Cg + Cn + Cc))=49 から計算:
        //   2Cg + Cn + Cc = 3Lj / 49^2
        const double denom = 3.0 * Lj / (49.0 * 49.0);
        const double Cn = denom - 2.0 * Cg - Cc;

        double gain = 0.0, bw = 0.0, ripple = 0.0;
        if (Cn > 0.0) {
            // Cn 正ならば外部の計算ルーチンを呼ぶ
            result r = calculation(Cg, Cc, Cn);
            gain   = r.gain;
            bw     = r.bandwidth;
            ripple = r.ripple;
        } else {
            // Cn ≤ 0 のときはすぐ制約違反扱いにする
            // gain, bw は適当な値 (この後 制約で落ちる)
            ripple = 1.0;  // 明らかに 0.1 を超えるように
        }

        // 目的関数は最小化なので符号を反転
        double f1 = -gain;
        double f2 = -bw;

        // 制約: g1(x) = ripple - 0.1 <= 0, g2(x) = -Cn <= 0
        double c1 = ripple - 0.1;
        double c2 = (Cn > 0.0 ? -Cn : -Cn);  // Cn ≤ 0 なら正、Cn>0 なら負

        return {f1, f2, c1, c2};
    }

    // ========== get_bounds() ==========
    // x[0]=Cg の範囲, x[1]=Cc の範囲
    std::pair<std::vector<double>, std::vector<double>> get_bounds() const {
        return { {Cg_min, Cc_min}, {Cg_max, Cc_max} };
    }

    // 下限・上限を保持するメンバを持っておく
    double Cg_min, Cg_max, Cc_min, Cc_max;
    void set_bounds(double _Cg_min, double _Cg_max,
                    double _Cc_min, double _Cc_max) {
        Cg_min = _Cg_min;  Cg_max = _Cg_max;
        Cc_min = _Cc_min;  Cc_max = _Cc_max;
    }

    // ========== get_nobj() ==========
    // 目的関数の数 = 2
    std::size_t get_nobj() const { return 2u; }

    // ========== get_nic() ==========
    // 不等式制約の数 = 2
    std::size_t get_nic() const { return 2u; }

    // ========== get_nec() ==========
    // 等式制約は使わないので 0
    std::size_t get_nec() const { return 0u; }
};

// run_nsga2_pagmo 関数の実装
void run_nsga2_pagmo(int pop_size,
                     int generations,
                     double Lj,
                     double Cg_min, double Cg_max,
                     double Cc_min, double Cc_max) {
    // 1) UDP を作って bounds を設定
    josephson_problem prob_udp(Lj);
    prob_udp.set_bounds(Cg_min, Cg_max, Cc_min, Cc_max);

    // 2) pagmo::problem にラップ
    problem prob{prob_udp};

    // 3) NSGA-II アルゴリズムを設定
    //    世代数 = generations, 交叉率=0.9, η_c=20, 変異率=1/2, η_m=20
    algorithm algo{nsga2(generations, 0.9, 20.0, 1.0/2.0, 20.0)};

    // 4) 初期集団を作成
    population pop{prob, static_cast<unsigned int>(pop_size)};

    // 5) 最適化実行
    pop = algo.evolve(pop);

    // 6) 結果出力: 最終世代の Pareto フロントのみを出力する
    //    pop.get_f() で (f1,f2,c1,c2) のリストが得られるので、
    //    制約を満たすものだけフィルタして表示します。
    const auto all_f = pop.get_f();
    std::cout << "# Cg\tCc\tCn\tgain\tbandwidth\tripple\n";
    for (std::size_t i = 0; i < all_f.size(); ++i) {
        double f1 = all_f[i][0], f2 = all_f[i][1];
        double c1 = all_f[i][2], c2 = all_f[i][3];
        // 制約 c1 <= 0, c2 <= 0 のものだけ
        if (c1 <= 0.0 && c2 <= 0.0) {
            // pop.get_x()[i] で (Cg, Cc) が得られる
            const auto xv = pop.get_x()[i];
            double Cg = xv[0];
            double Cc = xv[1];
            // Cn を再計算
            double denom = 3.0 * Lj / (49.0 * 49.0);
            double Cn = denom - 2.0 * Cg - Cc;
            double gain      = -f1;
            double bandwidth = -f2;
            double ripple    = c1 + 0.1;  // c1 = ripple - 0.1
            std::cout << Cg << "\t"
                      << Cc << "\t"
                      << Cn << "\t"
                      << gain << "\t"
                      << bandwidth << "\t"
                      << ripple << "\n";
        }
    }
}