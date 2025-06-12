#include "function.hpp"    // result calculation(double Cg, double Cc, double Cn)
#include <random>
#include <vector>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <fstream>
#include <sys/wait.h>
#include <unistd.h>
#include <thread>

namespace {

// 個体構造体
struct Individual {
    double x0, x1;    // x0 = Cg, x1 = Cc
    double f1, f2;    // 目的関数値（最小化用に -gain, -bandwidth にする）
    double ripple;    // 制約値
    int rank;
    double crowding;
};

// グローバル RNG
static std::mt19937_64 rng{std::random_device{}()};

// 制約式から Cn を計算
double compute_Cn(double Cg, double Cc, double Lj) {
    // √(3Lj/(2Cg+Cn+Cc))=49 → 2Cg+Cn+Cc = 3Lj/49^2
    return (3.0 * change_Lj(Lj))/ (49.0 * 49.0) - 2.0 * Cg - Cc;
}

// 評価＆制約判定
// ripple>0.1 なら false を返し，個体は破棄
bool evaluate(Individual &ind, double Lj, std::vector<ele_unit> ele, std::vector<std::string> jl_source) {

    //change_param();
    double Cg = ind.x0;
    double Cc = ind.x1;
    double Cn = compute_Cn(Cg, Cc, Lj);
    change_param(ele, "Cg", Cg);
    change_param(ele, "Cc", Cc);
    if (Cn <= 0.0) return false;
    change_param(ele, "Cn", Cn);
    // 外部計算
    result r = calculation(ele, jl_source);
    if (r.ripple > 0.1) return false;
    // NSGA-II は最小化なので符号を反転
    ind.f1     = -r.gain;
    ind.f2     = -r.bandwidth;
    ind.ripple = r.ripple;
    return true;
}

// 非優越ソート
std::vector<std::vector<int>> fast_nondominated_sort(std::vector<Individual>& pop) {
    int N = pop.size();
    std::vector<int> dom_count(N,0);
    std::vector<std::vector<int>> dominated(N);
    std::vector<std::vector<int>> fronts;
    std::vector<int> front0;
    for(int p=0; p<N; ++p){
        for(int q=0; q<N; ++q){
            if(p==q) continue;
            bool p_dom_q = (pop[p].f1 <= pop[q].f1 && pop[p].f2 <= pop[q].f2)
                           && (pop[p].f1 < pop[q].f1 || pop[p].f2 < pop[q].f2);
            bool q_dom_p = (pop[q].f1 <= pop[p].f1 && pop[q].f2 <= pop[p].f2)
                           && (pop[q].f1 < pop[p].f1 || pop[q].f2 < pop[p].f2);
            if(p_dom_q) dominated[p].push_back(q);
            else if(q_dom_p) dom_count[p]++;
        }
        if(dom_count[p]==0){
            front0.push_back(p);
            pop[p].rank = 0;
        }
    }
    fronts.push_back(front0);
    int i=0;
    while(!fronts[i].empty()){
        std::vector<int> next;
        for(int p: fronts[i]){
            for(int q: dominated[p]){
                if(--dom_count[q] == 0){
                    pop[q].rank = i+1;
                    next.push_back(q);
                }
            }
        }
        ++i;
        fronts.push_back(next);
    }
    fronts.pop_back();
    return fronts;
}

// クラウディング距離計算
void assign_crowding(std::vector<Individual>& pop, const std::vector<int>& front) {
    int l = front.size();
    if(l==0) return;
    for(int idx: front) pop[idx].crowding = 0.0;
    for(int m=0; m<2; ++m){
        auto cmp = [&](int a,int b){
            return (m==0 ? pop[a].f1 < pop[b].f1 : pop[a].f2 < pop[b].f2);
        };
        std::vector<int> sorted = front;
        std::sort(sorted.begin(), sorted.end(), cmp);
        pop[sorted[0]].crowding = pop[sorted[l-1]].crowding = 1e9;
        double fmin = (m==0? pop[sorted[0]].f1 : pop[sorted[0]].f2);
        double fmax = (m==0? pop[sorted[l-1]].f1 : pop[sorted[l-1]].f2);
        for(int i=1; i<l-1; ++i){
            double prev = (m==0? pop[sorted[i-1]].f1 : pop[sorted[i-1]].f2);
            double next = (m==0? pop[sorted[i+1]].f1 : pop[sorted[i+1]].f2);
            pop[sorted[i]].crowding += (next - prev) / (fmax - fmin);
        }
    }
}

// 二項トーナメント
int tournament(const std::vector<Individual>& pop, int a, int b){
    if(pop[a].rank < pop[b].rank) return a;
    if(pop[a].rank > pop[b].rank) return b;
    return pop[a].crowding > pop[b].crowding ? a : b;
}

// SBX 交叉（Cg,Cc だけ）
std::pair<Individual,Individual> crossover(const Individual& p1, const Individual& p2,
                                           double eta_c, double pc) {
    std::uniform_real_distribution<> uni(0.0,1.0);
    Individual c1=p1, c2=p2;
    if(uni(rng) < pc){
        double u = uni(rng);
        double beta = (u<=0.5) ? std::pow(2*u,1.0/(eta_c+1))
                               : std::pow(1/(2*(1-u)),1.0/(eta_c+1));
        double x1 = 0.5*((1+beta)*p1.x0 + (1-beta)*p2.x0);
        double x2 = 0.5*((1-beta)*p1.x0 + (1+beta)*p2.x0);
        c1.x0 = x1; c2.x0 = x2;
        u = uni(rng);
        beta = (u<=0.5) ? std::pow(2*u,1.0/(eta_c+1))
                       : std::pow(1/(2*(1-u)),1.0/(eta_c+1));
        x1 = 0.5*((1+beta)*p1.x1 + (1-beta)*p2.x1);
        x2 = 0.5*((1-beta)*p1.x1 + (1+beta)*p2.x1);
        c1.x1 = x1; c2.x1 = x2;
    }
    return {c1,c2};
}

// 多項式突然変異（Cg,Cc だけ）
void mutate(Individual& ind, double eta_m, double pm, double Cg_min, double Cg_max,
            double Cc_min, double Cc_max) {
    std::uniform_real_distribution<> uni(0.0,1.0);
    auto poly = [&](double xi, double xmin, double xmax){
        double u = uni(rng), delta;
        if(u < 0.5) delta = std::pow(2*u, 1.0/(eta_m+1)) - 1;
        else         delta = 1 - std::pow(2*(1-u), 1.0/(eta_m+1));
        double xnew = xi + delta*(xmax - xmin);
        return std::clamp(xnew, xmin, xmax);
    };
    if(uni(rng) < pm) ind.x0 = poly(ind.x0, Cg_min, Cg_max);
    if(uni(rng) < pm) ind.x1 = poly(ind.x1, Cc_min, Cc_max);
}

struct ProcInfo {
    pid_t pid;
    int fd;
};

ProcInfo spawn_eval(Individual ind, double Lj,
                    const std::vector<ele_unit>& ele,
                    const std::vector<std::string>& jl_source) {
    int fds[2];
    if (pipe(fds) == -1) {
        perror("pipe");
        return {-1, -1};
    }
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        close(fds[0]);
        close(fds[1]);
        return {-1, -1};
    }
    if (pid == 0) {
        close(fds[0]);
        bool ok = evaluate(ind, Lj, ele, jl_source);
        (void)!write(fds[1], &ok, sizeof(ok));
        if (ok) {
            (void)!write(fds[1], &ind, sizeof(ind));
        }
        close(fds[1]);
        _exit(0);
    }
    close(fds[1]);
    return {pid, fds[0]};
}

