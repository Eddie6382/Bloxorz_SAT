# SAT Solver to Solve Bloxorz Game

![alt text](https://otfried.org/courses/cs109/bloxorz4.png)

## Description
Convert a "single-used tile" [1] Bloxorz game into a CNF formula, then use MiniSAT to check whether there exist a path from start node to the goal hole
## Usage
```
   make 
   ./bloxorz <input map file>
```
## Map generator
reference: [Cellular Automata Method for Generating Random Cave-Like Levels](http://roguebasin.com/?title=Cellular_Automata_Method_for_Generating_Random_Cave-Like_Levels)

It adopt cellular automata to generate cave-like structures. First fille the map randomly. The basic rule is, a tile becomes a wall if it was a wall and 4 or more of its eight neighbors were walls, or if it was not a wall and 5 or more neighbors were. Put more succinctly, a tile is a wall if the 3x3 region centered on it contained at least 5 walls

**generate map**  
To randomly generate a MxN map, with p% area being occupied, r1_cutoff, r2_cutoff, r iterations (the more iterations, the less noise), type
```
cd input
./mapGen M N r1_cutoff r2_cutoff r
```
```1``` represents tile, ```0``` represent boundary, for strat node (```2```) and end node ```3```, you need to add them in manual. For example, ```m1.in```.  
```
011110
110311
110111
010111
110111
000211
```

## Result
**Test cases**  
Maps are stores in ```./input``` directory, ```1``` represents tiles, ```2``` represents start node, ```3``` represents goal hole, ```0``` represents boundaries.

**Output results**  
Results are stored in ```./output``` directory

**Example**  
If it is SAT, it will also print out a possible paths and actions you should adapt   

Result of ```m1.in```
```
map boundary (_m, _n) = (6, 6), size = 26
starting index = (5, 3)
ending index   = (1, 3)

nodes size     = 78
moves size     = 312
unary var size = 2028

=====  Begin to solve  =====
==============================[MINISAT]===============================
| Conflicts |     ORIGINAL     |          LEARNT          | Progress |
|           | Clauses Literals | Clauses Literals  Lit/Cl |          |
======================================================================
|         6 |   21270    45150 |       3        8     2.7 |  0.000 % |
======================================================================
SAT

state U_5_3_S_0
                        step 0 : UP
state U_5_4_Ly_1
                        step 1 : LEFT
state U_4_4_Ly_2
                        step 2 : LEFT
state U_3_4_Ly_3
                        step 3 : LEFT
state U_2_4_Ly_4
                        step 4 : LEFT
state U_1_4_Ly_5
                        step 5 : DOWN
state  U_1_3_S_6

Total memory usage: 5980
Total time usage: 0.036443 s
```

## Reference
[1] Van Der Zanden, Tom C., and Hans L. Bodlaender. "PSPACE-completeness of Bloxorz and of games with 2-buttons." arXiv preprint arXiv:1411.5951 (2014).  
[2] Costa, Diogo M. "Computational complexity of games and puzzles." arXiv preprint arXiv:1807.04724 (2018).  
[3] Zhou, Neng-Fa. "In pursuit of an efficient SAT encoding for the Hamiltonian cycle problem." International Conference on Principles and Practice of Constraint Programming. Springer, Cham, 2020