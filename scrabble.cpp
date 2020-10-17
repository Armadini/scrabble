#include "scrabble.h"

#include "formatting.h"
#include <iomanip>
#include <iostream>
#include <map>
#include <string>

using namespace std;

// Given to you. this does not need to be changed
Scrabble::Scrabble(const ScrabbleConfig& config)
        : hand_size(config.hand_size),
          minimum_word_length(config.minimum_word_length),
          tile_bag(TileBag::read(config.tile_bag_file_path, config.seed)),
          board(Board::read(config.board_file_path)),
          dictionary(Dictionary::read(config.dictionary_file_path)) {}

void Scrabble::add_players() {
    // Get number of players to be added
    std::cout << "How many players?" << endl;
    int num_players;
    cin >> num_players;
    cin.ignore();

    // Retrieve player names, create HumanPlayer object, add players vector
    string player_name;
    for (int i = 0; i < num_players; i++) {
        // Get Player Name
        std::cout << "Player " << i + 1 << " name:" << endl;
        getline(cin, player_name);

        // Fill tile collection and remove from tile bag
        std::vector<TileKind> player_tiles = tile_bag.remove_random_tiles(hand_size);
        // TileCollection tiles;
        // for (int tile=0; tile<player_tiles.size(); tile++) {
        //     tiles.add_tile(player_tiles[tile]);
        // }

        // Create shared_ptr to HumanPlayer object and add to players vector
        shared_ptr<HumanPlayer> p = make_shared<HumanPlayer>(player_name, hand_size);
        p->add_tiles(player_tiles);
        players.push_back(p);
    }
}

// Game Loop should cycle through players and get and execute that players move
// until the game is over.
void Scrabble::game_loop() {
    // TODO: implement this.

    // standard_tile_bag = tile_bag;

    while (true) {
        bool all_pass = false;
        // later used to check if all players passed their turn
        // Loop through each player and ask for their move, then execute
        for (auto& player : players) {
            // cout << "It is " << PLAYER_NAME_COLOR << {player->get_name()} << rang::style::reset << "'s turn." <<
            // endl; cout << PLAYER_NAME_COLOR << {player->get_name()} << rang::style::reset << "has " << SCORE_COLOR <<
            // {player->get_score()} << rang::style::reset << "points." << endl;
            board.print(cout);
            std::cout << "It is " << player->get_name() << "'s turn." << endl;
            std::cout << player->get_name() << " has " << player->get_points() << " points." << endl;
            // std::cout << player->get_name() << "'s hand: " << player->print_hand() << endl;

            // Get players move
            bool b = true;
            Move move;
            while (b) {
                try {
                    move = player->get_move(board, dictionary);
                    b = false;
                    // } catch (MoveException e) {
                    //     cout << e.what();
                } catch (CommandException e) {
                    cout << e.what();
                }
            }
            // Check the type of move and act accordingly
            if (move.kind == MoveKind::PASS) {
                // if the player wants to pass, add a pass move to the moves vector
                moves.push_back(move);
            } else if (move.kind == MoveKind::EXCHANGE) {
                // if the player wants to exchange, get the tiles they want to exchange
                std::vector<TileKind> tiles_to_exchange = move.tiles;
                int num_tiles = tiles_to_exchange.size();
                // for each tile character, create a TileKind object and push it to the tile_bag
                for (auto& tile : tiles_to_exchange) {
                    tile_bag.add_tile(tile);
                }
                // Remove the tiles from the player and add n random tiles
                player->remove_tiles(tiles_to_exchange);
                player->add_tiles(tile_bag.remove_random_tiles(num_tiles));
                // add the move to the moves vector
                moves.push_back(move);
            } else if (move.kind == MoveKind::PLACE) {
                PlaceResult p = board.place(move);
                int t_points = p.points;
                if (move.tiles.size() == hand_size) {
                    t_points += 50;
                }
                std::cout << "That move got you " << t_points << " points!" << std::endl;
                player->add_points(t_points);
                std::cout << player->get_name() << " now has " << player->get_points() << " points!" << std::endl;
                moves.push_back(move);

                player->remove_tiles(move.tiles);
                player->add_tiles(tile_bag.remove_random_tiles(move.tiles.size()));
            }
            // checks if the last player.size() number of moves have been pass. If so, the while loop will break.
            if (moves.size() >= players.size()) {
                all_pass = true;
                for (int m = moves.size() - players.size(); m < moves.size(); m++) {
                    if (moves[m].kind != MoveKind::PASS)
                        all_pass = false;
                }
            }

            // ends player loop if all players have passed
            if (all_pass)
                break;
        }
        // ends game loop if all players have passed
        if (all_pass) {
            std::cout << "All players passed their move. GAME OVER..." << std::endl;
            break;
        }
    }

    // Useful cout expressions with fancy colors. Expressions in curly braces, indicate values you supply.
    // cout << "You gained " << SCORE_COLOR << {points} << rang::style::reset << " points!" << endl;
    // cout << "Your current score: " << SCORE_COLOR << {points} << rang::style::reset << endl;
    // cout << endl << "Press [enter] to continue.";
}

// Performs final score subtraction. Players lose points for each tile in their
// hand. The player who cleared their hand receives all the points lost by the
// other players.
void Scrabble::final_subtraction(vector<shared_ptr<Player>>& plrs) {
    // TODO: implement this method.
    // Do not change the method signature.

    // winner represents player with 0 tiles left
    std::shared_ptr<Player> winner = nullptr;
    // total is the total tiles in all hands to add to winners score
    int total = 0;
    for (auto player : plrs) {
        // if the player has no tiles remaining, they are the "winner" and will have the "total" added to their score
        if (player->count_tiles() == 0) {
            winner = player;
        } else {
            // gets the value of the players current hand, adds it to the total, and subtracts it from the player's
            // score
            unsigned int hand_val = player->get_hand_value();
            player->subtract_points(hand_val);
            total += hand_val;
        }
    }
    // if there is a winner, add the total of other player's hands to their score
    if (winner != nullptr) {
        winner->add_points(total);
    }
}

// You should not need to change this function.
void Scrabble::print_result() {
    // Determine highest score
    size_t max_points = 0;
    for (auto player : this->players) {
        if (player->get_points() > max_points) {
            max_points = player->get_points();
        }
    }

    // Determine the winner(s) indexes
    vector<shared_ptr<Player>> winners;
    for (auto player : this->players) {
        if (player->get_points() >= max_points) {
            winners.push_back(player);
        }
    }

    std::cout << (winners.size() == 1 ? "Winner:" : "Winners: ");
    for (auto player : winners) {
        std::cout << SPACE << PLAYER_NAME_COLOR << player->get_name();
    }
    std::cout << rang::style::reset << endl;

    // now print score table
    std::cout << "Scores: " << endl;
    std::cout << "---------------------------------" << endl;

    // Justify all integers printed to have the same amount of character as the high score, left-padding with spaces
    std::cout << setw(static_cast<uint32_t>(floor(log10(max_points) + 1)));

    for (auto player : this->players) {
        std::cout << SCORE_COLOR << player->get_points() << rang::style::reset << " | " << PLAYER_NAME_COLOR
                  << player->get_name() << rang::style::reset << endl;
    }
}

// You should not need to change this.
void Scrabble::main() {
    add_players();
    game_loop();
    final_subtraction(this->players);
    print_result();
}

//-fsanitize=address