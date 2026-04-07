#include <bits/stdc++.h>
#include <filesystem>
#include <chrono>
#include <functional>
#include <cstdlib>
#include <new>

long long memoria_actual = 0;
long long memoria_pico = 0;

#if defined(__unix__) || defined(__APPLE__)
  #include <sys/resource.h>
  #include <unistd.h>
#endif

using namespace std;
namespace fs = std::filesystem;

static const string INPUT_DIR       = "data/array_input";
static const string OUTPUT_DIR      = "data/array_output";
static const string MEASUREMENT_DIR = "data/measurements";

void mergeSort(std::vector<int>& arr, int left, int right);
void quickSort(std::vector<int>& arr, int low, int high);
std::vector<int> sortArray(std::vector<int>& arr);

static void run_mergesort(vector<int>& v){ if(!v.empty()) mergeSort(v, 0, (int)v.size()-1); }
static void run_quicksort(vector<int>& v){ if(!v.empty()) quickSort(v, 0, (int)v.size()-1); }
static void run_stdsort  (vector<int>& v){ auto out = sortArray(v); v.swap(out); }

// Versiones new (con std::size_t)
void* operator new(std::size_t size) {
    memoria_actual += size;
    if (memoria_actual > memoria_pico) memoria_pico = memoria_actual;
    return std::malloc(size);
}

void* operator new[](std::size_t size) {
    memoria_actual += size;
    if (memoria_actual > memoria_pico) memoria_pico = memoria_actual;
    return std::malloc(size);
}

// Versiones delete SIN tamaño
void operator delete(void* p) noexcept {
    std::free(p);
}

void operator delete[](void* p) noexcept {
    std::free(p);
}

// Versiones delete CON tamaño (Sized deallocation - C++14 en adelante)
void operator delete(void* p, std::size_t size) noexcept {
    memoria_actual -= size;
    std::free(p);
}

void operator delete[](void* p, std::size_t size) noexcept {
    memoria_actual -= size;
    std::free(p);
}

// ===== IO =====
static bool readArray(const string& path, vector<int>& out){
    ifstream in(path);
    if(!in) return false;
    out.clear();
    long long x;
    while(in >> x) out.push_back((int)x);
    return true;
}

static void writeArray(const string& path, const vector<int>& v){
    fs::create_directories(fs::path(path).parent_path());
    ofstream out(path);
    for(size_t i=0;i<v.size();++i){ if(i) out << ' '; out << v[i]; }
    out << '\n';
}

static string deriveOutputPath(const string& in, const string& algoritmo){
    string name = fs::path(in).filename().string();
    const string suf = ".txt";
    if(name.size() >= suf.size() && name.compare(name.size()-suf.size(), suf.size(), suf) == 0){
        name.replace(name.size()-suf.size(), suf.size(), "_" + algoritmo + ".out.txt");
    }
    else{
        name += "_" + algoritmo + ".out.txt";
    }
    return (OUTPUT_DIR + "/" + name);
}

static void CreacionMeasurements(const string& algoritmo, const string& array_nombre, size_t n, double time_ms, long long mem_est_bytes){
    fs::create_directories(MEASUREMENT_DIR);
    const string csv = MEASUREMENT_DIR + "/measurements_" + algoritmo + ".csv";
    const bool new_file = !fs::exists(csv);
    ofstream out(csv, ios::app);
    if(!out) return;
    if(new_file)
        out << "algoritmo,array_nombre,n,time_ms,mem_est_bytes\n";
    out << algoritmo << ',' << array_nombre << ',' << n << ',' << fixed << setprecision(6) << time_ms << ',' << mem_est_bytes << '\n';
}

// ===== Mapa nombre->función =====
using SortFn = function<void(vector<int>&)>;
static unordered_map<string, SortFn> build_algo_map(){
    return {
        {"mergesort",run_mergesort}, {"quicksort",run_quicksort}, {"stdsort",run_stdsort}, {"sort",run_stdsort}
    };
}

