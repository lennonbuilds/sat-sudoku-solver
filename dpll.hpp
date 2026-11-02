#ifndef DPLL_HPP
#define DPLL_HPP

#include <vector>

//Returns 1 if true, 0 if false, -1 if unassigned.
int evaluateLiteral(int literal, const std::vector<int>& assignment);

//Returns 1 if the clause is true, 0 if false, -1 if undetermined.
int evaluateClause(const std::vector<int>& clause, const std::vector<int>& assignment);

//Makes assignments that are forced by unit clauses (unit propagation).
//Returns false if a contradiction is found.
bool unitPropagate(const std::vector<std::vector<int>>& formula, std::vector<int>& assignment); //CHANGES assignment vector therefore not constant

//Checks if the entire formula is satisfied (every clause must be true).
bool isFormulaSatisfied(const std::vector<std::vector<int>>& formula, const std::vector<int>& assignment);

//DPLL chooses a variable to assign.
int chooseVariable(const std::vector<int>& assignment);

//Returns a true/false value for whether a satisfying assignment was found.
bool dpll(const std::vector<std::vector<int>>& formula,std::vector<int>& assignment);
#endif