bool collect_eval(ProcInfo p, Individual &out) {
    if (p.pid < 0) return false;
    bool ok = false;
    if (read(p.fd, &ok, sizeof(ok)) != (ssize_t)sizeof(ok)) ok = false;
    if (ok) {
        Individual tmp;
        if (read(p.fd, &tmp, sizeof(tmp)) == (ssize_t)sizeof(tmp)) {
            out = tmp;
        } else {
            ok = false;
        }
    }
    close(p.fd);
    waitpid(p.pid, nullptr, 0);
    return ok;
}

std::vector<Individual> init_population(int pop_size, std::vector<ele_unit> ele, std::vector<std::string> jl_source, double Lj, double Cg_min, double Cg_max, double Cc_min, double Cc_max) {
    std::uniform_real_distribution<> dCg(Cg_min, Cg_max);
    std::uniform_real_distribution<> dCc(Cc_min, Cc_max);
    std::vector<Individual> pop;
    pop.reserve(pop_size);
    unsigned int max_proc = std::thread::hardware_concurrency();
    if (max_proc == 0) max_proc = 2;
    std::vector<ProcInfo> running;
    int trial = 1;
    while(pop.size() < (size_t)pop_size) {
        while(running.size() < max_proc) {
            Individual ind;
            ind.x0 = dCg(rng);
            ind.x1 = dCc(rng);
            ProcInfo p = spawn_eval(ind, Lj, ele, jl_source);
            if (p.pid == -1) {
                if(evaluate(ind, Lj, ele, jl_source)) pop.push_back(ind);
            } else {
                running.push_back(p);
            }
            std::cout << " This is the " << trial << "th trial" << std::endl;
            ++trial;
            if(pop.size() + running.size() >= (size_t)pop_size) break;
        }
        if(!running.empty()) {
            ProcInfo p = running.front();
            running.erase(running.begin());
            Individual res;
            if(collect_eval(p, res)) pop.push_back(res);
        }
    }
    for(auto &p : running) {
        Individual res;
        if(collect_eval(p, res) && pop.size() < (size_t)pop_size) pop.push_back(res);
    }
    return pop;
}

} // anonymous

