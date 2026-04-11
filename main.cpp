#include <ncurses.h>
#include <chrono>
#include <thread>
#include <string>
#include <vector>

static const int TICK_MS  = 50;  // 20 fps
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
    }

    bool collides_with(int brow, int bcol) const {
        return !boom && brow == HELI_ROW && bcol >= col && bcol < col + LEN;
    }

    void set_speed(int score) {
        speed_interval = (score < 50) ? 3 : (score < 150) ? 2 : 1;
    }

    void draw(int max_col) const {
        if (boom) {
            attron(COLOR_PAIR(4) | A_BOLD);
            mvprintw(HELI_ROW, boom_col, "**BOOM!**");
            attroff(COLOR_PAIR(4) | A_BOLD);
            return;
        }
        if (col <= -LEN || col >= max_col) return;
        attron(COLOR_PAIR(2) | A_BOLD);
        int draw_col  = col;
        int char_skip = 0;
        if (draw_col < 0) { char_skip = -draw_col; draw_col = 0; }
        int n = LEN - char_skip;
        if (draw_col + n > max_col) n = max_col - draw_col;
        if (n > 0) mvaddnstr(HELI_ROW, draw_col, BODY + char_skip, n);
        attroff(COLOR_PAIR(2) | A_BOLD);
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

    void move_left()  { if (col > 1) --col; }
    void move_right() { if (col < max_col - LEN - 1) ++col; }
    int  barrel_col() const { return col + BARREL; }

    void draw() const {
        attron(COLOR_PAIR(1) | A_BOLD);
        mvprintw(row, col, "%s", BODY);
        attroff(COLOR_PAIR(1) | A_BOLD);
    }
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

    void draw() const {
        if (!active) return;
        attron(COLOR_PAIR(3) | A_BOLD);
        mvaddch(row, col, '|');
        attroff(COLOR_PAIR(3) | A_BOLD);
    }
};

// ---------------------------------------------------------------------------
// Game
// ---------------------------------------------------------------------------
class Game {
    int max_row = 0, max_col = 0;

    Helicopter heli;
    Tank       tank;
    Bullet     bullet;

    int  score = 0;
    int  lives = 3;
    int  tick  = 0;

    void handle_input() {
        int ch;
        while ((ch = getch()) != ERR) {
            if (ch == 'q' || ch == 'Q') { lives = 0; return; }
            if (ch == KEY_LEFT  || ch == 'a') tank.move_left();
            if (ch == KEY_RIGHT || ch == 'd') tank.move_right();
            if (ch == ' ' && !bullet.active)
                bullet.fire(tank.row, tank.barrel_col());
        }
    }

    void update() {
        bool escaped = heli.update(tick, max_col);
        if (escaped) --lives;

        bullet.update();

        if (bullet.active && heli.collides_with(bullet.row, bullet.col)) {
            heli.hit();
            bullet.active = false;
            score += 10;
            heli.set_speed(score);
        }

        ++tick;
    }

    void draw() const {
        erase();

        // HUD
        attron(COLOR_PAIR(5));
        mvprintw(0, 1,           "SCORE: %-5d", score);
        mvprintw(0, max_col/2-5, "ARTILLERY");
        mvprintw(0, max_col-14,  "LIVES:");
        for (int i = 0; i < lives; i++) mvaddch(0, max_col - 7 + i, '*');
        attroff(COLOR_PAIR(5));

        // Ground
        attron(COLOR_PAIR(1));
        for (int c = 0; c < max_col; c++) mvaddch(max_row - 2, c, '=');
        attroff(COLOR_PAIR(1));

        heli.draw(max_col);
        tank.draw();
        bullet.draw();

        refresh();
    }

    void title_screen() const {
        clear();
        attron(COLOR_PAIR(5) | A_BOLD);
        mvprintw(max_row/2 - 2, max_col/2 - 9,  "*** ARTILLERY ***");
        mvprintw(max_row/2,     max_col/2 - 15, "A / D  or  LEFT/RIGHT  - move tank");
        mvprintw(max_row/2 + 1, max_col/2 - 15, "SPACE  - fire     Q - quit");
        mvprintw(max_row/2 + 3, max_col/2 - 11, "Press any key to start");
        attroff(COLOR_PAIR(5) | A_BOLD);
        refresh();
        nodelay(stdscr, FALSE);
        getch();
        nodelay(stdscr, TRUE);
    }

    void game_over_screen() const {
        clear();
        attron(COLOR_PAIR(4) | A_BOLD);
        mvprintw(max_row/2,     max_col/2 - 8,  "*** GAME OVER ***");
        mvprintw(max_row/2 + 1, max_col/2 - 8,  "Final score: %-5d", score);
        mvprintw(max_row/2 + 3, max_col/2 - 10, "Press any key to exit");
        attroff(COLOR_PAIR(4) | A_BOLD);
        refresh();
        nodelay(stdscr, FALSE);
        getch();
    }

public:
    Game() {
        getmaxyx(stdscr, max_row, max_col);
        tank = Tank(max_row - 3, max_col);
    }

    void run() {
        title_screen();
        while (lives > 0) {
            handle_input();
            if (lives <= 0) break;
            update();
            draw();
            std::this_thread::sleep_for(std::chrono::milliseconds(TICK_MS));
        }
        game_over_screen();
    }
};

// ---------------------------------------------------------------------------
// Entry point
// ---------------------------------------------------------------------------
int main()
{
    initscr();
    cbreak();
    noecho();
    nodelay(stdscr, TRUE);
    keypad(stdscr, TRUE);
    curs_set(0);
    start_color();
    init_pair(1, COLOR_GREEN,  COLOR_BLACK);  // tank / ground
    init_pair(2, COLOR_CYAN,   COLOR_BLACK);  // helicopter
    init_pair(3, COLOR_YELLOW, COLOR_BLACK);  // bullet
    init_pair(4, COLOR_RED,    COLOR_BLACK);  // explosion / game-over
    init_pair(5, COLOR_WHITE,  COLOR_BLACK);  // HUD / title

    Game game;
    game.run();

    endwin();
    return 0;
}
