#ifndef ZOBRIST_H
#define ZOBRIST_H

// I only use the number of unique pieces, not the total number of pieces.  This
// is because we want a key match on "equivilent" boards.  i.e. if we transposed
// two same-colored rooks, the board is functionally the same.
const int BOARD_SIZE = 8 * 8;   // size of chess board
const int NUM_PIECES = 6;       // rook, knight, bishop, king, queen, pawn
const int NUM_COLORS = 2;       // white, black

enum eChessPiece { EMPTY,
                   B_ROOK, B_KNIGHT, B_BISHOP, B_KING, B_QUEEN, B_PAWN,
                   W_ROOK, W_KNIGHT, W_BISHOP, W_KING, W_QUEEN, W_PAWN };
enum eColor { BLACK, WHITE };

class ChessBoard
{
    uint64_t m_aZobristTable[BOARD_SIZE][NUM_PIECES][NUM_COLORS];
    eChessPiece m_aBoard[BOARD_SIZE];

    void InitializeZobristTable();
    void PopulateChessBoard();

public:
    ChessBoard();
    uint64_t CalculateZobristKey(eColor sideToMove) const;
    uint64_t UpdateZobristKey(uint64_t oldKey, eChessPiece piece, int oldPos, int newPos) const;
    void MovePiece(int oldPos, int newPos);
};

#endif

