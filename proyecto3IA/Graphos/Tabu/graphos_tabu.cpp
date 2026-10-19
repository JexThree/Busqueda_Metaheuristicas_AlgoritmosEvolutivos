
#define NO_CXX_MODULES 1

#include <OptFrame/printable/printable.hpp>
#include <OptFCore/FCore.hpp>
#include <OptFCore/FCoreAll.hpp>
#include <OptFrame/Core.hpp>
#include <OptFrame/Heuristics.hpp>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <numeric>
#include <random>
#include <string>
#include <utility>
#include <vector>

using namespace std;
using namespace optframe;

// =====================================================
// Tipos OptFrame
// =====================================================

using SolucionGC = vector<int>;
using ESolutionGC = pair<SolucionGC, Evaluation<int>>;

class ProblemContext {
public:
    int n;
    int k;
    vector<pair<int, int>> aristas;
    vector<vector<int>> adyacencia;

    ProblemContext(int n_, int k_, double prob, unsigned semilla)
        : n(n_), k(k_), adyacencia(n_) {

        mt19937 rng(semilla);
        uniform_real_distribution<double> real(0.0, 1.0);

        // Coloreado oculto: garantiza k-coloreabilidad.
        vector<int> colorOculto(n);
        for (int i = 0; i < n; ++i)
            colorOculto[i] = i % k;

        shuffle(colorOculto.begin(), colorOculto.end(), rng);

        // El algoritmo de busqueda no recibe estos colores.
        for (int u = 0; u < n; ++u) {
            for (int v = u + 1; v < n; ++v) {
                if (colorOculto[u] != colorOculto[v] &&
                    real(rng) < prob) {
                    aristas.emplace_back(u, v);
                    adyacencia[u].push_back(v);
                    adyacencia[v].push_back(u);
                }
            }
        }
    }
};

// =====================================================
// Evaluacion OptFrame: minimizar aristas conflictivas
// =====================================================

int contarConflictos(
    const ProblemContext& p,
    const SolucionGC& colores
) {
    int conflictos = 0;

    for (const auto& [u, v] : p.aristas) {
        if (colores[u] == colores[v])
            ++conflictos;
    }

    return conflictos;
}

Evaluation<int> evaluar(
    sref<ProblemContext> p,
    const SolucionGC& colores
) {
    return Evaluation<int>{contarConflictos(*p, colores)};
}

// =====================================================
// Busqueda Tabu
// =====================================================

class TabuColoring {
private:
    sref<ProblemContext> problema;
    sref<FEvaluator<ESolutionGC, MINIMIZE, ProblemContext>> evaluador;

    int maxIteraciones;
    int tenenciaTabu;
    mt19937 rng;

    SolucionGC mejorSolucion;
    int mejorValor = 0;

    // tabuHasta[nodo][color]:
    // iteracion a partir de la cual puede usarse ese color.
    vector<vector<int>> tabuHasta;

    vector<int> obtenerNodosEnConflicto(
        const SolucionGC& colores
    ) const {
        vector<bool> conflicto(problema->n, false);

        for (const auto& [u, v] : problema->aristas) {
            if (colores[u] == colores[v]) {
                conflicto[u] = true;
                conflicto[v] = true;
            }
        }

        vector<int> nodos;
        for (int i = 0; i < problema->n; ++i) {
            if (conflicto[i])
                nodos.push_back(i);
        }

        return nodos;
    }

    // Delta = conflictos nuevos - conflictos eliminados.
    int calcularDelta(
        int nodo,
        int nuevoColor,
        const SolucionGC& colores
    ) const {
        int colorAnterior = colores[nodo];
        int delta = 0;

        for (int vecino : problema->adyacencia[nodo]) {
            if (colores[vecino] == nuevoColor)
                ++delta;

            if (colores[vecino] == colorAnterior)
                --delta;
        }

        return delta;
    }

public:
    TabuColoring(
        sref<ProblemContext> p,
        sref<FEvaluator<ESolutionGC, MINIMIZE, ProblemContext>> e,
        int iteraciones,
        int tenencia,
        unsigned semilla
    )
        : problema(p),
          evaluador(e),
          maxIteraciones(iteraciones),
          tenenciaTabu(tenencia),
          rng(semilla),
          tabuHasta(p->n, vector<int>(p->k, 0)) {}

