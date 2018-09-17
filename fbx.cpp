/*
* MIT License
*
* Copyright(c) 2018 Jimmie Bergmann
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files(the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions :
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
*/

#include "fbx.hpp"
#include <memory>
#include <limits>
#include <stack>
#include <sstream>
#include <fstream>
#include "miniz.h"

namespace Fbx
{

    // Helper class for reading properties.
    namespace
    {
        class PropertyReader
        {

        public:

            PropertyReader(std::ifstream & file, Record * record) :
                m_file(file),
                m_pRecord(record)
            {}

            size_t readPrimitive(uint8_t code) const
            {
                switch (code)
                {
                case 'C': return readPrimitiveValue<bool>();
                case 'Y': return readPrimitiveValue<int16_t>();
                case 'I': return readPrimitiveValue<int32_t>();
                case 'L': return readPrimitiveValue<int64_t>();
                case 'F': return readPrimitiveValue<float>();
                case 'D': return readPrimitiveValue<double>();
                default: throw std::runtime_error("Unkown primitive property type:" + code); break;
                }
                return 0;
            }

            size_t readArray(uint8_t code) const
            {
                uint32_t arrayLength;
                uint32_t encoding;
                uint32_t compressedLength;
                m_file.read(reinterpret_cast<char*>(&arrayLength), 4);
                m_file.read(reinterpret_cast<char*>(&encoding), 4);
                m_file.read(reinterpret_cast<char*>(&compressedLength), 4);

                if (encoding == 0)
                {
                    switch (code)
                    {
                    case 'i': return readUncompressedArray<int32_t>(arrayLength);
                    case 'f': return readUncompressedArray<float>(arrayLength);
                    case 'd': return readUncompressedArray<double>(arrayLength);
                    case 'l': return readUncompressedArray<int64_t>(arrayLength);
                    case 'b': return readUncompressedArray<bool>(arrayLength);
                    default: throw std::runtime_error("Unkown array property type:" + code); break;
                    }
                }
                if (encoding == 1)
                {
                    switch (code)
                    {
                    case 'i': return readCompressedArray<int32_t>(arrayLength, compressedLength);
                    case 'f': return readCompressedArray<float>(arrayLength, compressedLength);
                    case 'd': return readCompressedArray<double>(arrayLength, compressedLength);
                    case 'l': return readCompressedArray<int64_t>(arrayLength, compressedLength);
                    case 'b': return readCompressedArray<bool>(arrayLength, compressedLength);
                    default: throw std::runtime_error("Unkown array property type:" + code); break;
                    }
                }
                else
                {
                    throw std::runtime_error("Unkown property encoding:" + std::to_string(encoding));
                }

                return 0;
            }

            size_t readRaw(uint8_t code) const
            {
                uint32_t size;
                m_file.read(reinterpret_cast<char*>(&size), 4);
                if (size == 0)
                {
                    if (code == 'S')
                    {
                        m_pRecord->properties().insert(new Property(std::string("")));
                    }
                    return 4;
                }

                uint8_t * pData = new uint8_t[size];
                m_file.read(reinterpret_cast<char*>(pData), size);

                switch (code)
                {
                case 'S': m_pRecord->properties().insert(new Property(std::string(pData, pData + size))); break;
                case 'R': m_pRecord->properties().insert(new Property(pData, size)); break;
                default: throw std::runtime_error("Unkown raw property type:" + code); break;
                }

                delete[] pData;
                return size + 4;
            }

        private:

            template<typename T>
            size_t readPrimitiveValue() const
            {
                T val;
                m_file.read(reinterpret_cast<char*>(&val), sizeof(T));
                m_pRecord->properties().insert(new Property(val));
                return sizeof(T);
            }

            template<typename T>
            size_t readUncompressedArray(uint32_t arrayLength) const
            {
                size_t size = arrayLength * sizeof(T);
                T * pData = new T[size];
                m_file.read(reinterpret_cast<char*>(pData), size);
                m_pRecord->properties().insert(new Property(pData, arrayLength));

                delete[] pData;
                return size + 12;
            }

