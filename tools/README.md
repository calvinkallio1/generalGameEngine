# gge — engine project tool

Run from a game's root as `vendored/engine/tools/gge`, or put `tools/` on your PATH.

    gge init  [-o DIR] [-e URL] [-I] [-T] <name>      new project (engine as submodule)
    gge scene [-o DIR] [-t full|overlay|hud] [-H HDR] [-n] [-f] <name>
    gge entity [-o DIR] [-H] [-f] <name>
    gge config [-f FILE] [-e HEADER] [key [value]]   gge.conf -> src/gge_config.h (GGE_* macros)
    gge man   [-s SECTION] [-k] <topic>              man pages: functions/types/macros (3), modules (7)
    gge ls    [-m MODULE] [-g REGEX] [-t]            list engine functions (-t: types and macros)
    gge genman                                       regenerate man3 + gge-api(7) from header comments

`gge <command> -h` prints that command's options.

Man pages: `tools/man/man3/*.3` are generated from `/** ... */` comments above prototypes,
typedefs and #defines in `src/engine/*/*.h` (`@return`, `@field name text` and `@see` lines are
honored), one page per function, struct, enum and macro; `gge-api(7)` indexes them by module.
Module and concept pages in `tools/man/man7/` are hand-written: start with `gge man overview`.
To use plain `man engine_push` or `man Camera` in any shell:

    export MANPATH="$HOME/generalGameEngine/tools/man:$MANPATH"

Templates in `tools/templates/*.in` use `@NAME@` (snake), `@UPPER@`, `@TITLE@` (words),
and `@TYPE@` (CamelCase) placeholders.
