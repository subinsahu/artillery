# Artillery

A retro-style terminal shooting game inspired by 80s/90s arcade classics. A helicopter flies across the top of the screen — move your tank and shoot it down before it escapes.

## Gameplay

```
*** ARTILLERY ***

  [-##-]>                          ← helicopter

       |                           ← bullet



              [==^==]              ← your tank
============================================
```

- The helicopter flies left to right. If it crosses the screen without being hit, you lose a life.
- Fire your bullet straight up and **lead your shot** — the bullet takes time to travel, so aim ahead of the helicopter.
- The helicopter speeds up after every hit — each kill makes the next one harder.

**Controls**

| Key | Action |
|-----|--------|
| `A` / `←` | Move tank left |
| `D` / `→` | Move tank right |
| `SPACE` | Fire |
| `Q` | Quit |

## Build

Requires a C++17 compiler and ncurses.

```bash
g++ -std=c++17 -o artillery main.cpp -lncurses
./artillery
```

On macOS, ncurses ships with Xcode Command Line Tools. On Debian/Ubuntu:

```bash
sudo apt install libncurses-dev
```

## Difficulty

The helicopter gains speed with each hit, starting slow and capping at maximum after two hits.

| Hits taken | speed_interval | Effect |
|------------|---------------|--------|
| 0 | 3 | moves every 3 ticks |
| 1 | 2 | moves every 2 ticks |
| 2+ | 1 | moves every tick (max) |


