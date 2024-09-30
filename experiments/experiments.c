#include "../maxsat_solver/my_maxsat_solver.h"
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include "ipasir.h"

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("Usage: %s <dimacs-file> <optimal-solution>\n", argv[0]);
        return 1;
    }

    int optimal_sol = atoi(argv[2]);
    struct timeval start, end;
    double exectime[4] = {0};
    maxsat_ret_t result;

    //maxsat config 1: algo: UNSAT->SAT encode: naive
    gettimeofday(&start, NULL);
    result = maxsat_solve(argv[1], UNSAT_SAT_NAIVE);
    gettimeofday(&end, NULL);

    if (check_solution(argv[1], result) < 0 || optimal_sol != result.solution)
    {
        printf("wrong solution %d excpected %d\n",result.solution,optimal_sol);
        return 1;
    }

    exectime[0] = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;


    //maxsat config 1: algo: UNSAT->SAT encode: serial counter
    gettimeofday(&start, NULL);
    result = maxsat_solve(argv[1], UNSAT_SAT_CNT);
    gettimeofday(&end, NULL);

    if (check_solution(argv[1], result) < 0 || optimal_sol != result.solution)
    {
        printf("wrong solution %d excpected %d\n",result.solution,optimal_sol);
        return 1;
    }

    exectime[1] = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;

    //maxsat config 1: algo: SAT->UNSAT encode: naive
    gettimeofday(&start, NULL);
    result = maxsat_solve(argv[1], SAT_UNSAT_NAIVE);
    gettimeofday(&end, NULL);

    if (check_solution(argv[1], result) < 0 || optimal_sol != result.solution)
    {
        printf("wrong solution %d excpected %d\n",result.solution,optimal_sol);
        return 1;
    }
  

    exectime[2] = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;

    //maxsat config 1: algo: SAT->UNSAT encode: serail counter
    gettimeofday(&start, NULL);
    result = maxsat_solve(argv[1], SAT_UNSAT_CNT);
    gettimeofday(&end, NULL);

    if (check_solution(argv[1], result) < 0 || optimal_sol != result.solution)
    {
        printf("wrong solution %d excpected %d\n",result.solution,optimal_sol);
        return 1;
    }

    exectime[3] = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;

    printf("$$$$$$$$$$$$$$$$$$$ exec-Times: $$$$$$$$$$$$$$$$$$$\n");
    printf("exec-Times:\n");
    printf("Lin UNSAT->SAT (naive): %fs\n", exectime[0]);
    printf("Lin UNSAT->SAT (serial counter): %fs\n", exectime[1]);
    printf("Lin SAT->UNSAT (naive): %fs\n", exectime[2]);
    printf("Lin SAT->UNSAT (serial counter): %fs\n", exectime[3]);

    free(result.model);

    return 0;
}