#include "function.hpp"
#include <random>
#include <vector>
#include <string>
#include <iostream>

#if __has_include(<pagmo/pagmo.hpp>)
#include <pagmo/pagmo.hpp>
#include <pagmo/algorithms/nsga2.hpp>
#include <pagmo/population.hpp>
#define PAGMO_AVAILABLE 1
#else
#pragma message("pagmo library not found, run_nsga2_pagmo_codex will be disabled")
#define PAGMO_AVAILABLE 0
#endif

#if PAGMO_AVAILABLE
using pagmo::vector_double;
namespace {
    static std::mt19937_64 rng{std::random_device{}()};
}

struct josephson_problem_codex {
    double Lj;
    std::vector<ele_unit> ele;
    std::vector<std::string> jl_source;
    double Cg_min, Cg_max, Cc_min, Cc_max;

    josephson_problem_codex(double Lj_, const std::vector<ele_unit>& e,
                            const std::vector<std::string>& jl)
        : Lj(Lj_), ele(e), jl_source(jl) {}

    result eval(double Cg, double Cc) const {
        auto ele_tmp = ele;
        change_param(ele_tmp, "Cg", Cg);
        change_param(ele_tmp, "Cc", Cc);
        double denom = 3.0 * change_Lj(Lj) / (49.0 * 49.0);
        double Cn = denom - 2.0 * Cg - Cc;
        if (Cn <= 0.0) {
            result r{0.0, 0.0, 1.0};
            return r;
        }
        change_param(ele_tmp, "Cn", Cn);
        return calculation(ele_tmp, jl_source);
    }

    vector_double fitness(const vector_double &x) const {
        result r = eval(x[0], x[1]);
        return {-r.gain, -r.bandwidth};
    }

    std::pair<vector_double, vector_double> get_bounds() const {
        return {{Cg_min, Cc_min}, {Cg_max, Cc_max}};
    }

    std::size_t get_nobj() const { return 2u; }

    std::string get_name() const { return "josephson_problem_codex"; }

    void set_bounds(double cgmin, double cgmax, double ccmin, double ccmax) {
        Cg_min = cgmin; Cg_max = cgmax;
        Cc_min = ccmin; Cc_max = ccmax;
    }
};
#endif // PAGMO_AVAILABLE

#if PAGMO_AVAILABLE
void run_nsga2_pagmo_codex(int pop_size,
                           int generations,
                           const std::vector<ele_unit>& ele,
                           const std::vector<std::string>& jl_source,
                           double Lj,
                           double Cg_min, double Cg_max,
                           double Cc_min, double Cc_max) {
    josephson_problem_codex prob_udp(Lj, ele, jl_source);
    prob_udp.set_bounds(Cg_min, Cg_max, Cc_min, Cc_max);
    problem prob{prob_udp};
    algorithm algo{ nsga2(generations, 0.9, 20.0, 1.0/2.0, 20.0) };
    population pop{prob};

    std::uniform_real_distribution<> dCg(Cg_min, Cg_max);
    std::uniform_real_distribution<> dCc(Cc_min, Cc_max);
    while (pop.size() < static_cast<unsigned>(pop_size)) {
        double cg = dCg(rng);
        double cc = dCc(rng);
        result r = prob_udp.eval(cg, cc);
        if (r.ripple > 0.1) continue;
        vector_double x = {cg, cc};
        pop.push_back(x);
    }

    pop = algo.evolve(pop);

    std::cout << "# Cg\tCc\tCn\tgain\tbandwidth\tripple\n";
    for (std::size_t i = 0; i < pop.size(); ++i) {
        vector_double x = pop.get_x()[i];
        result r = prob_udp.eval(x[0], x[1]);
        double cg = x[0];
        double cc = x[1];
        double denom = 3.0 * change_Lj(Lj) / (49.0 * 49.0);
        double cn = denom - 2.0 * cg - cc;
        if (r.ripple <= 0.1 && cn > 0.0) {
            std::cout << cg << "\t" << cc << "\t" << cn
                      << "\t" << r.gain << "\t" << r.bandwidth
                      << "\t" << r.ripple << "\n";
        }
    }
}
#else
void run_nsga2_pagmo_codex(int, int, const std::vector<ele_unit>&,
                           const std::vector<std::string>&, double,
                           double, double, double, double) {
    std::cerr << "run_nsga2_pagmo_codex is disabled because pagmo library is not available." << std::endl;
}
#endif
