// Demonstration code for Zobrist Hash using the Mersenne Twister
// Compiles under Visual Studio 2010 under Windows and gcc under Linux.
// By Toby Jones

#include <cassert>
#include <cstdint>
#include <iostream>
#include "mersenne.h"

// I only use the number of unique pieces, not the total number of pieces.  This
// is because we want a key match on "equivilent" boards.  i.e. if we transposed
// two same-colored rooks, the board is functionally the same.
static const int BOARD_SIZE = 8 * 8;    // size of chess board
static const int NUM_PIECES = 6;        // rook, knight, bishop, king, queen, pawn
static const int NUM_COLORS = 2;        // white, black

// Use a constant instead of transposing the key depending on the side to play.
// This is more efficient, since I can more easily undo operations (so I can
// incrementally update the key).
static const uint64_t BLACK_TO_MOVE = 0x8913125CFB309AFC;   // Random number to XOR into black moves

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

void ChessBoard::InitializeZobristTable()
{
    MersenneTwister rng;

    // Use the Mersenne Twister to fill up the Zobrist random table
    for(int ii = 0; ii < BOARD_SIZE; ii++)
    {
        for(int jj = 0; jj < NUM_PIECES; jj++)
        {
            for(int kk = 0; kk < NUM_COLORS; kk++)
            {
                m_aZobristTable[ii][jj][kk] = rng.Rand64();
            }
        }
    }
}

void ChessBoard::PopulateChessBoard()
{
    // To test different hash possibilities, change this to reflect
    // the initial board state to test.
    int ii;
    for(ii = 16; ii < (BOARD_SIZE - 16); ii++)
    {
        m_aBoard[ii] = EMPTY;
    }

    // Set up pawns
    for(ii = 0; ii < 8; ii++)
    {
        m_aBoard[ii + 8] = W_PAWN;
    }
    for(ii = 0; ii < 8; ii++)
    {
        m_aBoard[ii + (BOARD_SIZE - 16)] = B_PAWN;
    }

    m_aBoard[0] = W_ROOK;
    m_aBoard[1] = W_KNIGHT;
    m_aBoard[2] = W_BISHOP;
    m_aBoard[3] = W_QUEEN;
    m_aBoard[4] = W_KING;
    m_aBoard[5] = W_BISHOP;
    m_aBoard[6] = W_KNIGHT;
    m_aBoard[7] = W_ROOK;

    m_aBoard[56] = B_ROOK;
    m_aBoard[57] = B_KNIGHT;
    m_aBoard[58] = B_BISHOP;
    m_aBoard[59] = B_QUEEN;
    m_aBoard[60] = B_KING;
    m_aBoard[61] = B_BISHOP;
    m_aBoard[62] = B_KNIGHT;
    m_aBoard[63] = B_ROOK;

    for(ii = 0; ii < BOARD_SIZE; ii++)
    {
        assert(m_aBoard[ii] >= EMPTY && m_aBoard[ii] <= W_PAWN);
    }
}

ChessBoard::ChessBoard()
{
    InitializeZobristTable();

    PopulateChessBoard();
}

uint64_t ChessBoard::CalculateZobristKey(
    eColor sideToMove) const
{
    uint64_t uZobristKey = 0;

    eChessPiece piece = EMPTY;
    eColor color      = BLACK;

    for(unsigned int ii = 0; ii < sizeof(m_aBoard) / sizeof(m_aBoard[0]); ii++)
    {
        if(EMPTY != m_aBoard[ii])
        {
            if(W_ROOK <= m_aBoard[ii])
            {
                color = WHITE;
            }
            else
            {
                color = BLACK;
            }

            piece = eChessPiece(m_aBoard[ii]);
            if(WHITE == color)
            {
                piece = eChessPiece(piece - NUM_PIECES);
            }

            uZobristKey ^= m_aZobristTable[ii][piece][color];
        }
    }

    if(BLACK == sideToMove)
    {
        uZobristKey ^= BLACK_TO_MOVE;
    }

    return uZobristKey;
}

uint64_t ChessBoard::UpdateZobristKey(
    uint64_t    oldKey,
    eChessPiece piece,
    int         oldPos,
    int         newPos) const
{
    assert(piece == m_aBoard[oldPos]);

    uint64_t newKey = oldKey;

    int color = piece < W_ROOK ? BLACK : WHITE;
    piece = piece < W_ROOK ? piece : eChessPiece(piece - NUM_PIECES);

    newKey ^= m_aZobristTable[oldPos][piece][color];    // remove piece from the key
    newKey ^= m_aZobristTable[newPos][piece][color];    // re-add piece to the key in new position

    newKey ^= BLACK_TO_MOVE;    // apply or undo previous setting

    return newKey;
}

void ChessBoard::MovePiece(int oldPos, int newPos)
{
    m_aBoard[newPos] = m_aBoard[oldPos];
    m_aBoard[oldPos] = EMPTY;
}

// Output the first 1024 generated numbers
static void TestMT()
{
    std::cout << "Outputting the first 1024 generated numbers" << std::endl;

    MersenneTwister rng;

    for(int ii = 0; ii < 1024; ii++)
    {
        std::cout << rng.Rand() << std::endl;
    }
}

// Demonstrate that a full hash calculation is the same as an incremental operation
static void TestZH()
{
    ChessBoard chessBoard;

    uint64_t initialZobristKey = chessBoard.CalculateZobristKey(WHITE);

    uint64_t newIncrementalZobristKey = chessBoard.UpdateZobristKey(initialZobristKey,
                                                                    W_PAWN,
                                                                    8,
                                                                    24);

    chessBoard.MovePiece(8, 24);
    uint64_t newFullZobristKey = chessBoard.CalculateZobristKey(BLACK);

    std::cout << "Initial Zobrist Key: ";
    std::cout << initialZobristKey << std::endl;

    std::cout << "Moving white pawn from a2 to a4..." << std::endl;

    std::cout << "New Zobrist Key (incremental): ";
    std::cout << newIncrementalZobristKey << std::endl;
    std::cout << "New Zobrist Key (full):        ";
    std::cout << newFullZobristKey << std::endl;

    if(newIncrementalZobristKey == newFullZobristKey)
    {
        std::cout << "Zobrist keys match." << std::endl;
    }
    else
    {
        std::cout << "Zobrist keys do _not_ match." << std::endl;
    }
}

int main()
{
    std::cout.setf(std::ios::showbase);
    std::cout.setf(std::ios::hex, std::ios::basefield);

    std::cout << "-Testing Mersenne Twister-" << std::endl;
    TestMT();

    std::cout << std::endl;

    std::cout << "-Testing Zobrist Hash-" << std::endl;
    TestZH();

    return 0;
}
