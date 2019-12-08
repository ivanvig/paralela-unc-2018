import sys
import numpy as np
import matplotlib.pyplot as plt

n_depth = np.arange(10)
ext_depth = np.swapaxes(np.tile(n_depth, (30,1)), 0, 1)

data_array = np.genfromtxt(sys.argv[1], delimiter=',')
mean = np.mean(data_array, axis=1)
std = np.std(data_array, axis=1)
# plt.errorbar(np.log2(n_thd), mean, yerr=std, fmt='--o', capsize=10)
plt.scatter(ext_depth, data_array, marker='x')
plt.xlabel("Log10(DEPTH)")
plt.ylabel("Tiempo [segundos]")
plt.title("DEPTH vs TIEMPO")
plt.grid()
plt.tight_layout()
plt.plot(n_depth, mean, '--')
plt.show()
