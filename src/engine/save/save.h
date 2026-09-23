#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/** The folder save slots are written to. The engine fills Engine.save during engine_init() from EngineConfig.org and app (a per-user folder such as ~/.local/share/org/app/ or the platform equivalent), or the current directory when either is NULL. Games only ever pass &e->save.
 *  @field dir absolute path with a trailing separator
 *  @see save_write, save_load, EngineConfig, gge-saves
 */
typedef struct Save { char dir[512]; } Save;

/** Write size bytes of data to the slot file <dir>/<slot>.sav with a header carrying version. data must be pointer-free (a GameState): the bytes are written raw, so a pointer would be meaningless when read back. The normal call is save_write(&e->save, "slot1", g, sizeof *g, SAVE_VERSION). Overwrites an existing slot. Fast enough to call on every checkpoint; a few hundred kilobytes is fine.
 *  @return true on success; failures (unwritable folder, disk full) are logged.
 *  @see save_load, save_exists, save_delete, gge-saves
 */
bool save_write(const Save *s, const char *slot, const void *data, size_t size, uint32_t version);
/** Read a slot into out. Fails (without touching out) if the file is missing, the version differs, or the size does not match, so a save from an older build never corrupts a running game: bump the version whenever the struct layout changes. After a successful load, scenes must rebuild their presentation state (camera, sprites, tilemap) from the loaded data, which on_enter does if you load before pushing the game scene.
 *  @return true on success.
 *  @see save_write, save_exists
 */
bool save_load (const Save *s, const char *slot, void *out, size_t size, uint32_t version);
/** Whether a slot file exists; use to enable a Continue item on the title menu. Does not check the version; a stale file makes save_load() fail, so handle that too.
 *  @return true if <slot>.sav exists.
 *  @see save_load
 */
bool save_exists(const Save *s, const char *slot);
/** Remove a slot file: "delete save" in a menu, or permadeath in a roguelike.
 *  @return true if it was removed.
 *  @see save_write
 */
bool save_delete(const Save *s, const char *slot);

/* engine-internal */
/** Engine-internal: resolve the save folder from org and app (creating it) or fall back to the working directory. Called by engine_init(); games never call it.
 *  @see engine_init
 */
void save_init(Save *s, const char *org, const char *app);
