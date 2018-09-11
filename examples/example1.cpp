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

            switch (prop->type())
            {
            case fbx::Property::Type::Boolean:
                std::cout << std::string((level + 1) * 3, ' ') << "Prop bool: " << prop->asBoolean() << std::endl;
                break;
            case fbx::Property::Type::Integer16:
                std::cout << std::string((level + 1) * 3, ' ') << "Prop i16: " << prop->asInteger16() << std::endl;
                break;
            case fbx::Property::Type::Integer32:
                std::cout << std::string((level + 1) * 3, ' ') << "Prop i32: " << prop->asInteger32() << std::endl;
                break;
            case fbx::Property::Type::Integer64:
                std::cout << std::string((level + 1) * 3, ' ') << "Prop i64: " << prop->asInteger64() << std::endl;
                break;
            case fbx::Property::Type::Float32:
                std::cout << std::string((level + 1) * 3, ' ') << "Prop f32: " << prop->asFloat32() << std::endl;
                break;
            case fbx::Property::Type::Float64:
                std::cout << std::string((level + 1) * 3, ' ') << "Prop f64: " << prop->asFloat64() << std::endl;
                break;
            case fbx::Property::Type::Integer32Array:
                std::cout << std::string((level + 1) * 3, ' ') << "Prop int32 Array:" << prop->size() << std::endl;
                break;
            case fbx::Property::Type::BooleanArray:
                std::cout << std::string((level + 1) * 3, ' ') << "Prop bool Array:" << prop->size() << std::endl;
                break;
            case fbx::Property::Type::Integer64Array:
                std::cout << std::string((level + 1) * 3, ' ') << "Prop int64 Array:" << prop->size() << std::endl;
                break;
            case fbx::Property::Type::Float32Array:
                std::cout << std::string((level + 1) * 3, ' ') << "Prop float32 Array:" << prop->size() << std::endl;
                break;
            case fbx::Property::Type::Float64Array:
                std::cout << std::string((level + 1) * 3, ' ') << "Prop float64 Array:" << prop->size() << std::endl;
                break;
            case fbx::Property::Type::String:
                std::cout << std::string((level + 1) * 3, ' ') << "Prop s: " << prop->asString() << std::endl;
                break;
            case fbx::Property::Type::Raw:
                std::cout << std::string((level + 1) * 3, ' ') << "Prop Raw: " << prop->asString() << std::endl;
                break;
            default:
                break;
            }

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