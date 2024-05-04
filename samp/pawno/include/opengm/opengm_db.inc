#if defined _opengm_db_included
	#endinput
#endif
#define _opengm_db_included

#include <a_mysql>

#define MYSQL_HOST "localhost"
//#define MYSQL_HOST "mysql-container" use this if you are using docker

#define MYSQL_USER "root"
#define MYSQL_PASS "123"
#define MYSQL_DB "samp"

#define MYSQL_STATUS_NOT_CONNECTED      (0)
#define MYSQL_STATUS_CONNECTED          (1)
#define MYSQL_STATUS_CONNECTION_LOST    (2)

#define MYSQL_RECONNECT_INTERVAL        (1000 * 10)

#define MYSQL_ROW_ID                    (0)
#define MYSQL_ROW_NAME                  (1)
#define MYSQL_ROW_PASSWORD              (2)
#define MYSQL_ROW_IP                    (3)
#define MYSQL_ROW_MONEY                 (4)
#define MYSQL_ROW_SCORE                 (5)
#define MYSQL_ROW_POSX                  (6)
#define MYSQL_ROW_POSY                  (7)
#define MYSQL_ROW_POSZ                  (8)
#define MYSQL_ROW_SKIN                  (9)

new MySQL:mysql;
new mysql_err;
new mysql_status;
new mysql_connection_timer;

forward OnPlayerAccountCheck(playerid);
forward OnPlayerRegister(playerid);

forward OnPlayerPasswordVerify(playerid, bool:success);
forward OnPlayerPasswordHash(playerid);
forward OnMysqlReconnect();

public OnPlayerAccountCheck(playerid)
{
   if(cache_num_rows()) 
   {
        cache_get_value_index_int(0, MYSQL_ROW_ID, pInfo[playerid][ID]);
        cache_get_value_index(0, MYSQL_ROW_PASSWORD, pInfo[playerid][Password], BCRYPT_HASH_LENGTH);
        cache_get_value_index(0, MYSQL_ROW_IP, pInfo[playerid][Ip], 16);
        cache_get_value_index_int(0, MYSQL_ROW_MONEY, pInfo[playerid][Money]);
        cache_get_value_index_int(0, MYSQL_ROW_SCORE, pInfo[playerid][Score]);
        cache_get_value_index_float(0, MYSQL_ROW_POSX, pInfo[playerid][posX]);
        cache_get_value_index_float(0, MYSQL_ROW_POSY, pInfo[playerid][posY]);
        cache_get_value_index_float(0, MYSQL_ROW_POSZ, pInfo[playerid][posZ]);
        cache_get_value_index_int(0, MYSQL_ROW_SKIN, pInfo[playerid][Skin]);

        ShowPlayerLoginDialog(playerid, "Please login to play.");
   }
   else ShowPlayerRegisterDialog(playerid, "You are not registered yet. Please register to play.");
}

public OnPlayerRegister(playerid)
{
   pInfo[playerid][ID] = cache_insert_id();

   bIsLoggedIn[playerid] = 1;
   SpawnPlayer(playerid);
}

public OnPlayerPasswordVerify(playerid, bool:success)
{
    if(success) 
    {
        bIsLoggedIn[playerid] = 1;
        SetSpawnInfo(playerid, NO_TEAM, pInfo[playerid][Skin], pInfo[playerid][posX], pInfo[playerid][posY], pInfo[playerid][posZ], 0.0, 0, 0, 0, 0, 0, 0);
        SpawnPlayer(playerid);
    }
    else OnPlayerInvalidPasswordAttempt(playerid);
}

public OnPlayerPasswordHash(playerid) 
{
    new query[256], rand = random(sizeof(ServerSpawnPoints));

    bcrypt_get_hash(pInfo[playerid][Password], BCRYPT_HASH_LENGTH);
    pInfo[playerid][Money] = 
    pInfo[playerid][Score] = 0;
    pInfo[playerid][posX] = ServerSpawnPoints[rand][X];
    pInfo[playerid][posY] = ServerSpawnPoints[rand][Y];
    pInfo[playerid][posZ] = ServerSpawnPoints[rand][Z];
    pInfo[playerid][Skin] = ServerSpawnSkins[random(sizeof(ServerSpawnSkins))];
    GetPlayerIp(playerid, pInfo[playerid][Ip], 16);

    SetSpawnInfo(playerid, NO_TEAM, pInfo[playerid][Skin], pInfo[playerid][posX], pInfo[playerid][posY], pInfo[playerid][posZ], ServerSpawnPoints[rand][A], 0, 0, 0, 0, 0, 0);

    mysql_format(mysql, query, sizeof(query), "INSERT INTO `players` (`username`, `password`, `ip`, `pos_x`, `pos_y`, `pos_z`, `skin`) VALUES ('%s', '%s', '%s', %f, %f, %f, %d)", 
    pInfo[playerid][Name], pInfo[playerid][Password], pInfo[playerid][Ip], pInfo[playerid][posX], pInfo[playerid][posY], pInfo[playerid][posZ], pInfo[playerid][Skin]);
    mysql_tquery(mysql, query, "OnPlayerRegister", "i", playerid);

    ResetPlayerMoney(playerid);
    SetPlayerScore(playerid, 0);
}

