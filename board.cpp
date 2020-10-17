#include "board.h"

#include "board_square.h"
#include "exceptions.h"
#include "formatting.h"
#include <fstream>
#include <iomanip>

using namespace std;

bool Board::Position::operator==(const Board::Position& other) const {
    return this->row == other.row && this->column == other.column;
}

bool Board::Position::operator!=(const Board::Position& other) const {
    return this->row != other.row || this->column != other.column;
}

Board::Position Board::Position::translate(Direction direction) const { return this->translate(direction, 1); }

Board::Position Board::Position::translate(Direction direction, ssize_t distance) const {
    if (direction == Direction::DOWN) {
        return Board::Position(this->row + distance, this->column);
    } else {
        return Board::Position(this->row, this->column + distance);
    }
}

Board Board::read(const string& file_path) {
    ifstream file(file_path);
    if (!file) {
        throw FileException("cannot open board file!");
    }

    size_t rows;
    size_t columns;
    size_t starting_row;
    size_t starting_column;
    file >> rows >> columns >> starting_row >> starting_column;
    Board board(rows, columns, starting_row, starting_column);

    // Add appropriate number of row vectors to squares vector in board
    board.squares.resize(rows);

    // Loop through board config file to create board based on template
    char c;
    // Loop for each row
    for (int row = 0; row < rows; row++) {
        // Loop for each column
        for (int col = 0; col < columns; col++) {
            // Get character at [row][col]
            file >> c;
            // Depending on char, create apropriate board square and add to board.squares at [row][col]
            switch (c) {
            case ('2'):
                board.squares[row].push_back(BoardSquare(2, 1));
                break;
            case ('3'):
                board.squares[row].push_back(BoardSquare(3, 1));
                break;
            case ('d'):
                board.squares[row].push_back(BoardSquare(1, 2));
                break;
            case ('t'):
                board.squares[row].push_back(BoardSquare(1, 3));
                break;
            case ('.'):
                board.squares[row].push_back(BoardSquare(1, 1));
                break;
            }
        }
        // Ignore newline at end of each row
        file.ignore();
    }
    return board;
}

size_t Board::get_move_index() const { return this->move_index; }

