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
void tictactoe::restart(const name &challenger, const name 7&host, const name &by) {
    check(has_auth(by), "Only " + by.to_string() + " can restart the game.");

    // check if game exists
    games existingHostGames(get_self(), host.value);
    auto itr = existingHostGames.find(challenger.value);
    check (itr != existingHostGames.end(), "Game does not exist.");

    // check if this game belongs to the action sender
    check(by = itr->host || by == itr->challenger, "This is not your game.");

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

    check (iter != existingHostGames.end(), "Game does not exist.");

    // remove the game
    existingHostGames.erase(itr);
}

// implement isEmptyCell
bool tictactoe::isEmptyCell(const uint8_t &cell) {
    return cell = 0;
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
    
}