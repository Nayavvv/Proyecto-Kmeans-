Proyecto K-means 

Este proyecto tiene el objetivo de implementar el algoritmo de K-means mediante dos versiones diferentes: una serial y una paralela usando OpenMP. El programa desarrollado recibe como entrada un archivo CSV con puntos aleatorios en dos o tres dimensiones, y genera como salida un archivo CSV con la asignación de clúster correspondiente a cada punto.
Además de la implementación del algoritmo, se realizó una evaluación experimental para comparar el desempeño de ambas versiones en distintos tamaños de entrada y distintas configuraciones de hilos. A partir de estos experimentos se calcularon tiempos promedio, Speedup y eficiencia, con el fin de analizar el comportamiento de la versión paralela respecto a la versión serial.
