#include "function.hpp"
#include <random>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <utility>

#if __has_include(<pagmo/pagmo.hpp>)
#include <pagmo/pagmo.hpp>
#include <pagmo/algorithms/nsga2.hpp>
#include <pagmo/population.hpp>
#include <pagmo/types.hpp>
#include <pagmo/archipelago.hpp>
#include <pagmo/islands/fork_island.hpp>
#define PAGMO_AVAILABLE 1
#else
#pragma message("pagmo library not found, run_nsga2_pagmo_island will be disabled")
#define PAGMO_AVAILABLE 0
#endif

#if PAGMO_AVAILABLE
using pagmo::vector_double;
using pagmo::problem;
using pagmo::algorithm;
using pagmo::population;
using pagmo::nsga2;
using pagmo::archipelago;
using pagmo::fork_island;

//======================================
// UDP: JosephsonCircuits を呼び出して (gain, bandwidth, ripple) を得る
//======================================
struct josephson_problem {
    double Lj;
    std::vector<ele_unit> ele;
    std::vector<std::string> jl_source;
    double Cg_min, Cg_max, Cc_min, Cc_max;

    josephson_problem() = default;
    josephson_problem(double Lj_, const std::vector<ele_unit>& ele_,
                      const std::vector<std::string>& jl_src_)
        : Lj(Lj_), ele(ele_), jl_source(jl_src_) {}

    result eval(double Cg, double Cc) const {
        auto tmp = ele;
        change_param(tmp, "Cg", Cg);
        change_param(tmp, "Cc", Cc);
        double denom = 3.0 * change_Lj(Lj) / (49.0 * 49.0);
        double Cn = denom - 2.0 * Cg - Cc;
        if (Cn <= 0.0) {
            return {0.0, 0.0, 1.0};
        }
        change_param(tmp, "Cn", Cn);
        return calculation(tmp, jl_source);
    }

    vector_double fitness(const vector_double &x) const {
        result r = eval(x[0], x[1]);
        double f1 = -r.gain;
        double f2 = -r.bandwidth;
        if (r.ripple > 0.1) {
            f1 = 1e6 + r.ripple;
            f2 = 1e6 + r.ripple;
        }
        return {f1, f2};
    }

    std::pair<vector_double, vector_double> get_bounds() const {
        return {{Cg_min, Cc_min}, {Cg_max, Cc_max}};
    }

    std::size_t get_nobj() const { return 2u; }
    std::size_t get_nic() const { return 0u; }
    std::size_t get_nec() const { return 0u; }

    void set_bounds(double _Cg_min, double _Cg_max,
                    double _Cc_min, double _Cc_max) {
        Cg_min = _Cg_min;  Cg_max = _Cg_max;
        Cc_min = _Cc_min;  Cc_max = _Cc_max;
    }

    std::string get_name() const { return "josephson_problem"; }
};

//======================================
// run_nsga2_pagmo_island : Pagmo Island Model を使った NSGA‐II 実行
// fork_island を使い、各島をプロセス単位で実行
//======================================
void run_nsga2_pagmo_island(int pop_size,
                            int generations,
                            const std::vector<ele_unit>& ele,
                            const std::vector<std::string>& jl_source,
                            double Lj,
                            double Cg_min, double Cg_max,
                            double Cc_min, double Cc_max) {
    josephson_problem prob_udp(Lj, ele, jl_source);
    prob_udp.set_bounds(Cg_min, Cg_max, Cc_min, Cc_max);

    pagmo::problem prob{prob_udp};

    pagmo::algorithm algo{ pagmo::nsga2(
        generations,
        0.9,
        20.0,
        1.0/2.0,
        20.0
    ) };

    archipelago archi;
    archi.set_nb_islands(pop_size);

    std::vector<population> pops;
    pops.reserve(pop_size);
    for (int i = 0; i < pop_size; ++i) {
        pops.emplace_back(prob, 1u);
    }

    for (int i = 0; i < pop_size; ++i) {
        archi.push_back(fork_island{algo, std::move(pops[i])});
    }

    archi.evolve();
    archi.wait_check();

    std::cout << "# Cg\tCc\tCn\tgain\tbandwidth\tripple\n";
    for (auto &isl : archi) {
        const auto &pop = isl.get_population();
        const auto xv = pop.get_x()[0];
        double Cg = xv[0];
        double Cc = xv[1];
        result r = prob_udp.eval(Cg, Cc);
        double denom = 3.0 * Lj / (49.0 * 49.0);
        double Cn = denom - 2.0 * Cg - Cc;
        if (r.ripple <= 0.1 && Cn > 0.0) {
            std::cout << Cg << "\t"
                      << Cc << "\t"
                      << Cn << "\t"
                      << r.gain << "\t"
                      << r.bandwidth << "\t"
                      << r.ripple << "\n";
        }
    }
}
#else
void run_nsga2_pagmo_island(int pop_size,
                            int generations,
                            const std::vector<ele_unit>& ele,
                            const std::vector<std::string>& jl_source,
                            double Lj,
                            double Cg_min, double Cg_max,
                            double Cc_min, double Cc_max) {
    (void)pop_size; (void)generations; (void)ele; (void)jl_source;
    (void)Lj; (void)Cg_min; (void)Cg_max; (void)Cc_min; (void)Cc_max;
    std::cerr << "run_nsga2_pagmo_island is disabled because pagmo library is not available." << std::endl;
}
#endif

