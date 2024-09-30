#define HAVE_IPASIR

#include "ipasir.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "my_maxsat_solver.h"

// Function to read and parse DIMACS file
void read_dimacs(FILE *file, void *solver, int *num_variables, int *num_clauses)
{
    char line[MAX_LINE_LENGTH];
    int blocking_vars = 0;

    while (fgets(line, sizeof(line), file))
    {
        // Ignore comment lines
        if (line[0] == 'c')
        {
            continue;
        }

        if (line[0] == 'p')
        {
            sscanf(line, "p cnf %d %d", num_variables, num_clauses);
            blocking_vars = *num_variables + 1;
        }

        // Reading clauses
        else
        {
            int literal;
            const char *ptr = line;
            while (sscanf(ptr, "%d", &literal) == 1)
            {

                if (literal == 0)
                {
                    // add blocking variable for each clause
                    ipasir_add(solver, -1 * blocking_vars);
                    ipasir_add(solver, 0);
                    
                    blocking_vars++;
                    break; // End of clause
                }
                else
                {
                    // Add literals to the current clause
                    ipasir_add(solver, literal);
                    
                }
                
                ptr = strchr(ptr, ' ') + 1;
            }
        }
    }
}

void naive_recurisve(void *solver, int start, int end, int k, int data[], int index)
{

    //end ouf subset
    if (index == k)
    {
        for (int i = 0; i < k; i++)
        {
            ipasir_add(solver, data[i]);
        }
        
        ipasir_add(solver, 0);
        return;
    }

    //end of variables or not enough left
    if (start > end || (end-start) < (k-1-index))
    {
        return;
    }

    //either include start or not, then call recursive
    data[index] = start;
    naive_recurisve(solver, start + 1, end, k, data, index + 1);

    naive_recurisve(solver, start + 1, end, k, data, index);
}

void enocde_at_most_k_unsat_naive(void *solver, int num_var, int num_cl, int k)
{
    printf("################## %d #################\n", k);
    
    //0 is special
    if (k == 0)
    {
        for (int i = num_var + 1; i <= num_var + num_cl; i++)
        {
            ipasir_add(solver, i);
            ipasir_add(solver, 0);
        }
        return;
    }

    //add all subsets of length k+1

    int data[k+1];

    naive_recurisve(solver, num_var + 1, num_var + num_cl, k + 1, data, 0);
}

void encode_at_most_k_unsat_serial_cnt(void *solver, int num_var, int num_cl, int k)
{
    printf("################## %d #################\n", k);
    
    //0 is special
    if (k == 0)
    {
        for (int i = num_var + 1; i <= num_var + num_cl; i++)
        {
            ipasir_add(solver, i);
            ipasir_add(solver, 0);
        }
        return;
    }

    int start_bv = num_var + 1;
    int aux_cnt = num_var + num_cl + 1;

    //mapping partial sums to numbers for sat-solver
    uint32_t s[num_cl][k];

    for (int i = 0; i < num_cl; i++)
    {
        for (int j = 0; j < k; j++)
        {
            s[i][j] = aux_cnt;
            aux_cnt++;
        }
    }

    //implement serial counter logic

    // (b_1 or s_1_1)
    ipasir_add(solver, start_bv);
    ipasir_add(solver, s[0][0]);
    ipasir_add(solver, 0);

    for (int j = 1; j < k; j++)
    {
        // (-s_1_j)
        ipasir_add(solver, -1 * s[1][j]);
        ipasir_add(solver, 0);
    }

    for (int i = 1; i < num_cl - 1; i++)
    {
        // (b_i or s_i_1)
        ipasir_add(solver, start_bv + i);
        ipasir_add(solver, s[i][1]);
        ipasir_add(solver, 0);

        // (-s_i-1_1 or s_i_1)
        ipasir_add(solver, -1 * s[i - 1][1]);
        ipasir_add(solver, s[i][1]);
        ipasir_add(solver, 0);

        for (int j = 1; j < k; j++)
        {
            // (b_i or -s_i-1_j-1 or s_i_j)
            ipasir_add(solver, start_bv + i);
            ipasir_add(solver, -1 * s[i - 1][j - 1]);
            ipasir_add(solver, s[i][j]);
            ipasir_add(solver, 0);

            // (-s_i-1_j or s_i_j)
            ipasir_add(solver, -1 * s[i - 1][j]);
            ipasir_add(solver, s[i][j]);
            ipasir_add(solver, 0);
        }

        // (b_i or -s_i-1_k)
        ipasir_add(solver, start_bv + i);
        ipasir_add(solver, -1 * s[i - 1][k - 1]);
        ipasir_add(solver, 0);
    }

    // (b_n or -s_n-1_k)
    ipasir_add(solver, num_var + num_cl);
    ipasir_add(solver, -1 * s[num_cl - 2][k - 1]);
    ipasir_add(solver, 0);

}

int *linear_unsat_to_sat(char *filepath, void (*enocde)(void *, int, int, int), int *solution, int *num_variables, int *num_clauses)
{
    int k = 0;

    while (1)
    {
        // Open the DIMACS file
        FILE *file = fopen(filepath, "r");
        if (file == NULL)
        {
            printf("Error opening file");
            return NULL;
        }

        // Initialize the solver
        void *solver = ipasir_init();

        int num_var = 0;
        int num_cl = 0;

        // Read the DIMACS file and feed the clauses to the solver
        read_dimacs(file, solver, &num_var, &num_cl);

        //encode cardinality constraint 
        (*enocde)(solver, num_var, num_cl, k);

        // Solve the SAT problem
        int result = ipasir_solve(solver);

        if (result == 10)
        {
            //SAT
            //We are done
            printf("SAT\n");

            // retrieve the solution (variable assignments)
            int *model = malloc(num_var * sizeof(int));
            *num_variables = num_var;
            *num_clauses = num_cl;
            *solution = num_cl - k;

            for (int i = 1; i <= num_var; i++)
            {
                model[i - 1] = ipasir_val(solver, i);
            }


            ipasir_release(solver);
            fclose(file);
            return model;
        }
        else if (result == 20)
        {
            //UNSAT
            //try next k
            printf("UNSAT\n");
            k++;
        }
        else
        {
            printf("Solver returned unknown result: %d\n", result);
        }

        // Release the solver
        ipasir_release(solver);
        fclose(file);
    }
    return NULL;
}

