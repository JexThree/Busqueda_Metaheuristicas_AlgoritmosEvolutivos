
#include <algorithm>
#include <cmath>
#include <iostream>
#include <numeric>
#include <utility>
#include <vector>

// OptFrame: estos encabezados deben preceder a los demás
#include <OptFrame/printable/printable.hpp>
#include <OptFCore/FCore.hpp>
#include <OptFCore/FCoreAll.hpp>
#include <OptFrame/Core.hpp>
#include <OptFrame/Heuristics.hpp>
#include <OptFrame/Heuristics/EA/RK/BRKGA.hpp>
#include <OptFrame/Search/InitialPopulation.hpp>

using namespace std;
using namespace optframe;

// Solucion: una permutacion de columnas.
// Evaluacion: cantidad de pares de damas que se atacan.
using ESolutionNQ =
    pair<vector<int>, Evaluation<int>>;

// =====================================================
// Contexto del problema
// =====================================================

class ProblemContext {
public:
    int n;

    explicit ProblemContext(int n_) : n(n_) {}
};

// =====================================================
// Evaluacion: minimizar conflictos diagonales
// =====================================================

Evaluation<int> evaluar(
    sref<ProblemContext> problema,
    const vector<int>& solucion
) {
    int conflictos = 0;

    for (int i = 0; i < problema->n; ++i) {
        for (int j = i + 1; j < problema->n; ++j) {
            // La dama i esta en (i, solucion[i])
            // La dama j esta en (j, solucion[j]).
            if (abs(solucion[i] - solucion[j]) == j - i) {
                ++conflictos;
            }
        }
    }

    return Evaluation<int>{conflictos};
}

// =====================================================
// Poblacion inicial de claves aleatorias para BRKGA
// =====================================================

class PoblacionInicial :
    public InitialEPopulation<
        pair<vector<double>, Evaluation<int>>
    > {
private:
    int n;
    sref<RandGen> rg;

public:
    explicit PoblacionInicial(
        int n_,
        sref<RandGen> generador
    ) : n(n_), rg(generador) {}

    bool canEvaluate() const override {
        return false;
    }

    VEPopulation<pair<vector<double>, Evaluation<int>>>
    generateEPopulation(
        unsigned populationSize,
        double /* timelimit */
    ) override {
        VEPopulation<pair<vector<double>, Evaluation<int>>> poblacion;

        for (unsigned i = 0; i < populationSize; ++i) {
            vector<double> claves(n);

            for (int j = 0; j < n; ++j) {
                claves[j] =
                    static_cast<double>(rg->rand() % 100000) /
                    100000.0;
            }

            poblacion.push_back({
                move(claves),
                Evaluation<int>{}
            });
        }

        return poblacion;
    }
};

// =====================================================
// Decodificador: claves aleatorias -> permutacion
// =====================================================

pair<Evaluation<int>, vector<int>> decodificar(
    sref<Evaluator<
        ESolutionNQ::first_type,
        ESolutionNQ::second_type,
        ESolutionNQ
    >> evaluador,
    const vector<double>& claves
) {
    vector<pair<double, int>> orden(claves.size());

    for (int i = 0; i < static_cast<int>(claves.size()); ++i) {
        orden[i] = {claves[i], i};
    }

    sort(
        orden.begin(),
        orden.end(),
        [](const auto& a, const auto& b) {
            return a.first < b.first;
        }
    );

    vector<int> solucion(claves.size());

    for (int i = 0; i < static_cast<int>(orden.size()); ++i) {
        solucion[i] = orden[i].second;
    }

    Evaluation<int> valor = evaluador->evaluate(solucion);

    return {valor, solucion};
}

// =====================================================
// Programa principal
// =====================================================

int main() {
    const int N = 8;

    sref<ProblemContext> problema{
        new ProblemContext(N)
    };

    // Evaluador OptFrame: MINIMIZE porque queremos cero conflictos.
    sref<FEvaluator<ESolutionNQ, MINIMIZE, ProblemContext>>
        evaluador{
            new FEvaluator<ESolutionNQ, MINIMIZE, ProblemContext>{
                problema, evaluar
            }
        };

    sref<RandGen> rg{new RandGen};

    // Adaptador de decodificacion de claves aleatorias de OptFrame.
    sref<DecoderRandomKeys<ESolutionNQ, double>> decoder{
        new FDecoderEvalRK<ESolutionNQ, double>{
            evaluador, decodificar
        }
    };

    // Parametros evolutivos.
    const unsigned poblacion = 100;
    const unsigned generaciones = 500;
    const double fraccionElite = 0.20;
    const double fraccionMutante = 0.10;
    const double probElite = 0.70;

    BRKGA<ESolutionNQ, double> algoritmo(
        decoder,
        PoblacionInicial(N, rg),
        poblacion,
        generaciones,
        fraccionElite,
        fraccionMutante,
        probElite,
        rg
    );

    cout << "=== N-DAMAS CON OPTFRAME BRKGA ===\n";
    cout << "Tamano del tablero: " << N << " x " << N << '\n';
    cout << "Poblacion: " << poblacion << '\n';
    cout << "Generaciones: " << generaciones << "\n\n";

    auto resultado = algoritmo.search(5.0);

    if (!resultado.best) {
        cerr << "No se obtuvo una solucion.\n";
        return 1;
    }

    const ESolutionNQ& mejor = *resultado.best;
    const vector<int>& solucion = mejor.first;

    cout << "Conflictos diagonales: ";
    mejor.second.print();

    cout << "Vector solucion: [";
    for (int i = 0; i < N; ++i) {
        cout << solucion[i];
        if (i + 1 < N) cout << ", ";
    }
    cout << "]\n\n";

    cout << "Tablero:\n";
    for (int fila = 0; fila < N; ++fila) {
        for (int columna = 0; columna < N; ++columna) {
            if (solucion[fila] == columna)
                cout << " D ";
            else
                cout << " . ";
        }
        cout << '\n';
    }

    bool resuelto = evaluar(problema, solucion).evaluation() == 0;

    if (resuelto)
        cout << "\nSe encontro una solucion valida.\n";
    else
        cout << "\nNo se encontro una solucion sin conflictos "
             << "en esta ejecucion.\n";

    return 0;
}
