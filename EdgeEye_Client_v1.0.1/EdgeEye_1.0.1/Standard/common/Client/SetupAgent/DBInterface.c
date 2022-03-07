#include <stdlib.h>
#include <stdio.h>
#include "sqlite3.h"
#include "DBInterface.h"

int ReadStatus()
{
	sqlite3 *db;
	char *errMsg = NULL;
	int rows, cols;
	char **result;
	int status = 4;

	if(sqlite3_open_v2(DBName, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL))
	{
       return -1;
	}

	sqlite3_get_table(db, "SELECT Status FROM `Status` WHERE Id = 1;", &result, &rows, &cols, &errMsg);

	if(rows >= 1)
	{
		status = atoi(*(result+1));
	}

	sqlite3_free_table(result);
	sqlite3_close(db);

	return status;
}