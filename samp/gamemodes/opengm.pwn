#include <opengm/opengm>

main()
{
    print("GameMode is starting...");
}

public OnGameModeInit()
{
    mysql_log(ALL);
    ConnectDatabase();
}

public OnGameModeExit()
{
    DisconnectDatabase();
}

public OnPlayerConnect(playerid)
{
    if (mysql_status != MYSQL_STATUS_CONNECTED)
    {
        KickWithReason(playerid, "Database is not connected.");
        return 0;
    }
    new query[128];

    ResetPlayerVariables(playerid);
    GetPlayerName(playerid, pInfo[playerid][Name], MAX_PLAYER_NAME);
    mysql_format(mysql, query, sizeof(query), "SELECT * FROM `players` WHERE `username`='%s'", pInfo[playerid][Name]);
    mysql_pquery(mysql, query, "OnPlayerAccountCheck", "i", playerid);
    return 1;
}

public OnPlayerDisconnect(playerid, reason)
{
    if (mysql_status != MYSQL_STATUS_CONNECTED)
        return 0;

    if (bIsLoggedIn[playerid])
    {
        new query[256];

        GetPlayerPos(playerid, pInfo[playerid][posX], pInfo[playerid][posY], pInfo[playerid][posZ]);
        GetPlayerIp(playerid, pInfo[playerid][Ip], 16);
        mysql_format(mysql, query, sizeof(query), "UPDATE `players` SET `ip` = '%s', `money` = %d, `score` = %d, `pos_x` = %f, `pos_y` = %f, `pos_z` = %f, `skin` = %d WHERE `id` = %d",
                     pInfo[playerid][Ip], pInfo[playerid][Money], pInfo[playerid][Score], pInfo[playerid][posX], pInfo[playerid][posY], pInfo[playerid][posZ], pInfo[playerid][Skin], pInfo[playerid][ID]);
        mysql_pquery(mysql, query);
    }
    return 1;
}

public OnPlayerSpawn(playerid)
{
    SetPlayerSkin(playerid, pInfo[playerid][Skin]);
    return 1;
}