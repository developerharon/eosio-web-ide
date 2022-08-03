#include "tictactoe.hpp";


// ensure that the action has the signature from the host
// ensure that the challenger and host are not the same player
// ensure that there is not existing game
// store the newly created game to the multi index table
void tictactoe::create(const name &challenger, name &host) {
    require_auth(host);
    check(challenger != host, "Challenger should not be the same as the host.");

    // check if game already exists
    games existingHostGames(get_self(), host.value);
    auto itr = existingHostGames.find(challenger.value);
    check(itr == existingHostGames.end(), "Game already exists");

    existingHostGames.emplace(host, [&](auto &g) {
        g.challenger = challenger;
        g.host = host;
        g.turn = host;
    });
}

// ensure that the action has the signature from the host/challenger
// ensure that the game exists
// ensure that the restart action is done by host/challenger
// reset the game
// store the updated game to the multi index table
void tictactoe::restart(const name &challenger, const name &host, const name &by) {
    check(has_auth(by), "Only " + by.to_string() + " can restart the game.");

    // check if game exists
    games existingHostGames(get_self(), host.value);
    auto itr = existingHostGames.find(challenger.value);
    check (itr != existingHostGames.end(), "Game does not exist.");

    // check if this game belongs to the action sender
    check(by == itr->host || by == itr->challenger, "This is not your game.");

    // Reset game
    existingHostGames.modify(itr, itr->host, [](auto &g) {
        g.resetGame();
    });
}

// ensure that the action has the signature from the host
// ensure that the game exists
// remove the game from the db
void tictactoe::close(const name &challenger, const name &host) {
    check(has_auth(host), "Only the host can close the game.");

    require_auth(host);

    // check if games exists
    games existingHostGames(get_self(), host.value);
    auto itr = existingHostGames.find(challenger.value);

    check(itr != existingHostGames.end(), "Game does not exist.");

    // remove the game
    existingHostGames.erase(itr);
}

// implement isEmptyCell
bool tictactoe::isEmptyCell(const uint8_t &cell) {
    return cell == 0;
}

// implement isValidMove
bool tictactoe::isValidMove(const uint16_t &row, const uint16_t &column, const std::vector<uint8_t> &board) {
    uint32_t movementLocation = row * game::boardWidth + column;
    bool isValid = movementLocation < board.size() && isEmptyCell(board[movementLocation]);
    return isValid;
}

// implement getWinner. 
// The winner is the first player who places three of their marks in a horizontal, vertical or diagonal row. 
name tictactoe::getWinner(const game &currentGame) {
    auto &board = currentGame.board;

    bool isBoardFull = true;

    // use bitwise AND operator to determine the consecutive values of each column, row and diagonal
    // since 3 = 0b11, 2 = 0b10, 1 = 0b01, 0=0b00
    std::vector<uint32_t> consecutiveColumn(game::boardWidth, 3);
    std::vector<uint32_t> consecutiveRow(game::boardHeight, 3);
    uint32_t consecutiveDiagonalBackslash = 3;
    uint32_t consecutiveDiagonalSlash = 3;

    for (uint32_t i = 0; i < board.size(); i++) {
        isBoardFull &= isEmptyCell(board[i]);
        uint16_t row = uint16_t(i / game::boardWidth);
        uint16_t column = uint16_t(i % game::boardWidth);

        // calculate consectutve row and column value
        consecutiveRow[column] = consecutiveRow[column] & board[i];
        consecutiveColumn[row] = consecutiveColumn[row] & board[i];

        // calculate consecutive diagonal \ value
        if (row == column) {
            consecutiveDiagonalBackslash = consecutiveDiagonalBackslash & board[i];
        }

        // calculate consecutive diagonal / value
        if (row + column == game::boardWidth - 1) {
            consecutiveDiagonalSlash = consecutiveDiagonalSlash & board[i];
        }
    }

    // Inspect the value of all consecutive row, column, and diagonal and determine winner
    std::vector<uint32_t> aggregate = { consecutiveDiagonalBackslash, consecutiveDiagonalSlash };
    aggregate.insert(aggregate.end(), consecutiveColumn.begin(), consecutiveColumn.end());
    aggregate.insert(aggregate.end(), consecutiveRow.begin(), consecutiveRow.end());

    for (auto value : aggregate) {
        if (value == 1) {
            return currentGame.host;
        }
        else if (value == 2) {
            return currentGame.challenger;
        }
    }

    // Draw if the board is full, otherwise the winner is not determined yet
    return isBoardFull ? draw : none;
}

// ensure that the action has the signature from the host/challenger
// ensure that the game exists
// ensure that the game is not finished yet
// ensure that the move action is done by host or challenger
// ensure that this is the right user's turn
// verify movement is valid
// update board with the new move
// change the move_turn to the other player
// Determine if there is a winner
// Store the updated game tothe multi index table
void tictactoe::move(const name &challenger, const name &host, const name &by, const uint16_t &row, const uint16_t &column) {
    check(has_auth(by), "The next move should be made by " + by.to_string());

    // check if game exists
    games existingHostGames(get_self(), host.value);
    auto itr = existingHostGames.find(challenger.value);
    check(itr != existingHostGames.end(), "Game does not exist.");

    // Check if this game hasn't ended yet. 
    check(itr-> winner == none, "The game has ended.");

    // check if this game belongs to the action sender
    check(by == itr->host || by == itr->challenger, "This is not your game.");

    // check if this is the action sender's turn
    check(by == itr->turn, "It's not your turn yet!");

    // check if user makes a valid movement
    check(isValidMove(row, column, itr->board), "Not a valid movement.");

    // Fill the cell, 1 for host, 2 for challenger
    // TODO could use constant for 1 and 2 as well. 
    const uint8_t cellValue = itr->turn == itr->host ? 1 : 2;
    const auto turn = itr->turn == itr->host ? itr->challenger : itr->host; 
    
    existingHostGames.modify(itr, itr->host, [&](auto &g) {
        g.board[row * game::boardWidth + column] = cellValue;
        g.turn = turn;
        g.winner = getWinner(g);
    });
}