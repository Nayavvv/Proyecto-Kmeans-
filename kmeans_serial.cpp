// Este código implementa K-means en versión serial
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <random>
#include <limits>
#include <cmath>
#include <stdexcept>
#include <unordered_set>
#include <chrono>

using namespace std;

struct Point {
    vector<double> coords;
    int cluster = -1;
};

// distancia euclidiana al cuadrado entre dos puntos
double squaredDistance(const vector<double>& a, const vector<double>& b) {
    double sum = 0.0;
    for (size_t i = 0; i < a.size(); i++) {
        double diff = a[i] - b[i];
        sum += diff * diff;
    }
    return sum;
}

vector<Point> readCSV(const string& filename, int& dimensions) {
    ifstream file(filename);
    if (!file.is_open()) {
        throw runtime_error("No se pudo abrir el archivo: " + filename);
    }

    vector<Point> points;
    string line;
    dimensions = -1;

    while (getline(file, line)) {
        if (line.empty()) continue;

        stringstream ss(line);
        string value;
        vector<double> coords;

        while (getline(ss, value, ',')) {
            coords.push_back(stod(value));
        }

        // Saber dimension de los datos 
        if (dimensions == -1) {
            dimensions = (int)coords.size();
            if (dimensions != 2 && dimensions != 3) {
                throw runtime_error("El CSV debe tener 2 o 3 columnas.");
            }
        }

        if ((int)coords.size() != dimensions) {
            throw runtime_error("Fila con numero de columnas inconsistente.");
        }

        points.push_back({coords, -1});
    }

    if (points.empty()) {
        throw runtime_error("El archivo CSV no contiene puntos.");
    }

    return points;
}

//K centroides aleatorios 
vector<vector<double>> initializeCentroids(const vector<Point>& points, int k) {
    if (k <= 0 || k > (int)points.size()) {
        throw runtime_error("k debe ser mayor que 0 y menor o igual al numero de puntos.");
    }

    vector<vector <double>> centroids;
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

//Asignacion de puntos a su centroide mas cercano 
bool assignClusters(vector<Point>& points, const vector<vector<double>>& centroids) {
    bool changed = false;

    for (auto& p : points) {
        double bestDist = numeric_limits<double>::max();
        int bestCluster = -1;

        for (int c = 0; c < (int)centroids.size(); c++) {
            double dist = squaredDistance(p.coords, centroids[c]);
            if (dist < bestDist) {
                bestDist = dist;
                bestCluster = c;
            }
        }

        if (p.cluster != bestCluster) {
            p.cluster = bestCluster;
            changed = true;
        }
    }

    return changed;
}

//Se recalculan los centroides
void updateCentroids(const vector<Point>& points, vector<vector<double>>& centroids) {
    int k = (int)centroids.size();
    int dims = (int)centroids[0].size();

    vector<vector<double>> sums(k, vector<double>(dims, 0.0));
    vector<int> counts(k, 0);

    for (const auto& p : points) {
        counts[p.cluster]++;
        for (int d = 0; d < dims; d++) {
            sums[p.cluster][d] += p.coords[d];
        }
    }

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

    //Distinguir datos por dimension 
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
    if (argc != 4 && argc != 5) {
    cerr << "Uso: " << argv[0] << " input.csv k output.csv [save_output]\n";
    return 1;
    }

    string inputFile = argv[1];
    int k = stoi(argv[2]);
    string outputFile = argv[3];

    int saveOutput = 1;
    if (argc == 5) {
        saveOutput = stoi(argv[4]);
    }

    try {
        int dims;
        vector<Point> points = readCSV(inputFile, dims);
        vector<vector<double>> centroids = initializeCentroids(points, k);

        const int maxIters = 1000;
        int iter = 0;
        bool changed = true;

        auto start = chrono::high_resolution_clock::now();

        //Ejecutar hasta converger o llegar al mexicmo de iteraciones 
        while (changed && iter < maxIters) {
            changed = assignClusters(points, centroids);
            updateCentroids(points, centroids);
            iter++;
        }

        auto end = chrono::high_resolution_clock::now();
        double elapsed_ms = chrono::duration<double, milli>(end - start).count();

        if (saveOutput == 1) {
            writeCSV(outputFile, points, dims);
        }

        cout << "K-means serial terminado.\n";
        cout << "Puntos: " << points.size() << "\n";
        cout << "Dimensiones: " << dims << "\n";
        cout << "Clusters (k): " << k << "\n";
        cout << "Iteraciones: " << iter << "\n";
        cout << "Salida: " << outputFile << "\n";
        cout << "Tiempo_ms: " << elapsed_ms << "\n";

    } catch (const exception& e) {
        cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
