
#include <algorithm>
#include <fstream>
#include <functional>
#include <iostream>
#include <random>
#include <string>
#include <vector>

using namespace std;

struct Grafo {
    int n;
    int k;
    vector<pair<int, int>> aristas;
    vector<vector<int>> vecinos;
};

struct Resultado {
    bool encontrado = false;
    vector<int> colores;
    unsigned long long nodosExplorados = 0;
};

Grafo generarGrafo(
    int n, int k, double probabilidad, unsigned semilla
) {
    mt19937 rng(semilla);
    uniform_real_distribution<double> prob(0.0, 1.0);

    // Coloreado oculto utilizado exclusivamente para
    // generar un grafo k-coloreable.
    vector<int> colorOculto(n);

    for (int i = 0; i < n; ++i)
        colorOculto[i] = i % k;

    shuffle(colorOculto.begin(), colorOculto.end(), rng);

    Grafo g{n, k, {}, vector<vector<int>>(n)};

    for (int u = 0; u < n; ++u) {
        for (int v = u + 1; v < n; ++v) {
            if (colorOculto[u] != colorOculto[v]
                && prob(rng) < probabilidad) {
                g.aristas.emplace_back(u, v);
                g.vecinos[u].push_back(v);
                g.vecinos[v].push_back(u);
            }
        }
    }

    return g;
}

void guardarGrafo(const Grafo& g, const string& archivo) {
    ofstream out(archivo);

    if (!out) {
        cerr << "Error al guardar " << archivo << '\n';
        return;
    }

    out << g.n << ' ' << g.aristas.size()
        << ' ' << g.k << '\n';

    for (auto [u, v] : g.aristas)
        out << u << ' ' << v << '\n';
}

void guardarSolucion(
    const Resultado& r,
    const string& archivo
) {
    ofstream out(archivo);

    if (!out) {
        cerr << "Error al guardar " << archivo << '\n';
        return;
    }

    out << "Solucion encontrada: "
        << (r.encontrado ? "si" : "no") << '\n';

    out << "Nodos explorados: "
        << r.nodosExplorados << '\n';

    if (r.encontrado) {
        for (int i = 0; i < static_cast<int>(r.colores.size()); ++i)
            out << i << ' ' << r.colores[i] << '\n';
    }
}

// Comprueba si un color es valido para el nodo.
bool colorValido(
    const Grafo& g,
    const vector<int>& colores,
    int nodo,
    int color
) {
    for (int vecino : g.vecinos[nodo]) {
        if (colores[vecino] == color)
            return false;
    }

    return true;
}

// Seleccion DSATUR: maximiza la cantidad de colores
// distintos usados por los vecinos ya coloreados.
// En caso de empate, elige el nodo de mayor grado.
int seleccionarNodo(
    const Grafo& g,
    const vector<int>& colores
) {
    int mejorNodo = -1;
    int mejorSaturacion = -1;
    int mejorGrado = -1;

    for (int u = 0; u < g.n; ++u) {
        if (colores[u] != -1)
            continue;

        vector<bool> coloresVecinos(g.k, false);
        int saturacion = 0;

        for (int v : g.vecinos[u]) {
            int c = colores[v];

            if (c != -1 && !coloresVecinos[c]) {
                coloresVecinos[c] = true;
                ++saturacion;
            }
        }

        int grado = static_cast<int>(g.vecinos[u].size());

        if (saturacion > mejorSaturacion ||
            (saturacion == mejorSaturacion &&
             grado > mejorGrado)) {
            mejorNodo = u;
            mejorSaturacion = saturacion;
            mejorGrado = grado;
        }
    }

    return mejorNodo;
}

Resultado backtracking(const Grafo& g) {
    Resultado resultado;
    resultado.colores.assign(g.n, -1);

    function<bool(int)> resolver = [&](int coloreados) -> bool {
        if (coloreados == g.n)
            return true;

        ++resultado.nodosExplorados;

        int nodo = seleccionarNodo(g, resultado.colores);

        for (int color = 0; color < g.k; ++color) {
            if (!colorValido(g, resultado.colores, nodo, color))
                continue;

            resultado.colores[nodo] = color;

            if (resolver(coloreados + 1))
                return true;

            // Backtracking: deshacer la asignacion.
            resultado.colores[nodo] = -1;
        }

        return false;
    };

    resultado.encontrado = resolver(0);

    if (!resultado.encontrado)
        resultado.colores.clear();

    return resultado;
}

void ejecutarInstancia(
    int n,
    int k,
    double probabilidad,
    unsigned semilla
) {
    cout << "\n============================\n";
    cout << "Instancia: " << n << " nodos, "
         << k << " colores\n";

    Grafo g = generarGrafo(n, k, probabilidad, semilla);

    string sufijo = to_string(n);

    guardarGrafo(g, "grafo_backtracking_" + sufijo + ".txt");

    cout << "Aristas: " << g.aristas.size() << '\n';
    cout << "Iniciando Backtracking con DSATUR...\n";

    Resultado resultado = backtracking(g);

    if (resultado.encontrado) {
        cout << "Coloreado valido encontrado.\n";
        cout << "Colores utilizados: " << k << '\n';
    } else {
        cout << "No existe un coloreado con "
             << k << " colores para este grafo.\n";
    }

    cout << "Nodos explorados: "
         << resultado.nodosExplorados << '\n';

    guardarSolucion(
        resultado,
        "solucion_backtracking_" + sufijo + ".txt"
    );

    cout << "Grafo: grafo_backtracking_"
         << sufijo << ".txt\n";

    cout << "Resultado: solucion_backtracking_"
         << sufijo << ".txt\n";
}

int main() {
    // Instancia de 50 nodos y 4 colores.
    ejecutarInstancia(
        50, 4, 0.20, 2026
    );

    // Instancia de 1000 nodos y 20 colores.
    ejecutarInstancia(
        1000, 20, 0.01, 2026
    );

    return 0;
}
