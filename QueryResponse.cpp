#include "QueryResponse.hpp"
#include <cstdio>
#include <iostream>

void QueryResponse::print_all() {
    // WRONG throwing this all out. trying a different approach
    for(int i = 0; i < num_columns; i++) {
        std::cout << column_names[i] << " = " << fields[i] << std::endl;
    }
}