            template<typename T>
            size_t readCompressedArray(uint32_t arrayLength, uint32_t compressedLength) const
            {
                mz_ulong uncompressedLength = arrayLength * sizeof(T);
                std::unique_ptr<T> pArray(new T[arrayLength]);
                std::unique_ptr<unsigned char> pCmpData(new unsigned char[compressedLength]);

                m_file.read(reinterpret_cast<char*>(pCmpData.get()), compressedLength);
                if (uncompress(reinterpret_cast<unsigned char*>(pArray.get()), &uncompressedLength, pCmpData.get(), compressedLength) != Z_OK)
                {
                    throw std::runtime_error("Failed to uncompress array of record: " + m_pRecord->name());
                }

                m_pRecord->properties().insert(new Property(pArray.get(), arrayLength));
                return compressedLength + 12;
            }

            std::ifstream & m_file;
            Record *        m_pRecord;

        };

        template<typename T>
        void writePrimitive(std::vector<uint8_t> & data, T value)
        {
            const uint8_t * pValue = reinterpret_cast<const uint8_t*>(&value);
            data.insert(data.end(), pValue, pValue + sizeof(T));
        }

        void writeRaw(std::vector<uint8_t> & data, const std::vector<uint8_t> & raw)
        {
            const uint32_t size = static_cast<uint32_t>(raw.size());
            const uint8_t * pSize = reinterpret_cast<const uint8_t*>(&size);
            data.insert(data.end(), pSize, pSize + 4);
            std::copy(raw.begin(), raw.end(), std::back_inserter(data));
        }

        template<typename T>
        void writeArray(std::vector<uint8_t> & data, const std::vector<Property::Value> & array)
        {
            uint32_t arrayLength = static_cast<uint32_t>(array.size());
            const uint8_t * pArrayLength = reinterpret_cast<const uint8_t*>(&arrayLength);
            uint32_t compressedLength = arrayLength * sizeof(T);
            const uint8_t * pCompressedLength = reinterpret_cast<const uint8_t*>(&compressedLength);

            data.insert(data.end(), pArrayLength, pArrayLength + 4);
            data.insert(data.end(), 4, 0);
            data.insert(data.end(), pCompressedLength, pCompressedLength + 4);

            for (uint32_t i = 0; i < arrayLength; i++)
            {
                const uint8_t * pValue = reinterpret_cast<const uint8_t*> (&array[i].boolean);
                data.insert(data.end(), pValue, pValue + sizeof(T));
            }
        }

    }


    // Property
    Property::Property(const bool primitive) :
        m_type(Type::Boolean)
    {
        m_primitive.boolean = primitive;
    }

    Property::Property(const int16_t primitive) :
        m_type(Type::Integer16)
    {
        m_primitive.integer16 = primitive;
    }

    Property::Property(const int32_t primitive) :
        m_type(Type::Integer32)
    {
        m_primitive.integer32 = primitive;
    }

    Property::Property(const int64_t primitive) :
        m_type(Type::Integer64)
    {
        m_primitive.integer64 = primitive;
    }

    Property::Property(const float primitive) :
        m_type(Type::Float32)
    {
        m_primitive.float32 = primitive;
    }

    Property::Property(const double primitive) :
        m_type(Type::Float64)
    {
        m_primitive.float64 = primitive;
    }

    Property::Property(const bool * array, const uint32_t count) :
        m_type(Type::BooleanArray)
    {
        for (uint32_t i = 0; i < count; ++i)
        {
            Value value;
            value.boolean = array[i];
            m_array.push_back(value);
        }
    }

    Property::Property(const int32_t * array, const uint32_t count) :
        m_type(Type::Integer32Array)
    {
        for (uint32_t i = 0; i < count; ++i)
        {
            Value value;
            value.integer32 = array[i];
            m_array.push_back(value);
        }
    }

    Property::Property(const int64_t * array, const uint32_t count) :
        m_type(Type::Integer64Array)
    {
        for (uint32_t i = 0; i < count; ++i)
        {
            Value value;
            value.integer64 = array[i];
            m_array.push_back(value);
        }
    }

    Property::Property(float * array, const uint32_t count) :
        m_type(Type::Float32Array)
    {
        for (uint32_t i = 0; i < count; ++i)
        {
            Value value;
            value.float32 = array[i];
            m_array.push_back(value);
        }
    }

    Property::Property(const double * array, const uint32_t count) :
        m_type(Type::Float64Array)
    {
        for (uint32_t i = 0; i < count; ++i)
        {
            Value value;
            value.float64 = array[i];
            m_array.push_back(value);
        }
    }

