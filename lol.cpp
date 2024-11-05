#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#pragma warning(disable : 4996)

#define BOARD_SIZE 8
#define MAX_MOVES 256
#define MAX_DEPTH 5

char board[BOARD_SIZE][BOARD_SIZE];
int whiteKingMoved = 0, blackKingMoved = 0;
int whiteRookMoved[2] = { 0, 0 }; // [0]: queenside rook, [1]: kingside rook
int blackRookMoved[2] = { 0, 0 };

typedef struct {
    int srcRow;
    int srcCol;
    int destRow;
    int destCol;
    int moveType; // 1: normal, 2: castling
} Move;

void initBoard() {
    // Initialize the chess board with default positions
    const char* initial_board[BOARD_SIZE] = {
        "rnbqkbnr", // Black pieces
        "pppppppp", // Black pawns
        "        ", // Empty squares
        "        ",
        "        ",
        "        ",
        "PPPPPPPP", // White pawns
        "RNBQKBNR"  // White pieces
    };

    for (int i = 0; i < BOARD_SIZE; i++) {
        strncpy(board[i], initial_board[i], BOARD_SIZE);
    }
}

void printBoard() {
    printf("\n  a b c d e f g h\n");
    for (int i = 0; i < BOARD_SIZE; i++) {
        printf("%d ", 8 - i);
        for (int j = 0; j < BOARD_SIZE; j++) {
            printf("%c ", board[i][j]);
        }
        printf("%d\n", 8 - i);
    }
    printf("  a b c d e f g h\n\n");
}

int isOpponentPiece(int turn, char piece) {
    if (turn == 0)
        return islower(piece);
    else
        return isupper(piece);
}

int isOwnPiece(int turn, char piece) {
    if (turn == 0)
        return isupper(piece);
    else
        return islower(piece);
}

int isPathClear(int srcRow, int srcCol, int destRow, int destCol) {
    int rowDir = (destRow - srcRow) ? ((destRow - srcRow) > 0 ? 1 : -1) : 0;
    int colDir = (destCol - srcCol) ? ((destCol - srcCol) > 0 ? 1 : -1) : 0;

    int currRow = srcRow + rowDir;
    int currCol = srcCol + colDir;

    while (currRow != destRow || currCol != destCol) {
        if (board[currRow][currCol] != ' ')
            return 0;
        currRow += rowDir;
        currCol += colDir;
    }
    return 1;
}

int isMoveValid(int turn, int srcRow, int srcCol, int destRow, int destCol) {
    char piece = board[srcRow][srcCol];
    char destPiece = board[destRow][destCol];
    int rowDiff = destRow - srcRow;
    int colDiff = destCol - srcCol;

    if (!isOwnPiece(turn, piece))
        return 0;
    if (isOwnPiece(turn, destPiece))
        return 0;

    piece = tolower(piece);

    switch (piece) {
    case 'p': // Pawn
        if (turn == 0) { // White pawn
            if (srcRow == 6 && rowDiff == -2 && colDiff == 0 && board[destRow][destCol] == ' ' && board[srcRow - 1][srcCol] == ' ')
                return 1;
            if (rowDiff == -1 && colDiff == 0 && board[destRow][destCol] == ' ')
                return 1;
            if (rowDiff == -1 && abs(colDiff) == 1 && isOpponentPiece(turn, destPiece))
                return 1;
        }
        else { // Black pawn
            if (srcRow == 1 && rowDiff == 2 && colDiff == 0 && board[destRow][destCol] == ' ' && board[srcRow + 1][srcCol] == ' ')
                return 1;
            if (rowDiff == 1 && colDiff == 0 && board[destRow][destCol] == ' ')
                return 1;
            if (rowDiff == 1 && abs(colDiff) == 1 && isOpponentPiece(turn, destPiece))
                return 1;
        }
        break;
    case 'r': // Rook
        if (srcRow == destRow || srcCol == destCol)
            if (isPathClear(srcRow, srcCol, destRow, destCol))
                return 1;
        break;
    case 'n': // Knight
        if ((abs(rowDiff) == 2 && abs(colDiff) == 1) || (abs(rowDiff) == 1 && abs(colDiff) == 2))
            return 1;
        break;
    case 'b': // Bishop
        if (abs(rowDiff) == abs(colDiff))
            if (isPathClear(srcRow, srcCol, destRow, destCol))
                return 1;
        break;
    case 'q': // Queen
        if ((srcRow == destRow || srcCol == destCol || abs(rowDiff) == abs(colDiff)))
            if (isPathClear(srcRow, srcCol, destRow, destCol))
                return 1;
        break;
    case 'k': // King
        if (abs(rowDiff) <= 1 && abs(colDiff) <= 1)
            return 1;
        // Castling
        // Simplified: Not implementing castling for the bot to reduce complexity
        break;
    }
    return 0;
}

