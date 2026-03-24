## Con esto creamos los datasets que utilizaremos para el experimento 

from sklearn.datasets import make_blobs
import numpy as np

sizes = [100000, 200000, 300000, 400000, 600000, 800000, 1000000] ## definimos todos los tamaños que queremos para los datasets
k = 8 ## definimos el numero de clusters (utilizamos este porque es el del notebook del profesor)

for dims in [2, 3]:
    for n_points in sizes:
        points, _ = make_blobs(
            n_samples=n_points,
            centers=k,
            n_features=dims,
            cluster_std=0.04,
            random_state=7,
            center_box=(0, 1.0)
        )

        points = np.round(np.abs(points), 3)
        filename = f"{n_points}_data_{dims}d.csv" #Los nombramos según su tamaño y la dimension 
        np.savetxt(filename, points, delimiter=",", fmt="%.3f")
        print(f"Generado: {filename}")