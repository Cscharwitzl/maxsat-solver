This project implements a maxsat solver. The purpose is to learn how maxsat solvers work by implementing a very basic approach.

The structure and the makefiles are based on https://github.com/biotomas/ipasir. The copyright and license required can be found in LINCENSE.
 
My implementation can be found in the subfolder ```maxsat_solver```. In the ```experiments``` folder is the c file which uses the maxsat solver in all configurations and measures the execution time and checks if the result was correct.

```sat``` folder has the source code of minisat and picosat. When calling make all two executables are compiled, each using a different sat solver.

```testcases``` folder has dimacs files. Not all of the maxsat solver configurations can solve all the testcases. Some testcases are to complicated. When you want to try such a testcase, you first have to comment out the corresponding maxsat_solve call in experiments.c - I know this is not optimal :(

You call the experiments like this:
``` bin\experiments-<sat-solver> <dimacs-file> <optimal-solution> ```

The maxsat solver implements two linear solving algorithms:
    1.) sarts with assuming all clauses can be satisfied (UNSAT -> SAT)
    2.) starts with assuming all the clauses are not satisfied (SAT -> UNSAT)

Both algorithms are not programmed using an incremental sat-solver approach.

There are two cardinality encodings available:
    1.) A naive approach which explicitly excludes all possible combinations
    2.) A serial counter approach

The cardinality encodings are based on the following document by Carsten Sinz:
Sinz, C. (2005). Towards an Optimal CNF Encoding of Boolean Cardinality Constraints. In: van Beek, P. (eds) Principles and Practice of Constraint Programming - CP 2005. CP 2005. Lecture Notes in Computer Science, vol 3709. Springer, Berlin, Heidelberg. https://doi.org/10.1007/11564751_73