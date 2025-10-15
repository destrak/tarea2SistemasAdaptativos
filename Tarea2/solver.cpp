#include <bits/stdc++.h>
#include <iostream>
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
// Greedy determinista
// =====================
int greedy(const Graph &G) {
    vector<int> degree(G.n);
    for (int i = 0; i < G.n; i++) degree[i] = G.adj[i].size();
    vector<bool> removed(G.n, false);
    priority_queue<pair<int,int>, vector<pair<int,int>>, greater<>> pq;
    for (int i = 0; i < G.n; i++) pq.push({degree[i], i});
    int count = 0;
    while (!pq.empty()) {
        auto [d,u] = pq.top(); pq.pop();
        if (removed[u] || degree[u] != d) continue;
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
    if (argc < 3 || string(argv[1]) != "-i") {
        cerr << "Uso: ./greed -i <instancia>" << endl;
        return 1;
    }

    string filename = argv[2];
    Graph G = readGraph(filename);

    auto start = high_resolution_clock::now();
    int sol = greedy(G);
    auto end = high_resolution_clock::now();
    double t = duration<double>(end - start).count();

    cout << "Solucion: " << sol << endl;
    cout << "Tiempo: " << t << " segundos" << endl;

    return 0;
}