    SolucionGC resolver() {
        const int n = problema->n;
        const int k = problema->k;

        SolucionGC actual(n);
        uniform_int_distribution<int> colorAleatorio(0, k - 1);

        for (int i = 0; i < n; ++i)
            actual[i] = colorAleatorio(rng);

        // La evaluacion inicial usa OptFrame.
        int conflictosActuales =
            evaluador->evaluate(actual).evaluation();

        mejorSolucion = actual;
        mejorValor = conflictosActuales;

        for (int iter = 1; iter <= maxIteraciones; ++iter) {
            if (mejorValor == 0)
                break;

            vector<int> nodos =
                obtenerNodosEnConflicto(actual);

            if (nodos.empty())
                break;

            // Elegir aleatoriamente un nodo en conflicto.
            uniform_int_distribution<int> elegirNodo(
                0, static_cast<int>(nodos.size()) - 1
            );

            int nodo = nodos[elegirNodo(rng)];
            int colorAnterior = actual[nodo];

            int mejorColor = -1;
            int mejorDelta = 0;
            int mejorValorVecino = 0;
            bool encontradoAdmisible = false;

            // Primera pasada: movimientos no tabu o con aspiracion.
            for (int color = 0; color < k; ++color) {
                if (color == colorAnterior)
                    continue;

                int delta = calcularDelta(nodo, color, actual);
                int valorVecino = conflictosActuales + delta;

                bool esTabu = iter < tabuHasta[nodo][color];
                bool aspiracion = valorVecino < mejorValor;

                if (esTabu && !aspiracion)
                    continue;

                if (!encontradoAdmisible ||
                    valorVecino < mejorValorVecino) {
                    encontradoAdmisible = true;
                    mejorColor = color;
                    mejorDelta = delta;
                    mejorValorVecino = valorVecino;
                }
            }

            // Si todos los movimientos son tabu, se permite
            // el mejor movimiento disponible para evitar bloqueo.
            if (!encontradoAdmisible) {
                for (int color = 0; color < k; ++color) {
                    if (color == colorAnterior)
                        continue;

                    int delta = calcularDelta(nodo, color, actual);
                    int valorVecino = conflictosActuales + delta;

                    if (mejorColor == -1 ||
                        valorVecino < mejorValorVecino) {
                        mejorColor = color;
                        mejorDelta = delta;
                        mejorValorVecino = valorVecino;
                    }
                }
            }

            if (mejorColor == -1)
                break;

            // Registrar el color anterior como tabu:
            // evita deshacer inmediatamente el movimiento.
            tabuHasta[nodo][colorAnterior] =
                iter + tenenciaTabu;

            actual[nodo] = mejorColor;
            conflictosActuales += mejorDelta;

            if (conflictosActuales < mejorValor) {
                mejorValor = conflictosActuales;
                mejorSolucion = actual;
            }

            if (iter % 1000 == 0) {
                cout << "Iteracion " << iter
                     << " | Actual: " << conflictosActuales
                     << " | Mejor: " << mejorValor << '\n';
            }
        }

        // Verificacion final completa mediante OptFrame.
        mejorValor = evaluador->evaluate(mejorSolucion).evaluation();

        return mejorSolucion;
    }

    int obtenerMejorValor() const {
        return mejorValor;
    }
};

// =====================================================
// Guardar instancias y soluciones
// =====================================================

bool guardarGrafo(
    const ProblemContext& p,
    const string& nombre
) {
    ofstream out(nombre);
    if (!out)
        return false;

    out << p.n << ' ' << p.aristas.size()
        << ' ' << p.k << '\n';

    for (const auto& [u, v] : p.aristas)
        out << u << ' ' << v << '\n';

    return true;
}

bool guardarSolucion(
    const SolucionGC& colores,
    int conflictos,
    const string& nombre
) {
    ofstream out(nombre);
    if (!out)
        return false;

    out << colores.size() << ' ' << conflictos << '\n';

    for (int i = 0; i < static_cast<int>(colores.size()); ++i)
        out << i << ' ' << colores[i] << '\n';

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
    int iteraciones,
    int tenencia
) {
    cout << "\n====================================\n";
    cout << "Coloreado Tabu: " << n << " nodos\n";
    cout << "Colores: " << k << '\n';
    cout << "====================================\n";

    sref<ProblemContext> problema{
        new ProblemContext(n, k, probabilidad, semilla)
    };

    const string sufijo = to_string(n);
    const string archivoGrafo = "grafo_tabu_" + sufijo + ".txt";
    const string archivoSolucion =
        "solucion_tabu_" + sufijo + ".txt";

    if (!guardarGrafo(*problema, archivoGrafo)) {
        cerr << "Error al guardar el grafo.\n";
        return;
    }

    cout << "Aristas: " << problema->aristas.size() << '\n';
    cout << "Grafo: " << archivoGrafo << '\n';

    sref<FEvaluator<ESolutionGC, MINIMIZE, ProblemContext>>
        evaluador{
            new FEvaluator<ESolutionGC, MINIMIZE, ProblemContext>{
                problema, evaluar
            }
        };

    TabuColoring algoritmo(
        problema,
        evaluador,
        iteraciones,
        tenencia,
        semilla + 1
    );

    cout << "Iteraciones maximas: " << iteraciones << '\n';
    cout << "Tenencia tabu: " << tenencia << '\n';

    SolucionGC colores = algoritmo.resolver();
    int conflictos = algoritmo.obtenerMejorValor();

    cout << "\nConflictos: " << conflictos << '\n';

    if (conflictos == 0)
        cout << "Se encontro un coloreado valido.\n";
    else
        cout << "No se encontro un coloreado valido en esta ejecucion.\n";

    if (!guardarSolucion(colores, conflictos, archivoSolucion))
        cerr << "Error al guardar la solucion.\n";
    else
        cout << "Solucion: " << archivoSolucion << '\n';
}

int main() {
    // Instancia de 50 nodos.
    ejecutarInstancia(
        50, 4, 0.20, 2026,
        10000, 10
    );

    // Instancia de 1000 nodos.
    ejecutarInstancia(
        1000, 20, 0.01, 2026,
        50000, 15
    );

    return 0;
}
