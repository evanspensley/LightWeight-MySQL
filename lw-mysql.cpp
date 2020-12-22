// Copyright Evan Spensley
#define MYSQLPP_MYSQL_HEADERS_BURIED
#include <mysql++/mysql++.h>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>

const char* g_username = "username";
const char* g_database = "database";
const char* g_server = "server";
const char* g_password = "password";

mysqlpp::Connection connectToDatabase(){
    mysqlpp::Connection myDB;
    try {
        myDB = mysqlpp::Connection(g_database, g_server, g_username, g_password);
    } catch (const std::exception& e) { 
        std::cerr << "Invalid database credentials. Change at top of file." << std::endl;
    }

    return myDB;
}

void executeQuery(std::ostream& output, std::string qString,
                    mysqlpp::Query query) {
    try {
        // Check query is correct
        // query << qString;
        query << qString;

        // Execute query
        query.parse();

        // Results is a 2D vector of mysqlpp::String objects.
        mysqlpp::StoreQueryResult result = query.store();
        // Print the results.
        output << "-----Query Result-----" << std::endl;
        for ( size_t row = 0; (row < result.size()); row++ ) {
            for ( size_t col = 0; col < result[row].size(); col++ ) {
                output << " | " << result[row][col].c_str() << " | ";
            }
            output << std::endl;
        }
        output << "------End Result------" << std::endl;
    } catch (mysqlpp::BadQuery e) {
        std::cerr << "Query: " << qString <<std::endl;
        std::cerr << "Query is not correct SQL syntax" <<std::endl;
    }
}

void interactive() {
    // Connect to database with: database, server, userID, password
    mysqlpp::Connection myDB = connectToDatabase();

    // Variable to build query string
    std::string qString;
    
    // Get user input for query
    while (std::getline(std::cin, qString, ';') && qString != "quit") { 
        mysqlpp::Query query = myDB.query();
        executeQuery(std::cout, qString, query);
    }
}

void loadFile(std::string& inPath, std::string& outPath) {
    std::ifstream inputFile(inPath);    
    std::ofstream outputFile(outPath);    

    // Connect to database with: database, server, userID, password
    mysqlpp::Connection myDB = connectToDatabase();

    std::string queryStr;
    while ( std::getline(inputFile, queryStr) ) {
        mysqlpp::Query query = myDB.query();
        executeQuery(outputFile, queryStr, query); 
    }
}

std::string generateLoadQuery(std::string& line) {
    // Create base insert query string
    std::string insertQ = "INSERT INTO ";

    // Split file on commas
    std::vector<std::string> splitLine;
    boost::split(splitLine, line , boost::is_any_of(","));

    // Start building query from split files (table name)
    insertQ += splitLine[0];

    // Strings to hold attributes and values
    std::string attributes = "(";
    std::string values = "(";

    // Build attribute and value strings
    for (int i = 1; i < splitLine.size(); i++) {
        std::vector<std::string> splitColon;
        boost::split(splitColon, splitLine[i], boost::is_any_of(":"));

        if (i != splitLine.size()-1) {
            attributes += " " + splitColon[0] + ", ";
            values += " " + splitColon[1] + ", ";
        } else {
            attributes += " " + splitColon[0];
            values += " " + splitColon[1];
        }
    }
    attributes += ")";
    values += ")";

    // Form full query string
    insertQ += attributes + " VALUES " + values + ";";

    // std::cout << insertQ;
    return insertQ;
}

std::tuple<std::string, std::string> generateTableQuery(std::string& line) {
    std::string tableName;

    // Create base insert query string
    std::string tableQ = "CREATE TABLE ";

    // Split file on commas
    std::vector<std::string> splitLine;
    boost::split(splitLine, line , boost::is_any_of(","));

    // Build attribute and value strings
    for (int i = 0; i < splitLine.size(); i++) {
        std::vector<std::string> splitColon;
        boost::split(splitColon, splitLine[i], boost::is_any_of(":"));

        if (splitColon[0] == "table") {
            tableName = splitColon[1];
            tableQ += splitColon[1];
            tableQ += "(";
            continue;
        }

        if (i == splitLine.size()-1)
            tableQ += " " + splitColon[0] + " " + splitColon[1]; 
        else
            tableQ += " " + splitColon[0] + " " + splitColon[1] + ","; 
    }
    tableQ += " ); ";

    return std::make_tuple(tableQ, tableName);
}

void loadData(std::string& path) {
    // Open file stream
    std::ifstream inputFile(path);    
    
    // Connect to database with: database, server, userID, password
    mysqlpp::Connection myDB = connectToDatabase();

    // Some necessary variables for the file IO
    std::string queryStr;

    int count = 1;
    // Read file line-by-line
    while ( std::getline(inputFile, queryStr) ) {
        // Create query string from current line
        queryStr = generateLoadQuery(queryStr);

        // Create mysql++ query
        mysqlpp::Query query = myDB.query();

        try {
            // Check query is correct
            query << queryStr; 

            // Execute Query
            query.parse();

            std::cout <<  "Data Line " + 
            std::to_string(count) + " Loaded" << std::endl;   
            count++;
        } catch (mysqlpp::BadQuery e) {
            std::cerr << "Query: " << queryStr << std::endl;
            std::cerr << "Query is not correct SQL syntax" <<std::endl;
        }
    }
}

void loadTable(std::string& path) {
    // Open file stream
    std::ifstream inputFile(path);    
    
    // Connect to database with: database, server, userID, password
    mysqlpp::Connection myDB = connectToDatabase();

    // Some necessary variables for the file IO
    std::string queryStr;
    std::string tableName;

    int count = 1;
    // Read file line-by-line
    while ( std::getline(inputFile, queryStr) ) {
        // Create query string from current line
        tie(queryStr, tableName) = generateTableQuery(queryStr);

        // Create mysql++ query
        mysqlpp::Query query = myDB.query();

        try {
            query << queryStr; 

            // Check query is correct
            query.parse();

            // Execute Query
            mysqlpp::StoreQueryResult result = query.store();

            std::cout << "Table " << tableName << " Created" << std::endl;
        } catch (mysqlpp::BadQuery e) {
            std::cerr << "Query: " << queryStr << std::endl;
            std::cerr << "Query is not correct SQL syntax" <<std::endl;
        }
    }
}

void dropTable(std::string tableName) {

    // Connect to database with: database, server, userID, password
    mysqlpp::Connection myDB = connectToDatabase();

    mysqlpp::Query query = myDB.query();

    query << "DROP TABLE " << tableName << ";";

    query.parse();

    mysqlpp::StoreQueryResult result = query.store();
    
    std::cout << "Table: " << tableName << " Dropped" << std::endl;
}

int main(int argc, char *argv[]) {
    // Ensure arguments are specified
    if (argc < 2) {
        std::cerr << "Not enough arguments";
        return 1;
    }

    std::string option = argv[1];

    if (option == "-I") {
        interactive();
    } else if (option == "-L" && argc >= 3) {
        std::string arg1 = argv[2];
        loadData(arg1);
    } else if (option == "-C") {
        std::string arg1 = argv[2];
        loadTable(arg1);
    } else if (option == "-W" && argc >= 3) {
        std::string arg1 = argv[2];
        std::string outputPath = argv[3];
        loadFile(arg1, outputPath);
    } else if (option == "-D") {
        std::string arg1 = argv[2];
        dropTable(arg1); 
    } else {
        std::cerr << "Invalid input" << std::endl;
        return 1;
    }
    // All done
    return 0; 
}

