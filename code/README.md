# Documentación

## Entrega

La entrega se realiza vía **aula.usm.cl** en formato `.zip`.

## Multiplicación de matrices

### Programa principal
- Este programa lo que hace es implementar dos algoritmos distintos de multiplicacion de matrices el algoritmo Naive y el algoritmo Strassen, lo primero que hace es leer las matrices de entrada de la carpeta matrix_input donde estas vienen de a pares y lo siguiente es ejecutar cada algoritmos para su respectivo par de matrices, guarda estos resultados en matrix_output diferenciados en el nombre de sus .txt con naive.out o strassen.out respectivamente, se registra a su vez una medicion del tiempo y el uso de memoria estimado y se guarda en un .csv dentro de la carpeta measurements.
### Scripts
- El codigo del algoritmo naive fue sacado de la pagina geeks for geeks y para cada fila de la primera matriz y columna de la segunda calcula su producto punto de la fila por la columna.

- El codigo del algoritmo strassen fue sacado de la pagina geeks for geeks y divide las matrices en sub-bloques calculando 7 productos/particiones combinando sumas y restas de los sub-bloques y reconstruye la matriz resultado con estas combinaciones.

- Para un buen funcionamiento de todo esto se realizo un makefile donde se tiene que ejecutar en la consola lo siguiente:

1. make
2. make run
3. make plots
## Ordenamiento de arreglo unidimensional

Algoritmos: MergeSort, QuickSort, std::sort.

### Programa principal
- Este programa implemente distintos tipos de sort solicitados en la tarea, recorre los .txt de array_input y los ejecuta registrando tiempo y memoria.
### Scripts
- Merge: Divide en mitades, ordena de forma recursiva y luego fusiona.
- Quick: Usando el pivote como el elemento medio particiona y ordena de forma recursiva.
- Sort: Este es el sort de la stl.

- Para un buen funcionamiento de todo esto se realizo un makefile donde se tiene que ejecutar en la consola lo siguiente:

1. make
2. make run
3. make plots