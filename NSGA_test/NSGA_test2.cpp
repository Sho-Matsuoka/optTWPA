// NSGA_custom_fixed.cpp
#include <pagmo/pagmo.hpp>
#include <pagmo/algorithms/nsga2.hpp>
#include <pagmo/utils/multi_objective.hpp>
#include <pagmo/population.hpp>
#include "gnuplot-iostream.h"
#include <iostream>
#include <vector>
#include <utility>
#include <algorithm>

using namespace std;

// ユーザー定義問題
struct my_problem {
    // x[0]=x1, x[1]=x2
    vector<double> fitness(const vector<double> &x) const {
        double f1 = x[0]*x[0] + x[1]*x[1];
        double f2 = (1.0 - x[0])*(1.0 - x[0]) + (1.0 - x[1])*(1.0 - x[1]) ;
        return {f1, f2};
    }
    // 変数の上下限を返す
    pair<vector<double>, vector<double>> get_bounds() const {
        return {{0.0, 0.0}, {1.0, 1.0}};
    }
    size_t get_nobj() const { return 2; }
};

int main() {
    using namespace pagmo;

    problem prob{my_problem{}};  // カスタム UDP
    //      アルゴリズム設定: NSGA-II
    //    - 世代数: 250
    //    - 交叉率 (cr): 0.9
    //    - 分布指数 for crossover (eta_c): 20
    //    - 変異確率 m = 1/2
    //    - 分布指数 for mutation (eta_m): 20
    algorithm algo{nsga2(250, 0.9, 20.0, 1.0/2.0, 20.0)};
    population pop{prob, 100u};

    pop = algo.evolve(pop);

    auto f = pop.get_f();
    auto fn_ds = fast_non_dominated_sorting(f);
    auto &fronts = get<0>(fn_ds);
    size_t front_size = fronts[0].size();
    auto sorted_idx = sort_population_mo(f);

    vector<pair<double,double>> all_pts, pareto_pts;
    for (size_t i = 0; i < f.size(); ++i)
        all_pts.emplace_back(f[i][0], f[i][1]);
    for (size_t k = 0; k < front_size; ++k) {
        auto i = sorted_idx[k];
        pareto_pts.emplace_back(f[i][0], f[i][1]);
    }

        Gnuplot gp;
        gp << "set terminal pngcairo size 800,600 enhanced font 'Arial,10'\n"
              "set output 'pareto_test2.png'\n"
              "set title 'Pareto Front'\n"
              "set xlabel 'f1'\n"
              "set ylabel 'f2'\n"
              "set xrange [-0.5:3]\n"
              "set yrange [-0.5:3]\n"
              "plot '-' with points pt 7 ps 0.5 lc rgb 'gray' title 'Feasible region', "
              "'-' with points pt 7 ps 1 lc rgb 'red' title 'Pareto front'\n";
        gp.send1d(all_pts);
        gp.send1d(pareto_pts);
    cout << "Done.\n";
    return 0;
}
