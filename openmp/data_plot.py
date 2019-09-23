import sys
import numpy as np
import matplotlib.pyplot as plt

n_thd = np.log2(np.array([1, 2, 4, 6, 8, 12, 16, 32, 64]))
ext_thd = np.swapaxes(np.tile(n_thd, (30,1)), 0, 1)

data_array = np.genfromtxt(sys.argv[1], delimiter=',')
mean = np.mean(data_array, axis=1)
std = np.std(data_array, axis=1)
# plt.errorbar(np.log2(n_thd), mean, yerr=std, fmt='--o', capsize=10)
plt.scatter(ext_thd, data_array, marker='x')
plt.plot(n_thd, mean, '--')
plt.show()
