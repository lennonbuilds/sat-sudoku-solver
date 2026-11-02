#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <cmath>

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Group.H>
#include <FL/fl_draw.H>
#include <FL/fl_ask.H>

#include "sudoku.hpp"
#include "dpll.hpp"
#include "cadical_solver.hpp"

//Blocks edits to solved cells without disabling or greying them out.
class SudokuCell : public Fl_Input {
public:
    SudokuCell(int x, int y, int w, int h)
        : Fl_Input(x, y, w, h) {}

    int handle(int event) override {
        if (locked && (event == FL_KEYDOWN || event == FL_PASTE)) {
            return 1;
        }
        return Fl_Input::handle(event);
    }
    void setLocked(bool value) { locked = value; }

private:
    bool locked = false;
};

using Grid = std::vector<std::vector<int>>;
using Formula = std::vector<std::vector<int>>;

//Sudoku grid and dropdown menus.
SudokuCell* cells[9][9];
Fl_Choice* puzzleChoice;
Fl_Choice* solverChoice;
Fl_Button* loadButton;

//Performance dashboard widgets.
Fl_Box* timeLabel;
Fl_Box* variablesLabel;
Fl_Box* clausesLabel;
Fl_Box* resultLabel;
Fl_Box* comparisonLabel;

//Original puzzle, saved separately from its solution.
Grid originalPuzzle;
bool hasOriginalPuzzle = false;
bool showingSolution = false;

//Performance measurements.
double dpllTime = 0.0;
double cadicalTime = 0.0;
bool hasComparison = false;

//Dashboard colours.
const Fl_Color panelColor = fl_rgb_color(48, 52, 60);
const Fl_Color cardColor = fl_rgb_color(65, 70, 80);
const Fl_Color blueColor = fl_rgb_color(55, 110, 220);
const Fl_Color redColor = fl_rgb_color(220, 75, 75);

//Ten example Sudoku puzzles.
const std::vector<Grid> examples = {

    //Example 1 - Classic
    {
        {5,3,0,0,7,0,0,0,0},
        {6,0,0,1,9,5,0,0,0},
        {0,9,8,0,0,0,0,6,0},
        {8,0,0,0,6,0,0,0,3},
        {4,0,0,8,0,3,0,0,1},
        {7,0,0,0,2,0,0,0,6},
        {0,6,0,0,0,0,2,8,0},
        {0,0,0,4,1,9,0,0,5},
        {0,0,0,0,8,0,0,7,9}
    },

    //Example 2
    {
        {0,0,0,2,6,0,7,0,1},
        {6,8,0,0,7,0,0,9,0},
        {1,9,0,0,0,4,5,0,0},
        {8,2,0,1,0,0,0,4,0},
        {0,0,4,6,0,2,9,0,0},
        {0,5,0,0,0,3,0,2,8},
        {0,0,9,3,0,0,0,7,4},
        {0,4,0,0,5,0,0,3,6},
        {7,0,3,0,1,8,0,0,0}
    },

    //Example 3 - Hard
    {
        {8,0,0,0,0,0,0,0,0},
        {0,0,3,6,0,0,0,0,0},
        {0,7,0,0,9,0,2,0,0},
        {0,5,0,0,0,7,0,0,0},
        {0,0,0,0,4,5,7,0,0},
        {0,0,0,1,0,0,0,3,0},
        {0,0,1,0,0,0,0,6,8},
        {0,0,8,5,0,0,0,1,0},
        {0,9,0,0,0,0,4,0,0}
    },

    //Example 4
    {
        {0,0,0,0,0,0,0,1,2},
        {0,0,0,0,0,0,0,3,4},
        {0,0,0,0,0,0,0,5,6},
        {0,0,0,0,0,0,0,7,8},
        {0,0,0,0,0,0,0,9,1},
        {0,0,0,0,0,0,0,2,3},
        {0,0,0,0,0,0,0,4,5},
        {0,0,0,0,0,0,0,6,7},
        {0,0,0,0,0,0,0,8,9}
    },

    //Example 5
    {
        {0,0,0,0,0,0,1,2,3},
        {0,0,0,0,0,0,4,5,6},
        {0,0,0,0,0,0,7,8,9},
        {0,0,0,0,0,0,2,3,1},
        {0,0,0,0,0,0,5,6,4},
        {0,0,0,0,0,0,8,9,7},
        {0,0,0,0,0,0,3,1,2},
        {0,0,0,0,0,0,6,4,5},
        {0,0,0,0,0,0,9,7,8}
    },

    //Example 6
    {
        {5,3,4,6,7,8,9,1,0},
        {6,7,2,1,9,5,3,4,0},
        {1,9,8,3,4,2,5,6,0},
        {8,5,9,7,6,1,4,2,0},
        {4,2,6,8,5,3,7,9,0},
        {7,1,3,9,2,4,8,5,0},
        {9,6,1,5,3,7,2,8,0},
        {2,8,7,4,1,9,6,3,0},
        {3,4,5,2,8,6,1,7,0}
    },

    //Example 7
    {
        {5,3,4,6,7,8,9,0,0},
        {6,7,2,1,9,5,3,0,0},
        {1,9,8,3,4,2,5,0,0},
        {8,5,9,7,6,1,4,0,0},
        {4,2,6,8,5,3,7,0,0},
        {7,1,3,9,2,4,8,0,0},
        {9,6,1,5,3,7,2,0,0},
        {2,8,7,4,1,9,6,0,0},
        {3,4,5,2,8,6,1,0,0}
    },

    //Example 8
    {
        {5,3,4,6,7,8,0,0,0},
        {6,7,2,1,9,5,0,0,0},
        {1,9,8,3,4,2,0,0,0},
        {8,5,9,7,6,1,0,0,0},
        {4,2,6,8,5,3,0,0,0},
        {7,1,3,9,2,4,0,0,0},
        {9,6,1,5,3,7,0,0,0},
        {2,8,7,4,1,9,0,0,0},
        {3,4,5,2,8,6,0,0,0}
    },

    //Example 9
    {
        {5,3,4,6,7,0,0,0,0},
        {6,7,2,1,9,0,0,0,0},
        {1,9,8,3,4,0,0,0,0},
        {8,5,9,7,6,0,0,0,0},
        {4,2,6,8,5,0,0,0,0},
        {7,1,3,9,2,0,0,0,0},
        {9,6,1,5,3,0,0,0,0},
        {2,8,7,4,1,0,0,0,0},
        {3,4,5,2,8,0,0,0,0}
    },

    //Example 10
    {
        {5,3,4,6,0,0,0,0,0},
        {6,7,2,1,0,0,0,0,0},
        {1,9,8,3,0,0,0,0,0},
        {8,5,9,7,0,0,0,0,0},
        {4,2,6,8,0,0,0,0,0},
        {7,1,3,9,0,0,0,0,0},
        {9,6,1,5,0,0,0,0,0},
        {2,8,7,4,0,0,0,0,0},
        {3,4,5,2,0,0,0,0,0}
    }
};

