
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <vector>

using namespace std;

struct Objeto {
    int id;
    double peso;
    double valor;

    double ratio() const {
        return valor / peso;
    }
};

struct Resultado {
    double valorMaximo = 0.0;
    double pesoTotal = 0.0;
    vector<int> objetosSeleccionados;
    unsigned long long nodosExplorados = 0;
};

class MochilaBacktracking {
private:
    vector<Objeto> objetos;
    double capacidad;

    double mejorValor = 0.0;
    double mejorPeso = 0.0;

    vector<int> seleccionActual;
    vector<int> mejorSeleccion;

    unsigned long long nodosExplorados = 0;

    // Cota optimista: estima el mejor valor posible
    // permitiendo fraccionar los objetos restantes.
    double calcularCota(
        int indice,
        double pesoActual,
        double valorActual
    ) const {
        if (pesoActual > capacidad)
            return 0.0;

        double pesoDisponible = capacidad - pesoActual;
        double cota = valorActual;

        for (int i = indice;
             i < static_cast<int>(objetos.size());
             ++i) {

            const Objeto& obj = objetos[i];

            if (obj.peso <= pesoDisponible) {
                pesoDisponible -= obj.peso;
                cota += obj.valor;
            } else {
                // Se toma solo una fraccion para construir
                // una cota superior, no una solucion real.
                cota += obj.ratio() * pesoDisponible;
                break;
            }
        }

        return cota;
    }

    void buscar(
        int indice,
        double pesoActual,
        double valorActual
    ) {
        ++nodosExplorados;

        // Registrar la mejor solucion encontrada.
        if (valorActual > mejorValor) {
            mejorValor = valorActual;
            mejorPeso = pesoActual;
            mejorSeleccion = seleccionActual;
        }

        // Todos los objetos ya fueron considerados.
        if (indice >= static_cast<int>(objetos.size()))
            return;

        // Poda: ni siquiera la cota optimista mejora
        // la mejor solucion conocida.
        double cota = calcularCota(
            indice, pesoActual, valorActual
        );

        if (cota <= mejorValor)
            return;

        const Objeto& obj = objetos[indice];

        // Rama 1: incluir el objeto si cabe.
        if (pesoActual + obj.peso <= capacidad) {
            seleccionActual.push_back(obj.id);

            buscar(
                indice + 1,
                pesoActual + obj.peso,
                valorActual + obj.valor
            );

            seleccionActual.pop_back();
        }

        // Rama 2: excluir el objeto.
        buscar(indice + 1, pesoActual, valorActual);
    }

public:
    MochilaBacktracking(
        vector<Objeto> datos,
        double capacidadMochila
    ) : objetos(move(datos)),
        capacidad(capacidadMochila) {

        // La poda por cota fraccionaria requiere
        // procesar los objetos por valor/peso descendente.
        sort(
            objetos.begin(),
            objetos.end(),
            [](const Objeto& a, const Objeto& b) {
                return a.ratio() > b.ratio();
            }
        );
    }

    Resultado resolver() {
        mejorValor = 0.0;
        mejorPeso = 0.0;
        nodosExplorados = 0;

        seleccionActual.clear();
        mejorSeleccion.clear();

        buscar(0, 0.0, 0.0);

        return {
            mejorValor,
            mejorPeso,
            mejorSeleccion,
            nodosExplorados
        };
    }
};

int main() {
    // Instancia de ejemplo: mochila con capacidad 5.
    vector<Objeto> objetos = {
        {0, 2, 3},
        {1, 3, 4},
        {2, 4, 5},
        {3, 5, 6}
    };

    const double capacidad = 5.0;

    MochilaBacktracking algoritmo(objetos, capacidad);
    Resultado resultado = algoritmo.resolver();

    cout << fixed << setprecision(2);

    cout << "===== MOCHILA 0/1 - BACKTRACKING =====\n";
    cout << "Capacidad: " << capacidad << '\n';
    cout << "Valor maximo: " << resultado.valorMaximo << '\n';
    cout << "Peso total: " << resultado.pesoTotal << '\n';

    cout << "Objetos seleccionados:\n";

    for (int id : resultado.objetosSeleccionados) {
        auto it = find_if(
            objetos.begin(),
            objetos.end(),
            [id](const Objeto& obj) {
                return obj.id == id;
            }
        );

        if (it != objetos.end()) {
            cout << "  Objeto " << id
                 << " | Peso: " << it->peso
                 << " | Valor: " << it->valor << '\n';
        }
    }

    cout << "Nodos explorados: "
         << resultado.nodosExplorados << '\n';

    return 0;
}
