#pragma once

#include <sal.h>
#include <sql.h>
#include <sqlext.h>

#define SQLQUERYSIZE 512
#define SQLDSNSSIZE 128
#define SQLSUCCEEDED(x) ((x == SQL_SUCCESS) || (x == SQL_SUCCESS_WITH_INFO))
#define SQLFAILED(x) !SQLSUCCEEDED(x)

// Test DSNs
#define X64DSN		L"SQLServer11 DSA64"
#define X32DSN		L"SQL Server11"

// Wrapper class that encapsulates all SQL related calls and handles

class PropsSQLHandler
{

private:
	SQLHENV     hEnv;
	SQLHDBC     hDbc;
	SQLHSTMT    hStmt;
	BOOL		bInitialized;	// are odbc required handles acquired for connection?
	BOOL		bConnected;		// is driver connected?
	
public:
	PropsSQLHandler();
	~PropsSQLHandler();
	
	// 
	SQLRETURN Init();
	SQLRETURN Connect(HWND hwnd, SQLWCHAR*systemDsnName = NULL);
	SQLRETURN AllocStatementHandle();
	SQLRETURN Query(SQLWCHAR*, SQLSMALLINT*);
	SQLRETURN ResetQuery();
	SQLRETURN GetColumnName(SQLSMALLINT columnNumber, WCHAR *pBuffer, int bufferSize);
	SQLRETURN FetchColumnData();
	SQLRETURN BindColumn(SQLSMALLINT columnNumber, WCHAR *pBuffer, int bufferSize, SQLLEN *colBindLength);
	SQLRETURN GetRowCount(SQLLEN*);
	SQLRETURN GetServerName(SQLWCHAR *, int);
};

