# bhcd: Bayesian Hierarchical Community Discovery
[![PyPI](https://img.shields.io/pypi/v/bhcd.svg)](https://pypi.org/project/bhcd)

An efficient Bayesian nonparametric model for discovering hierarchical community structure in social networks. 

## Parameter Tuning
There are five parameters (alpha, beta, lambda, delta, gamma) to be tuned, lines within interval (0,1) and satisifies the following
constraint.

alpha > beta
lambda > delta

## Usage

Python wrapper. 
You can run `python bhcd.py` to get the hierachical tree.


