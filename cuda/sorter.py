import numpy as np

sin_ordenar = np.fromfile('sin_ordenar', dtype=np.int32)
ordenado = np.fromfile('ordenada', dtype=np.int32)

ordenado_python = np.sort(sin_ordenar)

print("CHECK: ", np.array_equal(ordenado, ordenado_python))

