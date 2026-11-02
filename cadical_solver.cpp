
#include "cadical_solver.hpp"
#include "cadical.hpp"

bool solveWithCadical(const std::vector<std::vector<int>>& formula, std::vector<int>& assignment) {
    CaDiCaL::Solver solver;

    solver.set("factorcheck", 0); //Disables the explicit variable declaration check in CaDiCaL build version.
    
    //Add each CNF clause to CaDiCaL.
    for (const auto& clause : formula) {
        for (int literal : clause) {
            solver.add(literal);
        }

        //Zero marks the end of a clause.
        solver.add(0);
    }

    //CaDiCaL returns 10 for SAT and 20 for UNSAT.
    int result = solver.solve();

    if (result != 10) {
        return false;
    }

    //Copy the satisfying assignment into our vector.
    for (int variable = 1; variable < static_cast<int>(assignment.size()); ++variable) {

        int value = solver.val(variable);

        if (value > 0) {
            assignment[variable] = 1;
        } else if (value < 0) {
            assignment[variable] = 0;
        }
    }

    return true;
}
