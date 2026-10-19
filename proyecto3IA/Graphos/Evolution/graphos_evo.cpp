
#include <algorithm>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <utility>
#include <vector>

// OptFrame: printable.hpp debe incluirse primero.
#include <OptFrame/printable/printable.hpp>
#include <OptFCore/FCore.hpp>
#include <OptFCore/FCoreAll.hpp>
#include <OptFrame/Core.hpp>
#include <OptFrame/Heuristics.hpp>
#include <OptFrame/Heuristics/EA/RK/BRKGA.hpp>
#include <OptFrame/Search/InitialPopulation.hpp>

using namespace std;
using namespace optframe;

// =====================================================
// Tipo de solucion y evaluacion
// =====================================================

// first: vector de colores de los nodos.
// second: evaluacion (numero de aristas conflictivas).
using ESolutionGC =
    pair<vector<int>, Evaluation<int>>;

// =====================================================
// Contexto del problema
// =====================================================

class ProblemContext {
public:
    int n;
    int k;
    vector<pair<int, int>> aristas;

    ProblemContext(
        int n_,
        int k_,
        double probabilidad,
        unsigned semilla
    ) : n(n_), k(k_) {

        mt19937 rng(semilla);
        uniform_real_distribution<double> real(0.0, 1.0);

        // Coloreado oculto: garantiza que el grafo
        // generado sea k-coloreable.
        vector<int> colorOculto(n);

        for (int i = 0; i < n; ++i) {
            colorOculto[i] = i % k;
        }

        shuffle(colorOculto.begin(), colorOculto.end(), rng);

        // Solo se agregan aristas entre clases diferentes.
        // El algoritmo no recibe colorOculto.
        for (int u = 0; u < n; ++u) {
            for (int v = u + 1; v < n; ++v) {
                if (colorOculto[u] != colorOculto[v] &&
                    real(rng) < probabilidad) {
                    aristas.emplace_back(u, v);
                }
            }
        }
    }
};

// =====================================================
// Funcion objetivo: minimizar aristas monocromaticas
// =====================================================

int contarConflictos(
    const ProblemContext& problema,
    const vector<int>& colores
) {
    int conflictos = 0;

    for (const auto& [u, v] : problema.aristas) {
        if (colores[u] == colores[v]) {
            ++conflictos;
        }
    }

    return conflictos;
}

Evaluation<int> evaluar(
    sref<ProblemContext> problema,
    const vector<int>& colores
) {
    return Evaluation<int>{
        contarConflictos(*problema, colores)
    };
}

