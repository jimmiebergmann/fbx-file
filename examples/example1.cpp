#include "../fbx.hpp"
#include <iostream>
#include <functional>

void printRecords(fbx::RecordList & records, size_t level = 0)
{
    for (fbx::Record * rec = records.front(); rec != nullptr; rec = rec->next())
    {
        std::cout << std::string(level * 3, ' ') << "Rec: " << rec->name() << std::endl;

        for (size_t i = 0; i < rec->size(); i++)
        {
            fbx::Property * prop = rec->at(i);
            std::cout << std::string((level + 1) * 3, ' ') << "Prop - " << prop->typeString() << ": " << prop->asString() << std::endl;
        }

        fbx::RecordList * pChildList = rec->childList();
        if (pChildList)
        {
            printRecords(*pChildList, level + 1);
        }
    }
}

int main()
{

    fbx::RecordList file;
    file.read("../models/blender-default.fbx");

    printRecords(file);

    fbx::Record * objects = file.find("Objects");
    if (objects && objects->childList())
    {
        fbx::Record * geometry = objects->childList()->find("Geometry");

        if (geometry && geometry->childList())
        {
            fbx::Record * vertices = geometry->childList()->find("Vertices");
            if (vertices && vertices->size())
            {
                fbx::Property * vertexArray = vertices->at(0);
                if (vertexArray->type() == fbx::Property::Type::Float64Array &&
                    vertexArray->size() % 3 == 0)
                {
                    double * vertexData = vertexArray->asFloat64Array();

                    size_t vCount = vertexArray->size() / 3;
                    for (size_t i = 0; i < vCount; i++)
                    {
                        double x = vertexData[i * 3 + 0];
                        double y = vertexData[i * 3 + 1];
                        double z = vertexData[i * 3 + 2];

                        std::cout << x << ",  " << y << ",  " << z << ",  " << std::endl;
                    }

                }
            }
        }
    }


    return 0;

}