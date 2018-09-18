#include "../fbx.hpp"
#include <iostream>

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

int main()
{
    Fbx::Record file;
    auto versionCheck = [](std::string magic, uint32_t version)
    {
        if (magic != "Kaydara FBX Binary  ")
        {
            throw std::runtime_error("Invalid magic string.");
        }
        if (version < 7100)
        {
            throw std::runtime_error("I'm not interested in version less than 7100.");
        }
    };

    file.read("../models/blender-default.fbx", versionCheck);
    //printRecord(&file);
    file.write("../bin/out-model.fbx");

    return 0;
}