static bool process_one(const string& algo_name, SortFn run_func, const string& path){
    vector<int> arr;
    if (!readArray(path, arr)) return false;

    const size_t MAX_N_GLOBAL = 10'000'000;
    if (arr.size() >= MAX_N_GLOBAL) return true;

    // Hacemos la copia del arreglo ANTES de tomar la lectura base
    auto arr_copy = arr;

    // tomamos la lectura de memoria base y reiniciamos el pico
    long long memoria_base = memoria_actual;
    memoria_pico = memoria_actual;

    // medimos tiempo y ejecutamos
    auto t0 = chrono::steady_clock::now();
    run_func(arr_copy);
    auto t1 = chrono::steady_clock::now();
    double ms = chrono::duration<double, std::milli>(t1 - t0).count();

    // escribimos el archivo de salida
    writeArray(deriveOutputPath(path, algo_name), arr_copy);

    // calculamos la memoria extra exacta usada por el algoritmo
    long long mem_bytes = memoria_pico - memoria_base;

    CreacionMeasurements(algo_name, fs::path(path).filename().string(), arr_copy.size(), ms, mem_bytes);
    return true;
}

int main(int argc, char** argv){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    auto algo_map = build_algo_map();

    string only_algo, only_file;
    for (int i=1; i<argc; ++i){
        string a = argv[i];
        if (a == "--only" && i+1 < argc)      only_algo = argv[++i];
        else if (a == "--file" && i+1 < argc) only_file = argv[++i];
    }

    if (!only_algo.empty() && !only_file.empty()){
        auto it = algo_map.find(only_algo);
        if (it == algo_map.end()) return 2;
        fs::create_directories(OUTPUT_DIR);
        fs::create_directories(MEASUREMENT_DIR);
        bool ok = process_one(only_algo, it->second, only_file);
        return ok ? 0 : 2;
    }

    if(!fs::exists(INPUT_DIR)) return 1;
    fs::create_directories(OUTPUT_DIR);
    fs::create_directories(MEASUREMENT_DIR);

    vector<string> files;
    for (auto& p : fs::directory_iterator(INPUT_DIR)){
        if (p.is_regular_file() && p.path().extension()==".txt"){
            files.push_back(p.path().string());
        }
    }
    sort(files.begin(), files.end());

    const vector<pair<string, SortFn>> algoritmos = {
        {"mergesort", algo_map.at("mergesort")},
        {"quicksort", algo_map.at("quicksort")},
        {"stdsort",   algo_map.at("stdsort")},
    };

    const size_t MAX_N_GLOBAL = 10'000'000;

    for (const auto& [algo_name, run_func] : algoritmos){
        size_t ok=0, fail=0, skips=0;

        for (const string& path : files){
            vector<int> arr;
            if (!readArray(path, arr)){ ++fail; continue; }
            if (arr.size() >= MAX_N_GLOBAL){ ++skips; continue; }

            // Hacemos la copia del arreglo ANTES de tomar la lectura base
            auto arr_copy = arr;

            // 1. Tomamos la lectura de memoria base y reiniciamos el pico
            long long memoria_base = memoria_actual;
            memoria_pico = memoria_actual;

            // 2. Medimos tiempo y ejecutamos
            auto t0 = chrono::steady_clock::now();
            run_func(arr_copy);
            auto t1 = chrono::steady_clock::now();
            double ms = chrono::duration<double, std::milli>(t1 - t0).count();

            // 3. Escribimos el archivo de salida
            writeArray(deriveOutputPath(path, algo_name), arr_copy);

            // 4. Calculamos la memoria extra exacta usada por el algoritmo
            long long mem_bytes = memoria_pico - memoria_base;

            // 5. Guardamos en el CSV
            CreacionMeasurements(algo_name, fs::path(path).filename().string(), arr_copy.size(), ms, mem_bytes);
            ++ok;
        }

        cout << "[DONE] " << algo_name
             << " | archivos=" << files.size()
             << " | procesados=" << ok
             << " | skips=" << skips
             << " | fallos=" << fail << "\n";
    }

    return 0;
}