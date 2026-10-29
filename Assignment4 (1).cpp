/*
 * Program: EECS 348 Assignment 4 - Sudoku Solver
 * What it does: Reads five Sudoku files and uses backtracking to find
 *               and print every solution. If there isn't one, it says so.
 * Input: puzzle1.txt through puzzle5.txt (1-9 = filled, _ = empty)
 * Output: File name, starting puzzle, and all solutions or no solution.
 * Author: Parker Curl
 * Date: October 15, 2026
 * Collaborators: None (no student collaborators)
 * Code sources:
 *   - Claude (Anthropic): original three-class program, board checks,
 *     printing, recursive backtracking, and file-processing structure.
 *   - ChatGPT (OpenAI): revised file validation, choosing the empty cell
 *     with the fewest choices (MRV), and related search changes.
 *   - ChatGPT also helped rewrite and label these comments. Review them
 *     and edit the explanations to match your own understanding.
 * Compile on Cycle: g++ -std=c++11 -Wall -Wextra -o Assignment4.exe Assignment4.cpp
 * Run: ./Assignment4.exe
 */

// Source: Claude for the original includes; ChatGPT added cctype.
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cctype> // ChatGPT: needed for checking whitespace in puzzle files

using namespace std;

// Source: Claude 
//original SudokuPuzzle class and basic board setup.
//this class holds the numbers and checks Sudoku rules.
class SudokuPuzzle {
public:
    static const int SIZE = 9;  // 9 rows and 9 columns
    static const int BLOCK = 3; // Each smaller box is 3x3
    static const int EMPTY = 0; // Zero means the spot is blank

    // Source: Claude  
    //start with a completely empty board.
    SudokuPuzzle() {
        for (int r = 0; r < SIZE; r++)
            for (int c = 0; c < SIZE; c++)
                grid[r][c] = EMPTY;
    }

    // Source: Claude's file-reading idea, changed by ChatGPT (correctness).
    // Claude skipped characters it didn't recognize. Now we only allow spaces/newlines, 1-9 and underscores
    bool loadFromFile(const string& filename) {
        ifstream in(filename.c_str());
        if (!in) return false; // File didn't open

        // Source: ChatGPT 
        //read into a separate board first, so bad file data won't partly replace the puzzle we're using.
        SudokuPuzzle loaded;
        int count = 0;
        char ch;
        while (in.get(ch)) {
            if (isspace(static_cast<unsigned char>(ch))) continue; // Skip spaces
            if (ch != '_' && (ch < '1' || ch > '9')) return false; //Bad character
            if (count == SIZE * SIZE) return false; //More than 81 cells
            loaded.grid[count / SIZE][count % SIZE] =
                (ch == '_') ? EMPTY : (ch - '0'); // Change text to a number
            ++count;
        }
        if (count != SIZE * SIZE) return false; // Not enough cells
        *this = loaded; //Only save it after the file checks out
        return true;
    }

    // Source: Claude 
    //print the board, using _ for blanks.
    void print() const {
        for (int r = 0; r < SIZE; r++) {
            for (int c = 0; c < SIZE; c++) {
                if (grid[r][c] == EMPTY) cout << '_';
                else cout << grid[r][c];
                if (c < SIZE - 1) cout << ' ';
            }
            cout << endl; // Next row
        }
    }

    // Source: Claude 
    //read or change one square on the board.
    int get(int row, int col) const { return grid[row][col]; }
    void set(int row, int col, int value) { grid[row][col] = value; }

    // Source: Claude 
    //see if a number is already in the same row, column, or 3x3 box. If it is, we can't use that number here.
    bool isValidPlacement(int row, int col, int value) const {
        for (int i = 0; i < SIZE; i++) {
            if (i != col && grid[row][i] == value) return false;
            if (i != row && grid[i][col] == value) return false;
        }

        //Find the top left square of the current 3x3 box.
        int startRow = (row / BLOCK) * BLOCK;
        int startCol = (col / BLOCK) * BLOCK;
        for (int r = startRow; r < startRow + BLOCK; r++) {
            for (int c = startCol; c < startCol + BLOCK; c++) {
                if ((r != row || c != col) && grid[r][c] == value) return false;
            }
        }
        return true; //No matching number was found
    }

    // Source: Claude
    //check that the numbers already in the puzzle
    // don't break any Sudoku rules before trying to solve it.
    bool givensAreValid() const {
        for (int r = 0; r < SIZE; r++) {
            for (int c = 0; c < SIZE; c++) {
                if (grid[r][c] != EMPTY &&
                    !isValidPlacement(r, c, grid[r][c])) {
                    return false;
                }
            }
        }
        return true;
    }