//Formats milliseconds.
std::string formatTime(double milliseconds) {

    std::ostringstream output;

    output << std::fixed << std::setprecision(4) << milliseconds << " ms";

    return output.str();
}

//Converts a Sudoku puzzle into CNF.
Formula buildFormula(const Grid& puzzle) {

    Formula formula;

    addCellConstraints(formula);
    addCellUniquenessConstraints(formula);
    addRowConstraints(formula);
    addColumnConstraints(formula);
    addBoxConstraints(formula);
    addPuzzleClues(formula, puzzle);

    return formula;
}

//Reads numbers from the GUI.
bool readGrid(Grid& puzzle) {

    puzzle.assign(9, std::vector<int>(9, 0));

    for (int row = 0; row < 9; row++) {
        for (int col = 0; col < 9; col++) {

            const char* input = cells[row][col]->value();

            if (input[0] == '\0') {
                continue;
            }

            if (input[0] < '1' || input[0] > '9' || input[1] != '\0') {
                fl_alert("Please enter digits from 1 to 9 only.");
                return false;
            }

            puzzle[row][col] = input[0] - '0';
        }
    }

    return true;
}

//Displays a solution.
//Blue = original clues (from pre-loaded puzzles / your own input).
//Red = numbers added by the solver.
//Solved cells are locked without changing their appearance.
void displaySolution(const Grid& solution,
                     const Grid& puzzle) {

    for (int row = 0; row < 9; row++) {
        for (int col = 0; col < 9; col++) {

            std::string text =
                std::to_string(solution[row][col]);

            if (puzzle[row][col] == 0) {
                cells[row][col]->textcolor(FL_RED);
            } else {
                cells[row][col]->textcolor(FL_DARK_BLUE);
            }

            cells[row][col]->value(text.c_str());

            //Lock this cell after solving.
            cells[row][col]->setLocked(true); //Keeps colours and block edits silently.
        }
    }
}

//Reset the performance dashboard.
void resetPerformance() {

    timeLabel->copy_label("Custom DPLL - Single run total solve time:\n--");
    variablesLabel->copy_label("729");
    clausesLabel->copy_label("--");
    resultLabel->copy_label("Not solved");
    comparisonLabel->copy_label("Run Compare Solvers to see results.");

    hasComparison = false;
}

