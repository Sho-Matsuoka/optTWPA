#include "function.hpp"
#include <random>
#include <vector>
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

    josephson_problem() = default;
    josephson_problem(double Lj_, const std::vector<ele_unit>& ele_,
                      const std::vector<std::string>& jl_src_)
        : Lj(Lj_), ele(ele_), jl_source(jl_src_) {}

    vector_double fitness(const vector_double &x) const {
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


struct Individual {
    double x0, x1;   // Cg, Cc
    double f1, f2;   // -gain, -bandwidth
    double ripple;   // ripple value
    int    rank;
    double crowding;
    bool   valid = false;
};

static std::mt19937_64 rng{std::random_device{}()};

double compute_Cn(double Cg, double Cc, double Lj) {
    return (3.0 * change_Lj(Lj))/(49.0 * 49.0) - 2.0 * Cg - Cc;
}

bool evaluate_single(Individual &ind, double Lj,
                     std::vector<ele_unit> ele,
                     std::vector<std::string> jl_source) {
    double Cg = ind.x0;
    double Cc = ind.x1;
    double Cn = compute_Cn(Cg, Cc, Lj);
    change_param(ele, "Cg", Cg);
    change_param(ele, "Cc", Cc);
    if (Cn <= 0.0) return false;
    change_param(ele, "Cn", Cn);
    result r = calculation(ele, jl_source);
    if (r.ripple > 0.1) return false;
    ind.f1 = -r.gain;
    ind.f2 = -r.bandwidth;
    ind.ripple = r.ripple;
    return true;
}

void evaluate_parallel(std::vector<Individual> &inds, double Lj,
                       const std::vector<ele_unit> &ele,
                       const std::vector<std::string> &jl_source,
                       unsigned max_proc) {
    std::size_t idx = 0;
    struct Data { double f1, f2, ripple; int ok; } d;
    while (idx < inds.size()) {
        unsigned batch = std::min<unsigned>(max_proc, inds.size() - idx);
        std::vector<pid_t> pids(batch);
        std::vector<int> fds(batch);
        for (unsigned b = 0; b < batch; ++b) {
            int pipefd[2];
            pipe(pipefd);
            pid_t pid = fork();
            if (pid == 0) {
                close(pipefd[0]);
                Individual tmp = inds[idx + b];
                bool ok = evaluate_single(tmp, Lj, ele, jl_source);
                Data out;
                out.ok = ok ? 1 : 0;
                out.f1 = tmp.f1;
                out.f2 = tmp.f2;
                out.ripple = tmp.ripple;
                write(pipefd[1], &out, sizeof(out));
                close(pipefd[1]);
                _exit(0);
            } else {
                close(pipefd[1]);
                pids[b] = pid;
                fds[b] = pipefd[0];
            }
        for (unsigned b = 0; b < batch; ++b) {
            read(fds[b], &d, sizeof(d));
            close(fds[b]);
            waitpid(pids[b], nullptr, 0);
            if (d.ok) {
                inds[idx + b].f1 = d.f1;
                inds[idx + b].f2 = d.f2;
                inds[idx + b].ripple = d.ripple;
                inds[idx + b].valid = true;
            } else {
                inds[idx + b].valid = false;
            }
        }
        idx += batch;
    }
    inds.erase(std::remove_if(inds.begin(), inds.end(),
                              [](const Individual &i){ return !i.valid; }),
               inds.end());
}
std::vector<Individual> init_population(int pop_size,
                                        const std::vector<ele_unit> &ele,
                                        const std::vector<std::string> &jl_source,
                                        double Lj,
                                        double Cg_min, double Cg_max,
                                        double Cc_min, double Cc_max,
                                        unsigned max_proc) {
    std::uniform_real_distribution<> dCg(Cg_min, Cg_max);
    std::uniform_real_distribution<> dCc(Cc_min, Cc_max);
    std::vector<Individual> pop;
    pop.reserve(pop_size);
    while ((int)pop.size() < pop_size) {
        std::vector<Individual> batch;
        for (unsigned i = 0; i < max_proc; ++i) {
            Individual ind;
            ind.x0 = dCg(rng);
            ind.x1 = dCc(rng);
            batch.push_back(ind);
        }
        evaluate_parallel(batch, Lj, ele, jl_source, max_proc);
        for (auto &ind : batch) {
            if ((int)pop.size() < pop_size) pop.push_back(ind);
        }
    return pop;
}
std::vector<std::vector<int>> fast_nondominated_sort(std::vector<Individual> &pop) {
    int N = pop.size();
    std::vector<int> dom_count(N, 0);
    std::vector<std::vector<int>> dominated(N);
    std::vector<std::vector<int>> fronts;
    std::vector<int> front0;
    for (int p = 0; p < N; ++p) {
        for (int q = 0; q < N; ++q) {
            if (p == q) continue;
            bool p_dom_q = (pop[p].f1 <= pop[q].f1 && pop[p].f2 <= pop[q].f2) &&
                           (pop[p].f1 < pop[q].f1 || pop[p].f2 < pop[q].f2);
            bool q_dom_p = (pop[q].f1 <= pop[p].f1 && pop[q].f2 <= pop[p].f2) &&
                           (pop[q].f1 < pop[p].f1 || pop[q].f2 < pop[p].f2);
            if (p_dom_q) dominated[p].push_back(q);
            else if (q_dom_p) dom_count[p]++;
        }
        if (dom_count[p] == 0) {
            front0.push_back(p);
            pop[p].rank = 0;
        }
    }
    fronts.push_back(front0);
    int i = 0;
    while (!fronts[i].empty()) {
        std::vector<int> next;
        for (int p : fronts[i]) {
            for (int q : dominated[p]) {
                if (--dom_count[q] == 0) {
                    pop[q].rank = i + 1;
                    next.push_back(q);
                }
            }
        }
        ++i;
        fronts.push_back(next);
    fronts.pop_back();
    return fronts;
}


    std::pair<vector_double, vector_double> get_bounds() const {
        return {{Cg_min, Cc_min}, {Cg_max, Cc_max}};

void assign_crowding(std::vector<Individual> &pop, const std::vector<int> &front) {
    int l = front.size();
    if (l == 0) return;
    for (int idx : front) pop[idx].crowding = 0.0;
    for (int m = 0; m < 2; ++m) {
        auto cmp = [&](int a, int b) {
            return (m == 0 ? pop[a].f1 < pop[b].f1 : pop[a].f2 < pop[b].f2);
        };
        std::vector<int> sorted = front;
        std::sort(sorted.begin(), sorted.end(), cmp);
        pop[sorted[0]].crowding = pop[sorted[l - 1]].crowding = 1e9;
        double fmin = (m == 0 ? pop[sorted[0]].f1 : pop[sorted[0]].f2);
        double fmax = (m == 0 ? pop[sorted[l - 1]].f1 : pop[sorted[l - 1]].f2);
        for (int i = 1; i < l - 1; ++i) {
            double prev = (m == 0 ? pop[sorted[i - 1]].f1 : pop[sorted[i - 1]].f2);
            double next = (m == 0 ? pop[sorted[i + 1]].f1 : pop[sorted[i + 1]].f2);
            pop[sorted[i]].crowding += (next - prev) / (fmax - fmin);
        }

    }
}
int tournament(const std::vector<Individual> &pop, int a, int b) {
    if (pop[a].rank < pop[b].rank) return a;
    if (pop[a].rank > pop[b].rank) return b;
    return pop[a].crowding > pop[b].crowding ? a : b;
}

std::pair<Individual, Individual> crossover(const Individual &p1, const Individual &p2,
                                            double eta_c, double pc) {
    std::uniform_real_distribution<> uni(0.0, 1.0);
    Individual c1 = p1, c2 = p2;
    if (uni(rng) < pc) {
        double u = uni(rng);
        double beta = (u <= 0.5) ? std::pow(2 * u, 1.0 / (eta_c + 1))
                                 : std::pow(1 / (2 * (1 - u)), 1.0 / (eta_c + 1));
        double x1 = 0.5 * ((1 + beta) * p1.x0 + (1 - beta) * p2.x0);
        double x2 = 0.5 * ((1 - beta) * p1.x0 + (1 + beta) * p2.x0);
        c1.x0 = x1; c2.x0 = x2;
        u = uni(rng);
        beta = (u <= 0.5) ? std::pow(2 * u, 1.0 / (eta_c + 1))
                          : std::pow(1 / (2 * (1 - u)), 1.0 / (eta_c + 1));
        x1 = 0.5 * ((1 + beta) * p1.x1 + (1 - beta) * p2.x1);
        x2 = 0.5 * ((1 - beta) * p1.x1 + (1 + beta) * p2.x1);
        c1.x1 = x1; c2.x1 = x2;
    return {c1, c2};
}
void mutate(Individual &ind, double eta_m, double pm,
            double Cg_min, double Cg_max,
            double Cc_min, double Cc_max) {
    std::uniform_real_distribution<> uni(0.0, 1.0);
    auto poly = [&](double xi, double xmin, double xmax) {
        double u = uni(rng), delta;
        if (u < 0.5) delta = std::pow(2 * u, 1.0 / (eta_m + 1)) - 1;
        else         delta = 1 - std::pow(2 * (1 - u), 1.0 / (eta_m + 1));
        double xnew = xi + delta * (xmax - xmin);
        return std::clamp(xnew, xmin, xmax);
    };
    if (uni(rng) < pm) ind.x0 = poly(ind.x0, Cg_min, Cg_max);
    if (uni(rng) < pm) ind.x1 = poly(ind.x1, Cc_min, Cc_max);
}

} // namespace

void run_nsga2_pagmo(int pop_size, int generations,
                     const std::vector<ele_unit> &ele,
                     const std::vector<std::string> &jl_source,
    const double pc = 0.9;
    const double eta_c = 20.0;
    const double eta_m = 20.0;
    const double pm = 1.0 / 2.0; // two variables

    unsigned max_proc = std::max(1u, std::thread::hardware_concurrency());

    auto pop = init_population(pop_size, ele, jl_source, Lj,
                               Cg_min, Cg_max, Cc_min, Cc_max, max_proc);

    for (int gen = 0; gen < generations; ++gen) {
        auto fronts = fast_nondominated_sort(pop);
        for (auto &f : fronts) assign_crowding(pop, f);

        std::vector<Individual> offspring;
        offspring.reserve(pop_size);
        while ((int)offspring.size() < pop_size) {
            std::uniform_int_distribution<> dist(0, pop_size - 1);
            int i1 = tournament(pop, dist(rng), dist(rng));
            int i2 = tournament(pop, dist(rng), dist(rng));
            auto [c1, c2] = crossover(pop[i1], pop[i2], eta_c, pc);
            mutate(c1, eta_m, pm, Cg_min, Cg_max, Cc_min, Cc_max);
            mutate(c2, eta_m, pm, Cg_min, Cg_max, Cc_min, Cc_max);
            std::vector<Individual> cand = {c1, c2};
            evaluate_parallel(cand, Lj, ele, jl_source, max_proc);
            for (auto &ind : cand) {
                if ((int)offspring.size() < pop_size && ind.valid)
                    offspring.push_back(ind);
            }

        std::vector<Individual> comb = pop;
        comb.insert(comb.end(), offspring.begin(), offspring.end());
        auto comb_fronts = fast_nondominated_sort(comb);
        std::vector<Individual> newpop;
        newpop.reserve(pop_size);
        for (auto &f : comb_fronts) {
            assign_crowding(comb, f);
            std::sort(f.begin(), f.end(), [&](int a, int b) {
                if (comb[a].rank != comb[b].rank)
                    return comb[a].rank < comb[b].rank;
                return comb[a].crowding > comb[b].crowding;
            });
            for (int idx : f) {
                if ((int)newpop.size() < pop_size)
                    newpop.push_back(comb[idx]);
            }
            if ((int)newpop.size() >= pop_size) break;
        }
        pop.swap(newpop);
    }


    std::string get_name() const { return "josephson_problem"; }
};


    auto final_fronts = fast_nondominated_sort(pop);
    for (int idx : final_fronts[0]) {
        double Cg = pop[idx].x0;
        double Cc = pop[idx].x1;
        double Cn = compute_Cn(Cg, Cc, Lj);
        std::cout << Cg << "\t" << Cc << "\t" << Cn
                  << "\t" << -pop[idx].f1
                  << "\t" << -pop[idx].f2
                  << "\t" << pop[idx].ripple << "\n";
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
