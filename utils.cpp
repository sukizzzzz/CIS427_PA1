#include "utils.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <iostream>
#include <sqlite3.h>
#include <string>
#include <sys/socket.h>    /* Unix; Windows: use winsock2.h for send() */

using namespace std;

int callback(void *data, int argc, char **argv, char **azColName) {
    fprintf(stderr, "%s: ", (const char*)data);

    for (int i = 0; i < argc; i++) {
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}


int count_rows(void *count, int argc, char **argv, char **azColName) {
    if (argc == 1) {
        *static_cast<int*>(count) = atoi(argv[0]);
    } else {
        fprintf(stderr, "Invalid use of count_users()");
    }
    return 0;
}


void create_users(sqlite3* db) {
    const char *sql;
    int rc;
    char *zErrMsg = 0;

    // Create the Users table
    sql = "CREATE TABLE IF NOT EXISTS Users (" \
        "ID INTEGER PRIMARY KEY AUTOINCREMENT," \
        "email TEXT NOT NULL," \
        "first_name TEXT," \
        "last_name TEXT," \
        "user_name TEXT NOT NULL," \
        "password TEXT," \
        "usd_balance DOUBLE NOT NULL);" ;

    rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
    if( rc != SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    } else {
        fprintf(stdout, "Users table created successfully\n");
    }
    
    // Check if there are any users and if not manually create one
    sql = "SELECT COUNT(*) FROM Users;";
    int user_count = 0;
    cout << "Checking user count" << endl;

    rc = sqlite3_exec(db, sql, count_rows, (void*) &user_count, &zErrMsg);
    if( rc != SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    } else {
        fprintf(stdout, "Operation done successfully\n");
    }

    if (user_count == 0) {
        fprintf(stdout, "Creating a user because no users currently exist\n");

        sql = "INSERT INTO Users (ID, email, first_name, last_name, user_name, password, usd_balance)" \
            "VALUES (1, 'joeshmoe@default.com', 'Joe', 'Shmoe', 'Default_User', 'password1', 100.00 );";

        rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
        if( rc != SQLITE_OK ){
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
        } else {
            fprintf(stdout, "Records created successfully\n");
        }
    } else {
        fprintf(stdout, "There are currently %d users in the database\n", user_count);
    }
}


void create_stocks(sqlite3* db) {
    const char *sql;
    int rc;
    char *zErrMsg = 0;

    // Create the Stocks table
    sql = "CREATE TABLE IF NOT EXISTS Stocks (" \
        "ID INTEGER PRIMARY KEY AUTOINCREMENT," \
        "stock_symbol VARCHAR(4) NOT NULL," \
        "stock_name VARCHAR(20) NOT NULL," \
        "stock_balance DOUBLE," \
        "user_id INTEGER," \
        "FOREIGN KEY (user_id) REFERENCES Users (ID) );";

    rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
    if( rc != SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    } else {
        fprintf(stdout, "Stocks table created successfully\n");
    }

    // add stocks to the stocks table if it is empty
    sql = "SELECT COUNT(*) FROM Stocks;";
    int stock_count = 0;
    cout << "Checking if Stocks table has entries" << endl;

    rc = sqlite3_exec(db, sql, count_rows, (void*) &stock_count, &zErrMsg);
    if( rc != SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    } else {
        fprintf(stdout, "Check operation was successful\n");
    }

    if (stock_count == 0) {
        fprintf(stdout, "Inserting stocks for the default user because no stocks currently exist\n");

        sql = "INSERT INTO Stocks (stock_symbol, stock_name, stock_balance, user_id) VALUES " \
            "('AAPL', 'Apple', 0, 1), " \
            "('MSFT', 'Microsoft', 0, 1), " \
            "('AMZN', 'Amazon', 0, 1), " \
            "('GOOG', 'Alphabet', 0, 1), "\
            "('META', 'Meta', 0, 1), " \
            "('NVDA', 'Nvidia', 0, 1), " \
            "('TSLA', 'Tesla', 0, 1);";

        rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
        if( rc != SQLITE_OK ){
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
        } else {
            fprintf(stdout, "Records created successfully\n");
        }
    } else {
        fprintf(stdout, "There are currently %d stocks in the database\n", stock_count);
    }
}


int buy_command(int socket, char* request, sqlite3* db) {
    // TODO make sure to test invalid strings like missing fields or not numbers

    // parse input string (strtok_r is thread-safe; saveptr holds state between calls)
    char *saveptr = nullptr;
    char *buy = strtok_r(request, " ", &saveptr);
    char *stock_symbol = strtok_r(nullptr, " ", &saveptr);
    char *amount_str = strtok_r(nullptr, " ", &saveptr);
    char *price_str = strtok_r(nullptr, " ", &saveptr);
    char *user_id = strtok_r(nullptr, " ", &saveptr);

    // check for format errors
    if (!buy || !stock_symbol || !amount_str || !price_str || !user_id) {
        fprintf(stderr, "BUY command recieved but incorrecly formatted\n");
        fprintf(stderr, "Error message sent to client\n");
        const char* error_code = "403 message format error\nCorrect format: BUY <stock_symbol> <amount_to_buy> <price_per_stock> <user_id>\n";
        send(socket, error_code, strlen(error_code), 0);
        return -1;
    }

    // convert the amount and price to doubles
    double amount_to_buy;
    double price_per_stock;
    try {
        amount_to_buy = std::stod(amount_str); // throws exception if conversion fails
        price_per_stock = std::stod(price_str);
    } catch (const std::exception&) {
        fprintf(stderr, "Amount or price are not valid numbers\n");
        const char* error_code = "403 message format error\nAmount and price fields must be numbers.\n";
        send(socket, error_code, strlen(error_code), 0);
        return -1;
    }

    // calculate stockprice and deduct it from the users and stocks balance
    double stockprice = amount_to_buy * price_per_stock;
    char sql[256];
    int rc;
    char *zErrMsg = 0;

    // get id and balance first
    snprintf(sql, sizeof(sql),
            "SELECT FROM Stocks WHERE ... "); // TODO finish here
    // insert to Stocks table
    double new_balance = 0;
    snprintf(sql, sizeof(sql), //TODO warning missing stock_name!!
             "INSERT INTO Stocks (stock_symbol, stock_balance, user_id) VALUES (%s, %f, %d);",       
             stock_symbol, new_balance, 1);

    // update balance in Users table


    // new record in the Stocks table is created or updated if one does exist

    // If the operation is successful return a message to the client with “200 OK”, 
    // the new usd_balance
    // and new stock_balance;
    // otherwise, an appropriate message should be displayed, e.g.: Not
    // enough balance, or user1 doesn’t exist, etc.
     
    // TODO finish

    return 0;
}


int sell_command(int socket, char* request, sqlite3* db) {

    // TODO

    return 0;
}


int shutdown_command(int socket, char* request, sqlite3* db) {

    // TODO

    return 0;
}


static int list_callback(void *data, int argc, char **argv, char **azColName) {
    string* result = static_cast<string*>(data);
   
    for (int i = 0; i < argc; i++) {
        if (argv[i]) {
            *result += argv[i];
        } else {
            *result += "NULL";
        }
        if (i < argc - 1) {
            *result += " ";
        }
    }
    *result += "\n";
   
    return 0;
}

int list_command(int socket, char* request, sqlite3* db) {
    /* default user id */
    int user_id = 1;
   
    string response;
    string records;
    char *zErrMsg = 0;
   
    /* query stocks for user */
    char sql[256];
    snprintf(sql, sizeof(sql),
             "SELECT ID, stock_symbol, stock_balance, user_id FROM Stocks WHERE user_id = %d;",
             user_id);
   
    int rc = sqlite3_exec(db, sql, list_callback, &records, &zErrMsg);
   
    if (rc != SQLITE_OK) {
        response = "403 message format error\n";
        response += "Database error: ";
        response += zErrMsg;
        response += "\n";
        sqlite3_free(zErrMsg);
    } else {
        response = "200 OK\n";
        response += "The list of records in the Stocks database for user ";
        response += to_string(user_id);
        response += ":\n";
       
        if (records.empty()) {
            response += "(No stocks found)\n";
        } else {
            response += records;
        }
    }
   
    send(socket, response.c_str(), response.length(), 0);
    return 0;
}

int balance_command(int socket, char* request, sqlite3* db) {
    /* default user id */
    int user_id = 1;
   
    string response;
    char sql[256];
    snprintf(sql, sizeof(sql),
             "SELECT first_name, last_name, usd_balance FROM Users WHERE ID = %d;",
             user_id);
   
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
   
    if (rc != SQLITE_OK) {
        response = "403 message format error\n";
        response += "Database error: ";
        response += sqlite3_errmsg(db);
        response += "\n";
        send(socket, response.c_str(), response.length(), 0);
        return -1;
    }
   
    rc = sqlite3_step(stmt);
   
    if (rc == SQLITE_ROW) {
        /* user found */
        const char* first_name = (const char*)sqlite3_column_text(stmt, 0);
        const char* last_name = (const char*)sqlite3_column_text(stmt, 1);
        double usd_balance = sqlite3_column_double(stmt, 2);
       
        response = "200 OK\n";
        response += "Balance for user ";
        if (first_name) response += first_name;
        response += " ";
        if (last_name) response += last_name;
       
        char balance_str[50];
        snprintf(balance_str, sizeof(balance_str), ": $%.2f\n", usd_balance);
        response += balance_str;
       
    } else if (rc == SQLITE_DONE) {
        /* user not found */
        response = "403 message format error\n";
        response += "User ";
        response += to_string(user_id);
        response += " doesn't exist\n";
    } else {
        /* database error */
        response = "403 message format error\n";
        response += "Database error: ";
        response += sqlite3_errmsg(db);
        response += "\n";
    }
   
    sqlite3_finalize(stmt);
    send(socket, response.c_str(), response.length(), 0);
    return 0;
}

int quit_command(int socket, char* request, sqlite3* db) {
    /* acknowledge quit */
    const char* response = "200 OK\n";
    send(socket, response, strlen(response), 0);
   
    /* signal server to close connection */
    return 1;
}
