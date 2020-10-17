#include "player.h"

#include "iostream"

using namespace std;

// TODO: implement member functions

void Player::add_points(size_t points) { this->points += points; }

void Player::subtract_points(size_t points) {
    if (this->points > points) {
        this->points -= points;
    } else {
        this->points = 0;
    }
}

size_t Player::get_points() const { return this->points; }

const std::string& Player::get_name() const { return this->name; }

size_t Player::count_tiles() const { return tiles.count_tiles(); }

void Player::add_tiles(const std::vector<TileKind>& tiles) {
    for (int i = 0; i < tiles.size(); i++) {
        this->tiles.add_tile(tiles[i]);
        hand_size += 1;
    }
}

void Player::remove_tiles(const std::vector<TileKind>& tiles) {
    for (int i = 0; i < tiles.size(); i++) {
        this->tiles.remove_tile(tiles[i]);
        hand_size -= 1;
    }
}

bool Player::has_tile(TileKind tile) {
    try {
        tiles.lookup_tile(tile.letter);
    } catch (...) {
        return false;
    }
    return true;
}

unsigned int Player::get_hand_value() const {
    unsigned int sum = 0;
    for (TileCollection::const_iterator t = this->tiles.cbegin(); t != this->tiles.cend(); t++) {
        sum += t->points;
    }
    return sum;
}

size_t Player::get_hand_size() const { return this->hand_size; }