PlaceResult Board::test_place(const Move& move) const {
    // TODO: complete implementation here
    // if there have been no moves, make sure the location of the move contains the start position
    if (this->get_move_index() == 0) {
        if (move.direction == Direction::DOWN) {
            if (move.column != start.column || move.row > start.row || move.row + move.tiles.size() <= start.row) {
                return PlaceResult("First move must include starting position.");
            }
        } else {
            if (move.row != start.row || move.column > start.column
                || move.column + move.tiles.size() <= start.column) {
                return PlaceResult("First move must include starting position.");
            }
        }
    }
    // perp is the direction opposite of the move. this is used to check the creation of adjacent words
    Direction perp;
    if (move.direction == Direction::DOWN) {
        perp = Direction::ACROSS;
    } else {
        perp = Direction::DOWN;
    }
    // main_word is the word we are creating with our tiles, adj_words are the adjacent words created
    string main_word = "";
    vector<string> adj_words;
    // total_points is all the points from all the adjacent words created
    int total_points = 0;
    // main points is the value of all the tiles in the main word we are creating (letter mult included)
    int main_points = 0;
    // multiplier is the word multiplier value of our main word
    int multiplier = 1;
    // pos is the position we are checking on the board
    Position pos(move.row, move.column);
    int i = 0;
    // if we find an existing adjacent tile, we will set this to true
    bool existing_tile = false;
    // while we haven't used all of our tiles, or there are more tiles after the ones we've placed, keep iterating
    while (i < move.tiles.size() || this->in_bounds_and_has_tile(pos)) {
        // if the position is in bounds, proceed. Otherwise, throw out of range error
        if (this->is_in_bounds(pos)) {
            // if we are at our first placement, we have to check all the positions before that and add them to our word
            if (i == 0) {
                // pos_b is the position before. the loop keeps adding the letters before ours if they exist
                Position pos_b = pos;
                pos_b = pos_b.translate(move.direction, -1);
                while (this->in_bounds_and_has_tile(pos_b)) {
                    existing_tile = true;
                    TileKind b_tile = this->at(pos_b).get_tile_kind();
                    char bc = b_tile.letter;
                    if (bc == '?') {
                        bc = b_tile.assigned;
                    }
                    main_word = bc + main_word;
                    main_points += b_tile.points;
                    pos_b = pos_b.translate(move.direction, -1);
                }
            }
            // if a tile already exists at that pos, add its letter to the word (account for '?' assigned)
            if (this->in_bounds_and_has_tile(pos)) {
                // if the move's start position already has a tile, return an invalid result
                if (i == 0) {
                    return PlaceResult("A tile already exists at that starting index!");
                }
                existing_tile = true;
                TileKind t = this->at(pos).get_tile_kind();
                main_points += t.points;
                char c = t.letter;
                if (c == '?') {
                    c = t.assigned;
                }
                main_word += c;
                // if a tile doesn't exist, add the next letter to the word and increment i
            } else {
                BoardSquare curr_square = this->at(pos);
                // accounting for the letter and word multipliers here
                multiplier *= curr_square.word_multiplier;
                main_points += move.tiles[i].points * curr_square.letter_multiplier;
                string adj_word;
                if (move.tiles[i].letter == '?') {
                    main_word += move.tiles[i].assigned;
                    adj_word = move.tiles[i].assigned;
                } else {
                    main_word += move.tiles[i].letter;
                    adj_word = move.tiles[i].letter;
                }
                // apos_p and apos_n are the adjacent positions after and before this square, respectively
                Position apos_p = pos.translate(perp);
                Position apos_n = pos.translate(perp, -1);
                // this is the potential adjacent word that we have created
                string initial = adj_word;
                int word_points = 0;
                int word_mult = curr_square.word_multiplier;
                // negative loop: if there are letters before ours, add them to the front of our adjacent word
                while (this->in_bounds_and_has_tile(apos_n)) {
                    existing_tile = true;
                    TileKind neg_tile = this->at(apos_n).get_tile_kind();
                    char nc = neg_tile.letter;
                    if (nc == '?') {
                        nc = neg_tile.assigned;
                    }
                    adj_word = nc + adj_word;
                    word_points += neg_tile.points;
                    apos_n = apos_n.translate(perp, -1);
                }
                // positive loop: if there are letters after ours, add them to the end of our adjacent word
                while (this->in_bounds_and_has_tile(apos_p)) {
                    existing_tile = true;
                    TileKind pos_tile = this->at(apos_p).get_tile_kind();
                    char pc = pos_tile.letter;
                    if (pc == '?') {
                        pc = pos_tile.assigned;
                    }
                    adj_word += pc;
                    word_points += pos_tile.points;
                    apos_p = apos_p.translate(perp);
                }
                // if we created an adjacent word, add it to adj_word vector and add its point values to total_points
                if (adj_word != initial) {
                    adj_words.push_back(adj_word);
                    total_points += (word_points + move.tiles[i].points * curr_square.letter_multiplier) * word_mult;
                }
                // only increment i if we placed a tile
                i++;
            }
            // translate the position always
            pos = pos.translate(move.direction);
        } else {
            return PlaceResult("Your word goes off the board!!!");
        }
    }
    // if we didn't find an existing adjacent tile and it's not the first move of the game, return an invalid
    // PlaceResult
    if (!existing_tile && move_index != 0) {
        return PlaceResult("New tiles must be placed by at least one existing tile!");
    }
    // add the main word to the adj_words
    if (main_word.length() > 1) {
        adj_words.push_back(main_word);
    }
    return PlaceResult(adj_words, main_points * multiplier + total_points);
}

PlaceResult Board::place(const Move& move) {
    // TODO: Complete implementation here
    // run the test_place to get the appropriate points
    PlaceResult p = test_place(move);
    // if the move isn't valid, return the test_place result
    // (Note: the move is always valid in gameplay b/c test_place has already been checked. this is only for grading
    // test cases)
    if (!p.valid) {
        return p;
    }
    int i = 0;
    Position pos(move.row, move.column);
    // iterate through each tile and place them if the square isn't occupied. if it is, skip to the next one
    while (i < move.tiles.size()) {
        if (!this->in_bounds_and_has_tile(pos)) {
            this->at(pos).set_tile_kind(move.tiles[i]);
            i++;
        }
        pos = pos.translate(move.direction);
    }
    // don't forget to increment the moves!
    move_index++;
    return PlaceResult(p.words, p.points);
}

// The rest of this file is provided for you. No need to make changes.

BoardSquare& Board::at(const Board::Position& position) { return this->squares.at(position.row).at(position.column); }

const BoardSquare& Board::at(const Board::Position& position) const {
    return this->squares.at(position.row).at(position.column);
}

bool Board::is_in_bounds(const Board::Position& position) const {
    return position.row < this->rows && position.column < this->columns;
}

