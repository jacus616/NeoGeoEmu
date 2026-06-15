package org.neogeoemu.app.data

import android.content.Context
import android.os.Environment
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import java.io.File

/**
 * Represents a discovered NeoGeo ROM.
 * PPSSPP-style game entry with metadata.
 */
data class GameInfo(
    val romPath: String,
    val title: String,
    val gameId: String,       // ROM set identifier (e.g. "samsho5")
    val isArcade: Boolean = true,
    val parentRom: String? = null, // Parent ROM set for clones
    val year: String? = null,
    val manufacturer: String? = null,
    val hasSavedata: Boolean = false
)

/**
 * Scans for NeoGeo ROMs on the device.
 * Expects ROMs in FBNeo-compatible format (non-merged sets).
 */
class GameScanner(private val context: Context) {

    companion object {
        // Standard NeoGeo ROM directories (like PPSSPP's /PSP/GAME)
        val DEFAULT_ROM_DIRS = listOf(
            "NeoGeo/ROMs",
            "NeoGeo",
            "RetroArch/roms/neogeo",
            "ROMs/NeoGeo"
        )

        val SUPPORTED_EXTENSIONS = setOf("zip")

        val NEOGEO_BIOS_FILES = setOf(
            "neogeo.zip",
            "uni-bios.rom",
            "uni-bios_4_0.rom"
        )
    }

    /**
     * Scan for all available NeoGeo games.
     */
    suspend fun scanForGames(): List<GameInfo> = withContext(Dispatchers.IO) {
        val games = mutableListOf<GameInfo>()
        val scannedPaths = mutableSetOf<String>()

        getSearchDirs().forEach { dir ->
            if (dir.exists() && dir.isDirectory) {
                dir.walkTopDown()
                    .filter { it.isFile && it.extension.lowercase() in SUPPORTED_EXTENSIONS }
                    .forEach { file ->
                        // Skip BIOS files
                        if (file.name.lowercase() in NEOGEO_BIOS_FILES) return@forEach
                        // Skip clone ROMs that have parents (unless scanning all)
                        val gameId = file.nameWithoutExtension.lowercase()
                        if (gameId !in scannedPaths) {
                            scannedPaths.add(gameId)
                            games.add(
                                GameInfo(
                                    romPath = file.absolutePath,
                                    title = formatGameTitle(gameId),
                                    gameId = gameId
                                )
                            )
                        }
                    }
            }
        }

        games.sortedBy { it.title }
    }

    /**
     * Check if NeoGeo BIOS is present.
     */
    fun hasBios(): Boolean {
        return getSearchDirs().any { dir ->
            dir.exists() && dir.isDirectory &&
            NEOGEO_BIOS_FILES.any { bios -> File(dir, bios).exists() }
        }
    }

    /**
     * Get the BIOS file path if found.
     */
    fun findBiosPath(): String? {
        getSearchDirs().forEach { dir ->
            if (dir.exists() && dir.isDirectory) {
                NEOGEO_BIOS_FILES.forEach { bios ->
                    val f = File(dir, bios)
                    if (f.exists()) return f.absolutePath
                }
            }
        }
        return null
    }

    private fun getSearchDirs(): List<File> {
        val dirs = mutableListOf<File>()
        // External storage
        val externalRoot = Environment.getExternalStorageDirectory()
        DEFAULT_ROM_DIRS.forEach { path ->
            dirs.add(File(externalRoot, path))
        }
        // App-specific external storage
        context.getExternalFilesDir(null)?.let { appExt ->
            dirs.add(File(appExt, "ROMs"))
        }
        return dirs
    }

    /**
     * Convert ROM ID to human-readable title.
     * In production, this would use FBNeo's driver database.
     */
    private fun formatGameTitle(gameId: String): String {
        return KNOWN_GAMES[gameId] ?: gameId.replace("_", " ")
            .replaceFirstChar { it.uppercase() }
    }

    companion object {
        // Sample of known game titles (full list would come from FBNeo driver DB)
        val KNOWN_GAMES = mapOf(
            "mslug" to "Metal Slug",
            "mslug2" to "Metal Slug 2",
            "mslug3" to "Metal Slug 3",
            "mslug4" to "Metal Slug 4",
            "mslug5" to "Metal Slug 5",
            "mslugx" to "Metal Slug X",
            "kof97" to "The King of Fighters '97",
            "kof98" to "The King of Fighters '98",
            "kof99" to "The King of Fighters '99",
            "kof2000" to "The King of Fighters 2000",
            "kof2002" to "The King of Fighters 2002",
            "kof2003" to "The King of Fighters 2003",
            "samsho" to "Samurai Shodown",
            "samsho2" to "Samurai Shodown II",
            "samsho3" to "Samurai Shodown III",
            "samsho4" to "Samurai Shodown IV",
            "samsho5" to "Samurai Shodown V",
            "samsho5sp" to "Samurai Shodown V Special",
            "rbff1" to "Real Bout Fatal Fury",
            "rbff2" to "Real Bout Fatal Fury 2",
            "rbffspec" to "Real Bout Fatal Fury Special",
            "fatfury1" to "Fatal Fury",
            "fatfury2" to "Fatal Fury 2",
            "fatfury3" to "Fatal Fury 3",
            "aof" to "Art of Fighting",
            "aof2" to "Art of Fighting 2",
            "aof3" to "Art of Fighting 3",
            "garou" to "Garou: Mark of the Wolves",
            "shocktr2" to "Shock Troopers: 2nd Squad",
            "blazstar" to "Blazing Star",
            "spinmast" to "Spin Master",
            "kabukikl" to "Kabuki Klash",
            "overtop" to "OverTop",
            "ncommand" to "Ninja Commando",
            "tophuntr" to "Top Hunter",
            "doubledr" to "Double Dragon",
            "gowcaizr" to "Voltage Fighter Gowcaizer",
            "sonicwi2" to "Aero Fighters 2",
            "sonicwi3" to "Aero Fighters 3",
            "turfmast" to "Neo Turf Masters",
            "neocup98" to "Neo Geo Cup '98",
            "maglord" to "Magical Lord",
            "ironclad" to "Ironclad",
            "kotm" to "King of the Monsters",
            "kotm2" to "King of the Monsters 2",
            "gpilots" to "Ghost Pilots",
            "sengoku" to "Sengoku",
            "sengoku2" to "Sengoku 2",
            "sengoku3" to "Sengoku 3"
        )
    }
}
