#if defined _opengm_included
	#endinput
#endif
#define _opengm_included

#include <a_samp>
#include <samp_bcrypt>

#if defined MAX_PLAYERS
    #undef MAX_PLAYERS
#endif
#define MAX_PLAYERS             (100)


#define COLOR_WHITE             (0xFFFFFFFF)
#define COLOR_RED               (0xFF0000FF)
#define COLOR_GREEN             (0x00FF00FF)
#define COLOR_BLUE              (0x0000FFFF)


/* --------- Player Variables --------- */

enum E_PlayerInfo {
    ID,
    Name[MAX_PLAYER_NAME],
    Password[BCRYPT_HASH_LENGTH],
    Ip[16],
    Money,
    Score,
    Float:posX,
    Float:posY,
    Float:posZ,
    Skin
}
new pInfo[MAX_PLAYERS][E_PlayerInfo];

new bIsLoggedIn[MAX_PLAYERS];
new invalidPasswordAttempts[MAX_PLAYERS];


/* --------- Server Variables --------- */

enum E_SpawnPoint {
    Float:X,
    Float:Y,
    Float:Z,
    Float:A
}
new const ServerSpawnPoints[][E_SpawnPoint] = {
    { 826.77, -1771.21 ,13.39 , 2.19 },
    { -2009.82, 130.82, 27.53, 183.36 },
    { 2044.92, 1216.87, 10.67, 186.14 }
};

new const ServerSpawnSkins[] = {
    0,
    1,
    2,
    3,
    4
};

stock KickWithReason(playerid, reason[]) {
    SendClientMessage(playerid, COLOR_RED, reason);
    SetTimerEx("KickEx", 50, false, "i", playerid);
}

forward KickEx(playerid);
public KickEx(playerid) {
    Kick(playerid);
}

stock ResetPlayerVariables(playerid) {
    bIsLoggedIn[playerid] =
    invalidPasswordAttempts[playerid] = 0;
}

#include <opengm/opengm_dialog>
#include <opengm/opengm_db>