//Clears all 81 cells.
void clearGrid(Fl_Widget*, void*) {

    for (int row = 0; row < 9; row++) {
        for (int col = 0; col < 9; col++) {

            cells[row][col]->value("");
            cells[row][col]->textcolor(FL_DARK_BLUE);

            //Unlocks the cell for new input.
            cells[row][col]->setLocked(false);
        }
    }

    hasOriginalPuzzle = false;
    showingSolution = false;

    resetPerformance();
}

//Load the selected example puzzle.
void loadExample(Fl_Widget*, void*) {

    int selected = puzzleChoice->value();

    if (selected < 0 ||
        selected >= static_cast<int>(examples.size())) {
        return;
    }

    originalPuzzle = examples[selected];

    hasOriginalPuzzle = true;
    showingSolution = false;

    for (int row = 0; row < 9; row++) {
        for (int col = 0; col < 9; col++) {

            int digit = originalPuzzle[row][col];

            //Unlocks the cell for the new puzzle.
            cells[row][col]->setLocked(false);

            cells[row][col]->textcolor(FL_DARK_BLUE);

            if (digit == 0) {
                cells[row][col]->value("");
            } else {

                std::string text =
                    std::to_string(digit);

                cells[row][col]->value(text.c_str());
            }
        }
    }

    resetPerformance();

    loadButton->color(FL_BACKGROUND_COLOR);
    loadButton->redraw();
}

//Retrieves the original puzzle.
bool getPuzzle(Grid& puzzle) {

    if (showingSolution && hasOriginalPuzzle) {
        puzzle = originalPuzzle;
        return true;
    }

    if (!readGrid(puzzle)) {
        return false;
    }

    originalPuzzle = puzzle;
    hasOriginalPuzzle = true;

    return true;
}

//Measure one complete Sudoku-solving pipeline for the selected solver.
//Includes CNF generation, solver initialization, SAT solving, and decoding.
//EXCLUDES reading the GUI and drawing the resulting grid.
bool runSolver(const Grid& puzzle, int solverIndex, Grid& solution, double& milliseconds, std::size_t& clauseCount) {

    auto start = std::chrono::steady_clock::now();

    // Build a fresh CNF formula for EACH timed solver run.
    Formula formula = buildFormula(puzzle);
    clauseCount = formula.size();

    std::vector<int> assignment(730, -1);
    bool solved = false;

    if (solverIndex == 0) {
        solved = dpll(formula, assignment);
    } else {
        //Wrapper includes CaDiCaL initialization and clause loading.
        solved = solveWithCadical(formula, assignment);
    }

    if (solved) {
        solution = decodeSudoku(assignment);
    }

    auto end = std::chrono::steady_clock::now();
    milliseconds = std::chrono::duration<double, std::milli>(end - start).count();

    return solved;
}

//Draws the comparison graph inside the dark panel.
class ComparisonChart : public Fl_Widget {

public:

    ComparisonChart(int x, int y, int w, int h)
        : Fl_Widget(x, y, w, h) {}

    void draw() override {

        fl_color(fl_rgb_color(30, 33, 39));
        fl_rectf(x(), y(), w(), h());

        if (!hasComparison) {

            fl_color(FL_WHITE);
            fl_font(FL_HELVETICA, 13);

            fl_draw("No comparison yet", x() + 10, y() + 45);

            return;
        }

        const int left = x() + 115;
        const int maxWidth = w() - 220;

        double maximum = std::max(dpllTime, cadicalTime);

        if (maximum <= 0.0) {
            maximum = 1.0;
        }

        int dpllWidth = static_cast<int>((dpllTime / maximum) * maxWidth);

        int cadicalWidth = static_cast<int>((cadicalTime / maximum) * maxWidth);

        fl_font(FL_HELVETICA, 13);
        fl_color(FL_WHITE);

        fl_draw("Custom DPLL", x() + 5, y() + 30);

        fl_draw("CaDiCaL", x() + 5, y() + 75);

        //Custom DPLL bar.
        fl_color(blueColor);
        fl_rectf(left, y() + 15, dpllWidth, 20);

        //CaDiCaL bar.
        fl_color(redColor);
        fl_rectf(left, y() + 60, cadicalWidth, 20);

        //Timing labels.
        fl_color(FL_WHITE);
        fl_font(FL_HELVETICA, 11);

        std::string dpllText = formatTime(dpllTime);

        std::string cadicalText = formatTime(cadicalTime);

        fl_draw(dpllText.c_str(), left + maxWidth + 5, y() + 30);

        fl_draw(cadicalText.c_str(), left + maxWidth + 5, y() + 75);
    }
};

