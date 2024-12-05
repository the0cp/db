#include <iostream>
#include <cstdlib>
#include <string>
#include <fstream>
#include <filesystem>
#include <map>
#include <chrono> 

#include "text_table.h"
#include "colors.h"
#include "bplus.h"



void printHelp();
void printOption();
bool isDbExist(const char *file);
void optionLoop();
void listDb();
void newDb(const std::string& arg);
void openDb(const std::string& arg);
void resetDb(const std::string& arg);
void dropDb(const std::string& arg);
void printTable(int *index, bplus::value_t *values);
void commandLoop(const std::string& dbName);
void insertRec(const std::string& arg);
void deleteRec(const std::string& arg);
void selectRec(const std::string& arg);
void updateRec(const std::string& arg);

void intToKeyT(bplus::key_t *a, int *b);


