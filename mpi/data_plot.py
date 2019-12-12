import matplotlib
import matplotlib.pyplot as plt
import numpy as np
import sys


labels = ['Simple', 'O3', 'OpenMp + O3']
sizes = [400,800,1200,1600,2000]

block_means = np.array([
    np.mean(np.genfromtxt(sys.argv[1], delimiter=','), axis=1),
    np.mean(np.genfromtxt(sys.argv[2], delimiter=','), axis=1),
    np.mean(np.genfromtxt(sys.argv[3], delimiter=','), axis=1),
])

nonblock_means = np.array([
    np.mean(np.genfromtxt(sys.argv[4], delimiter=','), axis=1),
    np.mean(np.genfromtxt(sys.argv[5], delimiter=','), axis=1),
    np.mean(np.genfromtxt(sys.argv[6], delimiter=','), axis=1),
])

x = np.arange(len(labels))  # the label locations
width = 0.35  # the width of the bars

fig, axs = plt.subplots(1, 3, sharey=True)
fig.suptitle('Comparacion entre version bloqueante y no bloqueante')
for ax, i in zip(axs, (2, 3, 4)):
    rects1 = ax.bar(x - width/2, block_means[:,i], width, label='Block')
    rects2 = ax.bar(x + width/2, nonblock_means[:,i], width, label='Nonblock')

    # Add some text for labels, title and custom x-axis tick labels, etc.
    ax.set_ylabel('Tiempo [Segundos]')
    ax.set_xlabel('Opciones de Compilacion')
    ax.set_title('Tamaño matriz: ' + str(sizes[i]) + 'x' + str(sizes[i]))
    ax.set_xticks(x)
    ax.set_xticklabels(labels)
    ax.legend()


    def autolabel(rects):
        """Attach a text label above each bar in *rects*, displaying its height."""
        for rect in rects:
            height = rect.get_height()
            ax.annotate(
                '{0:.2f}'.format(height),
                xy=(rect.get_x() + rect.get_width() / 2, height),
                xytext=(0, 3),  # 3 points vertical offset
                textcoords="offset points",
                ha='center', va='bottom')


    autolabel(rects1)
    autolabel(rects2)

# fig.tight_layout()

plt.show()
