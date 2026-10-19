
#include <algorithm>
#include <cmath>
#include <deque>
#include <iomanip>
#include <iostream>
#include <random>
#include <utility>

using namespace std;

using Point = pair<double, double>;

constexpr double MIN_X = -512.0;
constexpr double MAX_X =  512.0;

// Evalua la funcion Eggholder.
double eggholder(double x, double y) {
    return -(y + 47.0) *
               sin(sqrt(abs(x / 2.0 + y + 47.0)))
           - x *
               sin(sqrt(abs(x - (y + 47.0))));
}

// Mantiene las coordenadas dentro del dominio.
double limitar(double value) {
    return clamp(value, MIN_X, MAX_X);
}

// Representacion discretizada para comparar puntos
// dentro de la lista tabú.
pair<int, int> claveTabu(const Point& p) {
    return {
        static_cast<int>(round(p.first)),
        static_cast<int>(round(p.second))
    };
}

int main() {
    // Generador aleatorio.
    random_device rd;
    mt19937 gen(rd());

    uniform_real_distribution<double> inicial(
        MIN_X, MAX_X
    );

    uniform_real_distribution<double> perturbacion(
        -30.0, 30.0
    );

    // Parametros.
    const int MAX_ITERACIONES = 10000;
    const int TAM_VECINDAD = 50;
    const int TENENCIA_TABU = 30;

    // Solucion inicial.
    Point actual{inicial(gen), inicial(gen)};
    Point mejor = actual;

    double valorActual =
        eggholder(actual.first, actual.second);

    double valorMejor = valorActual;

    // Memoria tabú FIFO.
    deque<pair<int, int>> listaTabu;

    listaTabu.push_back(claveTabu(actual));

    for (int iter = 0;
         iter < MAX_ITERACIONES;
         ++iter) {

        Point mejorVecino = actual;
        double valorMejorVecino = INFINITY;
        bool encontrado = false;

        // Explorar candidatos de la vecindad.
        for (int i = 0; i < TAM_VECINDAD; ++i) {
            Point vecino{
                limitar(actual.first + perturbacion(gen)),
                limitar(actual.second + perturbacion(gen))
            };

            double valor =
                eggholder(vecino.first, vecino.second);

            auto clave = claveTabu(vecino);

            bool esTabu =
                find(listaTabu.begin(),
                     listaTabu.end(),
                     clave) != listaTabu.end();

            // Criterio de aspiracion:
            // aceptar un tabu si mejora el mejor global.
            bool aspiracion = valor < valorMejor;

            if (esTabu && !aspiracion) {
                continue;
            }

            if (!encontrado || valor < valorMejorVecino) {
                mejorVecino = vecino;
                valorMejorVecino = valor;
                encontrado = true;
            }
        }

        // Si todos los vecinos fueron tabú,
        // reiniciar en un punto aleatorio.
        if (!encontrado) {
            actual = {inicial(gen), inicial(gen)};
            valorActual =
                eggholder(actual.first, actual.second);
            continue;
        }

        // Moverse al mejor vecino admisible.
        actual = mejorVecino;
        valorActual = valorMejorVecino;

        // Actualizar el mejor global.
        if (valorActual < valorMejor) {
            mejor = actual;
            valorMejor = valorActual;
        }

        // Registrar la solucion en la memoria tabú.
        listaTabu.push_back(claveTabu(actual));

        if (static_cast<int>(listaTabu.size()) >
            TENENCIA_TABU) {
            listaTabu.pop_front();
        }

        if ((iter + 1) % 1000 == 0) {
            cout << "Iteracion " << iter + 1
                 << " | Mejor f = "
                 << setprecision(10)
                 << valorMejor << '\n';
        }
    }

    cout << "\n===== RESULTADO FINAL =====\n";
    cout << setprecision(10);
    cout << "x = " << mejor.first << '\n';
    cout << "y = " << mejor.second << '\n';
    cout << "f(x,y) = " << valorMejor << '\n';

    return 0;
}