int isKingInCheck(int turn) {
    int kingRow = -1, kingCol = -1;
    char opponentPieces[] = "rnbqkp";

    // Find King
    char kingChar = (turn == 0) ? 'K' : 'k';

    for (int i = 0; i < BOARD_SIZE; i++)
        for (int j = 0; j < BOARD_SIZE; j++)
            if (board[i][j] == kingChar) {
                kingRow = i;
                kingCol = j;
                break;
            }

    if (kingRow == -1 || kingCol == -1)
        return 1; // King not found, in check

    // Check for attacks by opponent pieces
    for (int i = 0; i < BOARD_SIZE; i++)
        for (int j = 0; j < BOARD_SIZE; j++)
            if (isOpponentPiece(turn, board[i][j])) {
                if (isMoveValid(1 - turn, i, j, kingRow, kingCol))
                    return 1;
            }
    return 0;
}

void movePiece(int turn, int srcRow, int srcCol, int destRow, int destCol, int moveType) {
    char piece = board[srcRow][srcCol];

    // Update moved flags for king and rooks
    if (piece == 'K') whiteKingMoved = 1;
    if (piece == 'k') blackKingMoved = 1;
    if (piece == 'R' && srcRow == 7 && srcCol == 0) whiteRookMoved[0] = 1;
    if (piece == 'R' && srcRow == 7 && srcCol == 7) whiteRookMoved[1] = 1;
    if (piece == 'r' && srcRow == 0 && srcCol == 0) blackRookMoved[0] = 1;
    if (piece == 'r' && srcRow == 0 && srcCol == 7) blackRookMoved[1] = 1;

    board[destRow][destCol] = board[srcRow][srcCol];
    board[srcRow][srcCol] = ' ';
}

int hasLegalMoves(int turn) {
    for (int srcRow = 0; srcRow < BOARD_SIZE; srcRow++) {
        for (int srcCol = 0; srcCol < BOARD_SIZE; srcCol++) {
            if (isOwnPiece(turn, board[srcRow][srcCol])) {
                for (int destRow = 0; destRow < BOARD_SIZE; destRow++) {
                    for (int destCol = 0; destCol < BOARD_SIZE; destCol++) {
                        int moveType = isMoveValid(turn, srcRow, srcCol, destRow, destCol);
                        if (moveType) {
                            // Make the move temporarily
                            char tempDest = board[destRow][destCol];
                            char tempSrc = board[srcRow][srcCol];
                            movePiece(turn, srcRow, srcCol, destRow, destCol, moveType);
                            int inCheck = isKingInCheck(turn);
                            // Undo the move
                            board[srcRow][srcCol] = tempSrc;
                            board[destRow][destCol] = tempDest;
                            if (!inCheck)
                                return 1;
                        }
                    }
                }
            }
        }
    }
    return 0;
}

int generateAllMoves(int turn, Move moves[]) {
    int moveCount = 0;

    for (int srcRow = 0; srcRow < BOARD_SIZE; srcRow++) {
        for (int srcCol = 0; srcCol < BOARD_SIZE; srcCol++) {
            if (isOwnPiece(turn, board[srcRow][srcCol])) {
                for (int destRow = 0; destRow < BOARD_SIZE; destRow++) {
                    for (int destCol = 0; destCol < BOARD_SIZE; destCol++) {
                        int moveType = isMoveValid(turn, srcRow, srcCol, destRow, destCol);
                        if (moveType) {
                            // Make the move temporarily
                            char tempDest = board[destRow][destCol];
                            char tempSrc = board[srcRow][srcCol];
                            movePiece(turn, srcRow, srcCol, destRow, destCol, moveType);
                            int inCheck = isKingInCheck(turn);
                            // Undo the move
                            board[srcRow][srcCol] = tempSrc;
                            board[destRow][destCol] = tempDest;
                            if (!inCheck) {
                                moves[moveCount].srcRow = srcRow;
                                moves[moveCount].srcCol = srcCol;
                                moves[moveCount].destRow = destRow;
                                moves[moveCount].destCol = destCol;
                                moves[moveCount].moveType = moveType;
                                moveCount++;
                                if (moveCount >= MAX_MOVES)
                                    return moveCount;
                            }
                        }
                    }
                }
            }
        }
    }

    return moveCount;
}

int evaluateBoard() {
    int score = 0;
    int pieceValues[128] = { 0 };

    pieceValues['p'] = -10;
    pieceValues['n'] = -30;
    pieceValues['b'] = -30;
    pieceValues['r'] = -50;
    pieceValues['q'] = -90;
    pieceValues['k'] = -900;

    pieceValues['P'] = 10;
    pieceValues['N'] = 30;
    pieceValues['B'] = 30;
    pieceValues['R'] = 50;
    pieceValues['Q'] = 90;
    pieceValues['K'] = 900;

    for (int i = 0; i < BOARD_SIZE; i++)
        for (int j = 0; j < BOARD_SIZE; j++)
            score += pieceValues[(int)board[i][j]];

    return score;
}

