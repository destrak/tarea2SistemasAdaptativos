// IterativeLocalSearch.cpp
// Iterated Local Search (ILS) para Maximum Independent Set (MISP)
// Criterio de detención: TIEMPO (segundos).
// Any-time behavior: imprime cada mejora (BEST ...) y el mejor final (FINAL_BEST ...).
//
// Formato de entrada (como tu ejemplo):
//   - Primera línea: n
//   - Luego: pares "u v" (0-based), aristas no dirigidas, hasta EOF.
//   - Se permite más de un par por línea.
//   - También acepta -i - para leer desde STDIN.
//
// Compilar (Linux):
//   g++ -O3 -std=c++20 -Wall -Wextra -o IterativeLocalSearch IterativeLocalSearch.cpp
//
// Ejecutar:
//   ./IterativeLocalSearch ILS -i instancia.graph -t 10 --seed 1 --alpha 0.50 --perturb 3 --ls 4000 --verbose 1

#include <chrono>
#include <cstdint>
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <numeric>
#include <random>
#include <string>
#include <vector>
#include <sstream>

using namespace std;

// -------------------- Timer --------------------
struct Timer {
    using Clock = std::chrono::steady_clock;
    Clock::time_point t0;
    Timer() : t0(Clock::now()) {}
    void reset() { t0 = Clock::now(); }
    double elapsed() const {
        using namespace std::chrono;
        return duration<double>(Clock::now() - t0).count();
    }
};

// -------------------- Grafo (n; luego pares u v 0-based) --------------------
struct Graph {
    int n = 0;
    long long m = 0;
    vector<vector<int>> adj; // 0-indexado

    // Lee el formato: primera línea N, luego pares u v (0-based) hasta EOF. Admite más de un par por línea.
    bool load_n_then_pairs_zerobased(istream& in) {
        adj.clear(); n=0; m=0;

        // leer N (saltando comentarios tipo c/%/# y vacías)
        auto is_comment_or_empty = [](const string& s){
            for (char c: s) {
                if (!isspace((unsigned char)c)) {
                    return (c=='c' || c=='%' || c=='#'); // comentarios comunes
                }
            }
            return true;
        };
        auto trim = [](string& s){
            size_t a = 0, b = s.size();
            while (a<b && isspace((unsigned char)s[a])) ++a;
            while (b>a && isspace((unsigned char)s[b-1])) --b;
            s.assign(s.begin()+a, s.begin()+b);
        };
        auto split = [](const string& s){
            vector<string> t; string cur;
            for (char c: s) {
                if (isspace((unsigned char)c)) {
                    if (!cur.empty()) { t.push_back(cur); cur.clear(); }
                } else cur.push_back(c);
            }
            if (!cur.empty()) t.push_back(cur);
            return t;
        };

        string line;
        int N = 0;
        while (getline(in, line)) {
            trim(line);
            if (line.empty() || is_comment_or_empty(line)) continue;
            try { N = stoi(line); } catch (...) { return false; }
            break;
        }
        if (N <= 0) return false;

        adj.assign(N, {});

        // leer pares hasta EOF
        while (getline(in, line)) {
            trim(line);
            if (line.empty() || is_comment_or_empty(line)) continue;
            auto tok = split(line);
            for (size_t i=0; i+1<tok.size(); i+=2) {
                int u, v;
                try { u = stoi(tok[i]); v = stoi(tok[i+1]); }
                catch (...) { continue; }
                if (u>=0 && v>=0 && u<N && v<N && u!=v) {
                    adj[u].push_back(v);
                    adj[v].push_back(u);
                }
            }
        }
        // normalizar y contar m
        long long twice = 0;
        for (int i=0;i<N;++i) {
            auto &L = adj[i];
            sort(L.begin(), L.end());
            L.erase(unique(L.begin(), L.end()), L.end());
            twice += (long long)L.size();
        }
        n = N; m = twice / 2;
        return true;
    }

    bool load(const string& path) {
        if (path == "-") {
            return load_n_then_pairs_zerobased(cin);
        } else {
            ifstream in(path);
            if (!in) return false;
            return load_n_then_pairs_zerobased(in);
        }
    }

    bool is_independent(const vector<int>& S) const {
        vector<char> mark(n, 0);
        for (int u : S) {
            if (mark[u]) return false;
            for (int v : adj[u]) mark[v] = 1;
        }
        return true;
    }

    bool are_adjacent(int u, int v) const {
        const auto& L = adj[u];
        return binary_search(L.begin(), L.end(), v);
    }
};

// -------------------- CLI Options --------------------
struct Options {
    bool ok = false;
    string error;

    string meta = "ILS";
    string instance_path;
    double time_limit = 10.0;