void run_nsga2_par(int pop_size,int generations, std::vector<ele_unit> ele, std::vector<std::string> jl_source, double Lj, double Cg_min, double Cg_max,  double Cc_min, double Cc_max) {
    // NSGA-II のパラメータ
    const double pc    = 0.9;
    const double eta_c = 20.0;
    const double eta_m = 20.0;
    const double pm    = 1.0/2.0;  // 2変数
    
    // 1. 初期集団
    auto pop = init_population(pop_size, ele, jl_source, Lj, Cg_min, Cg_max, Cc_min, Cc_max);

    // 2. 世代進化ループ
    for(int gen=0; gen<generations; ++gen) {
        // 非優越ソート & クラウディング距離
        auto fronts = fast_nondominated_sort(pop);
        for(auto &f : fronts) assign_crowding(pop, f);

        // オフスプリング生成
        std::vector<Individual> offspring;
        offspring.reserve(pop_size);
        unsigned int max_proc = std::thread::hardware_concurrency();
        if (max_proc == 0) max_proc = 2;
        std::vector<ProcInfo> running;
        while((int)offspring.size() < pop_size) {
            while(running.size() < max_proc && (offspring.size() + running.size()) < (size_t)pop_size) {
                std::uniform_int_distribution<> dist(0, pop_size-1);
                int i1 = tournament(pop, dist(rng), dist(rng));
                int i2 = tournament(pop, dist(rng), dist(rng));
                auto [c1, c2] = crossover(pop[i1], pop[i2], eta_c, pc);
                mutate(c1, eta_m, pm, Cg_min, Cg_max, Cc_min, Cc_max);
                mutate(c2, eta_m, pm, Cg_min, Cg_max, Cc_min, Cc_max);
                ProcInfo p1 = spawn_eval(c1, Lj, ele, jl_source);
                if (p1.pid == -1) {
                    if(evaluate(c1, Lj, ele, jl_source)) offspring.push_back(c1);
                } else {
                    running.push_back(p1);
                }
                if((offspring.size() + running.size()) < (size_t)pop_size) {
                    ProcInfo p2 = spawn_eval(c2, Lj, ele, jl_source);
                    if (p2.pid == -1) {
                        if(evaluate(c2, Lj, ele, jl_source)) offspring.push_back(c2);
                    } else {
                        running.push_back(p2);
                    }
                }
            }
            if(!running.empty()) {
                ProcInfo p = running.front();
                running.erase(running.begin());
                Individual res;
                if(collect_eval(p, res)) offspring.push_back(res);
            }
        }
        for(auto &p : running) {
            Individual res;
            if((int)offspring.size() >= pop_size) break;
            if(collect_eval(p, res)) offspring.push_back(res);
        }

        // 3. 次世代選抜
        std::vector<Individual> comb = pop;
        comb.insert(comb.end(), offspring.begin(), offspring.end());
        auto comb_fronts = fast_nondominated_sort(comb);
        std::vector<Individual> newpop;
        newpop.reserve(pop_size);
        for(auto &f : comb_fronts){
            assign_crowding(comb, f);
            // front 内ソート： rank asc, crowding desc
            std::sort(f.begin(), f.end(), [&](int a,int b){
                if(comb[a].rank != comb[b].rank)
                    return comb[a].rank < comb[b].rank;
                return comb[a].crowding > comb[b].crowding;
            });
            for(int idx : f){
                if((int)newpop.size() < pop_size)
                    newpop.push_back(comb[idx]);
            }
            if((int)newpop.size() >= pop_size) break;
        }
        pop.swap(newpop);
    }

    // 結果出力：Pareto front
    auto final_fronts = fast_nondominated_sort(pop);
    std::ofstream ofs("nsga2_result.csv");
    ofs << "Cg,Cc,Cn,gain,bandwidth,ripple\n";
    for(int idx : final_fronts[0]){
        double Cg = pop[idx].x0;
        double Cc = pop[idx].x1;
        double Cn = compute_Cn(Cg,Cc,Lj);
        double gain = -pop[idx].f1;
        double bw   = -pop[idx].f2;
        std::cout << Cg << "\t" << Cc << "\t" << Cn
                  << "\t" << gain  // gain
                  << "\t" << bw    // bandwidth
                  << "\t" << pop[idx].ripple
                  << "\n";
        ofs << Cg << ','
            << Cc << ','
            << Cn << ','
            << gain << ','
            << bw << ','
            << pop[idx].ripple << '\n';
    }
    ofs.close();
}
