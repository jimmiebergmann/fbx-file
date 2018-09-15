#include "../fbx.hpp"
#include <iostream>
#include <chrono>

void printRecord(const Fbx::Record * record, size_t level = 0)
{
    std::cout << std::string(level * 3, ' ') << "Rec: " << record->name() << std::endl;

    for (auto p : record->properties())
    {
        std::cout << std::string((level + 1) * 3, ' ') << "Prop - " << p->code() << ": " << p->string() << std::endl;
    }
    for (auto r : *record)
    {
        printRecord(r, level + 1);
    }
}

void printFile(const Fbx::File & file)
{
    for(auto r : file)
    {
        printRecord(r);
    }
}

int main()
{
    Fbx::File file;

    auto t_start = std::chrono::high_resolution_clock::now();
        
    file.read("../models/blender-default.fbx");

    auto t_now = std::chrono::high_resolution_clock::now();
    std::chrono::milliseconds elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(t_now - t_start);
    std::cout << "Read time: " << elapsed.count() << "ms." << std::endl;

   // file.write("../bin/out-model.fbx");

    printFile(file);

    return 0;
}