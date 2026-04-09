// matrix_multiplication.cpp
#include <bits/stdc++.h>
#include <filesystem>
#include <chrono>
using namespace std;
namespace fs = std::filesystem;

vector<vector<int>> multiply_naive(vector<vector<int>>& A, vector<vector<int>>& B);
vector<vector<int>> multiply_strassen(vector<vector<int>>& A, vector<vector<int>>& B);


#ifndef ALGO_NAME
#define ALGO_NAME "algo"
#endif

static const string INPUT_DIR  = "data/matrix_input";
static const string OUTPUT_DIR = "data/matrix_output";
static const string MEASUREMENT_DIR   = "data/measurements";
/*****
* static bool LecturaMatriz
******
* Lee una matriz desde un archivo de texto y la almacena en un vector de vectores de enteros. Valida que todas las filas tengan la misma cantidad de columnas.
******
* Input :
* const string& path : Ruta al archivo de texto de entrada.
* vector<vector<int>>& M : Referencia a la matriz bidimensional donde se guardarán los datos.
******
* Returns :
* bool, retorna true si el archivo se leyó correctamente y la matriz no está vacía, false en caso de error o inconsistencia en las columnas.
*****/
static bool LecturaMatriz(const string& path, vector<vector<int>>& M){
    ifstream in(path);
    if (!in) {
        return false; 
    }

    M.clear();
    string line;
    size_t columnas = 0;
    while(getline(in, line)){
        if(line.empty()){
            continue;
        }

        istringstream iss(line);
        vector<int> fila;
        int x;
        while(iss >> x){
            fila.push_back(x);
        }

        if(!fila.empty()){
            if(columnas == 0){
                columnas = fila.size();
            }
            
            if(fila.size() != columnas){
                return false;
            }
            
            M.push_back(std::move(fila));
        }
    }
    return !M.empty();
}
/*****
* static bool writeMatrix
******
* Escribe el contenido de una matriz bidimensional en un archivo de texto, separando los elementos por espacios y cada fila con un salto de línea.
******
* Input :
* const string& path : Ruta del archivo de destino.
* const vector<vector<int>>& M : Matriz bidimensional que se desea escribir.
******
* Returns :
* bool, retorna true si la escritura fue exitosa, false si el flujo de salida falló.
*****/
static bool writeMatrix(const string& path, const vector<vector<int>>& M) {
    ofstream out(path);
    if (!out){ 
        return false;
    }

    for (const auto& row : M) {
        for (size_t j = 0; j < row.size(); ++j) {
            if (j) out << ' ';
            out << row[j];
        }
        out << '\n';
    }
    return true;
}
/*****
* static void appendMeasurement
******
* Registra las métricas de la ejecución (timestamp, nombre del algoritmo, nombre de la matriz, dimensión, tiempo y memoria estimada) añadiendo una fila a un archivo CSV. Crea el archivo con encabezados si no existe.
******
* Input :
* const string& algoritmo : Nombre del algoritmo evaluado.
* const string& matriz_nombre : Nombre del archivo de la primera matriz.
* int n : Dimensión de la matriz.
* double time_ms : Tiempo de ejecución medido en milisegundos.
* long long mem_est_bytes : Memoria estimada en bytes utilizada por el algoritmo.
******
* Returns :
* void, no retorna ningún valor.
*****/
static void appendMeasurement(const string& algoritmo, const string& matriz_nombre,
                              int n, double time_ms, long long mem_est_bytes) {
    const string csv = MEASUREMENT_DIR + "/measurements_" + algoritmo + ".csv";
    const bool new_file = !fs::exists(csv);
    ofstream out(csv, ios::app);
    if (!out) return;

    if (new_file) {
        out << "timestamp_ms,algoritmo,matriz_nombre,n,time_ms,mem_est_bytes\n";
    }

    auto now = chrono::time_point_cast<chrono::milliseconds>(chrono::system_clock::now());
    out << now.time_since_epoch().count() << ','
        << algoritmo << ','
        << matriz_nombre << ','
        << n << ','
        << std::fixed << std::setprecision(6) << time_ms << ','  // ms con decimales
        << mem_est_bytes << '\n';
}
/*****
* static string deriveOutputPath
******
* Genera la ruta completa del archivo de salida reemplazando el sufijo "_1.txt" del archivo original por una cadena que indica el algoritmo utilizado y ubicándolo en el directorio de salida.
******
* Input :
* const string& in1 : Ruta original de la primera matriz de entrada.
* const string& algoritmo : Nombre del algoritmo ejecutado.
******
* Returns :
* string, retorna la ruta completa del archivo formateado para almacenar la matriz resultante.
*****/
static string deriveOutputPath(const string& in1, const string& algoritmo) {
    fs::path p1(in1);
    string name = p1.filename().string();

    const string suf = "_1.txt";
    if (name.size() >= suf.size() && name.compare(name.size()-suf.size(), suf.size(), suf) == 0){
        name.replace(name.size()-suf.size(), suf.size(), "_" + algoritmo + ".out.txt");
    } 

    else {
        name += "_" + algoritmo + ".out.txt";
    }
    return (OUTPUT_DIR + "/" + name);
}
/*****
* static long long EstimarMemBytes
******
* Calcula una estimación teórica del consumo de memoria en bytes basándose en el tipo de algoritmo ("naive" o "strassen") y la dimensión de la matriz.
******
* Input :
* const string& algo : Nombre del algoritmo a evaluar.
* int n : Dimensión (cantidad de filas/columnas) de la matriz.
******
* Returns :
* long long, retorna la cantidad estimada de bytes requeridos.
*****/
static long long EstimarMemBytes(const string& algo, int n) {
    if (algo == "naive"){
        return 3LL * n * n * sizeof(int);
    } 
    else{
        return 8LL * n * n * sizeof(int);
    }
}
/*****
* static bool processPair
******
* Coordina el flujo completo para multiplicar dos matrices: las lee desde el disco, verifica restricciones de tamaño (omite Strassen para n >= 1024), ejecuta el algoritmo correspondiente, mide el tiempo de ejecución, guarda el resultado y registra las métricas.
******
* Input :
* const string& path1 : Ruta al archivo de la primera matriz.
* const string& path2 : Ruta al archivo de la segunda matriz.
* const string& algo : Nombre del algoritmo de multiplicación a utilizar.
******
* Returns :
* bool, retorna true al finalizar el proceso exitosamente o si se omitió deliberadamente, indicando que no hubo un error crítico de ejecución.
*****/
static bool processPair(const string& path1, const string& path2, const string& algo) {
    vector<vector<int>> A, B;

    LecturaMatriz(path1, A);
    LecturaMatriz(path2, B);

    int n = (int)A.size();

    // Evitar Strassen en casos grandes
    if (algo == "strassen" && n >= 1024) {
        cout << "[SKIP] " << algo << "  "
            << fs::path(path1).filename().string()
            << " + " << fs::path(path2).filename().string()
            << "  -> omitido (n=" << n << " demasiado grande)\n";
        return true;
    }


    auto t0 = chrono::steady_clock::now();
    vector<vector<int>> C = (algo == "naive")
    ? multiply_naive(A, B)
    : multiply_strassen(A, B);

    auto t1 = chrono::steady_clock::now();
    double ms = chrono::duration<double, std::milli>(t1 - t0).count();

    const string out_path = deriveOutputPath(path1, algo);
    writeMatrix(out_path, C);
    
    long long mem_est = EstimarMemBytes(algo, n);
    appendMeasurement(algo, fs::path(path1).filename().string(), n, ms, mem_est);

    cout << "[OK] " << algo << "  "
         << fs::path(path1).filename().string() << " + "
         << fs::path(path2).filename().string()
         << "  -> " << ms << " ms | out: " << out_path << "\n";

    return true;
}

