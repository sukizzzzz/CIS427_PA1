#ifndef UTILS_HPP
#define UTILS_HPP
#include <sqlite3.h>

/** Callback function for the sqlite database.
 *  Prints each record processed in each SELECT statement executed within the SQL argument.
 *  From https://www.tutorialspoint.com/sqlite/sqlite_c_cpp.htm
 */
int callback(void *data, int argc, char **argv, char **azColName);

/** Used to count the number of entries in a table.
 *  Meant to be passed to sqlite3_exec with an sql query of SELECT COUNT(*) FROM table;
 */
int count_rows(void *count, int argc, char **argv, char **azColName);

/** This creates the Users table if it doesn't exits and adds a default user 
 *  if there are no users in the table 
 */
void create_users(sqlite3* db);

/** This creates the Stocks table if it doesn't exits and inserts the mag 7 stocks
 *  for the default user if the table is empty.
 */
void create_stocks(sqlite3* db);

/** This will buy an amount of stocks and respond to the client
 *  with the new balance. It creates or updates a record in the stock
 *  table if one does not exist.
 */
int buy_command(int socket, char* request, sqlite3* db);


int sell_command(int socket, char* request, sqlite3* db);


static int list_callback(void *data, int argc, char **argv, char **azColName);

int list_command(int socket, char* request, sqlite3* db);


int balance_command(int socket, char* request, sqlite3* db);


int shutdown_command(int socket, char* request, sqlite3* db);


int quit_command(int socket, char* request, sqlite3* db);

#endif
