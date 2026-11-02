
#include <iostream>
#include <vector>
#include <chrono>
#include <iomanip>
#include <algorithm>
#include <numeric>

#include "sudoku.hpp"
#include "dpll.hpp"
#include "cadical_solver.hpp"
#include "cadical.hpp"

//Measures time using a steady clock.
using Clock = std::chrono::steady_clock;

//Converts two timestamps into milliseconds.
double elapsedMilliseconds(Clock::time_point start, Clock::time_point end) {
    std::chrono::duration<double, std::milli> elapsed = end - start;
    return elapsed.count();
}

//Calculate the average of a set of measurements.
double calculateAverage(const std::vector<double>& times) {
    double total = std::accumulate(times.begin(), times.end(), 0.0);
    return total / times.size();
}

//Calculate the median of a set of measurements.
double calculateMedian(std::vector<double> times) {
    std::sort(times.begin(), times.end());

    int n = static_cast<int>(times.size());

    if (n % 2 == 0) {
        return (times[n / 2 - 1] + times[n / 2]) / 2.0;
    }

    return times[n / 2];
}

// Print the results for one solver.
void printResults(const std::string& name, const std::vector<double>& solveTimes, const std::vector<double>& totalTimes) {
    std::cout << name << "\n";
    std::cout << "  Solver-call average: " << calculateAverage(solveTimes) << " ms\n";
    std::cout << "  Solver-call median:  " << calculateMedian(solveTimes) << " ms\n";
    std::cout << "  End-to-end average:  " << calculateAverage(totalTimes) << " ms\n";
    std::cout << "  End-to-end median:   " << calculateMedian(totalTimes) << " ms\n\n";
}

int main() {

    //Insert sudoku hint here:


    std::vector<std::vector<int>> puzzle = {
        {8,0,0,0,0,0,0,0,0},
        {0,0,3,6,0,0,0,0,0},
        {0,7,0,0,9,0,2,0,0},
        {0,5,0,0,0,7,0,0,0},
        {0,0,0,0,4,5,7,0,0},
        {0,0,0,1,0,0,0,3,0},
        {0,0,1,0,0,0,0,6,8},
        {0,0,8,5,0,0,0,1,0},
        {0,9,0,0,0,0,4,0,0}
    };



    //Build the Sudoku CNF formula once.
    //CNF generation is EXCLUDED from both benchmarks.

    std::vector<std::vector<int>> formula;

    addCellConstraints(formula);
    addCellUniquenessConstraints(formula);
    addRowConstraints(formula);
    addColumnConstraints(formula);
    addBoxConstraints(formula);
    addPuzzleClues(formula, puzzle);

    const int runs = 100; //Runs 100 times

    std::vector<double> dpllSolveTimes;
    std::vector<double> dpllTotalTimes;

    std::vector<double> cadicalSolveTimes;
    std::vector<double> cadicalTotalTimes;

    //Reserve memory to avoid repeated vector reallocations.
    dpllSolveTimes.reserve(runs);
    dpllTotalTimes.reserve(runs);
    cadicalSolveTimes.reserve(runs);
    cadicalTotalTimes.reserve(runs);

    // -----------------------------------------
    // Benchmark 1: Custom DPLL solver
    // -----------------------------------------

    for (int i = 0; i < runs; i++) {

        auto totalStart = Clock::now();

        //Create a fresh assignment for every run.
        std::vector<int> assignment(730, -1);

        auto solveStart = Clock::now();
        bool solved = dpll(formula, assignment);
        auto solveEnd = Clock::now();
        auto totalEnd = Clock::now();

        if (!solved) {
            std::cerr << "DPLL failed on run " << i + 1 << "\n"; return 1;
        }

        dpllSolveTimes.push_back(elapsedMilliseconds(solveStart, solveEnd));

        dpllTotalTimes.push_back(elapsedMilliseconds(totalStart, totalEnd));
    }

    // -----------------------------------------
    // Benchmark 2: CaDiCaL solver
    // -----------------------------------------

    for (int i = 0; i < runs; i++) {

        auto totalStart = Clock::now();

        //Create a fresh CaDiCaL solver for every run, because SAT solvers can maintain internal state.
        CaDiCaL::Solver solver;

        solver.set("factorcheck", 0); //Need this for current CaDiCaL build version.

        //Loads the same CNF formula into CaDiCaL.
        //This is NOT included in solver-call time.
        for (const auto& clause : formula) {

            for (int literal : clause) {
                solver.add(literal);
            }

            //Zero marks the end of a clause in CaDiCaL.
            solver.add(0);
        }

        //Start timing immediately BEFORE solving.
        auto solveStart = Clock::now();
        int result = solver.solve();
        auto solveEnd = Clock::now();
        auto totalEnd = Clock::now();

        if (result != 10) {
            std::cerr << "CaDiCaL failed on run " << i + 1 << " (status " << result << ")\n";
            return 1;
        }

        cadicalSolveTimes.push_back(elapsedMilliseconds(solveStart, solveEnd));
        cadicalTotalTimes.push_back(elapsedMilliseconds(totalStart, totalEnd));
    }


    // -----------------------------------------
    // Display results
    // -----------------------------------------

    std::cout << std::fixed << std::setprecision(4);

    std::cout << "\nBenchmark results (" << runs << " runs)\n";
    std::cout << "----------------------------------------\n";
    std::cout << "CNF clauses: " << formula.size() << "\n\n";

    printResults("Custom DPLL", dpllSolveTimes, dpllTotalTimes);
    printResults("CaDiCaL", cadicalSolveTimes, cadicalTotalTimes);
    
    return 0;
}
