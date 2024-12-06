#include "include/main.h"

using namespace std;
using namespace bplus;

using CommandHandler = void(*)(const string& arg);
using OptionHandler = void(*)(const string& arg);

BPlusTree *db_ptr;

map<string, OptionHandler> optionMap = {
    {"", [](const string&){}},
    {"exit", [](const string&){exit(0);}},
    {"clear", [](const string&){system(TERM_CLEAR);}},
    {"help", [](const string&){printOption();}},
    {"new", [](const string& arg){newDb(arg);}},
    {"list", [](const string& arg){listDb();}},
    {"use", [](const string& arg){openDb(arg);}},
    {"drop", [](const string& arg){dropDb(arg);}},
    {"reset", [](const string& arg){resetDb(arg);}},
};

map<string, CommandHandler> commandMap = {
    {"", [](const string&){}},
    {"clear", [](const string&){system(TERM_CLEAR);}},
    {"help", [](const string&){printHelp();}},
    {"insert", [](const string& arg){insertRec(arg);}},
    {"delete", [](const string& arg){deleteRec(arg);}},
    {"select", [](const string& arg){selectRec(arg);}},
    {"update", [](const string& arg){updateRec(arg);}},
};

void printOption(){
    cout<<R"(
    +-----------------------------------------------------------------------------+
    | )"<< TERM_CYAN << "help " << TERM_RESET <<       R"(                                                        show help menu |
    | )"<< TERM_CYAN << "exit " << TERM_RESET <<       R"(                                                          exit console |
    | )"<< TERM_CYAN << "clear " << TERM_RESET <<       R"(                                                         clear screen |
    | )"<< TERM_CYAN << "list " << TERM_RESET <<       R"(                                               list available database |
    | )"<< TERM_CYAN << "new " << TERM_RESET << "{name}" << R"(                                            create a new database |
    | )"<< TERM_CYAN << "use " << TERM_RESET << "{name}" << R"(                                                login to database |
    | )"<< TERM_CYAN << "reset " << TERM_RESET << "{name}" << R"(                                                 reset database |
    | )"<< TERM_CYAN << "drop " << TERM_RESET << "{name}" << R"(                                                   drop database |
    +-----------------------------------------------------------------------------+               
    )" << endl;
}

void printHelp(){
    cout<<R"(
    +-----------------------------------------------------------------------------+
    | )"<< TERM_CYAN << "help " << TERM_RESET <<       R"(                                                        show help menu |
    | )"<< TERM_CYAN << "exit " << TERM_RESET <<       R"(                                                         exit database |
    | )"<< TERM_CYAN << "select " << TERM_RESET << "id {id}" << R"(                                              search by index |
    | )"<< TERM_CYAN << "select " << TERM_RESET << "in {b},{e}" << R"(                                  search in range of (b,e) |
    | )"<< TERM_CYAN << "insert " << TERM_RESET << "{id} {name} {value} {data}" << R"(                           insert a record |
    | )"<< TERM_CYAN << "update " << TERM_RESET << "{id} {name} {value} {data}" << R"(                           update a record |
    | )"<< TERM_CYAN << "delete " << TERM_RESET << "id {id}" << R"(                                                 delete by id |
    +-----------------------------------------------------------------------------+               
    )" << endl;
}

void printTable(int *index, value_t *values){
    TextTable table('-', '|', '+');
    table.add(" id ");
    table.add(" name ");
    table.add(" value ");
    table.add(" data ");
    table.endRow();

    table.add(to_string(*index));
    table.add(values -> name);
    table.add(to_string(values -> value));
    table.add(values -> data);
    table.endRow();

    cout << table << endl;
}

void optionLoop(){
    string userCmd;
    while(true){
        cout << endl << TERM_GREEN << "(" << TERM_CYAN << "#" <<TERM_GREEN << ") > " << TERM_RESET;
        getline(cin, userCmd);
        size_t pos = userCmd.find(' ');
        string cmd = userCmd.substr(0, pos);
        string arg = userCmd.substr(pos + 1);
        auto it = optionMap.find(cmd);
        if(it != optionMap.end()){
            it -> second(arg);
        }else{
            cout << TERM_RED << "Error: " \
            << TERM_RESET << "INVALID COMMAND" \
            << endl;
        }
    }
}