ComparisonChart* comparisonChart;

//Solve with the selected SAT solver.
void solveSudoku(Fl_Widget*, void*) {

    Grid puzzle;

    if (!getPuzzle(puzzle)) {
        return;
    }

    Grid solution;
    double milliseconds = 0.0;
    std::size_t clauseCount = 0;

    bool solved = runSolver(puzzle, solverChoice->value(), solution, milliseconds, clauseCount);

    std::string timing = (solverChoice->value() == 0 ? "Custom DPLL" : "CaDiCaL") + std::string(" - Single run total solve time:\n") + formatTime(milliseconds);
    timeLabel->copy_label(timing.c_str());
    clausesLabel->copy_label(std::to_string(clauseCount).c_str());

    //A single solver run is NOT a two solver comparison.
    hasComparison = false;
    comparisonLabel->copy_label("Run Compare Solvers to see results.");
    comparisonChart->redraw();

    if (!solved) {

        resultLabel->copy_label("UNSAT");

        fl_alert("No solution found. Check your puzzle.");

        return;
    }

    resultLabel->copy_label("SAT");

    //This also locks all 81 cells.
    displaySolution(solution, puzzle);

    showingSolution = true;
}

//Compares both solvers on the same puzzle.
void compareSolvers(Fl_Widget*, void*) {

    Grid puzzle;

    if (!getPuzzle(puzzle)) {
        return;
    }

    Grid dpllSolution;
    Grid cadicalSolution;
    std::size_t dpllClauses = 0;
    std::size_t cadicalClauses = 0;

    //Each solver independently pays the full CNF generation cost.
    bool dpllSolved = runSolver(puzzle, 0, dpllSolution, dpllTime, dpllClauses);
    bool cadicalSolved = runSolver(puzzle, 1, cadicalSolution, cadicalTime, cadicalClauses);

    clausesLabel->copy_label(std::to_string(dpllClauses).c_str());

    if (!dpllSolved || !cadicalSolved) {

        resultLabel->copy_label("UNSAT");

        comparisonLabel->copy_label("At least one solver found no solution.");

        hasComparison = false;
        comparisonChart->redraw();

        return;
    }

    resultLabel->copy_label("SAT");

    if (solverChoice->value() == 0) {

        //Displays and locks the DPLL solution.
        displaySolution(dpllSolution, puzzle);

        std::string timing =
            "Custom DPLL - Single run total solve time:\n" + formatTime(dpllTime);

        timeLabel->copy_label(timing.c_str());

    } else {

        //Displays and locks the CaDiCaL solution.
        displaySolution(cadicalSolution, puzzle);

        std::string timing =
            "CaDiCaL - Single run total solve time:\n" + formatTime(cadicalTime);

        timeLabel->copy_label(timing.c_str());
    }

    showingSolution = true;
    hasComparison = true;

    double faster = std::min(dpllTime, cadicalTime);
    double slower = std::max(dpllTime, cadicalTime);

    std::string message;

    if (faster <= 0.0) {

        message = "Both solvers completed.";

    } else {

        double ratio = slower / faster;

        std::ostringstream output;

        output << std::fixed << std::setprecision(2);

        if (dpllTime < cadicalTime) {

            output << "Custom DPLL produced the Sudoku solution\n" << ratio << "x faster than CaDiCaL in this run.";

        } else {

            output << "CaDiCaL produced the Sudoku solution\n" << ratio << "x faster than Custom DPLL in this run.";
        }

        message = output.str();
    }

    comparisonLabel->copy_label(message.c_str());
    comparisonChart->redraw();
}

//Highlights 'Load Puzzle' when the dropdown selection changes.
void puzzleSelectionChanged(Fl_Widget*, void*) {
    loadButton->color(fl_rgb_color(90, 135, 205));
    loadButton->redraw();
}

