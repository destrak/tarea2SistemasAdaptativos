# Iterative Local Search (ILS) — Maximum Independent Set Problem (MISP)

> **Autor:** Nicolàs Jarpa- Nicolàs Pina 
> **Institución:** Universidad del Bío-Bío — Ingeniería Civil Química  
> **Proyecto:** Metaheurísticas aplicadas al problema del Conjunto Independiente Máximo  
> **Año:** 2025  

---

## Descripción General
Este proyecto implementa la metaheurística **Iterated Local Search (ILS)** para resolver el **problema del Conjunto Independiente Máximo (Maximum Independent Set Problem, MISP)**.  
El programa lee instancias `.graph`, ejecuta la búsqueda dentro de un tiempo límite definido y muestra las mejoras y el mejor resultado final encontrado.

---

## Requisitos

-  **Sistema Operativo:** Linux 
-  **Compilador:** C++20 o superior (`g++`, `clang++`)
-  **Formato de entrada:** archivo `.graph` con el siguiente formato:

```text
n
u v
u v
...

Donde:

    n → número total de vértices.

    Cada línea posterior representa una arista no dirigida entre los vértices u y v (indexados desde 0).

Ejemplo de instancia

6
0 1  
1 2  
2 4
3 4  
4 5

Compilación

Ejecuta el siguiente comando en tu terminal:

g++ -O3 -std=c++20 -Wall -Wextra -o IterativeLocalSearch IterativeLocalSearch.cpp

Esto generará el binario ejecutable IterativeLocalSearch en el mismo directorio.
Ejecución

Formato general:

./IterativeLocalSearch ILS -i <archivo.graph> -t <tiempoSegundos> \
  --seed <semilla> --alpha <valorRCL> --perturb <k> --ls <iteracionesLS> --verbose <nivel>

Parámetros principales
Parámetro	Descripción	Ejemplo
ILS	Metaheurística a ejecutar (solo ILS implementado).	ILS
-i	Ruta al archivo .graph o - para leer desde STDIN.	-i new_3000_dataset/erdos_n3000_p0c0.1_1.graph
-t	Tiempo máximo de ejecución (segundos).	-t 10
--seed	Semilla aleatoria para reproducibilidad.	--seed 1
--alpha	Factor de aleatoriedad de la construcción inicial (0–1).	--alpha 0.50
--perturb	Intensidad de la perturbación.	--perturb 3
--ls	Iteraciones máximas de búsqueda local.	--ls 4000
--verbose	Nivel de detalle de salida (0 = mínimo, 1 = informativo).	--verbose 1
Ejemplo de uso

./IterativeLocalSearch ILS -i new_3000_dataset/erdos_n3000_p0c0.1_1.graph \
  -t 10 --seed 1 --alpha 0.50 --perturb 3 --ls 4000 --verbose 1

Salida esperada

# Vertices: 3000  Edges: 450000
BEST 115 TIME 0.000421
BEST 118 TIME 0.002034
FINAL_BEST 118 FOUND_AT 0.002034

Interpretación de la salida
Etiqueta	Significado
BEST <val> TIME <t>	Se encontró una mejor solución de tamaño <val> en <t> segundos.
FINAL_BEST <val> FOUND_AT <t>	Mejor solución final obtenida y el instante en que se encontró.
# Vertices, # Edges	(Solo con --verbose 1) muestra el tamaño del grafo leído.
 Recomendaciones

     Cambiar la semilla (--seed) permite ejecutar variantes reproducibles.

     Aumentar --perturb o --ls mejora la exploración pero incrementa el tiempo de cómputo.

     Para guardar los resultados en un archivo:

./IterativeLocalSearch ILS -i instancia.graph -t 10 > resultados.txt

    Para ejecutar múltiples instancias automáticamente, se puede usar un script bash que recorra el dataset y registre FINAL_BEST y FOUND_AT en un CSV.

Ejemplo de estructura de proyecto

IterativeLocalSearch-MISP/
├── IterativeLocalSearch.cpp
├── README.md
├── ejemplo.graph
├── new_3000_dataset/
│   ├── erdos_n3000_p0c0.1_1.graph
│   ├── erdos_n3000_p0c0.2_1.graph
│   └── ...
└── scripts/
    └── run_experiments.sh

Autor

Nicolàs Jarpa-Nicolàs Pina
Estudiantes de Ingeniería Civil Informatica– Universidad de Concepciòn



Concepción, Chile
Licencia

Este proyecto se distribuye bajo licencia MIT.
Puedes usarlo, modificarlo y distribuirlo libremente citando al autor original.
