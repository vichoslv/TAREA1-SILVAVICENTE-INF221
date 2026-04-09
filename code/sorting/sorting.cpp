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

/*****
* static void run_mergesort
******
* Llama a la función principal mergeSort, estableciendo los límites correctos desde el inicio hasta el final del vector.
******
* Input :
* vector<int>& v : Referencia al vector de enteros que se desea ordenar.
******
* Returns :
* void, no retorna ningún valor.
*****/
static void run_mergesort(vector<int>& v){ if(!v.empty()) mergeSort(v, 0, (int)v.size()-1); }
/*****
* static void run_quicksort
******
* Llama a la función principal quickSort, estableciendo los límites correctos desde el inicio hasta el final del vector.
******
* Input :
* vector<int>& v : Referencia al vector de enteros que se desea ordenar.
******
* Returns :
* void, no retorna ningún valor.
*****/
static void run_quicksort(vector<int>& v){ if(!v.empty()) quickSort(v, 0, (int)v.size()-1); }
/*****
* static void run_stdsort
******
* Ejecuta la función sortArray (que encapsula el ordenamiento) y reemplaza el contenido del vector original con el resultado ordenado.
******
* Input :
* vector<int>& v : Referencia al vector de enteros que se desea ordenar.
******
* Returns :
* void, no retorna ningún valor.
*****/
static void run_stdsort  (vector<int>& v){ auto out = sortArray(v); v.swap(out); }

/*****
* void* operator new
******
* Sobrecarga global del operador new. Reserva memoria dinámica y lleva un registro de la memoria total actual, actualizando el pico máximo histórico de consumo.
******
* Input :
* std::size_t size : Cantidad de memoria en bytes que se solicita reservar.
******
* Returns :
* void*, retorna un puntero al inicio del bloque de memoria asignado.
*****/
void* operator new(std::size_t size) {
    memoria_actual += size;
    if (memoria_actual > memoria_pico) memoria_pico = memoria_actual;
    return std::malloc(size);
}
/*****
* void* operator new[]
******
* Sobrecarga global del operador new para arreglos. Reserva memoria y actualiza los contadores globales para medir el pico máximo de memoria.
******
* Input :
* std::size_t size : Cantidad de memoria en bytes que se solicita reservar para el arreglo.
******
* Returns :
* void*, retorna un puntero al inicio del bloque de memoria del arreglo.
*****/
void* operator new[](std::size_t size) {
    memoria_actual += size;
    if (memoria_actual > memoria_pico) memoria_pico = memoria_actual;
    return std::malloc(size);
}

/*****
* void operator delete
******
* Sobrecarga global del operador delete sin tamaño. Libera la memoria previamente asignada. No ajusta la variable memoria_actual.
******
* Input :
* void* p : Puntero al bloque de memoria que se desea liberar.
******
* Returns :
* void, no retorna ningún valor.
*****/
void operator delete(void* p) noexcept {
    std::free(p);
}
/*****
* void operator delete[]
******
* Sobrecarga global del operador delete para arreglos sin tamaño. Libera el bloque de memoria previamente asignado a un arreglo.
******
* Input :
* void* p : Puntero al arreglo de memoria que se desea liberar.
******
* Returns :
* void, no retorna ningún valor.
*****/
void operator delete[](void* p) noexcept {
    std::free(p);
}
/*****
* void operator delete (con tamaño)
******
* Sobrecarga global del operador delete que recibe el tamaño (C++14). Resta la memoria liberada del contador memoria_actual y luego libera el bloque.
******
* Input :
* void* p : Puntero al bloque de memoria a liberar.
* std::size_t size : Tamaño en bytes del bloque que se está liberando.
******
* Returns :
* void, no retorna ningún valor.
*****/
void operator delete(void* p, std::size_t size) noexcept {
    memoria_actual -= size;
    std::free(p);
}
/*****
* void operator delete[] (con tamaño)
******
* Sobrecarga global del operador delete para arreglos que recibe el tamaño (C++14). Actualiza el contador de uso actual restando los bytes y luego libera la memoria.
******
* Input :
* void* p : Puntero al arreglo en memoria a liberar.
* std::size_t size : Tamaño en bytes del arreglo que se está liberando.
******
* Returns :
* void, no retorna ningún valor.
*****/
void operator delete[](void* p, std::size_t size) noexcept {
    memoria_actual -= size;
    std::free(p);
}