    Property::Property(const char * p_string) :
        m_type(Type::String),
        m_raw(p_string, p_string + strlen(p_string))
    {
    }
    Property::Property(const std::string & p_string) :
        m_type(Type::String),
        m_raw(p_string.c_str(), p_string.c_str() + p_string.size())
    {
    }

    Property::Property(const uint8_t * p_raw, const uint32_t size) :
        m_type(Type::Raw),
        m_raw(p_raw, p_raw + size)
    {
    }

    Property::Type Property::type() const
    {
        return m_type;
    }

    uint8_t Property::code() const
    {
        /*'C', 'Y', 'I', 'L', 'F', 'D', 'b', 'i', 'l', 'f', 'd', 'S', 'R'*/
        return "CYILFDbilfdSR"[static_cast<size_t>(m_type)];
    }

    uint32_t Property::size() const
    {
        switch (m_type)
        {
            case Type::Boolean: return 1;
            case Type::Integer16: return 2;
            case Type::Integer32:
            case Type::Float32: return 4;
            case Type::Integer64:
            case Type::Float64: return 8;
            case Type::BooleanArray: return static_cast<uint32_t>(m_array.size());
            case Type::Integer32Array:
            case Type::Float32Array: return static_cast<uint32_t>(m_array.size());
            case Type::Integer64Array:
            case Type::Float64Array: return static_cast<uint32_t>(m_array.size());
            case Type::String:
            case Type::Raw: return static_cast<uint32_t>(m_raw.size());
            default: break;
        }

        return 0;
    }

    Property::Value & Property::primitive()
    {
        return m_primitive;
    }
    const Property::Value & Property::primitive() const
    {
        return m_primitive;
    }

    std::vector<Property::Value> & Property::array()
    {
        return m_array;
    }
    const std::vector<Property::Value> & Property::array() const
    {
        return m_array;
    }

    std::string Property::string() const
    {
        switch (m_type)
        {
            case Type::Boolean: return m_primitive.boolean ? "true" : "false";
            case Type::Integer16: return std::to_string(m_primitive.integer16);
            case Type::Integer32: return std::to_string(m_primitive.integer32);
            case Type::Integer64: return std::to_string(m_primitive.integer64);
            case Type::Float32: return std::to_string(m_primitive.float32);
            case Type::Float64: return std::to_string(m_primitive.float64);
            case Type::BooleanArray: return "array(boolean)";
            case Type::Integer32Array: return "array(integer32)";
            case Type::Integer64Array: return "array(integer64)";
            case Type::Float32Array: return "array(float32)";
            case Type::Float64Array: return "array(float64)";
            case Type::String:
            case Type::Raw: return std::string(m_raw.begin(), m_raw.end());
            default: break;
        }

        return "";
    }

    std::vector<uint8_t> & Property::raw()
    {
        return m_raw;
    }
    const std::vector<uint8_t> & Property::raw() const
    {
        return m_raw;
    }

    bool Property::isPrimitive() const
    {
        return m_type >= Type::Boolean && m_type <= Type::Float64;
    }

    bool Property::isArray() const
    {
        return m_type >= Type::BooleanArray && m_type <= Type::Float64Array;
    }

    bool Property::isString() const
    {
        return m_type == Type::String;
    }

    bool Property::isRaw() const
    {
        return m_type == Type::Raw;
    }


    // Property list
    PropertyList::PropertyList()
    {

    }

    PropertyList::~PropertyList()
    {
        for (auto it = m_properties.begin(); it != m_properties.end(); ++it)
        {
            delete *it;
        }
    }

    size_t PropertyList::size() const
    {
        return m_properties.size();
    }

    PropertyList::Iterator PropertyList::insert(Property * p)
    {
        return m_properties.insert(m_properties.end(), p);
    }
    PropertyList::Iterator PropertyList::insert(Iterator position, Property * p)
    {
        return m_properties.insert(position, p);
    }

    PropertyList::Iterator PropertyList::begin()
    {
        return m_properties.begin();
    }
    PropertyList::ConstIterator PropertyList::begin() const
    {
        return m_properties.begin();
    }

    PropertyList::Iterator PropertyList::end()
    {
        return m_properties.end();
    }
    PropertyList::ConstIterator PropertyList::end() const
    {
        return m_properties.end();
    }

