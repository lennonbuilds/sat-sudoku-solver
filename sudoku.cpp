#include "sudoku.hpp"

//Every possible cell and digit combination gets its own boolean variable
//that represents whether that digit is placed in that cell. 
int sudokuVariable(int row, int col, int digit) { //(e.g: top left cell containing digit 5 is sudokuVariable(0,0,5))
    return (row * 81) + (col * 9) + digit;
}



void addCellConstraints(std::vector<std::vector<int>>& formula) {

    for (int row = 0; row < 9; row++) {             //
                                                    //Iterates over every cell.
        for (int col = 0; col < 9; col++) {         //

            std::vector<int> clause;

            for (int digit = 1; digit <= 9; digit++) {  //Iterates over every posible digit in the cell.

                clause.push_back(sudokuVariable(row, col, digit)); //Adds corresponding SAT variable to clause.
            }

            formula.push_back(clause); //Adds completed clause to CNF formula.
        }
    }
}



void addCellUniquenessConstraints(std::vector<std::vector<int>>& formula) {

    for (int row = 0; row < 9; row++) {

        for (int col = 0; col < 9; col++) {

            for (int digit1 = 1; digit1 <= 9; digit1++) {

                for (int digit2 = digit1 + 1; digit2 <= 9; digit2++) {

                    int variable1 = sudokuVariable(row, col, digit1);
                    int variable2 = sudokuVariable(row, col, digit2);

                    //The clause created will be made up of two negative literals and this is added to the formula.
                    formula.push_back({-variable1, -variable2}); //(e.g: {-3,-7} ensures that the first cell cannot contain both 3 and 7).
                }
            }
        }
    }
}




void addRowConstraints(std::vector<std::vector<int>>& formula) {

    for (int row = 0; row < 9; row++) {

        for (int digit = 1; digit <= 9; digit++) {

            for (int col1 = 0; col1 < 9; col1++) {

                for (int col2 = col1 + 1; col2 < 9; col2++) {

                    int variable1 = sudokuVariable(row, col1, digit);
                    int variable2 = sudokuVariable(row, col2, digit);

                    formula.push_back({-variable1, -variable2});
                }
            }
        }
    }
}



void addColumnConstraints(std::vector<std::vector<int>>& formula) {

    for (int col = 0; col < 9; col++) {

        for (int digit = 1; digit <= 9; digit++) {

            for (int row1 = 0; row1 < 9; row1++) {

                for (int row2 = row1 + 1; row2 < 9; row2++) {

                    int variable1 = sudokuVariable(row1, col, digit);
                    int variable2 = sudokuVariable(row2, col, digit);

                    formula.push_back({-variable1, -variable2});
                }
            }
        }
    }
}




void addBoxConstraints(std::vector<std::vector<int>>& formula) {

    for (int boxRow = 0; boxRow < 3; boxRow++) {       
        for (int boxCol = 0; boxCol < 3; boxCol++) {    //First two loops select one of the 9 boxes.

            for (int digit = 1; digit <= 9; digit++) {

                for (int cell1 = 0; cell1 < 9; cell1++) {
                    for (int cell2 = cell1 + 1; cell2 < 9; cell2++) {

                        int row1 = boxRow * 3 + cell1 / 3;
                        int col1 = boxCol * 3 + cell1 % 3;

                        int row2 = boxRow * 3 + cell2 / 3;
                        int col2 = boxCol * 3 + cell2 % 3;

                        int variable1 = sudokuVariable(row1, col1, digit);
                        int variable2 = sudokuVariable(row2, col2, digit);

                        formula.push_back({-variable1, -variable2});
                    }
                }
            }
        }
    }
}



void addPuzzleClues(std::vector<std::vector<int>>& formula, const std::vector<std::vector<int>>& puzzle) {

    for (int row = 0; row < 9; row++) {
        for (int col = 0; col < 9; col++) {

            int digit = puzzle[row][col];

            if (digit != 0) {
                int variable = sudokuVariable(row, col, digit);
                formula.push_back({variable});
            }
        }
    }
}



std::vector<std::vector<int>> decodeSudoku(const std::vector<int>& assignment) {
    std::vector<std::vector<int>> grid(9, std::vector<int>(9, 0));

    for (int row = 0; row < 9; row++) {
        for (int col = 0; col < 9; col++) {

            for (int digit = 1; digit <= 9; digit++) {

                int variable = sudokuVariable(row, col, digit);

                if (assignment[variable] == 1) { //If the boolean variable is true we have found the digit belonging in that cell.
                    grid[row][col] = digit;
                    break;
                }
            }
        }
    }

    return grid;
}
