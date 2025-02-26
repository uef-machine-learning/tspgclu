#!/usr/bin/env python

import numpy as np
import matplotlib.pyplot as plt
import tspg
x=np.loadtxt('data/s1_small.txt')

# the graph is reprecented as num_tsp different linear orderings between the data x
paths = tspg.create_graph(x.tolist(),distance="l2",num_tsp=4)

plt.figure(figsize=(6, 6))
plt.scatter(x[:, 0], x[:, 1], marker='o', color='b')

for tsp in paths:
	for i in range(0,len(tsp)-1):
		# Draw line betwen consequtive x according the order in tsp  
		plt.plot([x[tsp[i],0], x[tsp[i+1],0]] , [x[tsp[i],1], x[tsp[i+1],1]], 'k-')

plt.title("The TSP graph")
plt.show()
