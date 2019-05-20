'''this wrapper runs the bhcd executable, with gml files as argument input and read the output tree files
'''
import os
import pdb

import networkx as nx
from ete3 import Tree

from bhcd import BHCD

TUNING_FILE = 'bhcd-tuning.gml'            
if __name__ == '__main__':
    st = open('ground_truth.txt').read()
    ground_truth_tree = Tree(st)    
    G = nx.read_gml(os.path.join('build', TUNING_FILE))
    alg = BHCD()
    delta_list = [0.5,1,2,3,4]
    lambda_list = [0.1,0.2,0.3,0.4]
    for delta in delta_list:
        for _lambda in lambda_list:
            alg._delta = delta
            alg._lambda = _lambda
            alg.fit(G)
            res = alg.tree.compare(ground_truth_tree, unrooted=True)
            ground_truth_tree = alg.tree.copy()
            print("==============", delta, _lambda, res['norm_rf'])