void commandLoop(const string& dbName){
    string userCmd;
    while(true){
        cout << endl << TERM_GREEN << "(" << TERM_CYAN <<"@" << dbName << TERM_GREEN << ")" << TERM_GREEN << " > " << TERM_RESET;
        getline(cin, userCmd);
        size_t pos = userCmd.find(' ');
        string cmd = userCmd.substr(0, pos);
        string arg = userCmd.substr(pos + 1);
        if(cmd == "exit"){
            break;
        }
        auto it = commandMap.find(cmd);
        if(it != commandMap.end()){
            it -> second(arg);
        } else{
            cout << TERM_RED << "Error: " \
            << TERM_RESET << "INVALID COMMAND" \
            << endl;
        }
    }
    cout << endl << TERM_GREEN << "Database closed" << endl;
}

bool isDbExist(const char *file){
    ifstream ifile(file);
    return ifile.good();
}

void listDb(){
    cout << endl;
    for(const auto& entry : filesystem::directory_iterator("./database/")){
        if(entry.is_regular_file() && entry.path().extension() == ".bin"){
            cout << TERM_BLUE << "  <" << entry.path().stem().string() << ">  ";
        }
    }
    cout << TERM_RESET << endl;
}

void newDb(const string& arg){
    string spath = "./database/" + arg + ".bin";
    if(isDbExist(spath.c_str())){
        cout << endl<< TERM_RED << "Error: " \
        << TERM_RESET << "Database " \
        << TERM_BLUE << spath \
        << TERM_RESET << " exists" \
        << endl; 
    }else{
        BPlusTree db(spath.c_str(), true);
        cout << endl << TERM_GREEN << "Succeed: " << TERM_RESET << "Database created" << endl;
    }

}

void openDb(const string& arg){
    string spath = "./database/" + arg + ".bin";
    if(!isDbExist(spath.c_str())){
        cout << endl << TERM_RED << "Error: " \
        << TERM_RESET << "Database " \
        << TERM_BLUE << spath \
        << TERM_RESET << " does not exist" \
        << endl; 
    }else{
        BPlusTree db(spath.c_str(), false);
        db_ptr = &db;
        cout << endl << TERM_GREEN << "Succeed: " << TERM_RESET << "Database initialized" << endl;
        commandLoop(arg);
    }
}

void resetDb(const string& arg){
    string spath = "./database/" + arg + ".bin";
    if(!isDbExist(spath.c_str())){
        cout << endl << TERM_RED << "Error: " \
        << TERM_RESET << "Database " \
        << TERM_BLUE << spath \
        << TERM_RESET << " does not exist" \
        << endl; 
    }else{
        BPlusTree db(spath.c_str(), true);
        cout << endl << TERM_GREEN << "Succeed: " << TERM_RESET << "Database reset" << endl;
    }
}

void dropDb(const string& arg){
    string spath = "./database/" + arg + ".bin";
    if(!isDbExist(spath.c_str())){
        cout << endl << TERM_RED << "Error: " \
        << TERM_RESET << "Database " \
        << TERM_BLUE << spath \
        << TERM_RESET << " does not exist" \
        << endl;
        return;
    }
    if(isDbExist(spath.c_str()) && filesystem::remove(spath.c_str()) == true){
        cout << endl << TERM_GREEN << "Succeed: " \
        << TERM_RESET << "Database " \
        << TERM_BLUE << arg \
        << TERM_RESET << " dropped" \
        << endl;
    }else{
        cout << endl << TERM_RED << "Error: " \
        << TERM_RESET << "Database " \
        << TERM_BLUE << spath \
        << TERM_RESET << " cannot be dropped" \
        << endl;
    }
}

void insertRec(const string& arg){
    istringstream argStream(arg);
    int *keyIndex = new int;
    value_t *insertData = new value_t;

    if(argStream >> *keyIndex >> insertData->name >> insertData->value >> insertData->data){
        bplus::key_t key;
        intToKeyT(&key, keyIndex);
        cout<<*keyIndex<<endl;
        auto start = chrono::high_resolution_clock::now();   
        // insert record 
        int returnCode = (*db_ptr).insert(key, *insertData);
        // clock stop
        auto end = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<std::chrono::microseconds>(end - start);
        if(returnCode == 0){
            cout << endl << TERM_GREEN << "Succeed: " \
            << TERM_RESET << "Insert index " \
            << TERM_BLUE << *keyIndex \
            << TERM_RESET << " took " \
            << duration.count() << " microseconds" \
            << endl;
        }else if(returnCode == 1){
            cout << endl << TERM_YELLOW << "Failed: " \
            << TERM_RESET << "Index " \
            << TERM_BLUE << *keyIndex \
            << TERM_RESET << " already existed" \
            << endl;
        }else{
            cout << endl << TERM_RED << "Error: " \
            << TERM_RESET << "Insert failed" \
            << endl;
        }
        

    }else{
        cout << endl << TERM_RED << "ERROR: " \
        << TERM_RESET << "Invalid Arguments, use " \
        << TERM_CYAN << "\"help\" " \
        << TERM_RESET << "for more infomation"\
        <<endl;
    }
}

