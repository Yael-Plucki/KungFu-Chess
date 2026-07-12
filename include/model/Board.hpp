#ifndef BOARD_H
#define BOARD_H

#include <vector>
#include <string> 
using namespace std;
class Board
{
private:
    vector<vector<string>> grid;

public:
    Board(const vector<vector<string>>& grid);

    int getRows() const;
    int getCols() const;

    bool isValidPosition(int row, int col) const;

    string getPiece(int row, int col) const;

    void setPiece(int row, int col, const string& piece);

    void print() const;
    };

#endif