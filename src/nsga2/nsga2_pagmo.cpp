#include "function.hpp"
#include <random>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <iostream>

#if defined(__has_include)
#  if __has_include(<pagmo/pagmo.hpp>)
#    include <pagmo/pagmo.hpp>
#    include <pagmo/algorithms/nsga2.hpp>
#    include <pagmo/population.hpp>
#    include <pagmo/types.hpp>
#    define PAGMO_AVAILABLE 1
#  else
#    pragma message("pagmo library not found, run_nsga2_pagmo will be disabled")
#    define PAGMO_AVAILABLE 0
#  endif
#else
#  pragma message("__has_include unavailable, pagmo optional support disabled")
#  define PAGMO_AVAILABLE 0
#endif

#if PAGMO_AVAILABLE
using namespace pagmo;

//======================================
// UDP: JosephsonCircuits を呼び出して (gain, bandwidth, ripple) を得る
//======================================
struct josephson_problem {
    double Lj;
    std::vector<ele_unit> ele;
    std::vector<std::string> jl_source;

    josephson_problem(double Lj_, const std::vector<ele_unit>& ele_,
                      const std::vector<std::string>& jl_src_)
        : Lj(Lj_), ele(ele_), jl_source(jl_src_) {}

    pagmo::vector_double fitness(const pagmo::vector_double &x) const {
        const double Cg = x[0];
        const double Cc = x[1];
        const double denom = 3.0 * Lj / (49.0 * 49.0);
        const double Cn = denom - 2.0 * Cg - Cc;

        double gain = 0.0, bw = 0.0, ripple = 0.0;
        if (Cn > 0.0) {
            change_param(const_cast<std::vector<ele_unit>&>(ele), "Cg", Cg);
            change_param(const_cast<std::vector<ele_unit>&>(ele), "Cc", Cc);
            change_param(const_cast<std::vector<ele_unit>&>(ele), "Cn", Cn);
            result r = calculation(ele, jl_source);
            gain   = r.gain;
            bw     = r.bandwidth;
            ripple = r.ripple;
        } else {
            ripple = 1.0;
        }

        double f1 = -gain;
        double f2 = -bw;
        double c1 = ripple - 0.1;
        double c2 = -Cn;
        return {f1, f2, c1, c2};
    }

    std::pair<pagmo::vector_double, pagmo::vector_double> get_bounds() const {
        return {{Cg_min, Cc_min}, {Cg_max, Cc_max}};
    }

    std::size_t get_nobj() const { return 2u; }
    std::size_t get_nic() const { return 2u; }
    std::size_t get_nec() const { return 0u; }

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

    pagmo::population pop{prob, static_cast<unsigned int>(pop_size)};
    pop = algo.evolve(pop);

    const auto all_f = pop.get_f();
    std::cout << "# Cg\tCc\tCn\tgain\tbandwidth\tripple\n";
    for (std::size_t i = 0; i < all_f.size(); ++i) {
        double f1 = all_f[i][0], f2 = all_f[i][1];
        double c1 = all_f[i][2], c2 = all_f[i][3];
        if (c1 <= 0.0 && c2 <= 0.0) {
            const auto xv = pop.get_x()[i];
            double Cg = xv[0];
            double Cc = xv[1];
            double denom = 3.0 * Lj / (49.0 * 49.0);
            double Cn = denom - 2.0 * Cg - Cc;
            double gain      = -f1;
            double bandwidth = -f2;
            double ripple    = c1 + 0.1;
            std::cout << Cg << "\t"
                      << Cc << "\t"
                      << Cn << "\t"
                      << gain << "\t"
                      << bandwidth << "\t"
                      << ripple << "\n";
        }
    }
}
#else
void run_nsga2_pagmo(int pop_size,
                     int generations,
                     const std::vector<ele_unit>& ele,
                     const std::vector<std::string>& jl_source,
                     double Lj,
                     double Cg_min, double Cg_max,
                     double Cc_min, double Cc_max) {
    (void)pop_size; (void)generations; (void)ele; (void)jl_source;
    (void)Lj; (void)Cg_min; (void)Cg_max; (void)Cc_min; (void)Cc_max;
    std::cerr << "run_nsga2_pagmo is disabled because pagmo library is not available." << std::endl;
}
#endif