void deleteRec(const string& arg){
    int *keyIndex = new int;
    string token;
    istringstream argStream(arg);
    if(argStream >> token && token == "id" &&\
        argStream >> *keyIndex){
        bplus::key_t key;
        intToKeyT(&key, keyIndex);
        auto start = chrono::high_resolution_clock::now();
        // clock start
        int returnCode = (*db_ptr).remove(key);
        // clock stop
        auto end = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<std::chrono::microseconds>(end - start);
        if(returnCode == 0){
            cout << endl << TERM_GREEN << "Succeed: " \
            << TERM_RESET <<"Delete index " \
            << TERM_BLUE << *keyIndex \
            << TERM_RESET << " took " \
            << duration.count() << " microseconds" \
            << endl;
        }else if(returnCode == -1){
            cout << endl << TERM_YELLOW << "Failed: " \
            << TERM_RESET << "Cannot find index " \
            << TERM_BLUE << *keyIndex \
            << TERM_RESET << " time: " \
            << duration.count() << " microseconds" \
            << endl;
        }
    }else{
        cout << endl << TERM_RED << "ERROR: " \
        << TERM_RESET << "Invalid Index, use " \
        << TERM_CYAN << "\"help\" " \
        << TERM_RESET << "for more infomation"\
        <<endl;
    }
}

void selectRec(const string& arg){
    if(arg.find("id") != string::npos){
        int* keyIndex = new int;
        string token;
        istringstream argStream(arg);
        if(argStream >> token && token == "id" &&\
        argStream >> *keyIndex){
            bplus::key_t key;
            value_t *keyValue = new value_t;
            intToKeyT(&key, keyIndex);

            auto start = chrono::high_resolution_clock::now();
            // select by id
            int returnCode = (*db_ptr).search(key, keyValue);
            // end clock
            auto end = chrono::high_resolution_clock::now();
            auto duration = chrono::duration_cast<std::chrono::microseconds>(end - start);

            if(returnCode == 0){
                printTable(keyIndex, keyValue);
                cout << endl << TERM_GREEN << "Succeed: " \
                << TERM_RESET << "executed search took " \
                << duration.count() << " microseconds" \
                << endl;
            }else{
                cout << endl << TERM_YELLOW << "Failed: " \
                << TERM_RESET << "Index: " \
                << TERM_BLUE << *keyIndex \
                << TERM_RESET << " doesn't exist, time: " \
                << duration.count() << " microseconds" \
                << endl;
            }
        }else{
            cout << endl << TERM_RED << "ERROR: " \
            << TERM_RESET << "Invalid Index, use " \
            << TERM_CYAN << "\"help\" " \
            << TERM_RESET << "for more infomation"\
            <<endl;
        }
    }else if(arg.find("in") != string::npos){
        istringstream argStream(arg);
        char comma;
        string token;
        int i_start, i_end;

        if(argStream >> token && token == "in" &&\
        argStream >> i_start &&\
        argStream >> comma && comma == ',' &&\
        argStream >> i_end){
            TextTable table('-', '|', '+');
            table.add("ID");
            table.add("Name");
            table.add("Value");
            table.add("Data");
            table.endRow();

            bplus::key_t key;
            value_t *keyValue = new value_t;

            auto start = chrono::high_resolution_clock::now();
            //select by range
            for(int i = i_start; i <= i_end; i++){
                intToKeyT(&key, &i);
                int returnCode = (*db_ptr).search(key, keyValue);
                if(returnCode == 0){
                    table.add(to_string(i));
                    table.add(keyValue -> name);
                    table.add(to_string(keyValue -> value));
                    table.add(keyValue -> data);
                    table.endRow();
                }
            }

            
            auto end = chrono::high_resolution_clock::now();
            auto duration = chrono::duration_cast<std::chrono::microseconds>(end - start);

            cout << table << endl;
            cout << "Select took " << duration.count() << " microseconds" << endl;
        }else{
            cout << endl << TERM_RED << "ERROR: " \
            << TERM_RESET << "Invalid Range, use " \
            << TERM_CYAN << "\"help\" " \
            << TERM_RESET << "for more infomation"\
            <<endl;
        }
    }else{
        cout << endl << TERM_RED << "ERROR: " \
        << TERM_RESET << "Invalid Arguments, use " \
        << TERM_CYAN << "\"help\" " \
        << TERM_RESET << "for more infomation"\
        <<endl;
    }
}

