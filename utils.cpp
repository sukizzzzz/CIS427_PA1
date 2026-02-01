#include "utils.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sqlite3.h>
#include <string>
#include <sys/socket.h>

using namespace std;

int callback(void *data, int argc, char **argv, char **azColName) {
    fprintf(stderr, "%s: ", (const char*)data);

    for (int i = 0; i < argc; i++) {
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}


int count_users(void *count, int argc, char **argv, char **azColName) {
    if (argc == 1) {
        *static_cast<int*>(count) = atoi(argv[0]);
    } else {
        fprintf(stderr, "Invalid use of count_users()");
    }
    return 0;
}


int buy_command(int socket, char* request, sqlite3* db) {
    char *buy = strtok(request, " ");
    char *stock_symbol = strtok(NULL, " ");
    char *stock_amount = strtok(NULL, " ");
    char *price = strtok(NULL, " ");
    char *user_id = strtok(NULL, " ");

    // check for format errors
    if (!buy || !stock_symbol || !stock_amount || !price || !user_id) {
        const char* error_code = "403 message format error\nCorrect format: BUY <stock_symbol> <amount> <price> <user_id>\n";
        send(socket, error_code, strlen(error_code), 0);
        return -1;
    }

    // TODO finish

    return 0;
}


int sell_command(int socket, char* request, sqlite3* db) {

    // TODO

    return 0;
}


int list_command(int socket, char* request, sqlite3* db) {

    // TODO

    return 0;
}


int balance_command(int socket, char* request, sqlite3* db) {

    // TODO

    return 0;
}


int shutdown_command(int socket, char* request, sqlite3* db) {

    // TODO

    return 0;
}


int quit_command(int socket, char* request, sqlite3* db) {

    // TODO

    return 0;
}