#include "human_player.h"

#include "exceptions.h"
#include "formatting.h"
#include "move.h"
#include "place_result.h"
#include "rang.h"
#include "tile_kind.h"
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <vector>

using namespace std;

// This method is fully implemented.
inline string& to_upper(string& str) {
    transform(str.begin(), str.end(), str.begin(), ::toupper);
    return str;
}

inline string& to_lower(string& str) {
    transform(str.begin(), str.end(), str.begin(), ::tolower);
    return str;
}

Move HumanPlayer::get_move(const Board& board, const Dictionary& d) const {
    // prints hand for player to see
    this->print_hand(cout);
    // gets the players move as a string
    std::string move_string;
    cin >> move_string;
    // creates a move object based on the players input
    Move m = this->parse_move(to_upper(move_string));
    // if the move object is pass or exchange, it is already checked for validity so we return it
    if (m.kind != MoveKind::PLACE)
        return m;
    // if the move is place, we test the validity of place, then either return m or throw an error
    PlaceResult p = board.test_place(m);
    if (p.valid) {
        for (auto word : p.words) {
            if (!d.is_word(to_lower(word))) {
                throw CommandException("One of the words created was invalid!");
            }
        }
        return m;
    } else {
        throw CommandException(p.error);
    }
}

vector<TileKind> HumanPlayer::parse_tiles(string& letters, bool place) const {
    // TODO: begin implementation here.
    vector<TileKind> tiles_to_exchange;
    // for each tile character, add a TileKind objet to the tiles_to_exchange vector
    for (int i = 0; i < letters.size(); i++) {
        TileKind t = tiles.lookup_tile(letters[i]);
        if (t.letter == '?' && place) {
            t.assigned = letters[i + 1];
            i++;
        }
        tiles_to_exchange.push_back(t);
    }
    return tiles_to_exchange;
}

Move HumanPlayer::parse_move(string& move_string) const {
    // TODO: begin implementation here.
    if (move_string == "PASS") {
        // if the player wants to pass, add a pass move to the moves vector
        return Move();
    } else if (move_string == "EXCHANGE") {
        // if the player wants to exchange tiles, get the tiles they want to exchange as a string then loop through it
        std::string letters;
        cin >> letters;
        cin.ignore();
        vector<TileKind> tiles_to_exchange = this->parse_tiles(letters, false);
        // return an exchange move
        return Move(tiles_to_exchange);
    } else if (move_string == "PLACE") {
        // get the direction and store it in 'd'
        Direction d;
        string s;
        cin >> s;
        if (s == "-") {
            d = Direction::ACROSS;
        } else if (s == "|") {
            d = Direction::DOWN;
        } else {
            throw domain_error("Incorrect PLACE direction");
        }
        // get the starting row and column index
        int row;
        cin >> row;
        row -= 1;
        int col;
        cin >> col;
        col -= 1;
        // get the tiles to be placed
        std::string letters;
        cin >> letters;
        cin.ignore();
        vector<TileKind> tiles_to_exchange = this->parse_tiles(letters, true);
        // return move object
        return Move(tiles_to_exchange, row, col, d);

    } else {
        throw;
    }
}

// This function is fully implemented.
void HumanPlayer::print_hand(ostream& out) const {
    const size_t tile_count = tiles.count_tiles();
    const size_t empty_tile_count = this->get_hand_size() - tile_count;
    const size_t empty_tile_width = empty_tile_count * (SQUARE_OUTER_WIDTH - 1);

    for (size_t i = 0; i < HAND_TOP_MARGIN - 2; ++i) {
        out << endl;
    }

    out << repeat(SPACE, HAND_LEFT_MARGIN) << FG_COLOR_HEADING << "Your Hand: " << endl << endl;

    // Draw top line
    out << repeat(SPACE, HAND_LEFT_MARGIN) << FG_COLOR_LINE << BG_COLOR_NORMAL_SQUARE;
    print_horizontal(tile_count, L_TOP_LEFT, T_DOWN, L_TOP_RIGHT, out);
    out << repeat(SPACE, empty_tile_width) << BG_COLOR_OUTSIDE_BOARD << endl;

    // Draw middle 3 lines
    for (size_t line = 0; line < SQUARE_INNER_HEIGHT; ++line) {
        out << FG_COLOR_LABEL << BG_COLOR_OUTSIDE_BOARD << repeat(SPACE, HAND_LEFT_MARGIN);
        for (auto it = tiles.cbegin(); it != tiles.cend(); ++it) {
            out << FG_COLOR_LINE << BG_COLOR_NORMAL_SQUARE << I_VERTICAL << BG_COLOR_PLAYER_HAND;

            // Print letter
            if (line == 1) {
                out << repeat(SPACE, 2) << FG_COLOR_LETTER << (char)toupper(it->letter) << repeat(SPACE, 2);

                // Print score in bottom right
            } else if (line == SQUARE_INNER_HEIGHT - 1) {
                out << FG_COLOR_SCORE << repeat(SPACE, SQUARE_INNER_WIDTH - 2) << setw(2) << it->points;

            } else {
                out << repeat(SPACE, SQUARE_INNER_WIDTH);
            }
        }
        if (tiles.count_tiles() > 0) {
            out << FG_COLOR_LINE << BG_COLOR_NORMAL_SQUARE << I_VERTICAL;
            out << repeat(SPACE, empty_tile_width) << BG_COLOR_OUTSIDE_BOARD << endl;
        }
    }

    // Draw bottom line
    out << repeat(SPACE, HAND_LEFT_MARGIN) << FG_COLOR_LINE << BG_COLOR_NORMAL_SQUARE;
    print_horizontal(tile_count, L_BOTTOM_LEFT, T_UP, L_BOTTOM_RIGHT, out);
    out << repeat(SPACE, empty_tile_width) << rang::style::reset << endl;
}
