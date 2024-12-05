#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main() {
    const int num_inserts = 100000;
    const char* pipe_name = "pipe";

    mkfifo(pipe_name,0666);

    std::ofstream logFile("time.log");

    std::streambuf *coutbuf = std::cout.rdbuf();
    std::cout.rdbuf(logFile.rdbuf());


    std::thread easydb_thread([pipe_name]() {
        std::system("./easydb < pipe &");
    });

    int fd = open(pipe_name, O_WRONLY);

    auto start_time = std::chrono::high_resolution_clock::now();
    auto start_duration = start_time.time_since_epoch();
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(start_duration).count();
    std::cout << "[" << milliseconds << "]: ";
    std::cout << "Start insert test" << std::endl;

    std::string command;
    command = "use db\n";
    
    write(fd, command.c_str(), command.length());
    std::this_thread::sleep_for(std::chrono::seconds(2));
    for (int i = 1; i <= num_inserts; ++i) {
        command = "insert " + std::to_string(i) + " name" + std::to_string(i) + " " + std::to_string(i) + " item" + std::to_string(i) + "@data\n";
        write(fd, command.c_str(), command.length());
    }
    command = "exit\n";
    write(fd, command.c_str(), command.length());
    command = "exit\n";
    write(fd, command.c_str(), command.length());

    auto end_time = std::chrono::high_resolution_clock::now();
    auto elapsed_time = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time);


    auto stop_duration = end_time.time_since_epoch();
    milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(stop_duration).count();
    std::cout << "[" << milliseconds << "]: ";

    std::cout << "Insert " << num_inserts << " records, time: " << elapsed_time.count() << " s" << std::endl;

    close(fd);

    easydb_thread.join();
    unlink(pipe_name);
    std::cout.rdbuf(coutbuf);
    return 0;
}