    uint64_t seed = 123456789ULL;
    double alpha = 0.30;   // 0..1, tamaño RCL ~ alpha * |cand|
    int perturb_k = 3;     // fuerza de perturbación
    int ls_iters = 2000;   // tope iteraciones en búsqueda local
    int verbose = 0;
};

static void print_usage() {
    cerr <<
      "Uso:\n"
      "  misp_ils <Metaheuristica> -i <instancia|-> -t <tiempoSegundos>\n"
      "            [--seed S] [--alpha A] [--perturb K] [--ls I] [--verbose 0/1]\n"
      "\n"
      "  Formato de instancia: primera línea N; luego pares 'u v' 0-based hasta EOF.\n"
      "  Use -i - para leer desde STDIN (ej: unzip -p ... | ./misp_ils -i -).\n"
      "\n"
      "Metaheurísticas: ILS\n";
}

Options parse_args(int argc, char** argv) {
    Options o;
    if (argc < 2) { o.error = "Faltan argumentos."; return o; }
    o.meta = argv[1];

    auto need = [&](int i){ return i+1 < argc; };
    for (int i = 2; i < argc; ++i) {
        string a = argv[i];
        if (a == "-i" && need(i))            o.instance_path = argv[++i];
        else if (a == "-t" && need(i))       o.time_limit = stod(argv[++i]);
        else if (a == "--seed" && need(i))   o.seed = stoull(argv[++i]);
        else if (a == "--alpha" && need(i))  o.alpha = stod(argv[++i]);
        else if (a == "--perturb" && need(i))o.perturb_k = stoi(argv[++i]);
        else if (a == "--ls" && need(i))     o.ls_iters = stoi(argv[++i]);
        else if (a == "--verbose" && need(i))o.verbose = stoi(argv[++i]);
        else {
            // ignorar desconocidos (permite agregar params propios)
        }
    }

    if (o.instance_path.empty()) { o.error = "Falta -i <instancia|->"; return o; }
    if (o.time_limit <= 0)       { o.error = "Tiempo -t debe ser > 0"; return o; }
    if (o.alpha < 0.0) o.alpha = 0.0;
    if (o.alpha > 1.0) o.alpha = 1.0;

    o.ok = true;
    return o;
}

// -------------------- ILS para MISP --------------------
struct ILS_MIS {
    const Graph& G;
    std::mt19937_64& rng;
    double alpha;   // controla el tamaño de RCL
    int perturb_k;  // fuerza perturbación
    int ls_iters;   // tope de LS
    int verbose;

    // any-time
    int best_val = -1;
    double best_time = 0.0;
    vector<int> best_set;

    ILS_MIS(const Graph& g, std::mt19937_64& r, double a, int pk, int lsi, int v)
        : G(g), rng(r), alpha(a), perturb_k(pk), ls_iters(lsi), verbose(v) {}

    // Construcción Greedy aleatoria con RCL por grado (ascendente).
    vector<int> construct() {
        vector<int> order(G.n);
        iota(order.begin(), order.end(), 0);
        sort(order.begin(), order.end(),
             [&](int a, int b){ return G.adj[a].size() < G.adj[b].size(); });

        vector<char> forbidden(G.n, 0);
        vector<int> S; S.reserve(G.n);

        while (true) {
            vector<int> cand;
            cand.reserve(G.n);
            for (int u : order) if (!forbidden[u]) cand.push_back(u);
            if (cand.empty()) break;

            int rcl = max(1, (int)ceil(alpha * (double)cand.size()));
            rcl = min(rcl, (int)cand.size());
            uniform_int_distribution<int> pick(0, rcl - 1);
            int u = cand[pick(rng)];

            S.push_back(u);
            forbidden[u] = 1;
            for (int v : G.adj[u]) forbidden[v] = 1;
        }
        return S;
    }

