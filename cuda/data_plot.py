import matplotlib
import matplotlib.pyplot as plt
import numpy as np
import sys


labels = ['64k', '128k']

global_means = [
    np.mean(np.genfromtxt(sys.argv[1])),
    np.mean(np.genfromtxt(sys.argv[2])),
]

shared_means = [
    np.mean(np.genfromtxt(sys.argv[3])),
    np.mean(np.genfromtxt(sys.argv[4])),
]

x = np.arange(len(labels))  # the label locations
width = 0.35  # the width of the bars

fig, ax = plt.subplots()
rects1 = ax.bar(x - width/2, global_means, width, label='Global')
rects2 = ax.bar(x + width/2, shared_means, width, label='Shared')

# Add some text for labels, title and custom x-axis tick labels, etc.
ax.set_ylabel('Tiempo [Segundos]')
ax.set_xlabel('Longitud de la lista')
ax.set_title('Comparacion entre memoria global y compartida')
ax.set_xticks(x)
ax.set_xticklabels(labels)
ax.legend()


def autolabel(rects):
    """Attach a text label above each bar in *rects*, displaying its height."""
    for rect in rects:
        height = rect.get_height()
        ax.annotate('{0:.2f}'.format(height),
                    xy=(rect.get_x() + rect.get_width() / 2, height),
                    xytext=(0, 3),  # 3 points vertical offset
                    textcoords="offset points",
                    ha='center', va='bottom')


autolabel(rects1)
autolabel(rects2)

fig.tight_layout()

plt.show()