int minimax(int depth, int turn, int alpha, int beta) {
    if (depth == 0) {
        return evaluateBoard();
    }

    Move moves[MAX_MOVES];
    int moveCount = generateAllMoves(turn, moves);

    if (moveCount == 0) {
        if (isKingInCheck(turn))
            return (turn == 0) ? -10000 : 10000; // Checkmate
        else
            return 0; // Stalemate
    }

    if (turn == 0) { // White's turn
        int maxEval = INT_MIN;
        for (int i = 0; i < moveCount; i++) {
            Move m = moves[i];
            char tempSrc = board[m.srcRow][m.srcCol];
            char tempDest = board[m.destRow][m.destCol];

            movePiece(turn, m.srcRow, m.srcCol, m.destRow, m.destCol, m.moveType);
            int eval = minimax(depth - 1, 1 - turn, alpha, beta);
            board[m.srcRow][m.srcCol] = tempSrc;
            board[m.destRow][m.destCol] = tempDest;

            if (eval > maxEval)
                maxEval = eval;
            if (maxEval > alpha)
                alpha = maxEval;
            if (beta <= alpha)
                break; // Beta cutoff
        }
        return maxEval;
    }
    else { // Black's turn (Bot)
        int minEval = INT_MAX;
        for (int i = 0; i < moveCount; i++) {
            Move m = moves[i];
            char tempSrc = board[m.srcRow][m.srcCol];
            char tempDest = board[m.destRow][m.destCol];

            movePiece(turn, m.srcRow, m.srcCol, m.destRow, m.destCol, m.moveType);
            int eval = minimax(depth - 1, 1 - turn, alpha, beta);
            board[m.srcRow][m.srcCol] = tempSrc;
            board[m.destRow][m.destCol] = tempDest;

            if (eval < minEval)
                minEval = eval;
            if (minEval < beta)
                beta = minEval;
            if (beta <= alpha)
                break; // Alpha cutoff
        }
        return minEval;
    }
}

void botMove() {
    Move moves[MAX_MOVES];
    int moveCount = generateAllMoves(1, moves);

    int bestEval = INT_MAX;
    Move bestMove;

    for (int i = 0; i < moveCount; i++) {
        Move m = moves[i];
        char tempSrc = board[m.srcRow][m.srcCol];
        char tempDest = board[m.destRow][m.destCol];

        movePiece(1, m.srcRow, m.srcCol, m.destRow, m.destCol, m.moveType);
        int eval = minimax(MAX_DEPTH - 1, 0, INT_MIN, INT_MAX);
        board[m.srcRow][m.srcCol] = tempSrc;
        board[m.destRow][m.destCol] = tempDest;

        if (eval < bestEval) {
            bestEval = eval;
            bestMove = m;
        }
    }

    movePiece(1, bestMove.srcRow, bestMove.srcCol, bestMove.destRow, bestMove.destCol, bestMove.moveType);

    printf("Black plays %c%d to %c%d\n",
        bestMove.srcCol + 'a', 8 - bestMove.srcRow,
        bestMove.destCol + 'a', 8 - bestMove.destRow);
}

int main() {
    initBoard();

    char input[10];
    int turn = 0; // 0 for White (Human), 1 for Black (Bot)

    while (1) {
        // Clear the console (Use "cls" instead of "clear" on Windows)
        system("clear");
        printBoard();

        if (isKingInCheck(turn)) {
            if (!hasLegalMoves(turn)) {
                if (turn == 0)
                    printf("Checkmate! Black wins!\n");
                else
                    printf("Checkmate! White wins!\n");
                break;
            }
            else {
                printf("Check!\n");
            }
        }
        else if (!hasLegalMoves(turn)) {
            printf("Stalemate!\n");
            break;
        }

        if (turn == 0) {
            printf("Your move (e.g., e2e4 or type 'exit' to quit): ");

            fgets(input, sizeof(input), stdin);

            if (strncmp(input, "exit", 4) == 0)
                break;

            // Parse input like "e2e4"
            int srcCol = input[0] - 'a';
            int srcRow = 8 - (input[1] - '0');
            int destCol = input[2] - 'a';
            int destRow = 8 - (input[3] - '0');

            // Simple bounds checking
            if (srcRow < 0 || srcRow >= BOARD_SIZE || srcCol < 0 || srcCol >= BOARD_SIZE ||
                destRow < 0 || destRow >= BOARD_SIZE || destCol < 0 || destCol >= BOARD_SIZE) {
                printf("Invalid move. Press Enter to continue.\n");
                getchar();
                continue;
            }

            int moveType = isMoveValid(turn, srcRow, srcCol, destRow, destCol);

            if (!moveType) {
                printf("Invalid move. Press Enter to continue.\n");
                getchar();
                continue;
            }

            // Make the move temporarily
            char tempDest = board[destRow][destCol];
            char tempSrc = board[srcRow][srcCol];
            movePiece(turn, srcRow, srcCol, destRow, destCol, moveType);

            // Check if own king is in check after move
            if (isKingInCheck(turn)) {
                // Undo the move
                board[srcRow][srcCol] = tempSrc;
                board[destRow][destCol] = tempDest;
                printf("Move puts own king in check. Press Enter to continue.\n");
                getchar();
                continue;
            }

            turn = 1 - turn; // Switch turns
        }
        else {
            printf("Black is thinking...\n");
            botMove();
            turn = 1 - turn; // Switch turns
        }
    }

    return 0;
}