    // Búsqueda local: 1-add repetidamente y luego intentos 2-por-1.
    void local_search(vector<int>& S) {
        vector<char> inS(G.n, 0);
        for (int u : S) inS[u] = 1;

        // conflicts[u] = vecinos en S
        vector<int> conflicts(G.n, 0);
        for (int u = 0; u < G.n; ++u) {
            int c = 0;
            for (int v : G.adj[u]) if (inS[v]) ++c;
            conflicts[u] = c;
        }

        auto can_add = [&](int u){ return !inS[u] && conflicts[u] == 0; };
        auto add = [&](int u){
            inS[u] = 1; S.push_back(u);
            for (int v : G.adj[u]) conflicts[v]++;
        };
        auto remove = [&](int u){
            inS[u] = 0;
            for (int v : G.adj[u]) conflicts[v]--;
        };

        bool improved = true;
        int it = 0;
        // Fase 1: 1-add
        while (improved && it < ls_iters) {
            improved = false; ++it;
            for (int u = 0; u < G.n; ++u) {
                if (can_add(u)) {
                    add(u);
                    improved = true;
                    break;
                }
            }
        }

        // Fase 2: 2-por-1 swaps
        improved = true;
        while (improved && it < ls_iters) {
            improved = false; ++it;

            int u_choice = -1, v_choice = -1, x_choice = -1;
            for (int u = 0; u < G.n; ++u) {
                if (inS[u] || conflicts[u] != 1) continue;
                int xu = -1;
                for (int v : G.adj[u]) if (inS[v]) { xu = v; break; }
                if (xu < 0) continue;

                for (int v = u + 1; v < G.n; ++v) {
                    if (inS[v] || conflicts[v] != 1) continue;
                    if (G.are_adjacent(u, v)) continue;
                    int xv = -1;
                    for (int w : G.adj[v]) if (inS[w]) { xv = w; break; }
                    if (xv < 0) continue;

                    if (xu == xv) { // mismo conflictivo en S
                        u_choice = u; v_choice = v; x_choice = xu;
                        break;
                    }
                }
                if (u_choice != -1) break;
            }

            if (u_choice != -1) {
                remove(x_choice);
                S.erase(find(S.begin(), S.end(), x_choice));
                add(u_choice);
                add(v_choice);
                improved = true;
            }
        }

        // compactar S
        vector<int> finalS; finalS.reserve(S.size());
        for (int u = 0; u < G.n; ++u) if (inS[u]) finalS.push_back(u);
        S.swap(finalS);
    }

    // Perturbación: remover k al azar; reparación greedy (grado asc).
    void perturb_and_repair(vector<int>& S) {
        if (S.empty()) return;
        shuffle(S.begin(), S.end(), rng);
        int k = min<int>(perturb_k, (int)S.size());
        S.resize((int)S.size() - k);

        vector<char> inS(G.n, 0);
        for (int u : S) inS[u] = 1;

        vector<int> order(G.n);
        iota(order.begin(), order.end(), 0);
        sort(order.begin(), order.end(),
             [&](int a, int b){ return G.adj[a].size() < G.adj[b].size(); });

        for (int u : order) {
            if (inS[u]) continue;
            bool ok = true;
            for (int v : G.adj[u]) if (inS[v]) { ok = false; break; }
            if (ok) { inS[u] = 1; S.push_back(u); }
        }
    }

    // Bucle principal ILS con any-time y tope de tiempo
    tuple<vector<int>, int, double> run(Timer& tim, double time_limit_sec) {
        best_val = -1; best_time = 0.0; best_set.clear();

        // Construcción inicial + LS
        vector<int> S = construct();
        if (!G.is_independent(S)) S.clear();
        local_search(S);

        // Reporte inicial
        best_set = S; best_val = (int)S.size(); best_time = tim.elapsed();
        cout << "BEST " << best_val << " TIME " << fixed << setprecision(6) << best_time << "\n";

        uniform_real_distribution<double> U01(0.0, 1.0);
        const double t_end = tim.elapsed() + time_limit_sec;

        while (tim.elapsed() < t_end) {
            vector<int> S2 = S;
            perturb_and_repair(S2);
            local_search(S2);

            if ((int)S2.size() > (int)S.size() || (((int)S2.size() == (int)S.size()) && U01(rng) < 0.05)) {
                S.swap(S2);
            }

            if ((int)S.size() > best_val) {
                best_val = (int)S.size();
                best_time = tim.elapsed();
                best_set = S;
                cout << "BEST " << best_val << " TIME " << fixed << setprecision(6) << best_time << "\n";
            }
        }
        return {best_set, best_val, best_time};
    }
};

// -------------------- main --------------------
int main(int argc, char** argv) {
    Options opt = parse_args(argc, argv);
    if (!opt.ok) {
        print_usage();
        cerr << "Error: " << opt.error << "\n";
        return 1;
    }
    if (opt.meta != "ILS") {
        cerr << "Unsupported metaheuristic: " << opt.meta << " (only ILS supported)\n";
        return 1;
    }

    Graph G;
    if (!G.load(opt.instance_path)) {
        cerr << "Error leyendo instancia: " << opt.instance_path << "\n";
        return 1;
    }
    if (opt.verbose) {
        cerr << "# Vertices: " << G.n << "  Edges: " << G.m << "\n";
    }

    std::mt19937_64 rng(opt.seed);
    Timer timer;
    timer.reset();

    ILS_MIS solver(G, rng, opt.alpha, opt.perturb_k, opt.ls_iters, opt.verbose);
    auto [best_set, best_val, best_t] = solver.run(timer, opt.time_limit);

    // Línea final obligatoria
    cout << "FINAL_BEST " << best_val << " FOUND_AT " << fixed << setprecision(6) << best_t << "\n";
    return 0;
}
