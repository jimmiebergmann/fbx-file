#include "../fbx.hpp"
#include <iostream>
#include <functional>

void printRecords(Fbx::RecordList & records, size_t level = 0)
{
    for (Fbx::Record * rec = records.front(); rec != nullptr; rec = rec->next())
    {
        std::cout << std::string(level * 3, ' ') << "Rec: " << rec->name() << std::endl;

        for (size_t i = 0; i < rec->propertyCount(); i++)
        {
            Fbx::Property * prop = rec->property(i);
            std::cout << std::string((level + 1) * 3, ' ') << "Prop - " << prop->typeString() << ": " << prop->asString() << std::endl;
        }

        Fbx::RecordList * pChildList = rec->childList();
        if (pChildList)
        {
            printRecords(*pChildList, level + 1);
        }
    }
}

int main()
{

    Fbx::RecordList file;
    file.read("../models/blender-default.fbx");

    printRecords(file);

    Fbx::Record * objects = file.find("Objects");
    if (objects && objects->childList())
    {
        Fbx::Record * geometry = objects->childList()->find("Geometry");

        if (geometry && geometry->childList())
        {
            Fbx::Record * vertices = geometry->childList()->find("Vertices");
            if (vertices && vertices->propertyCount())
            {
                Fbx::Property * vertexArray = vertices->property(0);
                if (vertexArray->type() == Fbx::Property::Type::Float64Array &&
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