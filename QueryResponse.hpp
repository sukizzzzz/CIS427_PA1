#ifndef QUERY_RESPONSE_HPP
#define QUERY_RESPONSE_HPP

#include <string>
#include <vector>

/** This class represents a response from the database after an sql query.
 *  A query response object should be passed as the 4th arg to sqlite3_exec() as follows:
 *  sqlite3_exec(db, sql, response_callback, (void*) myQueryResponseObject, &zErrMsg)
 */
class QueryResponse {
public:
    int num_columns;
    std::vector<std::string> fields;
    std::vector<std::string> column_names;

    void print_all();
};

#endif /* QUERY_RESPONSE_HPP */
