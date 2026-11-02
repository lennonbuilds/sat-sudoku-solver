#ifndef CADICAL_SOLVER_HPP
#define CADICAL_SOLVER_HPP

#include <vector>

//This function uses the CaDiCaL SAT solver to solve a sudoku puzzle.
bool solveWithCadical(const std::vector<std::vector<int>>& formula, std::vector<int>& assignment);
#endif
