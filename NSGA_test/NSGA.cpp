// NSGA_plot_full_fixed.cpp
#include <pagmo/pagmo.hpp>
#include <pagmo/problems/zdt.hpp>
#include <pagmo/algorithms/nsga2.hpp>
#include <pagmo/utils/multi_objective.hpp>  // sort_population_mo, fast_non_dominated_sorting
#include <pagmo/population.hpp>
#include "gnuplot-iostream.h"
#include <iostream>
#include <vector>
#include <utility>

int main() {
    using namespace pagmo;

    // 1) 問題とアルゴリズム設定
    problem prob{zdt(1, 3)};
    algorithm algo{nsga2(250, 0.9, 20.0, 1.0/3.0, 20.0)};
    population pop{prob, 100u};

    // 2) 最適化実行
    pop = algo.evolve(pop);

    // 3) 目的関数値の取得
    auto f = pop.get_f();  // 全解の f1,f2

    // 4) Pareto フロントの取得
    auto fn_ds = pagmo::fast_non_dominated_sorting(f);
    auto &fronts = std::get<0>(fn_ds);         // タプルの最初の要素が vector<vector<uint>> fronts
    std::size_t front_size = fronts[0].size(); // fronts[0] が Pareto 最前列

    // 5) 全解インデックスを非優越＋クラウディングでソート
    auto sorted_idx = pagmo::sort_population_mo(f);

    // 6) プロット用データ整形
    std::vector<std::pair<double, double>> all_pts, pareto_pts;
    all_pts.reserve(f.size());
    pareto_pts.reserve(front_size);
    for (std::size_t i = 0; i < f.size(); ++i) {
        all_pts.emplace_back(f[i][0], f[i][1]);
    }
    // sorted_idx の先頭 front_size 個が Pareto front
    for (std::size_t k = 0; k < front_size; ++k) {
        std::size_t i = sorted_idx[k];
        pareto_pts.emplace_back(f[i][0], f[i][1]);
    }

    // 7) gnuplot で描画 & ファイル出力
    Gnuplot gp;
    gp << "set terminal pngcairo size 800,600 enhanced font 'Arial,10'\n"
          "set output 'pareto_full.png'\n"
          "set title 'Feasible Region & Pareto Front (ZDT1, n=3)'\n"
          "set xlabel 'f1'\n"
          "set ylabel 'f2'\n"
          "plot '-' with points pt 7 ps 0.5 lc rgb 'gray' title 'Feasible region', "
          "'-' with points pt 7 ps 1 lc rgb 'red' title 'Pareto front'\n";
    gp.send1d(all_pts);
    gp.send1d(pareto_pts);

    std::cout << "Plot saved to pareto_full.png\n";
    return 0;
}