    Property * PropertyList::front()
    {
        return m_properties.front();
    }
    const Property * PropertyList::front() const
    {
        return m_properties.front();
    }

    Property * PropertyList::back()
    {
        return m_properties.back();
    }
    const Property * PropertyList::back() const
    {
        return m_properties.back();
    }

    PropertyList::Iterator PropertyList::erase(Property * property)
    {
        return m_properties.end();
    }
    PropertyList::Iterator PropertyList::erase(Iterator position)
    {
        return m_properties.end();
    }

    void PropertyList::clear()
    {
        for (auto it = m_properties.begin(); it != m_properties.end(); ++it)
        {
            delete *it;
        }
        m_properties.clear();
    }

    PropertyList::PropertyList(PropertyList & pl)
    {
    }


    // Record class.
    Record::Record() :
        m_name(""),
        m_pParent(nullptr)
    {
    }

    Record::Record(const std::string & p_name) :
        Record()
    {
        name(p_name);
    }

    Record::Record(const std::string & name, Record * parent) :
        Record(name)
    {
        if (parent != nullptr)
        {
            parent->insert(this);
        }
    }

    Record::~Record()
    {
        for (auto it = m_nestedList.begin(); it != m_nestedList.end(); ++it)
        {
            delete *it;
        }
    }

    void Record::read(const std::string & filename)
    {
        read(filename, [](std::string, uint32_t) {});
    }

    void Record::read(const std::string & filename, std::function<void(std::string, uint32_t)> onHeaderRead)
    {
        std::ifstream file(filename, std::ios::binary);
        if (file.is_open() == false)
        {
            throw std::runtime_error("Failed to open file.");
        }

        // Get file size.
        file.seekg(0, std::ios::end);
        std::streampos streamPos = file.tellg();
        if (streamPos > static_cast<std::streampos>(std::numeric_limits<uint32_t>::max()))
        {
            throw std::runtime_error("Input file size is too big.");
        }
        const std::size_t fileSize = static_cast<std::size_t>(streamPos);
        file.seekg(0, std::ios::beg);

        // Read header.
        std::string magic(20, '\0');
        file.read(&magic[0], 20);
        file.seekg(23);
        uint32_t version;
        file.read(reinterpret_cast<char*>(&version), 4);

        // call on header function.
        onHeaderRead(magic, version);

        if (file.eof())
        {
            throw std::runtime_error("Invalid FBX file.");
        }

        // Read record.
        std::stack<std::pair<Record *, size_t>> recordStack;
        recordStack.push(std::make_pair(this, fileSize));

        while (recordStack.size())
        {
            uint32_t endOffset = 0;
            uint32_t numProperties = 0;
            uint32_t propertyListLen = 0;
            uint8_t nameLen = 0;
            const size_t recordPos = static_cast<size_t>(file.tellg());

            file.read(reinterpret_cast<char*>(&endOffset), 4);
            file.read(reinterpret_cast<char*>(&numProperties), 4);
            file.read(reinterpret_cast<char*>(&propertyListLen), 4);
            file.read(reinterpret_cast<char*>(&nameLen), 1);

            std::string name(nameLen, '\0');
            file.read(&name[0], nameLen);

            if (file.eof())
            {
                throw std::runtime_error("Invalid record header.");
            }

            if (endOffset == 0) // End of nested list.
            {
                recordStack.pop();
                continue;
            }

            const auto & stackTop = recordStack.top();
            Record * pParentRecord = stackTop.first;
            size_t parentEndOffset = stackTop.second;

            // Validate endoffset.
            if (endOffset > fileSize)
            {
                throw std::runtime_error("Record end offset exceeding file size.");
            }
            if (endOffset >= parentEndOffset)
            {
                throw std::runtime_error("Record end offset exceeding parent record.");
            }

            // Validate property list length.
            if (recordPos + propertyListLen > endOffset)
            {
                throw std::runtime_error("Invalid record property list length.");
            }

            // Create and add new record.
            Record * pNewRecord = new Record(name, pParentRecord);
            recordStack.push(std::make_pair(pNewRecord, endOffset));

            // Read properties.
            size_t propertiesByteRead = 0;
            PropertyReader reader(file, pNewRecord);

            for (uint32_t i = 0; i < numProperties; ++i)
            {
                uint8_t code;
                file.read(reinterpret_cast<char*>(&code), 1);

                if (code == 'S' || code == 'R') // String/raw.
                {
                    propertiesByteRead += reader.readRaw(code) + 1;
                }
                else if (code < 'Z') // primitives.
                {
                    propertiesByteRead += reader.readPrimitive(code) + 1;
                }
                else // arrays.
                {
                    propertiesByteRead += reader.readArray(code) + 1;
                }
            }

            // Make sure all property bytes are extracted.
            if (propertiesByteRead != propertyListLen)
            {
                throw std::runtime_error("Invalid property list length of record: " + pParentRecord->name());
            }

            // Error check end record of nested list.
            const size_t curFilePos = static_cast<size_t>(file.tellg());
            if (parentEndOffset <= curFilePos)
            {
                throw std::runtime_error("Missing nested list end of record: " + pParentRecord->name());
            }

            // Exit record if no nested list is present.
            if (endOffset == curFilePos)
            {
                recordStack.pop();
                continue;
            }

            // Read nested list next loop.
        }
    }

