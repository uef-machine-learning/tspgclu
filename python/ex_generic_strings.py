#!/usr/bin/python3

import numpy as np
import matplotlib.pyplot as plt
import random

from tspg import tspg_generic

#Only needed in case of string distance:
# pip install rapidfuzz
from rapidfuzz.distance import Levenshtein

class EditDistance:
	def __init__(self,x):
		self.x = x
		self.size = len(x)
	def distance(self,a,b):
		d = Levenshtein.distance(self.x[a],self.x[b])
		d = float(d*d)
		return d
		
# Takes around 113 seconds for a 2D dataset size 100k:
# x=np.loadtxt('data/b2.txt')
# example_generic(x,100)

def example_generic_strings(x,numclu):
	dist = EditDistance(x)
	labels = tspg_generic(dist,numclu,num_tsp=5)
	# breakpoint()
	# print(labels)
	clu = {}
	keys = np.unique(labels)
	print("Five samples from each of the clusters:")
	for i in keys: clu[i] = []
	for i in range(0,len(x)): clu[labels[i]].append(i)
	for i in keys:
		kl = random.sample(clu[i],5)
		print("clu=%d: "%(i),end="")
		for j in kl:
			print(x[j],end=" ")
		print("")
		
infn = "data/birkbeckU.txt"
f = open(infn, "r")
x = f.read().splitlines()
example_generic_strings(x,50)