    // Source: ChatGPT - replaces Claude's findEmptyCell() method.
    //Claude used the first blank square. This checks every blank square and picks the one with the fewest numbers that could go there.
    bool findBestEmptyCell(int& row, int& col, int& candidateCount) const {
        bool found = false;
        candidateCount = 10; // More than the nine possible digits
        for (int r = 0; r < SIZE; ++r) {
            for (int c = 0; c < SIZE; ++c) {
                if (grid[r][c] != EMPTY) continue; //Already filled
                int choices = 0;
                for (int value = 1; value <= 9; ++value) {
                    if (isValidPlacement(r, c, value)) ++choices;
                }
                if (choices < candidateCount) {
                    row = r;
                    col = c;
                    candidateCount = choices;
                    found = true;
                }
                if (candidateCount == 0) return true; //This path can't work
            }
        }
        return found; // False means there weren't any blanks left
    }

private:
    int grid[SIZE][SIZE]; // Source: Claude: stores the puzzle numbers
};

// Source: Claude 
//original SudokuSolver class and backtracking idea.
// Source: ChatGPT  
//changed which empty cell gets chosen during the search.
class SudokuSolver {
public:
    // Source: Claude
    //keep a copy so we can still print the starting puzzle.
    void solveAndPrintAll(const SudokuPuzzle& original) {
        solutionCount = 0;
        SudokuPuzzle working = original;

        if (working.givensAreValid()) {
            search(working); // Only search if the starting numbers are valid
        }

        if (solutionCount == 0) {
            cout << "No solution found" << endl;
        } else if (solutionCount > 1) {
            cout << "Total solutions found: " << solutionCount << endl;
        }
    }

private:
    // Source: ChatGPT
    //larger unsigned number than Claude's original int.
    unsigned long long solutionCount;

    // Source: Claude's recursive backtracking, updated by ChatGPT for MRV.
    // Pick a blank, try 1-9, keep going, then undo the number and try again.
    void search(SudokuPuzzle& puzzle) {
        // Source: ChatGPT - candidateCount is how many numbers fit in the blank.
        int row = 0, col = 0, candidateCount = 0;

        // Source: ChatGPT
        // use the MRV method instead of the first blank.
        //If no blank is found,the puzzle is complete, so print it.
        if (!puzzle.findBestEmptyCell(row, col, candidateCount)) {
            solutionCount++;
            cout << "Solution " << solutionCount << ":" << endl;
            puzzle.print();
            cout << endl;
            return;
        }

        // Source: ChatGPT
        //if a blank has zero choices, stop this path.
        if (candidateCount == 0) return;

        // Source: Claude
        //try each number, go deeper, then backtrack.
        for (int value = 1; value <= 9; value++) {
            if (puzzle.isValidPlacement(row, col, value)) {
                puzzle.set(row, col, value); //Try this number
                search(puzzle); // Work on the next blank
                puzzle.set(row, col, SudokuPuzzle::EMPTY); //Undo and try again
            }
        }
    }
};

// Source: Claude 
// the original app class. It runs all five puzzle files.
class SudokuApp {
public:
    // Source: Claude
    //list the five files we're supposed to solve.
    SudokuApp() {
        files.push_back("puzzle1.txt");
        files.push_back("puzzle2.txt");
        files.push_back("puzzle3.txt");
        files.push_back("puzzle4.txt");
        files.push_back("puzzle5.txt");
    }

    // Source: Claude 
    //go through the five files one at a time.
    void run() {
        for (size_t i = 0; i < files.size(); i++) {
            processPuzzle(files[i]);
        }
    }

private:
    vector<string> files; // Source: Claude: names of the input files
    SudokuSolver solver;  // Source: Claude: object that solves the puzzle

    // Source: Claude
    //load, print, and solve each puzzle.
    void processPuzzle(const string& filename) {
        cout << "==============================" << endl;
        cout << "File: " << filename << endl;
        cout << "==============================" << endl;

        SudokuPuzzle puzzle;
        if (!puzzle.loadFromFile(filename)) {
            cout << "Error: could not read a valid 9x9 puzzle from "
                 << filename << endl << endl;
            return;
        }

        cout << "Original puzzle:" << endl;
        puzzle.print();
        cout << endl;

        solver.solveAndPrintAll(puzzle);
        cout << endl;
    }
};

// Source: Claude 
//make the app object and start it.
int main() {
    SudokuApp app;
    app.run();
    return 0;
}
