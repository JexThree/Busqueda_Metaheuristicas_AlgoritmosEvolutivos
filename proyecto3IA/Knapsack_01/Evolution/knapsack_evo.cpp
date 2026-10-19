
#define NO_CXX_MODULES 1

#include <algorithm>
#include <iostream>
#include <numeric>
#include <random>
#include <utility>
#include <vector>

#include <OptFrame/printable/printable.hpp>
#include <OptFCore/FCore.hpp>
#include <OptFCore/FCoreAll.hpp>
#include <OptFrame/Core.hpp>
#include <OptFrame/Heuristics.hpp>
#include <OptFrame/Heuristics/EA/RK/BRKGA.hpp>
#include <OptFrame/Search/InitialPopulation.hpp>

using namespace optframe;
using namespace std;

// =====================================================
// Representacion de una solucion evaluada
// =====================================================

using ESolutionKP =
    pair<vector<bool>, Evaluation<int>>;

// =====================================================
// Datos del problema
// =====================================================

class ProblemContext {
public:
    int n;
    int capacidad;
    vector<int> pesos;
    vector<int> valores;

    ProblemContext(
        int capacidad_,
        vector<int> pesos_,
        vector<int> valores_
    ) :
        n(static_cast<int>(pesos_.size())),
        capacidad(capacidad_),
        pesos(move(pesos_)),
        valores(move(valores_))
    {}
};

// =====================================================
// Evaluador: maximiza el valor total
// El decodificador garantiza la factibilidad
// =====================================================

Evaluation<int> evaluar(
    sref<ProblemContext> problema,
    const vector<bool>& seleccion
) {
    int valorTotal = 0;

    for (int i = 0; i < problema->n; ++i) {
        if (seleccion[i]) {
            valorTotal += problema->valores[i];
        }
    }

    return Evaluation<int>{valorTotal};
}

// =====================================================
// Poblacion inicial de claves aleatorias
// =====================================================

class PoblacionInicialBRKGA :
    public InitialEPopulation<
        pair<vector<double>, Evaluation<int>>
    > {

private:
    int n;
    sref<RandGen> rg;

public:
    explicit PoblacionInicialBRKGA(
        int cantidad,
        sref<RandGen> generador
    ) : n(cantidad), rg(generador) {}

    bool canEvaluate() const override {
        return false;
    }

    VEPopulation<pair<vector<double>, Evaluation<int>>>
    generateEPopulation(
        unsigned populationSize,
        double /* timelimit */
    ) override {
        VEPopulation<pair<vector<double>, Evaluation<int>>> pop;

        for (unsigned i = 0; i < populationSize; ++i) {
            vector<double> claves(n);

            for (int j = 0; j < n; ++j) {
                claves[j] =
                    static_cast<double>(rg->rand() % 100000) /
                    100000.0;
            }

            pop.push_back({
                move(claves),
                Evaluation<int>{}
            });
        }

        return pop;
    }
};

// =====================================================
// Decodificador: claves aleatorias -> mochila factible
// =====================================================

pair<Evaluation<int>, vector<bool>> decodificar(
    sref<
        Evaluator<
            ESolutionKP::first_type,
            ESolutionKP::second_type,
            ESolutionKP
        >
    > evaluador,
    const vector<double>& claves,
    sref<ProblemContext> problema
) {
    vector<int> orden(claves.size());
    iota(orden.begin(), orden.end(), 0);

    sort(
        orden.begin(),
        orden.end(),
        [&claves](int a, int b) {
            return claves[a] < claves[b];
        }
    );

    vector<bool> seleccion(problema->n, false);
    int pesoTotal = 0;

    for (int i : orden) {
        if (pesoTotal + problema->pesos[i]
            <= problema->capacidad) {
            seleccion[i] = true;
            pesoTotal += problema->pesos[i];
        }
    }

    Evaluation<int> valor = evaluador->evaluate(seleccion);

    return {valor, seleccion};
}

// =====================================================
// Programa principal
// =====================================================

int main() {
    // Instancia de prueba
    const int capacidad = 5;

    vector<int> pesos  = {2, 3, 4, 5};
    vector<int> valores = {3, 4, 5, 6};

    sref<ProblemContext> problema{
        new ProblemContext(capacidad, pesos, valores)
    };

    // Evaluador de OptFrame
    sref<FEvaluator<ESolutionKP, MAXIMIZE, ProblemContext>>
        evaluador{
            new FEvaluator<ESolutionKP, MAXIMIZE, ProblemContext>{
                problema, evaluar
            }
        };

    // Generador aleatorio
    sref<RandGen> rg{new RandGen};

    // Decodificador de claves aleatorias
    sref<DecoderRandomKeys<ESolutionKP, double>> decoder{
        new FDecoderEvalRK<ESolutionKP, double>{
            evaluador,
            [problema](
                auto eval,
                const vector<double>& claves
            ) {
                return decodificar(eval, claves, problema);
            }
        }
    };

    // Parámetros del BRKGA
    const unsigned tamPoblacion = 50;
    const unsigned generaciones = 300;
    const double fraccionElite = 0.20;
    const double fraccionMutante = 0.10;
    const double probabilidadElite = 0.70;

    BRKGA<ESolutionKP, double> algoritmo(
        decoder,
        PoblacionInicialBRKGA(problema->n, rg),
        tamPoblacion,
        generaciones,
        fraccionElite,
        fraccionMutante,
        probabilidadElite,
        rg
    );

    cout << "=== KNAPSACK 0/1 CON OPTFRAME BRKGA ===\n";
    cout << "Capacidad: " << capacidad << '\n';
    cout << "Objetos: " << problema->n << '\n';
    cout << "Poblacion: " << tamPoblacion << '\n';
    cout << "Generaciones: " << generaciones << "\n\n";

    auto resultado = algoritmo.search(5.0);

    if (!resultado.best) {
        cerr << "El algoritmo no devolvio una solucion.\n";
        return 1;
    }

    const ESolutionKP& mejor = *resultado.best;

    int pesoTotal = 0;
    int valorTotal = 0;

    cout << "Objetos seleccionados:\n";

    for (int i = 0; i < problema->n; ++i) {
        if (mejor.first[i]) {
            cout << "  Objeto " << i
                 << " | peso = " << problema->pesos[i]
                 << " | valor = " << problema->valores[i]
                 << '\n';

            pesoTotal += problema->pesos[i];
            valorTotal += problema->valores[i];
        }
    }

    cout << "\nPeso total: " << pesoTotal
         << " / " << capacidad << '\n';
    cout << "Valor total: " << valorTotal << '\n';

    return 0;
}
