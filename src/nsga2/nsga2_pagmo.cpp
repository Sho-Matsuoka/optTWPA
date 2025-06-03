#include "function.hpp"    // result calculation(double Cg, double Cc, double Cn)
#include <random>
#include <vector>
#include <string>          // ← 追加
#include <algorithm>
#include <cmath>
#include <iostream>
#include <pagmo/pagmo.hpp>    // pagmo::problem など
#include <pagmo/algorithms/nsga2.hpp>
#include <pagmo/population.hpp>
#include <pagmo/types.hpp>

using namespace pagmo;


//======================================
// UDP: JosephsonCircuits を呼び出して (gain, bandwidth, ripple) を得る
//======================================
struct josephson_problem {
    // メンバ変数
    double Lj;
    std::vector<ele_unit> ele;
    std::vector<std::string> jl_source;

    // コンストラクタ：Lj, ele, jl_source をメンバにコピー
    josephson_problem(double Lj_,
                      const std::vector<ele_unit>& ele_,
                      const std::vector<std::string>& jl_src_)
        : Lj(Lj_), ele(ele_), jl_source(jl_src_) {}

    // ---------- 必須 (1) : fitness ----------
        pagmo::vector_double fitness(const pagmo::vector_double &x) const {        // x[0] = Cg, x[1] = Cc
        const double Cg = x[0];
        const double Cc = x[1];
        // Cn を式から計算
        const double denom = 3.0 * Lj / (49.0 * 49.0);
        const double Cn = denom - 2.0 * Cg - Cc;

        double gain = 0.0, bw = 0.0, ripple = 0.0;
        if (Cn > 0.0) {
            // ele, jl_source はメンバ。読み取り専用なので一旦 const_cast して渡す
            change_param(const_cast<std::vector<ele_unit>&>(ele), "Cg", Cg);
            change_param(const_cast<std::vector<ele_unit>&>(ele), "Cc", Cc);
            change_param(const_cast<std::vector<ele_unit>&>(ele), "Cn", Cn);
            result r = calculation(ele, jl_source);
            gain   = r.gain;
            bw     = r.bandwidth;
            ripple = r.ripple;
        } else {
            // Cn <= 0 は制約違反とみなす
            ripple = 1.0;  
        }

        // 目的関数は最小化なので符号を反転
        double f1 = -gain;
        double f2 = -bw;
        // 制約 c1(x) = ripple - 0.1 <= 0, c2(x) = -Cn <= 0
         double c1 = ripple - 0.1;           // ripple <= 0.1 → c1 = ripple - 0.1 <= 0
         double c2 = -Cn;                    // Cn > 0 → -Cn <= 0
         return {f1, f2, c1, c2};    }

    // ---------- 必須 (2) : get_bounds ----------
     std::pair<pagmo::vector_double, pagmo::vector_double> get_bounds() const {
         return {{Cg_min, Cc_min}, {Cg_max, Cc_max}};
     }

    // ---------- 必須 (3) : 目的関数・制約の数 ----------
    std::size_t get_nobj() const { return 2u; }  // 目的 f1, f2
    std::size_t get_nic() const { return 2u; }   // 不等式制約 c1, c2
    std::size_t get_nec() const { return 0u; }   // 等式制約なし

    // 範囲 (bounds) を保持するメンバ
    double Cg_min, Cg_max, Cc_min, Cc_max;
    void set_bounds(double _Cg_min, double _Cg_max,
                    double _Cc_min, double _Cc_max) {
        Cg_min = _Cg_min;  Cg_max = _Cg_max;
        Cc_min = _Cc_min;  Cc_max = _Cc_max;
    }
};


//======================================
// run_nsga2_pagmo ：Pagmo を使った NSGA‐II 実行
//======================================
void run_nsga2_pagmo(int pop_size,
                     int generations,
                     const std::vector<ele_unit>& ele,
                     const std::vector<std::string>& jl_source,
                     double Lj,
                     double Cg_min, double Cg_max,
                     double Cc_min, double Cc_max) {
    // 1) UDP を作成し、bounds をセット
    josephson_problem prob_udp(Lj, ele, jl_source);
    prob_udp.set_bounds(Cg_min, Cg_max, Cc_min, Cc_max);

    // 2) pagmo::problem にラップ  ← ここがコンパイルエラーなく通るはず
    pagmo::problem prob{prob_udp};    

    // 3) NSGA-II のアルゴリズムを作る
    pagmo::algorithm algo{ pagmo::nsga2(
        generations,    // 世代数
        0.9,            // 交叉率
        20.0,           // η_c (交叉分布指数)
        1.0/2.0,        // 変異率 p_m = 1/次元 (ここは次元=2 なので 1/2)
        20.0            // η_m (変異分布指数)
    ) };

    // 4) 初期集団 (pop_size 個) を自動生成
    pagmo::population pop{prob, static_cast<unsigned int>(pop_size)};

    // 5) 最適化を実行
    pop = algo.evolve(pop);

    // 6) 結果出力: 最終世代の Pareto フロント
    const auto all_f = pop.get_f();
    std::cout << "# Cg\tCc\tCn\tgain\tbandwidth\tripple\n";
    for (std::size_t i = 0; i < all_f.size(); ++i) {
        double f1 = all_f[i][0], f2 = all_f[i][1];
        double c1 = all_f[i][2], c2 = all_f[i][3];
        // 制約 c1 <= 0 (ripple ≤ 0.1) 且つ c2 <= 0 (Cn > 0) のものだけ
        if (c1 <= 0.0 && c2 <= 0.0) {
            const auto xv = pop.get_x()[i];  // {Cg, Cc}
            double Cg = xv[0];
            double Cc = xv[1];
            // Cn を計算しなおす
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
