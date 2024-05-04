#if defined _opengm_dialog_included
	#endinput
#endif
#define _opengm_dialog_included


#define DIALOG_LOGIN            (0)
#define DIALOG_REGISTER         (1)

#define MIN_PASSWORD_LENGTH     4
#define MAX_PASSWORD_LENGTH     30
#define MAX_INVALID_PASSWORD_ATTEMPTS 3


public OnDialogResponse(playerid, dialogid, response, listitem, inputtext[])
{
    if(dialogid == DIALOG_LOGIN)
    {
        if(!response) ShowPlayerLoginDialog(playerid, "Please login to play.");
        else
        {
            new const len = strlen(inputtext);
            if(len < MIN_PASSWORD_LENGTH || len > MAX_PASSWORD_LENGTH) OnPlayerInvalidPasswordAttempt(playerid);
            else bcrypt_verify(playerid, "OnPlayerPasswordVerify", inputtext, pInfo[playerid][Password]);
        }
    }
    else if(dialogid == DIALOG_REGISTER)
    {
        if(!response) ShowPlayerRegisterDialog(playerid, "Please register to play.");
        else
        {
           new const len = strlen(inputtext);
           if(len < MIN_PASSWORD_LENGTH || len > MAX_PASSWORD_LENGTH) ShowPlayerRegisterDialog(playerid, "Password length must be between "#MIN_PASSWORD_LENGTH" and "#MAX_PASSWORD_LENGTH" characters.");
           else bcrypt_hash(playerid, "OnPlayerPasswordHash", inputtext, BCRYPT_COST);
        }
    }
}


stock ShowPlayerLoginDialog(playerid, info[]) {
    ShowPlayerDialog(playerid, DIALOG_LOGIN, DIALOG_STYLE_PASSWORD, "Login", info, "Login", "Cancel");
}

stock ShowPlayerRegisterDialog(playerid, info[]) {
    ShowPlayerDialog(playerid, DIALOG_REGISTER, DIALOG_STYLE_PASSWORD, "Register", info, "Register", "Cancel");
}