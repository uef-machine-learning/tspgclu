#!/usr/bin/env python

import numpy as np
import matplotlib.pyplot as plt

import tspg
x=np.loadtxt('data/s1_small.txt')


# the graph is reprecented as num_tsp different linear orderings between the data points
paths = tspg.create_graph(x,distance="l2",num_tsp=4)

points=x
plt.figure(figsize=(6, 6))
plt.scatter(points[:, 0], points[:, 1], marker='o', color='b')

for tsp in paths:
	for i in range(0,len(tsp)-1):
		# Draw line betwen consequtive points according the order in tsp  
		plt.plot([points[tsp[i],0], points[tsp[i+1],0]] , [points[tsp[i],1], points[tsp[i+1],1]], 'k-')

plt.title("The TSP graph")
plt.show()