    void Record::write(const std::string & filename) const
    {
        write(filename, 7100);
    }

    void Record::write(const std::string & filename, const uint32_t version) const
    {
        std::ofstream file(filename, std::ios::binary);
        if (file.is_open() == false)
        {
            throw std::runtime_error("Failed to open file.");
        }

        std::vector<uint8_t> data;

        // Write FBX header.
        const uint8_t magic[21] = "Kaydara FBX Binary  ";
        const uint8_t * pVersion = reinterpret_cast<const uint8_t*>(&version);

        data.insert(data.end(), magic, magic + 21);
        data.push_back(0x1A);
        data.push_back(0);
        data.insert(data.end(), pVersion, pVersion + 4);

        std::stack<std::tuple<ConstIterator, ConstIterator, uint32_t>> stack;
        if (m_nestedList.size())
        {
            stack.push(std::make_tuple(m_nestedList.begin(), m_nestedList.end(), 0));
        }

        while (stack.size())
        {
            auto & top = stack.top();
            ConstIterator & currentIt = std::get<0>(top);
            ConstIterator & endIt = std::get<1>(top);
            uint32_t & parentStart = std::get<2>(top);

            if (parentStart != 0)
            {
                const uint32_t currentPos = static_cast<uint32_t>(data.size());
                const uint8_t * pCurrentPos = reinterpret_cast<const uint8_t *>(&currentPos);
                data[parentStart] = pCurrentPos[0];
                data[parentStart + 1] = pCurrentPos[1];
                data[parentStart + 2] = pCurrentPos[2];
                data[parentStart + 3] = pCurrentPos[3];
            }

            if (currentIt == endIt)
            {
                data.insert(data.end(), 13, 0);
                stack.pop();

                continue;
            }

            const Record * pRecord = *currentIt;
            parentStart = static_cast<uint32_t>(data.size());
            ++currentIt;

            // Write record header.
            const auto & properties = pRecord->properties();
            const uint32_t numProperties = static_cast<uint32_t>(properties.size());
            const uint8_t * pNumProperties = reinterpret_cast<const uint8_t*>(&numProperties);
            const std::string & name = pRecord->name();
            const uint8_t * pName = reinterpret_cast<const uint8_t*>(&name[0]);
            const uint8_t nameLength = static_cast<uint8_t>(name.size());
            data.insert(data.end(), 4, 0);
            data.insert(data.end(), pNumProperties, pNumProperties + 4);
            const uint32_t propertiesOffset = static_cast<uint32_t>(data.size());
            data.insert(data.end(), 4, 0);
            data.push_back(nameLength);
            data.insert(data.end(), pName, pName + nameLength);

            // Write record properties.
            const uint32_t propertyStart = static_cast<uint32_t>(data.size());
            for (auto pIt = properties.begin(); pIt != properties.end(); ++pIt)
            {
                Property * pProperty = *pIt;
                data.push_back(pProperty->code());

                switch (pProperty->code())
                {
                case 'I': writePrimitive(data, pProperty->primitive().integer32); break;
                case 'L': writePrimitive(data, pProperty->primitive().integer64); break;
                case 'F': writePrimitive(data, pProperty->primitive().float32); break;
                case 'D': writePrimitive(data, pProperty->primitive().float64); break;
                case 'C': writePrimitive(data, pProperty->primitive().boolean); break;
                case 'Y': writePrimitive(data, pProperty->primitive().integer16); break;
                case 'i': writeArray<int32_t>(data, pProperty->array()); break;
                case 'l': writeArray<int64_t>(data, pProperty->array()); break;
                case 'f': writeArray<float>(data, pProperty->array()); break;
                case 'd': writeArray<double>(data, pProperty->array()); break;
                case 'b': writeArray<bool>(data, pProperty->array()); break;
                case 'R':
                case 'S': writeRaw(data, pProperty->raw()); break;
                default: break;
                }
            }

            // Set properties length
            const uint32_t propertiesEnd = static_cast<uint32_t>(data.size());
            const uint32_t propertiesLength = propertiesEnd - propertyStart;
            const uint8_t * pPropertiesLength = reinterpret_cast<const uint8_t*>(&propertiesLength);
            data[propertiesOffset] = pPropertiesLength[0];
            data[propertiesOffset + 1] = pPropertiesLength[1];
            data[propertiesOffset + 2] = pPropertiesLength[2];
            data[propertiesOffset + 3] = pPropertiesLength[3];

            // Add nested list.
            if (pRecord->size())
            {
                stack.push(std::make_tuple(pRecord->begin(), pRecord->end(), 0));
            }
            
        }

        file.write(reinterpret_cast<const char*>(&data[0]), data.size());
    }


