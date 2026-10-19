# Búsqueda, metaheurísticas y algoritmos evolutivos.

En el siguiente proyecto se esta haciendo uso del framework OptFrame para implementar los algoritmos:
1. BackTracking.
2. Búsqueda Tabú.
3. Estrategia Evolutiva.

Enfocado en los siguientes problemas:
1. Problema de las N-Damas.
2. Problema de la mochila knapsack 0/1.
3. Problema del coloreado de grafos (Graph Coloring)
4. Función Eggholder.

Se utilizarán determinados algoritmos para resolver cada uno de los problemas. Esto se lleva a cabo con el fin de comparar cada uno de los algoritmos y determinar cual de todos ellos tienen un mejor desempeño.

## Ejemplos de compilación.
Para llevar a cabo la compilación adecuadamente es necesario entrar a la carpeta /proyecto3IA/<Problema de resolver>/<Algoritmo a compilar>

### Problema de las N-Damas.
1. Mediante BackTracking: g++ -std=c++20 -O2 -Wall -Wextra nDamas_BT.cpp -o nDamas_BT
2. Mediante Tabú: g++ -std=c++20 -O2 -Wall -Wextra   -I "<Carpeta del proyecto>/optframe/include"   nDamas_tabu.cpp -o nDamas_tabu
3. Mediante algoritmo genético: g++ -std=c++20 -O2 -Wall -Wextra -I "<Carpeta del proyecto>/optframe/include" nDamas_evo.cpp -o nDamas_evo

### Problema de la mochila knapsack 0/1.
1. Mediante BackTracking: g++ -std=c++20 -O2 -Wall -Wextra knapsack_BT.cpp -o knapsack_BT
2. Mediante Tabú: g++ -std=c++20 -O2 -Wall -Wextra -I "<Carpeta del proyecto>/optframe/include" knapsack_tabu.cpp -o knapsack_tabu
3. Mediante algoritmo genético: g++ -std=c++20 -O2 -Wall -Wextra -I "<Carpeta del proyecto>/optframe/include" knapsack_evo.cpp -o knapsack_evo

### Problema del coloreado de grafos (Graph Coloring)
1. Mediante BackTracking: g++ -std=c++20 -O2 -Wall -Wextra graphos_BT.cpp -o graphos_BT
2. Mediante Tabú: g++ -std=c++20 -O2 -Wall -Wextra -I "<Carpeta del proyecto>/optframe/include" graphos_tabu.cpp -o graphos_tabu
3. Mediante algoritmo genético: g++ -std=c++20 -O2 -Wall -Wextra -I "<Carpeta del proyecto>/optframe/include" graphos_evo.cpp -o graphos_evo

### Función Eggholder.
2. Mediante Tabú: g++ -std=c++20 -O2 eggH_tabu.cpp -o eggH_tabu
3. Mediante algoritmo genético: g++ -std=c++20 -O2 -I "<Carpeta de proyecto>/optframe/include" eggH_evo.cpp -o eggH_evo

## Ejemplos de ejecución.
Para ejecutar los archivos se tiene que colocar en la dirección en la que estos hayan sido compilados.

### Problema de las N-Damas.
1. Mediante BackTracking: ./nDamas_BT
2. Mediante Tabú: ./nDamas_tabu
3. Mediante algoritmo genético: ./nDamas_evo

### Problema de la mochila knapsack 0/1.
1. Mediante BackTracking: ./knapsack_BT
2. Mediante Tabú: ./knapsack_tabu
3. Mediante algoritmo genético: ./knapsack_evo

### Problema del coloreado de grafos (Graph Coloring)
1. Mediante BackTracking: ./graphos_BT
2. Mediante Tabú: ./graphos_tabu
3. Mediante algoritmo genético: ./graphos_evo

### Función Eggholder.
2. Mediante Tabú: ./eggH_tabu
3. Mediante algoritmo genético: ./eggH_evo