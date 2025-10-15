#include <bits/stdc++.h>
using namespace std;

struct Stats {
    int count = 0;
    double sum_best = 0.0, sum_best2 = 0.0;
    double sum_time = 0.0, sum_time2 = 0.0;
};

int main(int argc, char* argv[]) {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    // Archivos de entrada y salida
    string input_file = "resultados_globalesRandom3000.csv";
    string output_file = "resumen_por_densidad_Aleatorizado_3000.csv";

    if (argc > 1) input_file = argv[1];
    if (argc > 2) output_file = argv[2];

    ifstream fin(input_file);
    if (!fin.is_open()) {
        cerr << " No se pudo abrir el archivo: " << input_file << "\n";
        return 1;
    }

    unordered_map<string, Stats> data;
    string line;
    getline(fin, line); // Saltar encabezado

    while (getline(fin, line)) {
        if (line.empty()) continue;

        stringstream ss(line);
        string densidad, idx, best_str, time_str, archivo;

        getline(ss, densidad, ',');
        getline(ss, idx, ',');
        getline(ss, best_str, ',');
        getline(ss, time_str, ',');
        getline(ss, archivo, ',');

        if (best_str == "NA" || time_str == "NA") continue;

        double best = stod(best_str);
        double t = stod(time_str);
        Stats &st = data[densidad];
        st.count++;
        st.sum_best += best;
        st.sum_best2 += best * best;
        st.sum_time += t;
        st.sum_time2 += t * t;
    }
    fin.close();

    // Escribir salida
    ofstream fout(output_file);
    if (!fout.is_open()) {
        cerr << " No se pudo crear archivo de salida: " << output_file << "\n";
        return 1;
    }

    fout << "densidad,count,mean_best,std_best,mean_time,std_time\n";

    vector<string> keys;
    for (auto &kv : data) keys.push_back(kv.first);
    sort(keys.begin(), keys.end());

    for (auto &d : keys) {
        const Stats &st = data[d];
        if (st.count == 0) continue;

        double mean_best = st.sum_best / st.count;
        double mean_time = st.sum_time / st.count;

        double var_best = (st.count > 1)
            ? (st.sum_best2 - st.count * mean_best * mean_best) / (st.count - 1)
            : 0.0;
        double var_time = (st.count > 1)
            ? (st.sum_time2 - st.count * mean_time * mean_time) / (st.count - 1)
            : 0.0;

        double std_best = sqrt(max(0.0, var_best));
        double std_time = sqrt(max(0.0, var_time));

        fout << fixed << setprecision(6)
             << d << "," << st.count << ","
             << mean_best << "," << std_best << ","
             << mean_time << "," << std_time << "\n";
    }

    fout.close();
    cout << " Archivo generado: " << output_file << "\n";
    return 0;
}