    const std::string & Record::name() const
    {
        return m_name;
    }

    void Record::name(const std::string & name)
    {
        if (name.size() > 255)
        {
            throw std::runtime_error("Exceeded record name length limit: " + std::to_string(name.size()));
        }
        m_name = name;   
    }

    Record * Record::parent()
    {
        return m_pParent;
    }
    const Record * Record::parent() const
    {
        return m_pParent;
    }

    void Record::parent(Record * parent)
    {
        if (parent == m_pParent)
        {
            return;
        }
        if (parent == this)
        {
            throw std::runtime_error("parent == this.");
        }
        if (parent != nullptr)
        {
            parent->insert(this);
        }
    }

    PropertyList & Record::properties()
    {
        return m_properties;
    }
    const PropertyList & Record::properties() const
    {
        return m_properties;
    }

    size_t Record::size() const
    {
        return m_nestedList.size();
    }

    Record::Iterator Record::insert(Record * record)
    {
        Record * pParent = record->parent();
        if (pParent)
        {
            pParent->erase(record);
        }

        record->m_pParent = this;
        return m_nestedList.insert(m_nestedList.end(), record);
    }

    Record::Iterator Record::insert(Iterator position, Record * record)
    {
        Record * pParent = record->parent();
        if (pParent)
        {
            pParent->erase(record);
        }

        record->m_pParent = this;
        return m_nestedList.insert(position, record);
    }

    Record::Iterator Record::begin()
    {
        return m_nestedList.begin();
    }
    Record::ConstIterator Record::begin() const
    {
        return m_nestedList.begin();
    }

    Record::Iterator Record::end()
    {
        return m_nestedList.end();
    }
    Record::ConstIterator Record::end() const
    {
        return m_nestedList.end();
    }

    Record * Record::front()
    {
        return m_nestedList.front();
    }
    const Record * Record::front() const
    {
        return m_nestedList.front();
    }

    Record * Record::back()
    {
        return m_nestedList.back();
    }
    const Record * Record::back() const
    {
        return m_nestedList.back();
    }

    Record::Iterator Record::find(const std::string & p_name)
    {
        for (auto it = m_nestedList.begin(); it != m_nestedList.end(); it++)
        {
            if ((*it)->name() == p_name)
            {
                return it;
            }
        }
        return m_nestedList.end();
    }
    Record::ConstIterator Record::find(const std::string & p_name) const
    {
        for (auto it = m_nestedList.begin(); it != m_nestedList.end(); it++)
        {
            if ((*it)->name() == p_name)
            {
                return it;
            }
        }
        return m_nestedList.end();
    }

    Record::Iterator Record::erase(Record * record)
    {
        return m_nestedList.end();
    }

    Record::Iterator Record::erase(Iterator position)
    {
        return m_nestedList.end();
    }

    void Record::clear()
    {
        for (auto it = m_nestedList.begin(); it != m_nestedList.end(); ++it)
        {
            delete *it;
        }
        m_nestedList.clear();
    }

    Record::Record(const Record & record)
    {
    }

}
