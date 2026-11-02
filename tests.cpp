#include <iostream>
#include <vector>
#include <cassert>

#include "sudoku.hpp"
#include "dpll.hpp"
#include "cadical_solver.hpp"

//Build the CNF formula for a given Sudoku puzzle.
std::vector<std::vector<int>> buildFormula(const std::vector<std::vector<int>>& puzzle) {
    std::vector<std::vector<int>> formula;

    addCellConstraints(formula);
    addCellUniquenessConstraints(formula);
    addRowConstraints(formula);
    addColumnConstraints(formula);
    addBoxConstraints(formula);
    addPuzzleClues(formula, puzzle);

    return formula;
}


bool isValidSudoku(const std::vector<std::vector<int>>& grid) {

    if (grid.size() != 9) {
        return false;
    }

    for (const auto& row : grid) {
        if (row.size() != 9) {
            return false;
        }
    }

    //Checks every row and column.
    for (int i = 0; i < 9; i++) {
        std::vector<bool> rowSeen(10, false);
        std::vector<bool> colSeen(10, false);

        for (int j = 0; j < 9; j++) {
            int rowDigit = grid[i][j];
            int colDigit = grid[j][i];

            if (rowDigit < 1 || rowDigit > 9 || colDigit < 1 || colDigit > 9) {
                return false;
            }

            if (rowSeen[rowDigit] || colSeen[colDigit]) {
                return false;
            }

            rowSeen[rowDigit] = true;
            colSeen[colDigit] = true;
        }
    }

    //Check each 3x3 box.
    for (int boxRow = 0; boxRow < 9; boxRow += 3) {
        for (int boxCol = 0; boxCol < 9; boxCol += 3) {

            std::vector<bool> seen(10, false);

            for (int row = boxRow; row < boxRow + 3; row++) {
                for (int col = boxCol; col < boxCol + 3; col++) {

                    int digit = grid[row][col];

                    if (seen[digit]) {
                        return false;
                    }

                    seen[digit] = true;
                }
            }
        }
    }

    return true;
}



int main() {

    //Test 1: A valid Sudoku should be solvable.
    std::vector<std::vector<int>> validPuzzle = {
        {5,3,0,0,7,0,0,0,0},
        {6,0,0,1,9,5,0,0,0},
        {0,9,8,0,0,0,0,6,0},
        {8,0,0,0,6,0,0,0,3},
        {4,0,0,8,0,3,0,0,1},
        {7,0,0,0,2,0,0,0,6},
        {0,6,0,0,0,0,2,8,0},
        {0,0,0,4,1,9,0,0,5},
        {0,0,0,0,8,0,0,7,9}
    };

    auto validFormula = buildFormula(validPuzzle);

    std::vector<int> dpllAssignment(730, -1);
    std::vector<int> cadicalAssignment(730, -1);

    assert(dpll(validFormula, dpllAssignment));
    assert(solveWithCadical(validFormula, cadicalAssignment));

    std::cout << "Test 1 passed: Both solvers solve a valid Sudoku.\n";

    
    //Test 1b: Check the decoded solutions.
    std::vector<std::vector<int>> expectedSolution = {
        {5,3,4,6,7,8,9,1,2},
        {6,7,2,1,9,5,3,4,8},
        {1,9,8,3,4,2,5,6,7},
        {8,5,9,7,6,1,4,2,3},
        {4,2,6,8,5,3,7,9,1},
        {7,1,3,9,2,4,8,5,6},
        {9,6,1,5,3,7,2,8,4},
        {2,8,7,4,1,9,6,3,5},
        {3,4,5,2,8,6,1,7,9}
    };

    auto dpllSolution = decodeSudoku(dpllAssignment);
    auto cadicalSolution = decodeSudoku(cadicalAssignment);

    assert(dpllSolution == expectedSolution);
    assert(cadicalSolution == expectedSolution);

    std::cout << "Test 1b passed: Both solvers produce the expected grid.\n";


    //Test 1c: Check that both completed grids follow Sudoku rules.
    assert(isValidSudoku(dpllSolution));
    assert(isValidSudoku(cadicalSolution));

    std::cout << "Test 1c passed: Both solutions follow Sudoku rules.\n";

    //Check that the validator rejects an incorrect grid.
    auto incorrectGrid = expectedSolution;
    incorrectGrid[0][0] = 3;

    assert(!isValidSudoku(incorrectGrid));

    std::cout << "Test 1d passed: Validator rejects an invalid grid.\n";




    //Test 2: Contradictory clues should be UNSAT.
    std::vector<std::vector<int>> invalidPuzzle = validPuzzle;

    invalidPuzzle[0][1] = 5; //(Two 5s in the first row).

    auto invalidFormula = buildFormula(invalidPuzzle);

    std::vector<int> invalidDpllAssignment(730, -1);
    std::vector<int> invalidCadicalAssignment(730, -1);

    assert(!dpll(invalidFormula, invalidDpllAssignment));
    assert(!solveWithCadical(invalidFormula, invalidCadicalAssignment));

    std::cout << "Test 2 passed: Both solvers reject contradictory clues.\n";

    std::cout << "\nAll tests passed!\n";

    return 0;
}
