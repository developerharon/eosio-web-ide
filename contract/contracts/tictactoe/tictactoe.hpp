// Import the eosio base library.
#include <eosio/eosio.hpp>

// Use the eosio namespace
using namespace eosio;

// Declare the class 
// Use [[eosio::contract(contract_name)]] attribute
// Inherit from the base class 
class [[eosio::contract("tictactoe")]] tictactoe : public contract {
    public:

        // introduce base class members.
        using contract::contract;

        // Use the base class constructor.
        tictactoe(name receiver, name code, datastream<const char*> ds) : contract(receiver, code, ds) { }

        // declare the game data structure.
        struct [[eosio::table]] game {
            static constexpr uint16_t boardWidth = 3;
            static constexpr uint16_t boardHeight = boardWidth;

            game() : board(boardWidth * boardHeight, 0) {}

            // account name of host, challenger and turn to store whose turn it is. 
            name challenger, host, turn;

            // none, draw, name of host, name of challenger
            name winner = none;

            std::vector<uint8_t> board;

            // initialize the board with empty cell
            void initializeBoard() {
                board.assign(boardWidth * boardHeight, 0);
            }

            // reset game 
            void resetGame() {
                initializeBoard();
                turn = host;
                winner = "none"_n;
            }

            // primary key accessor
            auto primary_key() const { return challenger.value; }

            // EOSLIB_SERIALIZE macro defining how the abi serializes/deserializes
            EOSLIB_SERIALIZE(game, (challenger)(host)(turn)(winner)(board))
        };

        // define the game data structure using the multi-index table template. 
        typedef eosio::multi-index<"games"_n, game> games;

        [[eosio::action]]
        void create(const name &challenger, name &host);

        [[eosio::action]]
        void restart(const name &challenger, const name &host, const name &by);

        [[eosio::action]]
        void close(const name &challenger, const name &host);

        [[eosio::action]]
        void move(const name &challenger, const name &host, const name &by, const uint16_t &row, const uint16_t &column);

    private:
        bool isEmptyCell(const uint8_t &cell);
        bool isValidMovel(const uint16_t &row, const uint16_t &column, const std::vector<uint8_t> &board);
        name getWinner(const game &currentGame);
};