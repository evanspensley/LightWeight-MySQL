##LightWeight-MySQL
A light-weight interactive program to interface with a MySQL RDMS

Compilation example with g++:
g++ -std=c++14 lw-mysql.cpp -o lw-mysql -lmysqlpp

Available options:
* -I 
 * Interactive mode:
   * Executes user input as a query and displays results
* -L <ins>file</ins>
 * Load a file:
   * Parses csv file and generates and executes queries
   * Example file: load_input.csv
* -C <ins>file</ins>
 * Create a table:
   * Adds new table to the database from an input csv file
   * Example file: create_input.csv
* -D <ins>table name</ins>
 * Drop a table: 
   * Drops specified table from database
* -W <ins>inputFile</ins> <ins>outputFile</ins>
 * Write data to file:
   * Writes query results of an input file to an output file.
   * Example input file: write_input.csv
