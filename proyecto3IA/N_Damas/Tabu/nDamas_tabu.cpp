
#define NO_CXX_MODULES 1

#include <OptFrame/printable/printable.hpp>
#include <OptFCore/FCore.hpp>
#include <OptFCore/FCoreAll.hpp>
#include <OptFrame/Core.hpp>
#include <OptFrame/Heuristics.hpp>

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <numeric>
#include <random>
#include <utility>
#include <vector>

using namespace std;
using namespace optframe;

using SolucionNQ = vector<int>;
using ESolutionNQ = pair<SolucionNQ, Evaluation<int>>;

class ProblemContext {
public:
    int n;

    explicit ProblemContext(int n_) : n(n_) {}
};

// =====================================================
// Evaluacion: minimizar parejas de damas en conflicto
// =====================================================

Evaluation<int> evaluar(
    sref<ProblemContext> problema,
    const SolucionNQ& solucion
) {
    int conflictos = 0;

    for (int i = 0; i < problema->n; ++i) {
        for (int j = i + 1; j < problema->n; ++j) {
            if (abs(solucion[i] - solucion[j]) == j - i) {
                ++conflictos;
            }
        }
    }

    return Evaluation<int>{conflictos};
}

class NQueensTabu {
private:
    sref<ProblemContext> problema;
    sref<FEvaluator<ESolutionNQ, MINIMIZE, ProblemContext>> evaluador;

    int maxIteraciones;
    int tenenciaTabu;
    mt19937 rng;

    int contarConflictos(const SolucionNQ& s) const {
        int conflictos = 0;
        int n = problema->n;

        for (int i = 0; i < n; ++i) {
            for (int j = i + 1; j < n; ++j) {
                if (abs(s[i] - s[j]) == j - i) {
                    ++conflictos;
                }
            }
        }

        return conflictos;
    }

    SolucionNQ generarInicial() {
        SolucionNQ s(problema->n);
        iota(s.begin(), s.end(), 0);
        shuffle(s.begin(), s.end(), rng);
        return s;
    }

public:
    NQueensTabu(
        sref<ProblemContext> p,
        sref<FEvaluator<ESolutionNQ, MINIMIZE, ProblemContext>> e,
        int iteraciones = 10000,
        int tenencia = 10,
        unsigned semilla = 2026
    )
        : problema(p),
          evaluador(e),
          maxIteraciones(iteraciones),
          tenenciaTabu(tenencia),
          rng(semilla) {}

    SolucionNQ resolver() {
        const int n = problema->n;

        SolucionNQ actual = generarInicial();
        SolucionNQ mejor = actual;

        int valorActual = contarConflictos(actual);
        int mejorValor = valorActual;

        // tabuHasta[a][b] indica hasta que iteracion
        // esta prohibido intercambiar las columnas de filas a y b.
        vector<vector<int>> tabuHasta(
            n, vector<int>(n, 0)
        );

        for (int iter = 1; iter <= maxIteraciones; ++iter) {
            if (mejorValor == 0) {
                break;
            }

            bool encontrado = false;
            SolucionNQ mejorVecino;
            int mejorVecinoValor = 0;
            int filaA = -1;
            int filaB = -1;

            // Vecindario: todos los intercambios de dos filas.
            for (int a = 0; a < n; ++a) {
                for (int b = a + 1; b < n; ++b) {
                    SolucionNQ vecino = actual;
                    swap(vecino[a], vecino[b]);

                    int valor = contarConflictos(vecino);

                    bool esTabu = iter < tabuHasta[a][b];
                    bool aspiracion = valor < mejorValor;

                    if (esTabu && !aspiracion) {
                        continue;
                    }

                    if (!encontrado || valor < mejorVecinoValor) {
                        encontrado = true;
                        mejorVecino = move(vecino);
                        mejorVecinoValor = valor;
                        filaA = a;
                        filaB = b;
                    }
                }
            }

            if (!encontrado) {
                break;
            }

            actual = move(mejorVecino);
            valorActual = mejorVecinoValor;

            // Marca el intercambio como tabú.
            tabuHasta[filaA][filaB] = iter + tenenciaTabu;

            if (valorActual < mejorValor) {
                mejor = actual;
                mejorValor = valorActual;
            }

            if (iter % 1000 == 0) {
                cout << "Iteracion " << iter
                     << " | Conflictos actuales: " << valorActual
                     << " | Mejor: " << mejorValor << '\n';
            }
        }

        return mejor;
    }
};

int main() {
    const int N = 8;

    sref<ProblemContext> problema{
        new ProblemContext(N)
    };

    sref<FEvaluator<ESolutionNQ, MINIMIZE, ProblemContext>>
        evaluador{
            new FEvaluator<ESolutionNQ, MINIMIZE, ProblemContext>{
                problema, evaluar
            }
        };

    NQueensTabu algoritmo(
        problema,
        evaluador,
        10000, // iteraciones maximas
        10,    // tenencia tabu
        2026   // semilla aleatoria
    );

    cout << "=== N-DAMAS: TABU SEARCH ===\n";
    cout << "Tablero: " << N << " x " << N << '\n';

    SolucionNQ mejor = algoritmo.resolver();
    int conflictos = evaluar(problema, mejor).evaluation();

    cout << "\nSolucion: ";
    for (int columna : mejor) {
        cout << columna << ' ';
    }

    cout << "\nConflictos: " << conflictos << "\n\n";
    cout << "Tablero:\n";

    for (int fila = 0; fila < N; ++fila) {
        for (int columna = 0; columna < N; ++columna) {
            cout << (mejor[fila] == columna ? " D " : " . ");
        }
        cout << '\n';
    }

    if (conflictos == 0) {
        cout << "\nSe encontro una solucion valida.\n";
    } else {
        cout << "\nNo se encontro una solucion sin conflictos.\n";
    }

    return 0;
}