/*****
* static bool readArray
******
* Lee un archivo de texto con enteros y los carga secuencialmente en el vector proporcionado. Limpia el vector de salida antes de la lectura.
******
* Input :
* const string& path : Ruta al archivo de texto de entrada.
* vector<int>& out : Vector de destino donde se guardarán los números.
******
* Returns :
* bool, retorna true si el archivo se abrió y leyó correctamente, false en caso contrario.
*****/
static bool readArray(const string& path, vector<int>& out){
    ifstream in(path);
    if(!in) return false;
    out.clear();
    long long x;
    while(in >> x) out.push_back((int)x);
    return true;
}
/*****
* static void writeArray
******
* Escribe los elementos de un vector de enteros en un archivo de texto, separados por espacios. Genera los directorios de destino si no existen.
******
* Input :
* const string& path : Ruta del archivo de destino.
* const vector<int>& v : Vector cuyos elementos serán escritos en el archivo.
******
* Returns :
* void, no retorna ningún valor.
*****/
static void writeArray(const string& path, const vector<int>& v){
    fs::create_directories(fs::path(path).parent_path());
    ofstream out(path);
    for(size_t i=0;i<v.size();++i){ if(i) out << ' '; out << v[i]; }
    out << '\n';
}
/*****
* static string deriveOutputPath
******
* A partir del nombre de un archivo de entrada y el algoritmo utilizado, genera la ruta completa del archivo de salida correspondiente.
******
* Input :
* const string& in : Ruta del archivo de entrada original.
* const string& algoritmo : Nombre del algoritmo ejecutado.
******
* Returns :
* string, retorna la ruta completa del archivo formateado para la salida.
*****/
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
/*****
* static void CreacionMeasurements
******
* Registra las métricas de la ejecución agregando una fila a un archivo CSV. Si el archivo no existe, lo crea e inserta los encabezados.
******
* Input :
* const string& algoritmo : Nombre del algoritmo evaluado.
* const string& array_nombre : Nombre del archivo procesado.
* size_t n : Cantidad de elementos del arreglo.
* double time_ms : Tiempo de ejecución medido en milisegundos.
* long long mem_est_bytes : Memoria extra pico registrada en bytes.
******
* Returns :
* void, no retorna ningún valor.
*****/
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

using SortFn = function<void(vector<int>&)>;
/*****
* static unordered_map<string, SortFn> build_algo_map
******
* Construye y retorna un diccionario que asocia el identificador en texto de cada algoritmo con su función lambda o envoltorio correspondiente.
******
* Input :
* No recibe parámetros.
******
* Returns :
* unordered_map<string, SortFn>, retorna el mapa de funciones de ordenamiento.
*****/
static unordered_map<string, SortFn> build_algo_map(){
    return {
        {"mergesort",run_mergesort}, {"quicksort",run_quicksort}, {"stdsort",run_stdsort}, {"sort",run_stdsort}
    };
}
/*****
* static bool process_one
******
* Se encarga del flujo completo para un solo archivo: lo carga en memoria, clona los datos, registra el tiempo y el pico de memoria dinámico, guarda el resultado y exporta métricas al CSV.
******
* Input :
* const string& algo_name : Nombre del algoritmo a ejecutar.
* SortFn run_func : Puntero/función envoltorio del algoritmo.
* const string& path : Ruta del archivo a procesar.
******
* Returns :
* bool, retorna true si el archivo se procesó con éxito o fue ignorado por tamaño excesivo, false si no se pudo leer.
*****/
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
/*****
* int main
******
* Punto de entrada del programa. Analiza los argumentos de línea de comandos para correr una prueba individual o iterar automáticamente sobre todos los archivos del directorio de entrada.
******
* Input :
* int argc : Cantidad de argumentos pasados por consola.
* char** argv : Arreglo de cadenas de texto con los argumentos.
******
* Returns :
* int, retorna 0 si la ejecución finaliza con éxito, 1 si no encuentra el directorio de origen, o 2 si hay errores de parámetros o lectura.
*****/
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