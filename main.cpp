#include <ncurses.h>
#include <chrono>
#include <thread>
#include "components.h"

static const int TICK_MS = 50;  // 20 fps

// ---------------------------------------------------------------------------
// Renderer  —  all ncurses calls live here
// ---------------------------------------------------------------------------
class Renderer {
    int max_row, max_col;

public:
    Renderer() : max_row(0), max_col(0) {}
    Renderer(int rows, int cols) : max_row(rows), max_col(cols) {}

    void draw_helicopter(const Helicopter& h) const {
        if (h.boom) {
            attron(COLOR_PAIR(4) | A_BOLD);
            mvprintw(HELI_ROW, h.boom_col, "**BOOM!**");
            attroff(COLOR_PAIR(4) | A_BOLD);
            return;
        }
        if (h.col <= -Helicopter::LEN || h.col >= max_col) return;
        attron(COLOR_PAIR(2) | A_BOLD);
        int draw_col  = h.col;
        int char_skip = 0;
        if (draw_col < 0) { char_skip = -draw_col; draw_col = 0; }
        int n = Helicopter::LEN - char_skip;
        if (draw_col + n > max_col) n = max_col - draw_col;
        if (n > 0) mvaddnstr(HELI_ROW, draw_col, Helicopter::BODY + char_skip, n);
        attroff(COLOR_PAIR(2) | A_BOLD);
    }

    void draw_tank(const Tank& t) const {
        attron(COLOR_PAIR(1) | A_BOLD);
        mvprintw(t.row, t.col, "%s", Tank::BODY);
        attroff(COLOR_PAIR(1) | A_BOLD);
    }

    void draw_bullet(const Bullet& b) const {
        if (!b.active) return;
        attron(COLOR_PAIR(3) | A_BOLD);
        mvaddch(b.row, b.col, '|');
        attroff(COLOR_PAIR(3) | A_BOLD);
    }

    void draw_hud(int score, int lives) const {
        attron(COLOR_PAIR(5));
        mvprintw(0, 1,           "SCORE: %-5d", score);
        mvprintw(0, max_col/2-5, "ARTILLERY");
        mvprintw(0, max_col-14,  "LIVES:");
        for (int i = 0; i < lives; i++) mvaddch(0, max_col - 7 + i, '*');
        attroff(COLOR_PAIR(5));
    }

    void draw_ground() const {
        attron(COLOR_PAIR(1));
        for (int c = 0; c < max_col; c++) mvaddch(max_row - 2, c, '=');
        attroff(COLOR_PAIR(1));
    }

    void draw_frame(const Helicopter& h, const Tank& t,
                    const Bullet& b, int score, int lives) const {
        erase();
        draw_hud(score, lives);
        draw_ground();
        draw_helicopter(h);
        draw_tank(t);
        draw_bullet(b);
        refresh();
    }

    void title_screen() const {
        clear();
        attron(COLOR_PAIR(5) | A_BOLD);
        mvprintw(max_row/2 - 2, max_col/2 - 9,  "*** ARTILLERY ***");
        mvprintw(max_row/2,     max_col/2 - 15,  "A / D  or  LEFT/RIGHT  - move tank");
        mvprintw(max_row/2 + 1, max_col/2 - 15,  "SPACE  - fire     Q - quit");
        mvprintw(max_row/2 + 3, max_col/2 - 11,  "Press any key to start");
        attroff(COLOR_PAIR(5) | A_BOLD);
        refresh();
        nodelay(stdscr, FALSE);
        getch();
        nodelay(stdscr, TRUE);
    }

    void game_over_screen(int score) const {
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
};

// ---------------------------------------------------------------------------
// Game  —  coordinates logic objects and renderer
// ---------------------------------------------------------------------------
class Game {
    int max_row = 0, max_col = 0;

    Helicopter heli;
    Tank       tank;
    Bullet     bullet;
    Renderer   renderer;

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
        if (heli.update(tick, max_col)) --lives;
        bullet.update();

        if (bullet.active && heli.collides_with(bullet.row, bullet.col)) {
            heli.hit();
            bullet.active = false;
            score += 10;
        }

        ++tick;
    }

public:
    Game() {
        getmaxyx(stdscr, max_row, max_col);
        tank     = Tank(max_row - 3, max_col);
        renderer = Renderer(max_row, max_col);
    }

    void run() {
        renderer.title_screen();
        while (lives > 0) {
            handle_input();
            if (lives <= 0) break;
            update();
            renderer.draw_frame(heli, tank, bullet, score, lives);
            std::this_thread::sleep_for(std::chrono::milliseconds(TICK_MS));
        }
        renderer.game_over_screen(score);
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