void updateRec(const string& arg){
    istringstream argStream(arg);

    int *keyIndex = new int;
    value_t *updateData = new value_t;

    if(argStream >> *keyIndex >> updateData->name >> updateData->value >> updateData->data){
        bplus::key_t key;
        intToKeyT(&key, keyIndex);
        auto start = chrono::high_resolution_clock::now();
        cout<<updateData->name<<updateData->value<<updateData->data<<endl;
        // clock start
        int returnCode = (*db_ptr).update(key, *updateData);
        // clock stop
        auto end = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<std::chrono::microseconds>(end - start);
        if(returnCode == 0){
            cout << endl << TERM_GREEN << "Succeed: " \
            << TERM_RESET << "Update index " \
            << TERM_BLUE << *keyIndex \
            << TERM_RESET << " took " \
            << duration.count() << " microseconds" \
            << endl;
        }else{
            cout << endl << TERM_YELLOW << "Failed: " \
            << TERM_RESET << "Cannot find index " \
            << TERM_BLUE << *keyIndex \
            << TERM_RESET << " time: " \
            << duration.count() << " microseconds" \
            << endl;
        }
    }else{
        cout << endl << TERM_RED << "ERROR: " \
        << TERM_RESET << "Invalid Arguments, use " \
        << TERM_CYAN << "\"help\" " \
        << TERM_RESET << "for more infomation"\
        <<endl;
    }
}

void intToKeyT(bplus::key_t *a, int *b){
    char key[16] = {0};
    snprintf(key, sizeof(key), "%d", *b);
    *a = key;
}

int main(int argc, char *argv[]){
    system(TERM_CLEAR);
    cout<<R"(
                                                  _ _     
                                                 | | |    
                          ___  __ _ ___ _   _  __| | |__  
                         / _ \/ _` / __| | | |/ _` | '_ \
                        |  __/ (_| \__ \ |_| | (_| | |_) |
                         \___|\__,_|___/\__, |\__,_|_.__/ 
                                         __/ |            
                                        |___/             
    +-----------------------------------------------------------------------------+
    |                 easydb - simple b+ database in Modern C++                   |
    |                             author: the0cp                                  |  
    +-----------------------------------------------------------------------------+
    |                               in Console                                    |
    +-----------------------------------------------------------------------------+
    | )"<< TERM_CYAN << "help " << TERM_RESET <<       R"(                                                        show help menu |
    | )"<< TERM_CYAN << "exit " << TERM_RESET <<       R"(                                                          exit console |
    | )"<< TERM_CYAN << "clear " << TERM_RESET <<       R"(                                                         clear screen |
    | )"<< TERM_CYAN << "list " << TERM_RESET <<       R"(                                               list available database |
    | )"<< TERM_CYAN << "new " << TERM_RESET << "{name}" << R"(                                            create a new database |
    | )"<< TERM_CYAN << "use " << TERM_RESET << "{name}" << R"(                                                login to database |
    | )"<< TERM_CYAN << "reset " << TERM_RESET << "{name}" << R"(                                                 reset database |
    | )"<< TERM_CYAN << "drop " << TERM_RESET << "{name}" << R"(                                                   drop database |
    +-----------------------------------------------------------------------------+
    |                               in Database                                   |
    +-----------------------------------------------------------------------------+
    | )"<< TERM_CYAN << "help " << TERM_RESET <<       R"(                                                        show help menu |
    | )"<< TERM_CYAN << "exit " << TERM_RESET <<       R"(                                                         exit database |
    | )"<< TERM_CYAN << "select " << TERM_RESET << "id {id}" << R"(                                              search by index |
    | )"<< TERM_CYAN << "select " << TERM_RESET << "in {b},{e}" << R"(                                  search in range of (b,e) |
    | )"<< TERM_CYAN << "insert " << TERM_RESET << "{id} {name} {value} {data}" << R"(                           insert a record |
    | )"<< TERM_CYAN << "update " << TERM_RESET << "{id} {name} {value} {data}" << R"(                           update a record |
    | )"<< TERM_CYAN << "delete " << TERM_RESET << "id {id}" << R"(                                                 delete by id |
    +-----------------------------------------------------------------------------+               
    )" << endl;
    optionLoop();
    
}
