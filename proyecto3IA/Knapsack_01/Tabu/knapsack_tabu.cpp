
#define NO_CXX_MODULES 1

#include <OptFrame/printable/printable.hpp>
#include <OptFCore/FCore.hpp>
#include <OptFrame/Core.hpp>

#include <algorithm>
#include <iostream>
#include <random>
#include <vector>

using namespace std;
using namespace optframe;

struct Objeto {
    int peso;
    int valor;
};

struct Solucion {
    vector<int> x;
    int peso = 0;
    int valor = 0;
};

class MochilaTabu {
private:
    vector<Objeto> objetos;
    int capacidad;
    int poblacion;
    int generaciones;
    int tenenciaTabu;
    mt19937 rng;

    void evaluar(Solucion& s) const {
        s.peso = 0;
        s.valor = 0;

        for (int i = 0; i < (int)objetos.size(); ++i) {
            if (s.x[i]) {
                s.peso += objetos[i].peso;
                s.valor += objetos[i].valor;
            }
        }
    }

    bool factible(const Solucion& s) const {
        return s.peso <= capacidad;
    }

    Solucion construirInicial() {
        const int n = (int)objetos.size();

        vector<int> orden(n);
        for (int i = 0; i < n; ++i)
            orden[i] = i;

        shuffle(orden.begin(), orden.end(), rng);

        Solucion s;
        s.x.assign(n, 0);

        for (int i : orden) {
            uniform_int_distribution<int> moneda(0, 1);

            if (moneda(rng)) {
                s.x[i] = 1;
                evaluar(s);

                if (!factible(s)) {
                    s.x[i] = 0;
                    evaluar(s);
                }
            }
        }

        return s;
    }

public:
    MochilaTabu(
        vector<Objeto> objetos_,
        int capacidad_,
        int poblacion_ = 50,
        int generaciones_ = 300,
        int tenenciaTabu_ = 7,
        unsigned semilla = 2026
    )
        : objetos(move(objetos_)),
          capacidad(capacidad_),
          poblacion(poblacion_),
          generaciones(generaciones_),
          tenenciaTabu(tenenciaTabu_),
          rng(semilla) {}

    Solucion resolver() {
        const int n = (int)objetos.size();

        // Generar población inicial de soluciones factibles.
        vector<Solucion> individuos;
        individuos.reserve(poblacion);

        Solucion mejorGlobal;
        mejorGlobal.x.assign(n, 0);
        evaluar(mejorGlobal);

        for (int i = 0; i < poblacion; ++i) {
            Solucion s = construirInicial();
            individuos.push_back(s);

            if (s.valor > mejorGlobal.valor)
                mejorGlobal = s;
        }

        // Elegir la mejor solución como punto de partida.
        Solucion actual = mejorGlobal;

        // tabuHasta[i] indica cuándo deja de estar prohibido
        // cambiar la selección del objeto i.
        vector<int> tabuHasta(n, 0);

        for (int gen = 1; gen <= generaciones; ++gen) {
            bool encontrado = false;
            Solucion mejorVecino;
            int movimientoElegido = -1;

            // Explorar vecinos cambiando un objeto.
            for (int i = 0; i < n; ++i) {
                Solucion vecino = actual;

                vecino.x[i] = 1 - vecino.x[i];
                evaluar(vecino);

                if (!factible(vecino))
                    continue;

                const bool esTabu = gen < tabuHasta[i];

                // Aspiración: permitir un movimiento tabú si
                // mejora la mejor solución global.
                const bool aspiracion =
                    vecino.valor > mejorGlobal.valor;

                if (esTabu && !aspiracion)
                    continue;

                if (!encontrado ||
                    vecino.valor > mejorVecino.valor) {
                    encontrado = true;
                    mejorVecino = vecino;
                    movimientoElegido = i;
                }
            }

            if (!encontrado) {
                // Recuperar una solución de la población
                // para intentar continuar la búsqueda.
                if (individuos.empty())
                    break;

                uniform_int_distribution<int> elegir(
                    0, (int)individuos.size() - 1
                );

                actual = individuos[elegir(rng)];
                fill(tabuHasta.begin(), tabuHasta.end(), 0);
                continue;
            }

            actual = mejorVecino;

            tabuHasta[movimientoElegido] =
                gen + tenenciaTabu + 1;

            if (actual.valor > mejorGlobal.valor)
                mejorGlobal = actual;
        }

        return mejorGlobal;
    }
};

int main() {
    vector<Objeto> objetos = {
        {2, 3},
        {3, 4},
        {4, 5},
        {5, 6}
    };

    const int capacidad = 5;
    const int numeroObjetos = 4;
    const int poblacion = 50;
    const int generaciones = 300;

    MochilaTabu algoritmo(
        objetos,
        capacidad,
        poblacion,
        generaciones,
        7,      // Tenencia tabú
        2026    // Semilla aleatoria
    );

    Solucion mejor = algoritmo.resolver();

    cout << "=== KNAPSACK 0/1 - TABU SEARCH ===\n";
    cout << "Capacidad: " << capacidad << "\n";
    cout << "Objetos: " << numeroObjetos << "\n";
    cout << "Poblacion: " << poblacion << "\n";
    cout << "Generaciones: " << generaciones << "\n";

    cout << "Objetos seleccionados:\n";

    for (int i = 0; i < (int)objetos.size(); ++i) {
        if (mejor.x[i]) {
            cout << "  Objeto " << i
                 << " | peso = " << objetos[i].peso
                 << " | valor = " << objetos[i].valor << "\n";
        }
    }

    cout << "Peso total: " << mejor.peso
         << " / " << capacidad << "\n";
    cout << "Valor total: " << mejor.valor << "\n";

    return 0;
}
