#include <cassert>
#include <cstdint>
#include "zobrist.h"
#include "mersenne.h"

// Use a constant instead of transposing the key depending on the side to play.
// This is more efficient, since I can more easily undo operations (so I can
// incrementally update the key).
static const uint64_t BLACK_TO_MOVE = 0x8913125CFB309AFC;   // Random number to XOR into black moves

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

eColor GetPieceColor(eChessPiece piece)
{
    return piece < W_ROOK ? BLACK : WHITE;
}

eChessPiece GetUncoloredPiece(eChessPiece piece)
{
    return piece < W_ROOK ? piece : eChessPiece(piece - NUM_PIECES);
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
            color = GetPieceColor(m_aBoard[ii]);
            piece = GetUncoloredPiece(m_aBoard[ii]);

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

    int color = GetPieceColor(piece);
    piece = GetUncoloredPiece(piece);

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

