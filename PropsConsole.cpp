// PropsConsole: 
//			A console application that connects to an SQL Server via ODBC and allows to execute and visualize query results.
//
// Usage:	
//			Since this is a test program, in its first version, it has some usability restrictions.
//			The program requires an ODBC system Data Server Name (DSN) properly installed in the machine hosting the SQL database.
//			There is support for both 32 and 64bit DSNs depending on what platform the program has been compiled for. As by default 
//			it is compiled for Win32, it can connect only via 32bit DSNs.
//			Upon starting, the program queries for a DSN and indicates whether or not the connection succeeded.
//			Currently if the connection fails it aborts.
//			If the connecton succeedes, the UI shows the server we are connected to and queries can be executed.
//			To start, select an existing database in the connected server by doing 'use DBname' and upon success 'select' statements 
//			can be executed against it at the prompt.
//			Program also accepts a system DSN from the command-line.
//
// Modules:	
//			PropsConsole, main application and query loop
//			PropsSQLHandler, class in charge to handle all SQL communication
//
// Author:	
//			Raul R. Chirinos, 2019
//			version 1.0.0.1
//
// Notes:	
//			No great attempt has been made to have the table results output to be properly formatted in the console.
//			This program has taken some ideas, not code, from sample programs and documentation provided by Microsoft.

#include "pch.h"
#include <windows.h>
#include <stdio.h>
#include "PropsSQLHandler.h"

#define QUERY_PROMPT L"SQL QUERY > "
#define MAX_COLUMNS 20				// max output table columns
#define MAX_COLUMN_DATA 50			// max characters per table cell
#define	MAX_SERVER_NAME	256			// buffer size to get server name 
#define CONSOLE_OFFSET_SIZE	128		// increase size of screen row buffer by this number when needed

// Function that handles gathering the result statement information
// and displays it
int DisplayQueryResults(SQLSMALLINT nColumns, PropsSQLHandler *pHandler);
void DisplayExitError(WCHAR* errorMessage);

int __cdecl wmain(int argc, WCHAR *argv[]) {
	SQLRETURN sqlRet;
	PropsSQLHandler *pHandler = new PropsSQLHandler();
	WCHAR wszQuery[SQLQUERYSIZE];
	WCHAR wszDSN[SQLDSNSSIZE];
	WCHAR wszErrorMessage[128];
	SQLSMALLINT nColumns;	// number of returned colums
	HANDLE hConsole;		// console handle

	// Get output console handle
	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	SetConsoleTextAttribute(hConsole, (WORD)(BACKGROUND_BLUE | FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED));
	if (!pHandler) {
		wsprintf(wszErrorMessage, L"Unable to instantiate SQL handler. Aborting...\n\n");
		goto done;
	}

	wprintf_s(L"Use Ctrl + z to exit\n\n");
	SetConsoleTextAttribute(hConsole, (WORD)(FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED));

	if (argc <= 1) {
		wprintf_s(L"Please enter 32bit DSN:\n\n");
		if (fgetws(wszDSN, sizeof(wszDSN), stdin) == NULL) {

			return -1;
		}
		else {
			// Remove CR from input
			wszDSN[wcslen(wszDSN) - 1] = '\0';
		}
	}
	else {
		// Keep the DSN to connect to 
		wcscpy_s(wszDSN, argv[1]);
	}

	wprintf_s(L"[DSN]: %ls\n", wszDSN);

	// Allocate an environment
	sqlRet = pHandler->Init();
	if (!SQLSUCCEEDED(sqlRet)) {
		wsprintf(wszErrorMessage, L"Unable to initialize SQL handler, aborting...\n");
		DisplayExitError(wszErrorMessage);
		goto done;
	}

	sqlRet = pHandler->Connect(NULL, (SQLWCHAR*)wszDSN);
	if (!SQLSUCCEEDED(sqlRet)) {
		wsprintf(wszErrorMessage, L"Could not connect to [%s], aborting...\n", wszDSN);
		DisplayExitError(wszErrorMessage);
		goto done;
	}
	else {
		WCHAR wszServer[MAX_SERVER_NAME];
		sqlRet = pHandler->GetServerName(wszServer, MAX_SERVER_NAME);
		wprintf(L"Connected to [%s]\n\n", wszServer);
	}

	sqlRet = pHandler->AllocStatementHandle();
	if (!SQLSUCCEEDED(sqlRet)) {
		wsprintf(wszErrorMessage, L"Encounter error with statement allocation, aborting...\n");
		DisplayExitError(wszErrorMessage);
		goto done;
	}

	// All good start sending queried to our opened DB
	wprintf(L"Enter SQL commands\n\n%ls", QUERY_PROMPT);

	while (fgetws(wszQuery, SQLQUERYSIZE - 1, stdin))
	{
		if (!(*wszQuery))
		{
			wprintf(QUERY_PROMPT);
			continue;
		}

		// Remove CR
		wszQuery[wcslen(wszQuery) - 1] = '\0';

		// Execute the query
		sqlRet = pHandler->Query(wszQuery, &nColumns);

		switch (sqlRet) {
			
		case SQL_SUCCESS:
		case SQL_SUCCESS_WITH_INFO:
			wprintf_s(L"Executed %ls columns %d\n", wszQuery, nColumns);
			if(nColumns)
				DisplayQueryResults(nColumns, pHandler);
			break;		

		case SQL_ERROR:
			wprintf_s(L"Failed to execute %ls\n", wszQuery);
			break;

		case SQL_NO_DATA:
			wprintf_s(L"No rows affected\n");
			break;

		case SQL_INVALID_HANDLE:
			wprintf_s(L"Invalid handle. Exiting\n");
			goto done;
		
		// Anything else not handled for the sake of this test program
		default:
			break;
		}

		pHandler->ResetQuery();
		wprintf(QUERY_PROMPT);
	}


done:

	delete pHandler;
}