int *linear_sat_to_unsat(char *filepath, void (*enocde)(void *, int, int, int), int *solution, int *num_variables, int *num_clauses)
{
    int *bestModel;
    int cost = INT32_MAX;

    // Initialize the solver
    void *solver = ipasir_init();

    // Open the DIMACS file
    FILE *file = fopen(filepath, "r");
    if (file == NULL)
    {
        printf("Error opening file");
        return NULL;
    }

    while (1)
    {
        int num_var = 0;
        int num_cl = 0;

        // Read the DIMACS file and feed the clauses to the solver
        read_dimacs(file, solver, &num_var, &num_cl);

        // first time mallo bestmodel
        if (cost == INT32_MAX)
        {
            bestModel = malloc(num_var * sizeof(int));
            cost = num_cl;
        }

        // Solve the SAT problem
        int result = ipasir_solve(solver);

        if (result == 20)
        {
            //UNSAT
            //we are done
            printf("UNSAT\n");

            *num_variables = num_var;
            *num_clauses = num_cl;
            *solution = cost;

            ipasir_release(solver);
            fclose(file);
            return bestModel;
        }
        else if (result != 10)
        {
            printf("Solver returned unknown result: %d\n", result);
            return NULL;
        }

        //SAT
        //try next lower k
        printf("SAT\n");

        // upadte bestmodel
        for (int i = 1; i <= num_var; i++)
        {
            bestModel[i - 1] = ipasir_val(solver, i);
        }

        cost = 0;
        // calc cost
        for (int i = num_var + 1; i <= num_var + num_cl; i++)
        {
            if (ipasir_val(solver, i) > 0)
                cost++;
        }

        // reset solver, close file
        ipasir_release(solver);
        rewind(file);
        solver = ipasir_init();

        // encode the cardinality constraints
        (*enocde)(solver, num_var, num_cl, num_cl - cost - 1);
    }
    return NULL;
}

int check_solution(char *filepath, maxsat_ret_t ret)
{
    // Open the DIMACS file
    FILE *file = fopen(filepath, "r");
    if (file == NULL)
    {
        perror("Error opening file");
        return -1;
    }

    char line[MAX_LINE_LENGTH];
    int cnt = 1;
    int *model = ret.model;
    int max_unsat_clauses = (ret.num_clauses - ret.solution);

    while (fgets(line, sizeof(line), file))
    {
        // Ignore comment lines and header
        if (line[0] == 'c' || line[0] == 'p')
        {
            continue;
        }

        // check clauses
        else
        {
            int literal;
            const char *ptr = line;
            while (sscanf(ptr, "%d", &literal) == 1)
            {
                if (literal == 0)
                {
                    //end of clause, either error or we found one of k
                    if (max_unsat_clauses == 0)
                    {
                        printf("error in clause %d\n", cnt);
                        fclose(file);
                        return -1;
                    }
                    max_unsat_clauses--;
                    cnt++;
                    break;
                }
                else if (model[abs(literal) - 1] == literal)
                {
                    //clause is satisfied continue
                    cnt++;
                    break;
                    ;
                }
                // Move to the next literal
                ptr = strchr(ptr, ' ') + 1;
            }
        }
    }
    fclose(file);
    return 0;
}

maxsat_ret_t maxsat_solve(char *filepath, config_t config){

    int * model;
    int solution = 0;
    int num_variables = 0;
    int num_clauses = 0;

    maxsat_ret_t ret = {NULL,0,0,0};

    switch (config)
    {
    case SAT_UNSAT_NAIVE:
        model = linear_unsat_to_sat(filepath, enocde_at_most_k_unsat_naive, &solution, &num_variables, &num_clauses);
        ret.model = model;
        ret.solution = solution;
        ret.num_clauses = num_clauses;
        ret.num_variables = num_variables;
        return ret;
        break;

    case SAT_UNSAT_CNT:
        model = linear_unsat_to_sat(filepath, encode_at_most_k_unsat_serial_cnt, &solution, &num_variables, &num_clauses);
        ret.model = model;
        ret.solution = solution;
        ret.num_clauses = num_clauses;
        ret.num_variables = num_variables;
        return ret;
        break;

    case UNSAT_SAT_NAIVE:
        model = linear_sat_to_unsat(filepath, enocde_at_most_k_unsat_naive, &solution, &num_variables, &num_clauses);
        ret.model = model;
        ret.solution = solution;
        ret.num_clauses = num_clauses;
        ret.num_variables = num_variables;
        return ret;
        break;

    case UNSAT_SAT_CNT:
        model = linear_sat_to_unsat(filepath, encode_at_most_k_unsat_serial_cnt, &solution, &num_variables, &num_clauses);
        ret.model = model;
        ret.solution = solution;
        ret.num_clauses = num_clauses;
        ret.num_variables = num_variables;
        return ret;
        break;
    
    default:
        return ret;
        break;
    }

}