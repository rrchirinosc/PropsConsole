# PropsConsole
A console application that connects to an SQL Server via ODBC and allows to execute and visualize query results.

Since this is a test program, in its first version, it has some usability restrictions.
The program requires an ODBC system Data Server Name (DSN) properly installed in the machine hosting t
he SQL database.

There is support for both 32 and 64bit DSNs depending on what platform the program has been compiled for. 
As by default it is compiled for Win32, it can connect only via 32bit DSNs.

Upon starting, the program queries for a DSN and indicates whether or not the connection succeeded.

Currently if the connection fails program aborts.

If the connecton succeedes, the UI shows the server we are connected to and queries can be executed.

To start, select an existing database in the connected server by doing 'use DBname' and upon success 'select' statements 
can be executed against it at the prompt.

Program also accepts a system DSN from the command-line.

Compilation details:

Project was created using Visual Studio 2017 -- not tested with other versions.

To build dowload source and execute PropsConsole.vcxproj.

This will launch Visual Studio with the project loaded.

Build the desired configuration and save the solution file (sln) for future use.