// =====================================================
// Poblacion inicial de claves aleatorias
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

        VEPopulation<pair<vector<double>, Evaluation<int>>> pop;

        for (unsigned i = 0; i < populationSize; ++i) {
            vector<double> claves(n);

            for (int j = 0; j < n; ++j) {
                claves[j] =
                    static_cast<double>(rg->rand() % 100000)
                    / 100000.0;
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
// Decodificador BRKGA
//
// 1. Ordena los nodos por su clave aleatoria.
// 2. Asigna colores según la posicion en el orden.
// 3. Evalua el numero de aristas conflictivas.
// =====================================================

pair<Evaluation<int>, vector<int>> decodificar(
    sref<Evaluator<
        ESolutionGC::first_type,
        ESolutionGC::second_type,
        ESolutionGC
    >> evaluador,
    const vector<double>& claves,
    sref<ProblemContext> problema
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

    vector<int> colores(problema->n);

    // Produce clases de color de tamaños equilibrados.
    for (int posicion = 0;
         posicion < static_cast<int>(orden.size());
         ++posicion) {
        int nodo = orden[posicion].second;
        colores[nodo] = posicion % problema->k;
    }

    Evaluation<int> valor = evaluador->evaluate(colores);

    return {valor, colores};
}

// =====================================================
// Guardar grafo en formato: n m k; luego aristas
// =====================================================

bool guardarGrafo(
    const ProblemContext& problema,
    const string& archivo
) {
    ofstream salida(archivo);

    if (!salida) {
        cerr << "No se pudo crear " << archivo << '\n';
        return false;
    }

    salida << problema.n << ' '
           << problema.aristas.size() << ' '
           << problema.k << '\n';

    for (const auto& [u, v] : problema.aristas) {
        salida << u << ' ' << v << '\n';
    }

    return true;
}

bool guardarSolucion(
    const vector<int>& colores,
    int conflictos,
    const string& archivo
) {
    ofstream salida(archivo);

    if (!salida) {
        cerr << "No se pudo crear " << archivo << '\n';
        return false;
    }

    salida << colores.size() << ' '
           << conflictos << '\n';

    for (int i = 0; i < static_cast<int>(colores.size()); ++i) {
        salida << i << ' ' << colores[i] << '\n';
    }

    return true;
}

// =====================================================
// Ejecutar una instancia
// =====================================================

void ejecutarInstancia(
    int n,
    int k,
    double probabilidad,
    unsigned semilla,
    double tiempoSegundos
) {
    cout << "\n========================================\n";
    cout << "GRAFO: " << n << " nodos\n";
    cout << "Colores disponibles: " << k << '\n';
    cout << "========================================\n";

    sref<ProblemContext> problema{
        new ProblemContext(n, k, probabilidad, semilla)
    };

    string sufijo = to_string(n);
    string archivoGrafo = "grafo_evolutivo_" + sufijo + ".txt";
    string archivoSolucion = "solucion_evolutiva_" + sufijo + ".txt";

    if (!guardarGrafo(*problema, archivoGrafo)) {
        return;
    }

    cout << "Aristas: " << problema->aristas.size() << '\n';
    cout << "Grafo guardado en: " << archivoGrafo << '\n';

    sref<FEvaluator<ESolutionGC, MINIMIZE, ProblemContext>>
        evaluador{
            new FEvaluator<ESolutionGC, MINIMIZE, ProblemContext>{
                problema, evaluar
            }
        };

    sref<RandGen> rg{new RandGen};

    sref<DecoderRandomKeys<ESolutionGC, double>> decoder{
        new FDecoderEvalRK<ESolutionGC, double>{
            evaluador,
            [problema](
                auto eval,
                const vector<double>& claves
            ) {
                return decodificar(eval, claves, problema);
            }
        }
    };

    // Parametros evolutivos de BRKGA.
    const unsigned tamPoblacion = (n == 50) ? 80 : 100;
    const unsigned generaciones = 500;
    const double fraccionElite = 0.20;
    const double fraccionMutante = 0.10;
    const double probabilidadElite = 0.70;

    BRKGA<ESolutionGC, double> algoritmo(
        decoder,
        PoblacionInicial(n, rg),
        tamPoblacion,
        generaciones,
        fraccionElite,
        fraccionMutante,
        probabilidadElite,
        rg
    );

    cout << "Poblacion: " << tamPoblacion << '\n';
    cout << "Generaciones maximas: " << generaciones << '\n';
    cout << "Tiempo limite: " << tiempoSegundos << " segundos\n";
    cout << "Ejecutando BRKGA...\n";

    auto resultado = algoritmo.search(tiempoSegundos);

    if (!resultado.best) {
        cerr << "OptFrame no devolvio una solucion para n = "
             << n << '\n';
        return;
    }

    const ESolutionGC& mejor = *resultado.best;
    const vector<int>& colores = mejor.first;

    int conflictos = contarConflictos(*problema, colores);

    cout << "\nMejor numero de conflictos: "
         << conflictos << '\n';

    if (conflictos == 0) {
        cout << "Resultado: coloreado valido.\n";
    } else {
        cout << "Resultado: quedan conflictos; "
             << "no se encontro un coloreado valido.\n";
    }

    cout << "Colores asignados por nodo:\n";

    for (int i = 0; i < n; ++i) {
        cout << i << ':' << colores[i];

        if ((i + 1) % 10 == 0 || i == n - 1) {
            cout << '\n';
        } else {
            cout << "  ";
        }
    }

    if (guardarSolucion(colores, conflictos, archivoSolucion)) {
        cout << "Solucion guardada en: "
             << archivoSolucion << '\n';
    }
}

int main() {
    cout << "ALGORITMO EVOLUTIVO BRKGA - OPTFRAME\n";

    // Instancia 1: 50 nodos, 4 colores.
    ejecutarInstancia(
        50,
        4,
        0.20,
        2026,
        5.0
    );

    // Instancia 2: 1000 nodos, 20 colores.
    ejecutarInstancia(
        1000,
        20,
        0.01,
        2026,
        10.0
    );

    cout << "\nEjecucion terminada.\n";
    return 0;
}
