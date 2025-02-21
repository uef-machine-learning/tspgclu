#!/usr/bin/env python
import numpy as np
import matplotlib.pyplot as plt

import tspg

def show_clusters_2d(x,labels,numclu):
	colormap = plt.cm.gist_ncar
	colorst = [colormap(i) for i in np.linspace(0, 0.9,numclu)]
	# print(colorst)
	u_labels = np.unique(labels)
	for i in u_labels:
		plt.scatter(x[labels == i , 0] , x[labels == i , 1] , label = i, color = colorst[i-1])
	plt.show()

# Fast version using built in distance functions written in C:
def example_vec(ds,numclu):
	# For higher quality:
	#  - increase number of tsp paths (num_tsp), (in range [2,100])
	labels,mergeOrder = tspg.tspg(ds,numclu,distance="l2",num_tsp=5,dtype="vec")
	show_clusters_2d(ds,labels,numclu)

x=np.loadtxt('data/s1.txt')

example_vec(x,15)

