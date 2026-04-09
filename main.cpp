#include <ncurses.h>
#include <chrono>
#include <thread>
#include <string>

static const char *HELI     = "[-##-]>";
static const char *TANK     = "[==^==]";
static const int   HELI_LEN = 7;
static const int   TANK_LEN = 7;
static const int   TICK_MS  = 50;   // 20 fps

int main()
{
    initscr();
    cbreak();
    noecho();
    nodelay(stdscr, TRUE);
    keypad(stdscr, TRUE);
    curs_set(0);
    start_color();
    init_pair(1, COLOR_GREEN,  COLOR_BLACK);   // tank / ground
    init_pair(2, COLOR_CYAN,   COLOR_BLACK);   // helicopter
    init_pair(3, COLOR_YELLOW, COLOR_BLACK);   // bullet
    init_pair(4, COLOR_RED,    COLOR_BLACK);   // explosion / game-over
    init_pair(5, COLOR_WHITE,  COLOR_BLACK);   // HUD / title

    int max_row, max_col;
    getmaxyx(stdscr, max_row, max_col);

    const int HELI_ROW = 3;
    const int TANK_ROW = max_row - 3;

    // ---- Title screen ----
    clear();
    attron(COLOR_PAIR(5) | A_BOLD);
    mvprintw(max_row/2 - 2, max_col/2 - 9, "*** ARTILLERY ***");
    mvprintw(max_row/2,     max_col/2 - 15, "A / D  or  LEFT/RIGHT  - move tank");
    mvprintw(max_row/2 + 1, max_col/2 - 15, "SPACE  - fire     Q - quit");
    mvprintw(max_row/2 + 3, max_col/2 - 11, "Press any key to start");
    attroff(COLOR_PAIR(5) | A_BOLD);
    refresh();
    nodelay(stdscr, FALSE);
    getch();
    nodelay(stdscr, TRUE);

    // ---- Game state ----
    int  heli_col      = 0;
    int  tank_col      = max_col / 2 - TANK_LEN / 2;
    int  bullet_row    = -1;
    int  bullet_col    = -1;
    bool bullet_active = false;
    int  score         = 0;
    int  lives         = 3;
    bool boom          = false;
    int  boom_col      = 0;
    int  boom_timer    = 0;
    int  tick          = 0;

    // ---- Main loop ----
    while (lives > 0) {

        // -- Input (drain all queued keys for smooth movement) --
        int ch;
        while ((ch = getch()) != ERR) {
            if (ch == 'q' || ch == 'Q') { lives = 0; break; }
            if ((ch == KEY_LEFT  || ch == 'a') && tank_col > 1)
                tank_col--;
            if ((ch == KEY_RIGHT || ch == 'd') && tank_col < max_col - TANK_LEN - 1)
                tank_col++;
            if (ch == ' ' && !bullet_active) {
                bullet_active = true;
                bullet_row    = TANK_ROW - 1;
                bullet_col    = tank_col + TANK_LEN / 2;  // fires from barrel ('^')
            }
        }
        if (lives <= 0) break;

        // -- Update helicopter --
        // Speed tier: slow → fast as score grows
        int heli_interval = (score < 50) ? 3 : (score < 150) ? 2 : 1;
        if (tick % heli_interval == 0) {
            heli_col++;
            if (heli_col > max_col) {
                heli_col = -HELI_LEN;
                if (!boom) {         // no penalty while explosion is playing
                    lives--;
                }
            }
        }

        // -- Update bullet --
        if (bullet_active) {
            bullet_row--;
            // Collision check
            if (!boom && bullet_row == HELI_ROW &&
                bullet_col >= heli_col && bullet_col < heli_col + HELI_LEN) {
                boom          = true;
                boom_col      = heli_col;
                boom_timer    = 10;
                score        += 10;
                bullet_active = false;
                heli_col      = -HELI_LEN;  // respawn off-screen left
            }
            if (bullet_row < 1)
                bullet_active = false;
        }

        // -- Expire explosion --
        if (boom && --boom_timer <= 0)
            boom = false;

        // -- Draw --
        erase();

        // HUD
        attron(COLOR_PAIR(5));
        mvprintw(0, 1,           "SCORE: %-5d", score);
        mvprintw(0, max_col/2-5, "ARTILLERY");
        mvprintw(0, max_col-14,  "LIVES:");
        for (int i = 0; i < lives; i++)
            mvaddch(0, max_col - 7 + i, '*');
        attroff(COLOR_PAIR(5));

        // Ground line
        attron(COLOR_PAIR(1));
        for (int c = 0; c < max_col; c++)
            mvaddch(max_row - 2, c, '=');
        attroff(COLOR_PAIR(1));

        // Helicopter (with partial clipping at edges)
        if (boom) {
            attron(COLOR_PAIR(4) | A_BOLD);
            mvprintw(HELI_ROW, boom_col, "**BOOM!**");
            attroff(COLOR_PAIR(4) | A_BOLD);
        } else if (heli_col > -HELI_LEN && heli_col < max_col) {
            attron(COLOR_PAIR(2) | A_BOLD);
            int draw_col  = heli_col;
            int char_skip = 0;
            if (draw_col < 0) { char_skip = -draw_col; draw_col = 0; }
            int n = HELI_LEN - char_skip;
            if (draw_col + n > max_col) n = max_col - draw_col;
            if (n > 0) mvaddnstr(HELI_ROW, draw_col, HELI + char_skip, n);
            attroff(COLOR_PAIR(2) | A_BOLD);
        }

        // Tank
        attron(COLOR_PAIR(1) | A_BOLD);
        mvprintw(TANK_ROW, tank_col, "%s", TANK);
        attroff(COLOR_PAIR(1) | A_BOLD);

        // Bullet
        if (bullet_active) {
            attron(COLOR_PAIR(3) | A_BOLD);
            mvaddch(bullet_row, bullet_col, '|');
            attroff(COLOR_PAIR(3) | A_BOLD);
        }

        refresh();
        tick++;
        std::this_thread::sleep_for(std::chrono::milliseconds(TICK_MS));
    }

    // ---- Game over ----
    clear();
    attron(COLOR_PAIR(4) | A_BOLD);
    mvprintw(max_row/2,     max_col/2 - 8,  "*** GAME OVER ***");
    mvprintw(max_row/2 + 1, max_col/2 - 8,  "Final score: %-5d", score);
    mvprintw(max_row/2 + 3, max_col/2 - 10, "Press any key to exit");
    attroff(COLOR_PAIR(4) | A_BOLD);
    refresh();
    nodelay(stdscr, FALSE);
    getch();
    endwin();
    return 0;
}
