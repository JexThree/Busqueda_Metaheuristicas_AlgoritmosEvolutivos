
#include <iostream>
#include <vector>

using namespace std;

class NReinas {
private:
    int N;

    // posicion[fila] = columna donde se coloca la dama.
    vector<int> posicion;

    // Indica si una columna ya esta ocupada.
    vector<bool> columnaOcupada;

    // Diagonales identificadas por fila - columna.
    vector<bool> diagonalPrincipal;

    // Diagonales identificadas por fila + columna.
    vector<bool> diagonalSecundaria;

    unsigned long long nodosExplorados = 0;

    bool backtracking(int fila) {
        ++nodosExplorados;

        // Se colocaron todas las damas.
        if (fila == N) {
            return true;
        }

        // Intentar colocar una dama en cada columna.
        for (int columna = 0; columna < N; ++columna) {
            int diagonal1 = fila - columna + N - 1;
            int diagonal2 = fila + columna;

            // Verificar si la posicion es segura.
            if (columnaOcupada[columna] ||
                diagonalPrincipal[diagonal1] ||
                diagonalSecundaria[diagonal2]) {
                continue;
            }

            // Colocar la dama.
            posicion[fila] = columna;
            columnaOcupada[columna] = true;
            diagonalPrincipal[diagonal1] = true;
            diagonalSecundaria[diagonal2] = true;

            // Intentar resolver las filas siguientes.
            if (backtracking(fila + 1)) {
                return true;
            }

            // Retroceder: retirar la dama y liberar recursos.
            posicion[fila] = -1;
            columnaOcupada[columna] = false;
            diagonalPrincipal[diagonal1] = false;
            diagonalSecundaria[diagonal2] = false;
        }

        // Ninguna columna funciona en esta fila.
        return false;
    }

public:
    explicit NReinas(int n)
        : N(n),
          posicion(n, -1),
          columnaOcupada(n, false),
          diagonalPrincipal(2 * n - 1, false),
          diagonalSecundaria(2 * n - 1, false) {}

    bool resolver() {
        nodosExplorados = 0;

        if (N <= 0) {
            return false;
        }

        return backtracking(0);
    }

    void mostrarTablero() const {
        cout << "\nTablero encontrado:\n\n";

        for (int fila = 0; fila < N; ++fila) {
            for (int columna = 0; columna < N; ++columna) {
                if (posicion[fila] == columna) {
                    cout << " D ";
                } else {
                    cout << " . ";
                }
            }
            cout << '\n';
        }
    }

    void mostrarSolucion() const {
        cout << "\nVector de solucion (fila -> columna):\n";

        for (int fila = 0; fila < N; ++fila) {
            cout << "Fila " << fila
                 << " -> columna " << posicion[fila] << '\n';
        }
    }

    unsigned long long obtenerNodosExplorados() const {
        return nodosExplorados;
    }
};

int main() {
    int N;

    cout << "=== N-DAMAS MEDIANTE BACKTRACKING ===\n";
    cout << "Introduce el numero de damas: ";
    cin >> N;

    if (!cin || N <= 0) {
        cerr << "Error: N debe ser un entero positivo.\n";
        return 1;
    }

    NReinas problema(N);

    if (problema.resolver()) {
        cout << "\nSe encontro una solucion valida.\n";
        problema.mostrarTablero();
        problema.mostrarSolucion();
    } else {
        cout << "\nNo se encontro una solucion.\n";
        cout << "Para N = 2 y N = 3 no existen soluciones.\n";
    }

    cout << "\nNodos explorados: "
         << problema.obtenerNodosExplorados() << '\n';

    return 0;
}