int main() {

    Fl_Window window(950, 650, "SAT Sudoku Solver");

    const int cellSize = 43;
    const int boxGap = 6;

    //Sudoku grid on the right.
    const int startX = 493;
    const int startY = 35;

    for (int row = 0; row < 9; row++) {
        for (int col = 0; col < 9; col++) {

            int x = startX + col * cellSize + (col / 3) * boxGap;

            int y = startY + row * cellSize + (row / 3) * boxGap;

            cells[row][col] = new SudokuCell(x, y, cellSize, cellSize);

            cells[row][col]->textsize(20);
            cells[row][col]->textfont(FL_HELVETICA_BOLD);
            cells[row][col]->textcolor(FL_DARK_BLUE);
            cells[row][col]->maximum_size(1);
            cells[row][col]->box(FL_BORDER_BOX);
        }
    }

    //Clear button above Sudoku grid.
    Fl_Button* clearButton = new Fl_Button(800, 5, 120, 25, "Clear Grid");

    clearButton->callback(clearGrid);

    //Puzzle dropdown.
    puzzleChoice = new Fl_Choice(625, 465, 220, 30, "Puzzle:");

    puzzleChoice->add("Example 1 - Classic");
    puzzleChoice->add("Example 2");
    puzzleChoice->add("Example 3 - Hard");
    puzzleChoice->add("Example 4");
    puzzleChoice->add("Example 5");
    puzzleChoice->add("Example 6");
    puzzleChoice->add("Example 7");
    puzzleChoice->add("Example 8");
    puzzleChoice->add("Example 9");
    puzzleChoice->add("Example 10");

    puzzleChoice->value(0);
    puzzleChoice->callback(puzzleSelectionChanged);

    //Solver dropdown.
    solverChoice = new Fl_Choice(625, 510, 220, 30, "Solver:");

    solverChoice->add("Custom DPLL");
    solverChoice->add("CaDiCaL");
    solverChoice->value(0);

    //'Load Puzzle' button.
    loadButton = new Fl_Button(525, 565, 160, 40, "Load Puzzle");

    loadButton->callback(loadExample);

    //'Solve Sudoku' button.
    Fl_Button* solveButton = new Fl_Button(715, 565, 160, 40, "Solve Sudoku");

    solveButton->callback(solveSudoku);

    
    //  
    //Dark performance dashboard
    //

    Fl_Group* performancePanel = new Fl_Group(20, 35, 430, 570);

    performancePanel->box(FL_FLAT_BOX);
    performancePanel->color(panelColor);

    //Dashboard heading.
    Fl_Box* heading = new Fl_Box(40, 55, 390, 35, "SAT Solver Performance");

    heading->labelfont(FL_HELVETICA_BOLD);
    heading->labelsize(20);
    heading->labelcolor(FL_WHITE);

    //Variables statistic.
    variablesLabel = new Fl_Box(40, 115, 120, 50, "729");

    variablesLabel->box(FL_FLAT_BOX);
    variablesLabel->color(cardColor);
    variablesLabel->labelcolor(FL_WHITE);
    variablesLabel->labelsize(19);

    //CNF clauses statistic.
    clausesLabel = new Fl_Box(175, 115, 120, 50, "--");

    clausesLabel->box(FL_FLAT_BOX);
    clausesLabel->color(cardColor);
    clausesLabel->labelcolor(FL_WHITE);
    clausesLabel->labelsize(19);

    //SAT or UNSAT result.
    resultLabel = new Fl_Box(310, 115, 120, 50, "Not solved");

    resultLabel->box(FL_FLAT_BOX);
    resultLabel->color(cardColor);
    resultLabel->labelcolor(FL_WHITE);
    resultLabel->labelsize(15);

    //Statistic captions.
    Fl_Box* variablesCaption = new Fl_Box(40, 165, 120, 25, "Variables");

    variablesCaption->labelcolor(FL_WHITE);

    Fl_Box* clausesCaption = new Fl_Box(175, 165, 120, 25, "CNF Clauses");

    clausesCaption->labelcolor(FL_WHITE);

    Fl_Box* resultCaption = new Fl_Box(310, 165, 120, 25, "Result");

    resultCaption->labelcolor(FL_WHITE);

    //Execution time.
    timeLabel = new Fl_Box(40, 200, 390, 68, "Custom DPLL - Single run total solve time:\n--");

    timeLabel->box(FL_FLAT_BOX);
    timeLabel->color(cardColor);
    timeLabel->labelcolor(FL_WHITE);
    timeLabel->labelfont(FL_HELVETICA_BOLD);
    timeLabel->labelsize(13);

    //Compare Solvers button.
    Fl_Button* compareButton = new Fl_Button(125, 313, 220, 40, "Compare Solvers");

    compareButton->callback(compareSolvers);

    //Comparison bar chart.
    comparisonChart = new ComparisonChart(35, 365, 400, 100);

    //Speed comparison result.
    comparisonLabel = new Fl_Box(40, 485, 390, 60, "Run Compare Solvers to see results.");

    comparisonLabel->labelcolor(FL_WHITE);
    comparisonLabel->labelsize(15);
    comparisonLabel->labelfont(FL_HELVETICA_BOLD);

    performancePanel->end();

    window.end();
    window.show();

    return Fl::run();
}
