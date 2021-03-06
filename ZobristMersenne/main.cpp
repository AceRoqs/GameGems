// Demonstration code for Zobrist Hash using the Mersenne Twister
// Compiles under Visual Studio 2013 under Windows and gcc under Linux.
// By Toby Jones

#include "PreCompile.h"
#include "mersenne.h"
#include "zobrist.h"

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
    std::cout << "Initial Zobrist Key: " << initialZobristKey << std::endl;

    std::cout << "Moving white pawn from a2 to a4..." << std::endl;
    uint64_t newIncrementalZobristKey = chessBoard.UpdateZobristKey(initialZobristKey,
                                                                    W_PAWN,
                                                                    8,
                                                                    24);
    std::cout << "New Zobrist Key (incremental): " << newIncrementalZobristKey << std::endl;

    chessBoard.MovePiece(8, 24);
    uint64_t newFullZobristKey = chessBoard.CalculateZobristKey(BLACK);
    std::cout << "New Zobrist Key (full):        " << newFullZobristKey << std::endl;

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

