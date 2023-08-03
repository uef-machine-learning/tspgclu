
# TSPg

The TSPg software implements the following algorithm that approximates Ward's agglomerative clustering:

S. Sieranoja, and P. Fränti, "Tsp-Graph for Agglomerative Clustering", https://papers.ssrn.com/sol3/papers.cfm?abstract_id=4328176

The software is provided with GNU Lesser General Public License, Version 3. https://www.gnu.org/licenses/lgpl.html. All files except those under data and contrib -folders are subject to this license. See LICENSE.txt.

Contact: samisi@cs.uef.fi

Please, let me know how the software works for you if you try it out.

# Python interface

## Install & test
```
git clone -b dev https://github.com/uef-machine-learning/tspgclu.git
cd tspgclu
pip install -r requirements.txt
pip install .
python/api_example.py
```

## Examples
See python/api_example.py

# Commandline interface
## Compile

```
make clean
make
```

## Use cases

### Numerical (vectorial) data
Cluster vectorial (numerical) S1 dataset (data/s1.txt) to 15 clusters:
```
./tspg data/s1.txt --type vec -T 10  -C 15 --algo tspgclu -o tmp/s1part.txt  --cfn tmp/centroids.txt
```
