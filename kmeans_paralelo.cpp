
// Este código implementa el algoritmo K-means en paralelo con OpenMP
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <random>
#include <limits>
#include <stdexcept>
#include <unordered_set>
#include <omp.h>

using namespace std;


struct Point {
    vector<double> coords;
    int cluster = -1;
};

// Calculamos la distancia euclidiana al cuadrado entre dos puntos
double squaredDistance(const vector<double>& a, const vector<double>& b) {
    double sum = 0.0;
    for (size_t i = 0; i < a.size(); i++) {
        double diff = a[i] - b[i];
        sum += diff * diff;
    }
    return sum;
}

// Lee el archivo csv y guarda sus puntos en un vector 
vector<Point> readCSV(const string& filename, int& dimensions) {
    ifstream file(filename);
    if (!file.is_open()) {
        throw runtime_error("No se pudo abrir el archivo: " + filename);
    }

    vector<Point> points;
    string line;
    dimensions = -1;

    while (getline(file, line)) {
        if (line.empty()) continue; //ignoramos líneas vacías

        stringstream ss(line);
        string value;
        vector<double> coords;

        while (getline(ss, value, ',')) {
            coords.push_back(stod(value));
        }
        // determina la dimensión 
        if (dimensions == -1) {
            dimensions = (int)coords.size();
            if (dimensions != 2 && dimensions != 3) {
                throw runtime_error("El CSV debe tener 2 o 3 columnas.");
            }
        }

        if ((int)coords.size() != dimensions) {
            throw runtime_error("Fila con numero de columnas inconsistente.");
        }

        Point p;
        p.coords = coords;
        p.cluster = -1;
        points.push_back(p);
    }

    if (points.empty()) {
        throw runtime_error("El archivo CSV no contiene puntos.");
    }

    return points;
}

//Incializamos los centroides a partir de k puntos random del dataset 
vector<vector<double>> initializeCentroids(const vector<Point>& points, int k) {
    if (k <= 0 || k > (int)points.size()) {
        throw runtime_error("k debe ser mayor que 0 y menor o igual al numero de puntos.");
    }

    vector<vector<double>> centroids;
    unordered_set<int> used;

    mt19937 gen(42);
    uniform_int_distribution<> dist(0, (int)points.size() - 1);

    while ((int)centroids.size() < k) {
        int idx = dist(gen);
        if (used.find(idx) == used.end()) {
            used.insert(idx);
            centroids.push_back(points[idx].coords);
        }
    }

    return centroids;
}

//Usando paralelización va asignando puntos al centroide más cercano 
bool assignClustersParallel(vector<Point>& points, const vector<vector<double>>& centroids) {
    int n = (int)points.size();
    int k = (int)centroids.size();
    int changed = 0;

    #pragma omp parallel for schedule(static)
    for (int i = 0; i < n; i++) {
        double bestDist = numeric_limits<double>::max();
        int bestCluster = -1;

        //Buscamos el centroide mas cercano al punto actual 
        for (int c = 0; c < k; c++) {
            double dist = squaredDistance(points[i].coords, centroids[c]);
            if (dist < bestDist) {
                bestDist = dist;
                bestCluster = c;
            }
        }

        // Actualiza si hubo un cambio de cluster 
        if (points[i].cluster != bestCluster) {
            points[i].cluster = bestCluster;

            #pragma omp atomic write
            changed = 1;
        }
    }

    return changed == 1;
}

//Actualizamos las posiciones de los centroides 
void updateCentroidsParallel(const vector<Point>& points, vector<vector<double>>& centroids) {
    int k = (int)centroids.size();
    int dims = (int)centroids[0].size();
    int numThreads = omp_get_max_threads();
    int n = (int)points.size();

    vector<vector<vector<double>>> localSums(
        numThreads, vector<vector<double>>(k, vector<double>(dims, 0.0))
    );
    vector<vector<int>> localCounts(numThreads, vector<int>(k, 0));

    #pragma omp parallel
    {
        int tid = omp_get_thread_num();

        #pragma omp for schedule(static)
        for (int i = 0; i < n; i++) {
            int cluster = points[i].cluster;
            localCounts[tid][cluster]++;

            for (int d = 0; d < dims; d++) {
                localSums[tid][cluster][d] += points[i].coords[d];
            }
        }
    }

    vector<vector<double>> sums(k, vector<double>(dims, 0.0));
    vector<int> counts(k, 0);

    for (int t = 0; t < numThreads; t++) {
        for (int c = 0; c < k; c++) {
            counts[c] += localCounts[t][c];
            for (int d = 0; d < dims; d++) {
                sums[c][d] += localSums[t][c][d];
            }
        }
    }
    
    //Nuevo centroide de cada cluster 
    for (int c = 0; c < k; c++) {
        if (counts[c] > 0) {
            for (int d = 0; d < dims; d++) {
                centroids[c][d] = sums[c][d] / counts[c];
            }
        }
    }
}

void writeCSV(const string& filename, const vector<Point>& points, int dims) {
    ofstream file(filename);
    if (!file.is_open()) {
        throw runtime_error("No se pudo crear el archivo de salida: " + filename);
    }

    // Guarda los puntos segun sus diemnsiones 
    if (dims == 2) {
        file << "x,y,cluster\n";
        for (const auto& p : points) {
            file << p.coords[0] << "," << p.coords[1] << "," << p.cluster << "\n";
        }
    } else {
        file << "x,y,z,cluster\n";
        for (const auto& p : points) {
            file << p.coords[0] << "," << p.coords[1] << "," << p.coords[2] << "," << p.cluster << "\n";
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc != 5 && argc != 6) {
    cerr << "Uso: " << argv[0] << " input.csv k output.csv num_threads [save_output]\n";
    return 1;
    }

    string inputFile = argv[1];
    int k = stoi(argv[2]);
    string outputFile = argv[3];
    int numThreads = stoi(argv[4]);
    int saveOutput = 1;
    if (argc == 6) {
        saveOutput = stoi(argv[5]);
    }

    omp_set_num_threads(numThreads);

    try {
        int dims;
        vector<Point> points = readCSV(inputFile, dims);
        vector<vector<double>> centroids = initializeCentroids(points, k);

        const int maxIters = 1000;
        int iter = 0;
        bool changed = true;

        double start = omp_get_wtime();
        
        // Ejecuta K-means hasta converger o llegar al máximo de iteraciones
        while (changed && iter < maxIters) {
            changed = assignClustersParallel(points, centroids);
            updateCentroidsParallel(points, centroids);
            iter++;
        }

        double end = omp_get_wtime();
        double elapsed_ms = (end - start) * 1000.0;

        if (saveOutput == 1) {
            writeCSV(outputFile, points, dims);
        }

        cout << "K-means OpenMP terminado.\n";
        cout << "Puntos: " << points.size() << "\n";
        cout << "Dimensiones: " << dims << "\n";
        cout << "Clusters (k): " << k << "\n";
        cout << "Hilos: " << numThreads << "\n";
        cout << "Iteraciones: " << iter << "\n";
        cout << "Salida: " << outputFile << "\n";
        cout << "Tiempo_ms: " << elapsed_ms << "\n";

    } catch (const exception& e) {
        cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
