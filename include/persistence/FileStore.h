#ifndef FILE_STORE_H
#define FILE_STORE_H

#include "league/FootballLeague.h"

// Saves and reloads the league through the shared FileManager (sequential text files).
// Files used: teams, stadiums, officials, fixtures and results (see DataFiles).
// Players, coaches, products and tickets belong to the other modules.
class FileStore {
public:
    // Writes each file by replacing it, so a failed save cannot leave a half-written file.
    static void save(const FootballLeague& league);

    // Rebuilds the league from the files. If any file is damaged the league is left
    // unchanged and a FileException is thrown. A missing file is treated as empty.
    static void load(FootballLeague& league);
};

#endif
