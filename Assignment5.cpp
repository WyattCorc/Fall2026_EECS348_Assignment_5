/*
 * Program: EECS 348 Assignment 5 - Sudoku Solver, modifications
 * What it does: Reads five Sudoku files and uses backtracking to find
 *               and outputs every solution to SolutionN.txt. If there isn't one, it says so.
 * Input: puzzle1.txt through puzzle5.txt (1-9 = filled, _ = empty)
 * Output: File name, starting puzzle, and all solutions as .txt files or no solution.
 * Author: Wyatt Corcoran, Original Code from Parker Curl
 * Date: October 29, 2026
 * Collaborators: Wyatt Corcoran (Not sure if I am technically the author)
 * Code sources:
 *   - Claude (Anthropic): original three-class program, board checks,
 *     printing, recursive backtracking, and file-processing structure.
 *   - ChatGPT (OpenAI): revised file validation, choosing the empty cell
 *     with the fewest choices (MRV), and related search changes.
 *   - ChatGPT also helped rewrite and label these comments. Review them
 *     and edit the explanations to match your own understanding.
 *   - Chatgpt, used for the second Round revisions,
 *     primarily for implementing the .txt output module
 * 
 */

// Source: Claude for the original includes; ChatGPT added cctype.
#include <iostream>//Wyatt: Manages i/o 
#include <fstream> //Wyatt: Read & write from files
#include <string> //Wyatt: Adds the string type into c++
#include <cctype> // ChatGPT: needed for checking whitespace in puzzle files

using namespace std;//Wyatt: used for standarized code syntax

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
        for (int r = 0; r < SIZE; r++)//Iterates Rows to make an empty board
            for (int c = 0; c < SIZE; c++)//Iterates Cols
                grid[r][c] = EMPTY;//Appends EMPTY to each location
    }

    // Source: Claude's file-reading idea, changed by ChatGPT (correctness).
    // Claude skipped characters it didn't recognize. Now we only allow spaces/newlines, 1-9 and underscores
    bool loadFromFile(const string& filename) {//Input filename
        ifstream in(filename.c_str());//Re-written by Wyatt for better error handling
        if (!in){ 
            cerr << "Error: Unable to open " << filename << '\n'; //added error handling msg here
            return false; //Fail run main
        }
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

    // ReWritten by Wyatt to account for new output format.
    //print the board, using _ for blanks.
    void print(ostream& out) const { //takes the output stream and the output and formats it for the file
        for (int r = 0; r < SIZE; r++){//Iterates through rows
            for (int c = 0; c < SIZE; c++){//Iterates through cols
                if (grid[r][c] == EMPTY)//If what it reads in the file is returned as empty
                    out << "_";//Amends that spot to _
                else
                    out << grid[r][c];//Otherwise puts the number in that spot
                if (c < SIZE-1)//Adds a space between outputs for readability
                    out << ' ';//Appends a ' ' to the point between values
                
                }
                out << '\n'; //Newline when iterating between rows
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
   // Revised by: Wyatt
    // Source: ChatGPT
    // Solve the puzzle and write every solution to the output file.
    void solveAndPrintAll(const SudokuPuzzle& original, ostream& out) {//Helper funct "runs" puzzles 1-5
        solutionCount = 0;//zeroes out the solution count
        SudokuPuzzle working = original;

        // Only attempt to solve a puzzle with valid starting numbers.
        if (working.givensAreValid()) {
            search(working, out);
        }

        // Report whether solutions were found.
        if (solutionCount == 0) {
            out << "No solution found." << '\n';//returns only this to the file if no solutions
        } else {
            out << "Solutions found: " << solutionCount << '\n';//Prints the solutions after
    }
    
}

private:
    // Source: ChatGPT
    //larger unsigned number than Claude's original int.
    unsigned long long solutionCount;

 // Revised by: Wyatt
// Source: Claude's backtracking, revised by ChatGPT
// Find all solutions using the Minimum Remaining Values method.
void search(SudokuPuzzle& puzzle, ostream& out) {//Updated the method to do ostream file I/O
    int row = 0, col = 0, candidateCount = 0;//Inits vals here at 0 for each puzzle

    // Find the empty cell with the fewest possible values.
    if (!puzzle.findBestEmptyCell(row, col, candidateCount)) {//If there is no new empty cell:
        solutionCount++; //Add a solution

        // Write the completed puzzle directly to the output file.
        puzzle.print(out); //uses the print member func of puzzle
        out << '\n';//New lines the end
        return;
    }

    // Stop searching this path if no valid number can be placed.
    if (candidateCount == 0)
        return;

    // Try each possible number and recursively continue.
    for (int value = 1; value <= 9; value++) {
        if (puzzle.isValidPlacement(row, col, value)) {//Check call
            puzzle.set(row, col, value); //Set call

            search(puzzle, out);//Recures here with the puzzle data

            // Sets previous val to EMPTY to properly backtrack
            puzzle.set(row, col, SudokuPuzzle::EMPTY);
        }
    }
}
};
class SudokuApp {
public:
    void run() {
        // Process puzzle1.txt through puzzle5.txt.
        for (int i = 1; i <= 5; i++) {
            string inputName = "puzzle" + to_string(i) + ".txt";
            string outputName = "solution" + to_string(i) + ".txt";

            processPuzzle(inputName, outputName);
        }
    }

// Source: Claude's original SudokuApp class
// Revised by: Wyatt
// Source: ChatGPT
// Process five puzzle files and save their solutions separately.
private:
    SudokuSolver solver;

    // Load a puzzle and write its solutions to the matching file.
    void processPuzzle(const string& inputName,
                       const string& outputName) {
        SudokuPuzzle puzzle;

        // Validate the input before creating the output file.
        if (!puzzle.loadFromFile(inputName)) {
            cerr << "Error: could not read a valid 9x9 puzzle from "
                 << inputName << '\n';
            return;
        }

        // Create or overwrite the solution file.
        ofstream outputFile(outputName.c_str());

        //Error handling for not being able to find or open output
        if (!outputFile) {
            cerr << "Error: could not open "
                 << outputName << '\n';
            return;
        }

        // Write all solutions to the file.
        solver.solveAndPrintAll(puzzle, outputFile);
    }
};

// Source: Claude 
//make the app object and start it.
int main() {
    SudokuApp app;//Inits SodokuApp class
    app.run();//Runs the previous class
    return 0;//Success run condition.
}