bool Board::in_bounds_and_has_tile(const Position& position) const {
    return is_in_bounds(position) && at(position).has_tile();
}

void Board::print(ostream& out) const {
    // Draw horizontal number labels
    for (size_t i = 0; i < BOARD_TOP_MARGIN - 2; ++i) {
        out << std::endl;
    }
    out << FG_COLOR_LABEL << repeat(SPACE, BOARD_LEFT_MARGIN);
    const size_t right_number_space = (SQUARE_OUTER_WIDTH - 3) / 2;
    const size_t left_number_space = (SQUARE_OUTER_WIDTH - 3) - right_number_space;
    for (size_t column = 0; column < this->columns; ++column) {
        out << repeat(SPACE, left_number_space) << std::setw(2) << column + 1 << repeat(SPACE, right_number_space);
    }
    out << std::endl;

    // Draw top line
    out << repeat(SPACE, BOARD_LEFT_MARGIN);
    print_horizontal(this->columns, L_TOP_LEFT, T_DOWN, L_TOP_RIGHT, out);
    out << endl;

    // Draw inner board
    for (size_t row = 0; row < this->rows; ++row) {
        if (row > 0) {
            out << repeat(SPACE, BOARD_LEFT_MARGIN);
            print_horizontal(this->columns, T_RIGHT, PLUS, T_LEFT, out);
            out << endl;
        }

        // Draw insides of squares
        for (size_t line = 0; line < SQUARE_INNER_HEIGHT; ++line) {
            out << FG_COLOR_LABEL << BG_COLOR_OUTSIDE_BOARD;

            // Output column number of left padding
            if (line == 1) {
                out << repeat(SPACE, BOARD_LEFT_MARGIN - 3);
                out << std::setw(2) << row + 1;
                out << SPACE;
            } else {
                out << repeat(SPACE, BOARD_LEFT_MARGIN);
            }

            // Iterate columns
            for (size_t column = 0; column < this->columns; ++column) {
                out << FG_COLOR_LINE << BG_COLOR_NORMAL_SQUARE << I_VERTICAL;
                const BoardSquare& square = this->squares.at(row).at(column);
                bool is_start = this->start.row == row && this->start.column == column;

                // Figure out background color
                if (square.word_multiplier == 2) {
                    out << BG_COLOR_WORD_MULTIPLIER_2X;
                } else if (square.word_multiplier == 3) {
                    out << BG_COLOR_WORD_MULTIPLIER_3X;
                } else if (square.letter_multiplier == 2) {
                    out << BG_COLOR_LETTER_MULTIPLIER_2X;
                } else if (square.letter_multiplier == 3) {
                    out << BG_COLOR_LETTER_MULTIPLIER_3X;
                } else if (is_start) {
                    out << BG_COLOR_START_SQUARE;
                }

                // Text
                if (line == 0 && is_start) {
                    out << "  \u2605  ";
                } else if (line == 0 && square.word_multiplier > 1) {
                    out << FG_COLOR_MULTIPLIER << repeat(SPACE, SQUARE_INNER_WIDTH - 2) << 'W' << std::setw(1)
                        << square.word_multiplier;
                } else if (line == 0 && square.letter_multiplier > 1) {
                    out << FG_COLOR_MULTIPLIER << repeat(SPACE, SQUARE_INNER_WIDTH - 2) << 'L' << std::setw(1)
                        << square.letter_multiplier;
                } else if (line == 1 && square.has_tile()) {
                    char l = square.get_tile_kind().letter == TileKind::BLANK_LETTER ? square.get_tile_kind().assigned
                                                                                     : ' ';
                    out << repeat(SPACE, 2) << FG_COLOR_LETTER << square.get_tile_kind().letter << l
                        << repeat(SPACE, 1);
                } else if (line == SQUARE_INNER_HEIGHT - 1 && square.has_tile()) {
                    out << repeat(SPACE, SQUARE_INNER_WIDTH - 1) << FG_COLOR_SCORE << square.get_points();
                } else {
                    out << repeat(SPACE, SQUARE_INNER_WIDTH);
                }
            }

            // Add vertical line
            out << FG_COLOR_LINE << BG_COLOR_NORMAL_SQUARE << I_VERTICAL << BG_COLOR_OUTSIDE_BOARD << std::endl;
        }
    }

    // Draw bottom line
    out << repeat(SPACE, BOARD_LEFT_MARGIN);
    print_horizontal(this->columns, L_BOTTOM_LEFT, T_UP, L_BOTTOM_RIGHT, out);
    out << endl << rang::style::reset << std::endl;
}
