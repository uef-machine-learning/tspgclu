#!/usr/bin/python3

import numpy as np
import matplotlib.pyplot as plt
import random

from tspg import tspg_generic

def show_clusters_2d(x,labels,numclu):
	colormap = plt.cm.gist_ncar
	colorst = [colormap(i) for i in np.linspace(0, 0.9,numclu)]
	# print(colorst)
	
	u_labels = np.unique(labels)
	
	for i in u_labels:
		plt.scatter(x[labels == i , 0] , x[labels == i , 1] , label = i, color = colorst[i-1])
	plt.show()


# Slower version using distance function provided by python:
# Can work with any kind of distance.
# Recommended only when no suitable distance function implemented in C++.
# Takes around two minutes for 100k (2D) dataset

# Class implementing distance measure needs to have following properties:
# - attribute 'size' that reflects the number of data objects
# - function distance(a,b) where parameters a,b are integers between 0..(size-1)
# - distance(a,b) must return a float value
# Name of class is arbitrary
# See examples DistanceMeasureL2 and EditDistance

class DistanceMeasureL2:
	def __init__(self,x):
		self.x = x
		self.size = len(x)
	def distance(self,a,b):
		dist = np.linalg.norm(self.x[a]-self.x[b])
		return dist
		
def example_generic(x,numclu):
	dist = DistanceMeasureL2(x)
	labels = tspg_generic(dist,numclu,num_tsp=5)
	# print(labels)
	show_clusters_2d(x,labels,numclu)
	
x=np.loadtxt('data/s1.txt')
example_generic(x,15)

# Takes around 113 seconds for a 2D dataset size 100k:
# x=np.loadtxt('data/b2.txt')
# example_generic(x,100)




