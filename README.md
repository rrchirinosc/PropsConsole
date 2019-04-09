# PropsConsole
A console application that connects to an SQL Server via ODBC and allows to execute and visualize query results.

Since this is a test program, in its first version, it has some usability restrictions.
The program requires an ODBC system Data Server Name (DSN) properly installed in the machine hosting t
he SQL database.

There is support for both 32 and 64bit DSNs depending on what platform the program has been compiled for. 
As by default it is compiled for Win32, it can connect only via 32bit DSNs.

Upon starting, the program queries for a DSN and indicates whether or not the connection succeeded.

Currently if the connection fails program aborts.

Once connected, queries can be executed at the prompt.

Program also accept the DSN from the command-line.
