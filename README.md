# PhosphorList

A fast, keyboard-driven terminal user interface for browsing, searching, and launching installed CLI commands.
Designed to be responsive, minimal, and scalable, inspired by tools like lazygit, fzf, and yazi.

---

## Features

### Command Listing

* Displays all available commands found in `$PATH`
* Commands are sorted alphabetically
* Scrollable list with keyboard navigation

### Tabs (Views)

* **[1] ALL** — shows all detected commands
* **[2] FAV** — shows only favorite commands
* Tabs are always visible at the top
* Switch tabs instantly using number keys

### Favorites

* Mark or unmark a command as favorite using `f`
* Favorites are persisted between runs
* Favorites view (`[2] FAV`) shows only starred commands

### Search

* Press `/` to enter search mode
* Type to filter commands by name
* Press `Enter` to apply search
* Press `Esc` to cancel search
* Search works within the active tab (ALL or FAV)

### Description Preview

* Press `d` to load a short description of the selected command
* Description is fetched using `whatis`
* Descriptions are cached in memory for performance
* No automatic preview (prevents UI lag)

### Command Execution

* Press `Enter` on a command to execute it
* A modal dialog appears to enter command arguments
* `Enter` executes the command
* `Esc` cancels execution

### Performance

* No blocking system calls during navigation
* Redraws occur only when state changes
* No background polling or timers
* Designed for instant response even with thousands of commands

---

## Keyboard Shortcuts

### Global

| Key | Action           |
| --- | ---------------- |
| `q` | Quit application |

### Navigation

| Key         | Action            |
| ----------- | ----------------- |
| `Up / Down` | Move selection    |
| `1`         | Switch to ALL tab |
| `2`         | Switch to FAV tab |

### Favorites

| Key | Action                               |
| --- | ------------------------------------ |
| `f` | Toggle favorite for selected command |

### Search

| Key     | Action            |
| ------- | ----------------- |
| `/`     | Enter search mode |
| `Enter` | Apply search      |
| `Esc`   | Cancel search     |

### Description

| Key | Action                   |
| --- | ------------------------ |
| `d` | Load command description |

### Execution

| Key     | Action                   |
| ------- | ------------------------ |
| `Enter` | Execute selected command |
| `Esc`   | Cancel execution dialog  |

---

## User Interface Layout

```
[1] ALL   [2] FAV
----------------------------------------
command1        *
command2
command3
...

----------------------------------------
Description
Short description loaded on demand

----------------------------------------
1 ALL   2 FAV   ↑↓ move   / search   f fav   d desc   q quit
```

---

## Project Structure

```
src/
 ├── main.c        Application state and event loop
 ├── ui.c          Rendering, layout, colors, dialogs
 ├── ui.h
 ├── commands.c   Command discovery and filtering
 ├── commands.h
 ├── favorites.c  Persistent favorites handling
 ├── favorites.h
 ├── sort.c       Sorting helpers
 ├── sort.h
```

---

## Build Requirements

* C compiler (clang or gcc)
* ncurses
* POSIX-compatible system (macOS, Linux)

### macOS

```sh
brew install ncurses
```

### Linux (Debian/Ubuntu)

```sh
sudo apt install libncurses-dev
```

---

## Build Instructions

```sh
make
./phl
```

To force a clean rebuild:

```sh
rm -f phl src/*.o
make
```

---

## Design Principles

* No blocking I/O in the main loop
* No automatic previews
* Explicit user actions trigger expensive operations
* UI redraws only on state changes
* Keyboard-first interaction
* Predictable behavior

---

## Limitations

* Description preview uses `whatis`, which depends on system man database
* Only short descriptions are shown (not full man pages)
* No mouse support
* No fuzzy search (substring matching only)

---

## Possible Future Improvements

* Fuzzy search (fzf-style)
* Full man page viewer with scrolling
* Recent commands tab
* Theme system
* Vim-style keybindings
* Mouse support
* Persistent description cache on disk

---

## License

MIT License
