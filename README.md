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
- The helicopter speeds up as your score climbs.

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

| Score | Helicopter speed |
|-------|-----------------|
| 0–49 | Slow |
| 50–149 | Medium |
| 150+ | Fast |