// Function that handles gathering the result statement information
// and displays it.
//TODO: Optimize, too many loops
int DisplayQueryResults(SQLSMALLINT nColumns, PropsSQLHandler *pHandler) {

	WCHAR colNameBuffer[MAX_COLUMN_DATA];	// buffer to contain column names
	SQLRETURN sqlRet;						// SQL return 
	SQLLEN colBindLength;
	bool bNoData;							// no more data in the statement flag
	WCHAR *pBuffers[MAX_COLUMNS];			// holds pointers to column data
	HANDLE hConsole;						// console handle
	
	// Get output console handle
	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	// Clean up columns buffer array
	ZeroMemory(&pBuffers, sizeof(WCHAR*)*MAX_COLUMNS);

	// Loop to output result table header, allocate cell buffers and 
	// bind column buffers with driver
	for (SQLSMALLINT column = 0; column < nColumns; column++) {

		// Display column names
		sqlRet = pHandler->GetColumnName(column + 1, colNameBuffer, MAX_COLUMN_DATA);
		if (SQLSUCCEEDED(sqlRet)) {
			SetConsoleTextAttribute(hConsole, (WORD)(BACKGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED));
			wprintf(L"%20s", colNameBuffer);
			if (column == nColumns - 1)
				wprintf(L"\n");
		}

		//Allocate column (table cell) data
		pBuffers[column] = (WCHAR*)malloc(MAX_COLUMN_DATA);
		if (pBuffers[column] == NULL)
			exit(-1);

		// Bind column data
		pHandler->BindColumn(column + 1, pBuffers[column], MAX_COLUMN_DATA, &colBindLength);
	}
	
	// Display column data
	SetConsoleTextAttribute(hConsole, (WORD)(FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED));

	int rowCount = 0;
	bNoData = false;

	CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
	if (GetConsoleScreenBufferInfo(hConsole, &consoleInfo) > 0) {
		int displayWidth = 10 + nColumns * 20;
		if (consoleInfo.dwSize.X < displayWidth) {
			consoleInfo.dwSize.X = displayWidth + 10;
			SetConsoleScreenBufferSize(hConsole, consoleInfo.dwSize);
		}
	}

	do {
		// Get whole statement data
		sqlRet = pHandler->FetchColumnData();


		if (sqlRet == SQL_NO_DATA_FOUND)
		{
			bNoData = true;
		}
		else
		{
			//There's data, display complete row preceeded by row number
			wprintf(L"%d\t", ++rowCount);
			for (SQLSMALLINT column = 0; column < nColumns; column++) {
				wprintf(L"%20s", pBuffers[column]);
				if (column == nColumns - 1)
					wprintf(L"\n");
			}
		}
	} while (!bNoData);
	
	// Clean up, unbind buffers and free them
	for (SQLSMALLINT column = 0; column < nColumns; column++) {
		// Unbind column
		pHandler->BindColumn(column + 1, NULL, 0, NULL);

		// Release memory used by binding
		if (pBuffers[column] != NULL) {
			free(pBuffers[column]);
			pBuffers[column] = NULL;
		}
	}
	
	return 0;
}

void DisplayExitError(WCHAR* errorMessage) {
	wprintf_s(errorMessage);
	getc(stdin);
}

