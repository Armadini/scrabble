#ifndef HUMAN_PLAYER_H
#define HUMAN_PLAYER_H

#include "move.h"
#include "player.h"
#include <fstream>
#include <string>
#include <vector>

class HumanPlayer : public Player {
public:
    HumanPlayer(const std::string& name, size_t hand_size) : Player(name, hand_size) {}

    Move get_move(const Board& board, const Dictionary& dictionary) const override;

private:
    Move parse_move(std::string& move_string) const;
    std::vector<TileKind> parse_tiles(std::string& letters, bool place) const;
    void print_hand(std::ostream& out) const;
};

#endif
