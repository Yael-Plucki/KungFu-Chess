#include <iostream>
#include <string>
#include <vector>
#include <sstream>

// פונקציה לבדיקת תקינות אסימון
bool isValidToken(const std::string& token) {
    if (token == ".") return true;
    if (token.length() == 2) {
        std::string colors = "wb";
        std::string pieces = "KQRBNP";
        return (colors.find(token[0]) != std::string::npos && 
                pieces.find(token[1]) != std::string::npos);
    }
    return false;
}
void parseBoard(){
    std::string line;
    std::vector<std::vector<std::string>> board;
    bool isParsingBoard = false;

    while (std::getline(std::cin, line)) {
        // נקה רווחים מיותרים מסוף השורה אם יש
        if (line == "Board:") {
            isParsingBoard = true;
            continue;
        }
        if (line == "Commands:") {
            isParsingBoard = false;
            break; // סיום עיבוד הלוח
        }
        if (!isParsingBoard || line.empty()) continue;

        std::stringstream ss(line);
        std::string token;
        std::vector<std::string> row;

        while (ss >> token) {
            if (!isValidToken(token)) {
            // כאן קורית השגיאה בטסטים שלך
                std::cout << "ERROR UNKNOWN_TOKEN" << std::endl;
                return 0;
            }
            row.push_back(token);
        }
    
        // בדיקת תקינות רוחב שורה...
        if (!row.empty()) {
            if (!board.empty() && row.size() != board[0].size()) {
                std::cout << "ERROR ROW_WIDTH_MISMATCH" << std::endl;
                return 0;
            }
            board.push_back(row);
        }
    }
    for(int i=0;i<board.size();i++){
        for(int j=0;j<board[i].size();j++){
            std::cout << board[i][j];
        }
    }

}

int main() {
    return 0;
}
