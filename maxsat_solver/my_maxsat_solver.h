#ifndef MY_MAXSAT_SOLVER_H
#define MY_MAXSAT_SOLVER_H

#define MAX_LINE_LENGTH (100)

//enum for choosing the algorithm and encoding
typedef enum {
    UNSAT_SAT_NAIVE,
    UNSAT_SAT_CNT,
    SAT_UNSAT_NAIVE,
    SAT_UNSAT_CNT,
} config_t;

//return struct model has to be free by the caller
typedef struct maxsat_ret {
    int * model;
    int solution;
    int num_variables;
    int num_clauses;
} maxsat_ret_t;

/**
 * This function calls the main function of the solver. Depending on config it calls the correct function, with the correct encoding
 */
maxsat_ret_t maxsat_solve(char *filepath, config_t config);

/**
 * This function checks, if a model returned by maxsat_solve really satisfied as many clauses of the dimacs file filepath as it claims in ret.
 */
int check_solution(char *filepath, maxsat_ret_t ret);



/*
 * This two functions are the two diffent algorithms implemented to solve the maxsat problem. 
 * filepath is the path to the input DIMACS file
 * encode is a pointer to a cardinality encoding function. (like the ones below)
 * solution, num_variables, num_clauses are output parameters. 
 */
int *linear_unsat_to_sat(char *filepath, void (*enocde)(void *, int, int, int), int *solution, int *num_variables, int * num_clauses);
int * linear_sat_to_unsat(char *filepath, void (*enocde)(void *, int, int, int), int *solution, int *num_variables, int * num_clauses);

/**
 * This two functions implement two diffrent ways to encode cardinality constraints which say that the number of unsatisfied clauses is at most k
 * The functions add the encdoed constraint to the solver.
 */
void enocde_at_most_k_unsat_naive(void *solver, int num_var, int num_cl, int k);
void encode_at_most_k_unsat_serial_cnt(void *solver, int num_var, int num_cl, int k);

#endif