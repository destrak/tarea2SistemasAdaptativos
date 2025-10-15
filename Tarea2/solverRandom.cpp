#include <bits/stdc++.h>
#include <iostream>
#include <random>
using namespace std;
using namespace chrono;

// =====================
// Estructura de Grafo
// =====================
struct Graph {
    int n;
    vector<vector<int>> adj;
};

// =====================
// Lectura del grafo
// =====================
Graph readGraph(const string &filename) {
    ifstream in(filename);
    if (!in.is_open()) {
        cerr << "No se pudo abrir archivo " << filename << endl;
        exit(1);
    }
    int n;
    in >> n;
    Graph G;
    G.n = n;
    G.adj.assign(n, {});
    int u, v;
    while (in >> u >> v) {
        G.adj[u].push_back(v);
        G.adj[v].push_back(u);
    }
    return G;
}

// =====================
// Greedy Aleatorizado
// =====================
int greedy_randomized(const Graph &G, double crit, double k) {
    vector<int> degree(G.n);
    for (int i = 0; i < G.n; i++) degree[i] = G.adj[i].size();
    vector<bool> removed(G.n, false);

    priority_queue<pair<int,int>, vector<pair<int,int>>, greater<>> pq;
    for (int i = 0; i < G.n; i++) pq.push({degree[i], i});

    int count = 0;
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> dis(0.0, 1.0);

    while (!pq.empty()) {
        vector<pair<int,int>> candidates;
        
        int size_dom = max(1, (int)round(k * (int)pq.size()));

        
        while (!pq.empty() && (int)candidates.size() < size_dom) {
            auto [d,u] = pq.top(); pq.pop();
            if (removed[u] || degree[u] != d) continue;
            candidates.push_back({d,u});
        }

        if (candidates.empty()) break;

        // Selección según crit
        double delta = dis(gen);
        int chosen_idx = 0;
        if (delta >= crit && (int)candidates.size() > 1) {
            uniform_int_distribution<> pick(0, (int)candidates.size()-1);
            chosen_idx = pick(gen);
        }

        int u = candidates[chosen_idx].second;

        // Los demás candidatos vuelven al heap si siguen válidos
        for (int i = 0; i < (int)candidates.size(); i++) {
            if (i != chosen_idx) {
                int v = candidates[i].second;
                if (!removed[v]) pq.push(candidates[i]);
            }
        }

        // Selección del nodo
        if (removed[u]) continue;
        count++;
        removed[u] = true;
        for (int v : G.adj[u]) {
            if (!removed[v]) {
                removed[v] = true;
                for (int w : G.adj[v]) {
                    if (!removed[w]) {
                        degree[w]--;
                        pq.push({degree[w], w});
                    }
                }
            }
        }
    }
    return count;
}

// =====================
// MAIN
// =====================
int main(int argc, char* argv[]) {
    if (argc < 5 || string(argv[1]) != "-i") {
        cerr << "Uso: ./GreedyRandomizado -i <instancia> <crit> <k_porcentaje>" << endl;
        return 1;
    }

    string filename = argv[2];
    double crit = stod(argv[3]);
    double k = stod(argv[4]);

    Graph G = readGraph(filename);

    auto start = high_resolution_clock::now();
    int sol = greedy_randomized(G, crit, k);
    auto end = high_resolution_clock::now();
    double t = duration<double>(end - start).count();

    cout << "Solucion: " << sol << endl;
    cout << "Tiempo: " << t << " segundos" << endl;

    return 0;
}