/*****
* int main
******
* Punto de entrada principal del programa. Permite ejecutar los algoritmos de multiplicación ("naive" y "strassen") sobre un par específico de matrices pasado por argumentos, o iterar automáticamente procesando todos los pares coincidentes en el directorio de entrada.
******
* Input :
* int argc : Cantidad de argumentos recibidos por la línea de comandos.
* char** argv : Arreglo de cadenas de texto con los argumentos.
******
* Returns :
* int, retorna 0 si todas las multiplicaciones finalizaron con éxito, 1 si faltan directorios o hay errores de argumentos, y 2 si falló el procesamiento de algún par.
*****/
int main(int argc, char** argv) {
    constexpr const char* SUF = "_1.txt";

    // Si te pasan 2 paths, corre ambos algoritmos sobre ese par
    if (argc == 3) {
        int ok = 0;
        ok += processPair(argv[1], argv[2], "naive")     ? 1 : 0;
        ok += processPair(argv[1], argv[2], "strassen")  ? 1 : 0;
        return (ok == 2) ? 0 : 1;
    }

    fs::path in_dir(INPUT_DIR);
    if (!fs::exists(in_dir)) {
        cerr << "[ERROR] No existe " << INPUT_DIR << "\n";
        return 1;
    }

    vector<fs::path> files1;
    for (auto& e : fs::directory_iterator(in_dir)) {
        if (!e.is_regular_file()) continue;
        auto name = e.path().filename().string();
        if (name.size() > strlen(SUF) &&
            name.substr(name.size() - strlen(SUF)) == SUF) {
            files1.push_back(e.path());
        }
    }
    sort(files1.begin(), files1.end());

    if (files1.empty()) {
        cerr << "[WARN] No hay *_1.txt en " << INPUT_DIR << "\n";
        return 0;
    }

    int ok = 0, fail = 0;
    for (const auto& p1 : files1) {
        string s1 = p1.filename().string();
        string s2 = s1;
        s2.replace(s2.size() - strlen(SUF), strlen(SUF), "_2.txt");
        fs::path p2 = in_dir / s2;

        if (!fs::exists(p2)) {
            cerr << "[WARN] Falta el par " << s2 << "\n";
            ++fail;
            continue;
        }

        // Corre ambos algoritmos para cada par
        if (processPair(p1.string(), p2.string(), "naive"))     ++ok; else ++fail;
        if (processPair(p1.string(), p2.string(), "strassen"))  ++ok; else ++fail;
    }

    cout << "[RESUMEN] OK: " << ok << "  Fallos: " << fail << "\n";
    return (fail == 0) ? 0 : 2;
}
