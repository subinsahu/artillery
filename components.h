#pragma once
#include <algorithm>

static const int HELI_ROW = 3;

// ---------------------------------------------------------------------------
// Helicopter 
// ---------------------------------------------------------------------------
struct Helicopter {
    static constexpr const char* BODY = "[-##-]>";
    static constexpr int         LEN  = 7;

    int  col            = 0;
    int  speed_interval = 3;   // ticks between moves (lower = faster)
    bool boom           = false;
    int  boom_timer     = 0;
    int  boom_col       = 0;

    // Returns true if helicopter escaped off the right edge.
    bool update(int tick, int max_col) {
        if (boom) {
            if (--boom_timer <= 0) boom = false;
            return false;
        }
        if (tick % speed_interval == 0) {
            ++col;
            if (col > max_col) {
                col = -LEN;
                return true;
            }
        }
        return false;
    }

    void hit() {
        boom_col   = col;
        boom       = true;
        boom_timer = 10;
        col        = -LEN;
        if (speed_interval > 1) --speed_interval;
    }

    bool collides_with(int brow, int bcol) const {
        return !boom && brow == HELI_ROW && bcol >= col && bcol < col + LEN;
    }
};

// ---------------------------------------------------------------------------
// Tank  
// ---------------------------------------------------------------------------
struct Tank {
    static constexpr const char* BODY   = "[==^==]";
    static constexpr int         LEN    = 7;
    static constexpr int         BARREL = 3;  // offset of '^' from left edge

    int row, col, max_col;

    Tank() : row(0), col(0), max_col(0) {}
    Tank(int r, int mc) : row(r), col(mc / 2 - LEN / 2), max_col(mc) {}

    void move_left()  { col = std::max(1, col - 3); }
    void move_right() { col = std::min(max_col - LEN - 1, col + 3); }
    int  barrel_col() const { return col + BARREL; }
};

// ---------------------------------------------------------------------------
// Bullet  
// ---------------------------------------------------------------------------
struct Bullet {
    int  row = -1, col = -1;
    bool active = false;

    void fire(int from_row, int from_col) {
        row    = from_row - 1;
        col    = from_col;
        active = true;
    }

    // Returns true if the bullet flew off the top of the screen.
    bool update() {
        if (!active) return false;
        --row;
        if (row < 1) { active = false; return true; }
        return false;
    }
};
