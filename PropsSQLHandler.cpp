#include "pch.h"
#include <Windows.h>

#include "PropsSQLHandler.h"

//
//	ODBC wrapper class
//

PropsSQLHandler::PropsSQLHandler() {
	hEnv = NULL;
	hDbc = NULL;
	hStmt = NULL;
	bInitialized = FALSE;
	bConnected = FALSE;
}


PropsSQLHandler::~PropsSQLHandler() {
	if (hStmt) {
		SQLFreeHandle(SQL_HANDLE_STMT, 
			hStmt);
	}
	
	if (hDbc) {
		if(bConnected) 
			SQLDisconnect(hDbc);
		SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
	}

	if (hEnv) {
		SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
	}
}

// Initialize connection environment
SQLRETURN PropsSQLHandler::Init() {

	if (SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv) == SQL_ERROR) {
		return SQL_ERROR;
	}

	if (SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3,0) == SQL_ERROR) {
		return SQL_ERROR;
	}

	// Get a connection handler
	if (SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc) == SQL_ERROR) {
		return SQL_ERROR;
	}

	bInitialized = TRUE;
	return SQL_SUCCESS;
}


// Connect to system DSN
SQLRETURN PropsSQLHandler::Connect(HWND hwnd, SQLWCHAR* systemDsnName) {
	SQLRETURN	sqlRet;
	WCHAR wszConnectionString[1024];
	SQLSMALLINT len;
	
	if (!bInitialized) {
		return SQL_ERROR;
	}

	if (systemDsnName == NULL) {
		sqlRet = SQLDriverConnect(hDbc,
					hwnd,
					NULL,
					0,
					wszConnectionString,
					sizeof(wszConnectionString),
					&len,
					SQL_DRIVER_COMPLETE);
	}
	else {
		sqlRet = SQLConnect(hDbc, systemDsnName, SQL_NTS, NULL, 0, NULL, 0);
	}
	
	if (SQLSUCCEEDED(sqlRet)) {
		bConnected = TRUE;
	}

	return sqlRet;
}

// Allocate query/statement handle 
SQLRETURN PropsSQLHandler::AllocStatementHandle() {

	SQLRETURN sqlRet = SQL_SUCCESS;

	if (SQLFAILED(SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt)))
		sqlRet = SQL_ERROR;

	return sqlRet;
}

// Exxecute query
SQLRETURN PropsSQLHandler::Query(SQLWCHAR *wszQuery, SQLSMALLINT *nColumns)
{
	SQLRETURN sqlRet = SQL_SUCCESS;

	if (!bConnected) {
		sqlRet = SQL_INVALID_HANDLE;
		return sqlRet;
	}		

	sqlRet = SQLExecDirect(hStmt, 
		wszQuery, 
		SQL_NTS);

	if (SQLSUCCEEDED(sqlRet)) {
		sqlRet = SQLNumResultCols(hStmt, nColumns);
	}

	return sqlRet;
}


// Flush previous query
SQLRETURN PropsSQLHandler::ResetQuery() {
	
	return SQLCloseCursor(hStmt);
}

// Get a statement result column name
SQLRETURN PropsSQLHandler::GetColumnName(SQLSMALLINT columnNumber, WCHAR *pBuffer, int bufferSize) {
	
	return SQLColAttribute(hStmt,
		columnNumber,
		SQL_DESC_NAME,
		pBuffer,
		bufferSize,
		NULL,
		NULL);
}

// Get statement data
SQLRETURN PropsSQLHandler::FetchColumnData() {
	return SQLFetch(hStmt);
}

// Bind statement column data with driver so it gets placed in given buffer
SQLRETURN PropsSQLHandler::BindColumn(SQLSMALLINT columnNumber, WCHAR *pBuffer, int bufferSize, SQLLEN *pcolBindLength) {
	return SQLBindCol(hStmt,
		columnNumber,
		SQL_C_WCHAR,
		pBuffer,
		bufferSize,
		pcolBindLength);
}

// Retrieve current statement row count
SQLRETURN PropsSQLHandler::GetRowCount(SQLLEN *rows)
{
	return SQLRowCount(hStmt, rows);
}

SQLRETURN PropsSQLHandler::GetServerName(WCHAR *server, int buffersize) {

	return SQLGetInfo(hDbc, 
		SQL_SERVER_NAME, 
		(SQLPOINTER)server, 
		buffersize, 
		NULL);
}
