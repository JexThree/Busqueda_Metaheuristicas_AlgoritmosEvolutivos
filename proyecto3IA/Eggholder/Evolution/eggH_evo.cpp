#define NO_CXX_MODULES 1

#include <cmath>
#include <iostream>
#include <utility>
#include <vector>
#include <algorithm>
#include <chrono>
#include <iomanip>

#include <OptFrame/printable/printable.hpp>
#include <OptFCore/FCore.hpp>
#include <OptFCore/FCoreAll.hpp>
#include <OptFrame/Core.hpp>
#include <OptFrame/Heuristics.hpp>
#include <OptFrame/Heuristics/EA/RK/BRKGA.hpp>
#include <OptFrame/Search/InitialPopulation.hpp>

using namespace optframe;
using namespace std;

// Representacion de una solucion: (x, y).
using Point = pair<double, double>;

// Solucion junto con su evaluacion.
using ESolutionEggholder =
    pair<Point, Evaluation<double>>;

// Contexto del problema.
class ProblemContext {};

// Funcion objetivo Eggholder.
double eggholder(double x, double y) {
    const double term1 =
        -(y + 47.0) *
        sin(sqrt(abs(x / 2.0 + y + 47.0)));

    const double term2 =
        -x *
        sin(sqrt(abs(x - (y + 47.0))));

    return term1 + term2;
}

// Evaluador compatible con OptFrame.
Evaluation<double> evaluateEggholder(
    sref<ProblemContext>,
    const Point& point
) {
    return Evaluation<double>{
        eggholder(point.first, point.second)
    };
}

// Generador de poblacion inicial de claves aleatorias.
class EggholderInitialPopulation
    : public InitialEPopulation<
          pair<vector<double>, Evaluation<double>>> {

    sref<RandGen> rng;

public:
    explicit EggholderInitialPopulation(
        sref<RandGen> generator
    ) : rng{generator} {}

    bool canEvaluate() const override {
        return false;
    }

    VEPopulation<
        pair<vector<double>, Evaluation<double>>
    > generateEPopulation(
        unsigned populationSize,
        double
    ) override {

        VEPopulation<
            pair<vector<double>, Evaluation<double>>
        > population;

        for (unsigned i = 0; i < populationSize; ++i) {
            vector<double> keys(2);

            for (double& key : keys) {
                key = (rng->rand() % 100000) / 100000.0;
            }

            population.push_back({
                keys,
                Evaluation<double>{}
            });
        }

        return population;
    }
};

// Decodifica dos claves [0,1] en coordenadas [-512,512].
pair<Evaluation<double>, Point> decodeEggholder(
    sref<Evaluator<
        Point,
        Evaluation<double>,
        ESolutionEggholder
    >> evaluator,
    const vector<double>& keys
) {
    const double x = -512.0 + 1024.0 * keys.at(0);
    const double y = -512.0 + 1024.0 * keys.at(1);

    Point point{x, y};

    Evaluation<double> value =
        evaluator->evaluate(point);

    return {value, point};
}

int main() {
    sref<RandGen> rng = new RandGen;

    // Crear el contexto y el evaluador.
    sref<ProblemContext> problem =
        new ProblemContext{};

    sref<FEvaluator<
        ESolutionEggholder,
        MINIMIZE,
        ProblemContext
    >> evaluator =
        new FEvaluator<
            ESolutionEggholder,
            MINIMIZE,
            ProblemContext
        >{problem, evaluateEggholder};

    // Configurar el decodificador.
    sref<DecoderRandomKeys<ESolutionEggholder, double>>
        decoder =
        new FDecoderEvalRK<ESolutionEggholder, double>{
            evaluator,
            decodeEggholder
        };

    // Parametros del algoritmo evolutivo.
    const unsigned populationSize = 100;
    const unsigned generations = 500;

    // BRKGA:
    // decodificador, poblacion inicial, generaciones,
    // tamano poblacion, fraccion elite, fraccion mutante,
    // probabilidad de herencia elite, generador aleatorio.
    BRKGA<ESolutionEggholder, double> algorithm(
        decoder,
        EggholderInitialPopulation(rng),
        generations,
        populationSize,
        0.20,
        0.10,
        0.70,
        rng
    );

    
    // Ejecutar BRKGA y medir el tiempo transcurrido.
    const auto inicio = chrono::steady_clock::now();

    // Límite de tiempo de 1 hora; el algoritmo también
    // está configurado para ejecutar hasta 500 generaciones.
    auto result = algorithm.search(3600.0);

    const auto fin = chrono::steady_clock::now();

    const double tiempoSegundos =
        chrono::duration<double>(fin - inicio).count();

    const long long tiempoMicrosegundos =
        chrono::duration_cast<chrono::microseconds>(
            fin - inicio
        ).count();

    if (!result.best) {
        cerr << "No se encontro una solucion.\n";
        return 1;
    }

    const ESolutionEggholder& best = *result.best;

    cout << fixed << setprecision(6);
    cout << "\n=== RESULTADOS BRKGA - EGGHOLDER ===\n";
    cout << "Poblacion: " << populationSize << '\n';
    cout << "Generaciones maximas configuradas: "
         << generations << '\n';

    cout << "Tiempo transcurrido: "
         << tiempoSegundos << " segundos\n";
    cout << "Tiempo transcurrido: "
         << tiempoMicrosegundos << " microsegundos\n";

    cout << "\nMejor solucion encontrada\n";
    cout << "x = " << best.first.first << '\n';
    cout << "y = " << best.first.second << '\n';
    cout << "f(x,y) = " << best.second.evaluation() << '\n';

    return 0;
}