stock OnPlayerInvalidPasswordAttempt(playerid) 
{
     invalidPasswordAttempts[playerid]++;
     if(invalidPasswordAttempts[playerid] >= MAX_INVALID_PASSWORD_ATTEMPTS) 
     {
        new text[144];

        format(text, sizeof(text), "Player %s has been kicked for too many invalid password attempts.", pInfo[playerid][Name]);
        SendClientMessageToAll(COLOR_RED, text);
        Kick(playerid);
     }
     else 
     {
         new dialogInfo[128];

         format(dialogInfo, sizeof(dialogInfo), "Invalid password attempts %d/%d", invalidPasswordAttempts[playerid], MAX_INVALID_PASSWORD_ATTEMPTS);
         ShowPlayerLoginDialog(playerid, dialogInfo);
     }
}

forward OnPlayerAccountSaved(playerid);
public OnPlayerAccountSaved(playerid)
{
    bIsLoggedIn[playerid] = 0;

    for(new i = GetPlayerPoolSize(); i != -1; i--)
        if(bIsLoggedIn[i]) 
            return;

    mysql_status = MYSQL_STATUS_CONNECTED;
}

public OnMysqlReconnect()
{
   new query[256];
   for(new i = GetPlayerPoolSize(); i != -1; i--) 
   {
        if(IsPlayerConnected(i) && bIsLoggedIn[i]) 
        {
           GetPlayerPos(i, pInfo[i][posX], pInfo[i][posY], pInfo[i][posZ]);
           GetPlayerIp(i, pInfo[i][Ip], 16);
           mysql_format(mysql, query, sizeof(query), "UPDATE `players` SET `ip` = '%s', `money` = %d, `score` = %d, `pos_x` = %f, `pos_y` = %f, `pos_z` = %f, `skin` = %d WHERE `id` = %d",
           pInfo[i][Ip], pInfo[i][Money], pInfo[i][Score], pInfo[i][posX], pInfo[i][posY], pInfo[i][posZ], pInfo[i][Skin], pInfo[i][ID]);
           mysql_tquery(mysql, query, "OnPlayerAcccountSaved", "i", i);
        }
   }
           
}

stock ConnectDatabase() {
    
    if(mysql_status == MYSQL_STATUS_CONNECTED)
        return;

    if(CreateConnection() != 0) 
         mysql_connection_timer = SetTimer("ConnectTask", MYSQL_RECONNECT_INTERVAL, true);
    else {
        CreateTable();
        mysql_status = MYSQL_STATUS_CONNECTED;
        print("Connected to database.");
    }
}

forward ConnectTask();
public ConnectTask()
{
    if(CreateConnection() != 0) {
        printf("Error connecting to database: %d", mysql_err);
        DisconnectDatabase();
    } else {
        print("Connected to database.");
        KillTimer(mysql_connection_timer);
        CreateTable();
        if(mysql_status == MYSQL_STATUS_CONNECTION_LOST) 
            OnMysqlReconnect();
        else 
            mysql_status = MYSQL_STATUS_CONNECTED;
    }
}



stock CreateConnection() {
    print("Connecting to database...");
    mysql = mysql_connect(MYSQL_HOST, MYSQL_USER, MYSQL_PASS, MYSQL_DB);  
    mysql_err = mysql_errno();
    return mysql_err;
}

stock DisconnectDatabase() {
    mysql_close(mysql);
}

stock CreateTable() {
    new const begin[] = "\
        CREATE TABLE IF NOT EXISTS `players` (`id` int NOT NULL AUTO_INCREMENT,`username` varchar(24) COLLATE utf8mb4_unicode_ci NOT NULL,\
        `password` varchar(129) COLLATE utf8mb4_unicode_ci NOT NULL,`ip` varchar(16) COLLATE utf8mb4_unicode_ci NOT NULL,";
    new const end[] = "`money` int NOT NULL DEFAULT '0',`score` int NOT NULL DEFAULT '0',`pos_x` float NOT NULL,`pos_y` float NOT NULL,`pos_z` float NOT NULL,\
        `skin` smallint UNSIGNED NOT NULL, PRIMARY KEY (`id`)) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;";

    new query[512];
    strcat(query, begin);
    strcat(query, end);

    mysql_query(mysql, query, false);
}

public OnQueryError(errorid, const error[], const callback[], const query[], MySQL:handle)
{
    if(mysql_status != MYSQL_STATUS_CONNECTED)
        return;
    
	switch(errorid)
	{
		case 2006,2013,2055:
		{
            if(mysql_status == MYSQL_STATUS_CONNECTED)
            {
                for(new i = GetPlayerPoolSize(); i != -1; i--) 
                    if(IsPlayerConnected(i)) 
                        Kick(i);

                mysql_status = MYSQL_STATUS_CONNECTION_LOST;
                DisconnectDatabase();
                ConnectDatabase();

            }
		}
	}
}
