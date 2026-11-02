#ifndef SUDOKU_HPP
#define SUDOKU_HPP

#include <vector>

//Converts a Sudoku row, column and digit into a SAT variable number
int sudokuVariable(int row, int col, int digit);

//Adds clauses requiring every cell to contain at least one digit
void addCellConstraints(std::vector<std::vector<int>>& formula);

//Ensures cells cannot contain twow different digits, ensuring cells contain exactly ONE digit.
void addCellUniquenessConstraints(std::vector<std::vector<int>>& formula);

//Ensures cells on a row contain different digits.
void addRowConstraints(std::vector<std::vector<int>>& formula);

//Ensures cells on a column contain different digits.
void addColumnConstraints(std::vector<std::vector<int>>& formula);

//Ensures cells in each 3x3 grid contain different digits.
void addBoxConstraints(std::vector<std::vector<int>>& formula);

//Reads the inputted puzzle clues.
void addPuzzleClues(std::vector<std::vector<int>>& formula, const std::vector<std::vector<int>>& puzzle);

//Returns a 9x9 vector containing the complete sudoku.
std::vector<std::vector<int>> decodeSudoku(const std::vector<int>& assignment);
#endif