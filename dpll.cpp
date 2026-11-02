#include "dpll.hpp"
#include <cstdlib>


int evaluateLiteral(int literal, const std::vector<int>& assignment) {

    //Get the variable number without its sign.
    int variable = std::abs(literal);

    //Find its current assignment.
    int value = assignment[variable];

    //If it hasn't been assigned, return -1.
    if (value == -1) {
        return -1;
    }

    //Positive literal: use the variable's value (e.g: x2).
    if (literal > 0) {
        return value;
    }

    //Negative literal: reverse the truth value (e.g: -x3).
    return 1 - value;
}


int evaluateClause(const std::vector<int>& clause, const std::vector<int>& assignment) {

    bool unassigned = false;

    for (int literal : clause) {

        int value = evaluateLiteral(literal, assignment);

        if (value == 1) {
            return 1;
        }

        if (value == -1) { 
            unassigned = true; //Don't return yet because another literal might be true.
        }
    }

    if (unassigned) {
        return -1; //Indicates clause is undetermined.
    }

    return 0;
}



bool unitPropagate(const std::vector<std::vector<int>>& formula, std::vector<int>& assignment) {

    bool changed = true;

    while (changed) { //While loop makes sure we keep checking until no more assignments can be made.
                      //(Chaning a variable in one clause can affect other clauses).

        changed = false;

        for (const auto& clause : formula) {

            int unassignedCount = 0; // 0 means contradiction
                                     // 1 means forced assignment (unit propogation)
                                     // 2+ means can't determine a value yet


            int lastUnassigned = 0;

            bool satisfied = false;

            for (int literal : clause) {

                int value = evaluateLiteral(literal, assignment);

                if (value == 1) {
                    satisfied = true;
                    break;
                }

                if (value == -1) {
                    unassignedCount++;
                    lastUnassigned = literal;
                }
            }

            // This clause is already satisfied
            if (satisfied) {
                continue;
            }

            // Every literal is false: contradiction
            if (unassignedCount == 0) {
                return false;
            }

            // Only one literal remains unassigned
            if (unassignedCount == 1) {

                int variable = std::abs(lastUnassigned);

                if (lastUnassigned > 0) {
                    assignment[variable] = 1;
                }
                else {
                    assignment[variable] = 0;
                }

                changed = true;
            }
        }
    }

    return true;
}



bool isFormulaSatisfied(const std::vector<std::vector<int>>& formula, const std::vector<int>& assignment) {

    for (const auto& clause : formula) {

        if (evaluateClause(clause, assignment) != 1) {
            return false;
        }
    }

    return true;
}



int chooseVariable(const std::vector<int>& assignment) {

    for (int i = 1; i < static_cast<int>(assignment.size()); i++) { //Starts at index 1 in assignments vector.

        if (assignment[i] == -1) {
            return i; //Returns variable number.
        }
    }

    return 0; //Returns 0 if unable to find an unassigned one.
}



bool dpll(const std::vector<std::vector<int>>& formula, std::vector<int>& assignment) {

    //Make any assignments that are forced and if it finds a contradiction (hence the ! / NOT) return false.
    if (!unitPropagate(formula, assignment)) {
        return false;
    }

    // Check if every clause is satisfied
    if (isFormulaSatisfied(formula, assignment)) {
        return true;
    }



    //Find a variable that hasn't been assigned (make a guess).
    //For my implementation, we will choose the lowest numbered unnasigned variable.
    int variable = chooseVariable(assignment);

    if (variable == 0) {
        return false;
    }

    //First try setting the variable to true.
    std::vector<int> trueBranch = assignment;
    trueBranch[variable] = 1;

    if (dpll(formula, trueBranch)) { //Recursive function, uses true branch.
        assignment = trueBranch;
        return true;
    }

    //If that failed try setting the variable to false.
    std::vector<int> falseBranch = assignment;
    falseBranch[variable] = 0;

    if (dpll(formula, falseBranch)) { //Recursive function, uses false branch.
        assignment = falseBranch;
        return true;
    }

    //Both choices led to contradictions, hence all outcomes are covered.
    return false;
}
