#include "fbx.hpp"
#include <memory>
#include <stack>
#include <fstream>
#include <sstream>
#include "miniz.h"

namespace Fbx
{

    static void CheckOverflow(const size_t size, size_t & pos, const size_t max);
    template<typename T> static void ToString(std::string & output, const T & in);
    static const std::string g_EmptyString = "";
    static std::string g_TempString = "";
    static const std::string g_TypeStrings[13] =
    {
        "Boolean", "Integer16", "Integer32", "Integer64", "Float32", "Float64", "BooleanArray",
        "Integer32Array", "Integer64Array", "Float32Array", "Float64Array", "String", "Raw"
    };


    // Property
    Property::Type Property::type() const
    {
        return m_type;
    }

    uint32_t Property::size() const
    {
        return m_size;
    }

    Record * Property::record() const
    {
        return m_pRecord;
    }

    bool Property::asBoolean() const
    {
        if (m_type == Type::Boolean)
        {
            return m_value.m_boolean;
        }
        return false;
    }

    int16_t Property::asInteger16() const
    {
        if (m_type == Type::Integer16)
        {
            return m_value.m_integer16;
        }
        return 0;
    }
    int32_t Property::asInteger32() const
    {
        if (m_type == Type::Integer32)
        {
            return m_value.m_integer32;
        }
        return 0;
    }

    int64_t Property::asInteger64() const
    {
        if (m_type == Type::Integer64)
        {
            return m_value.m_integer64;
        }
        return 0;
    }

    float Property::asFloat32() const
    {
        if (m_type == Type::Float32)
        {
            return m_value.m_float32;
        }
        return 0.0f;
    }

    double Property::asFloat64() const
    {
        if (m_type == Type::Float64)
        {
            return m_value.m_float64;
        }
        return 0.0f;
    }

    bool * Property::asBooleanArray() const
    {
        if (m_type == Type::BooleanArray)
        {
            return m_value.m_pBooleanArray;
        }
        return nullptr;
    }

    int32_t * Property::asInteger32Array() const
    {
        if (m_type == Type::Integer32Array)
        {
            return m_value.m_pInteger32Array;
        }
        return nullptr;
    }

    int64_t * Property::asInteger64Array() const
    {
        if (m_type == Type::Integer64Array)
        {
            return m_value.m_pInteger64Array;
        }
        return nullptr;
    }

    float * Property::asFloat32Array() const
    {
        if (m_type == Type::Float32Array)
        {
            return m_value.m_pFloat32Array;
        }
        return nullptr;
    }

    double * Property::asFloat64Array() const
    {
        if (m_type == Type::Float64Array)
        {
            return m_value.m_pFloat64Array;
        }
        return nullptr;
    }

    const std::string & Property::asString() const
    {
        switch (m_type)
        {
        case Type::String:
            return *m_value.m_pString;
        case Type::Raw:
            g_TempString = "Raw"; return g_TempString;
        case Type::Boolean:
            ToString(g_TempString, m_value.m_boolean); return g_TempString;
        case Type::Integer16:
            ToString(g_TempString, m_value.m_integer16); return g_TempString;
        case Type::Integer32:
            ToString(g_TempString, m_value.m_integer32); return g_TempString;
        case Type::Integer64:
            ToString(g_TempString, m_value.m_integer64); return g_TempString;
        case Type::Float32:
            ToString(g_TempString, m_value.m_float32); return g_TempString;
        case Type::Float64:
            ToString(g_TempString, m_value.m_float64); return g_TempString;
        case Type::BooleanArray:
            g_TempString = "Array(Boolean)"; return g_TempString;
        case Type::Integer32Array:
            g_TempString = "Array(Integer32)"; return g_TempString;
        case Type::Integer64Array:
            g_TempString = "Array(Integer64)"; return g_TempString;
        case Type::Float32Array:
            g_TempString = "Array(Float32)"; return g_TempString;
        case Type::Float64Array:
            g_TempString = "Array(Float64)"; return g_TempString;
        default:
            break;
        }

        return g_EmptyString;
    }

    std::string Property::typeString() const
    {
        return g_TypeStrings[static_cast<size_t>(m_type)];
    }

    std::string Property::typeString(const Type type)
    {
        return g_TypeStrings[static_cast<size_t>(type)];
    }

    const uint8_t * Property::asRaw() const
    {
        if (m_type == Type::Raw)
        {
            return m_value.m_pRaw;
        }
        return nullptr;
    }


    Property::Property(Record * record, const bool value) :
        m_type(Type::Boolean),
        m_size(1),
        m_pRecord(record)
    {
        m_value.m_boolean = value;
    }

    Property::Property(Record * record, const int16_t value) :
        m_type(Type::Integer16),
        m_size(2),
        m_pRecord(record)
    {
        m_value.m_integer16 = value;
    }

    Property::Property(Record * record, const int32_t value) :
        m_type(Type::Integer32),
        m_size(4),
        m_pRecord(record)
    {
        m_value.m_integer32 = value;
    }
    
    Property::Property(Record * record, const int64_t value) :
        m_type(Type::Integer64),
        m_size(8),
        m_pRecord(record)
    {
        m_value.m_integer64 = value;
    }

    Property::Property(Record * record, const float value) :
        m_type(Type::Float32),
        m_size(4),
        m_pRecord(record)
    {
        m_value.m_float32 = value;
    }

    Property::Property(Record * record, const double value) :
        m_type(Type::Float64),
        m_size(8),
        m_pRecord(record)
    {
        m_value.m_float64 = value;
    }

    Property::Property(Record * record, const bool * value, const uint32_t size) :
        m_type(Type::BooleanArray),
        m_size(size),
        m_pRecord(record)
    {
        if (size)
        {
            m_value.m_pBooleanArray = new bool[size];
            if (value)
            {
                memcpy(m_value.m_pBooleanArray, value, size);
            }
        }
    }

    Property::Property(Record * record, const int32_t * value, const uint32_t size) :
        m_type(Type::Integer32Array),
        m_size(size),
        m_pRecord(record)
    {
        if (size)
        {
            m_value.m_pInteger32Array = new int32_t[size];
            if (value)
            {
                memcpy(m_value.m_pInteger32Array, value, size * sizeof(int32_t));
            }
        }
    }

    Property::Property(Record * record, const int64_t * value, const uint32_t size) :
        m_type(Type::Integer64Array),
        m_size(size),
        m_pRecord(record)
    {
        if (size)
        {
            m_value.m_pInteger64Array = new int64_t[size];
            if (value)
            {
                memcpy(m_value.m_pInteger64Array, value, size * sizeof(int64_t));
            }
        }
    }

    Property::Property(Record * record, const float * value, const uint32_t size) :
        m_type(Type::Float32Array),
        m_size(size),
        m_pRecord(record)
    {
        if (size)
        {
            m_value.m_pFloat32Array = new float[size];
            if (value)
            {
                memcpy(m_value.m_pFloat32Array, value, size * sizeof(float));
            }
        }
    }

    Property::Property(Record * record, const double * value, const uint32_t size) :
        m_type(Type::Float64Array),
        m_size(size),
        m_pRecord(record)
    {
        if (size)
        {
            m_value.m_pFloat64Array = new double[size];
            if (value)
            {
                memcpy(m_value.m_pFloat64Array, value, size * sizeof(double));
            }
        }
    }

    Property::Property(Record * record, const std::string & value) :
        m_type(Type::String),
        m_size(static_cast<uint32_t>(value.size())),
        m_pRecord(record)
    {
        m_value.m_pString = new std::string;
        *m_value.m_pString = value;
    }

    Property::Property(Record * record, const uint8_t * value, const uint32_t size) :
        m_type(Type::Raw),
        m_size(size),
        m_pRecord(record)
    {
        if (size)
        {
            m_value.m_pRaw = new uint8_t[size];
            if(value)
            {
                memcpy(m_value.m_pRaw, value, size);
            }
        }
    }

    Property::~Property()
    {
        switch (m_type)
        {
        case Type::String:
            delete m_value.m_pString;
            break;
        case Type::BooleanArray:
            if (m_value.m_pBooleanArray) { delete[] m_value.m_pBooleanArray; }
            break;
        case Type::Integer32Array:
            if (m_value.m_pInteger32Array) { delete[] m_value.m_pInteger32Array; }
            break;
        case Type::Integer64Array:
            if (m_value.m_pInteger64Array) { delete[] m_value.m_pInteger64Array; }
            break;
        case Type::Float32Array:
            if (m_value.m_pFloat32Array) { delete[] m_value.m_pFloat32Array; }
            break;
        case Type::Float64Array:
            if (m_value.m_pFloat64Array) { delete[] m_value.m_pFloat64Array; }
            break;
        case Type::Raw:
            if (m_value.m_pRaw) { delete[] m_value.m_pRaw; }
            break;
        default:
            break;
        }
    }



    // Record
    const std::string & Record::name() const
    {
        return m_name;
    }

    void Record::setName(const std::string & name)
    {
        m_pParentList->setRecordName(this, name);
    }

    size_t Record::propertyCount() const
    {
        return m_properties.size();
    }

    Property * Record::property(const size_t index) const
    {
        return m_properties.at(index);
    }

    RecordList * Record::parentList() const
    {
        return m_pParentList;
    }

    RecordList * Record::childList() const
    {
        return m_pChildList;
    }

    Record * Record::prev() const
    {
        return m_pPrevRecord;
    }

    Record * Record::next() const
    {
        return m_pNextRecord;
    }

    Record::Record(RecordList * parent, const std::string & p_name) :
        m_name(""),
        m_pParentList(parent),
        m_pChildList(nullptr),
        m_pPrevRecord(nullptr),
        m_pNextRecord(nullptr)
    {
        if (p_name.size() > 255)
        {
            throw std::runtime_error("Invalid record name.");
        }
        m_name = p_name;
    }

    Record::~Record()
    {
        if (m_pChildList)
        {
            delete m_pChildList;
        }
    }


    RecordList::RecordList()
    {

    }

    RecordList::RecordList(const std::string & filename)
    {
        read(filename);
    }

    RecordList::~RecordList()
    {
        for (auto it = m_recordList.begin(); it != m_recordList.end(); it++)
        {
            delete *it;
        }
    }

    void RecordList::read(const std::string & filename)
    {
        std::ifstream fin(filename, std::ifstream::binary);
        if (fin.is_open() == false)
        {
            throw std::runtime_error("Failed to open file.");
        }

        std::size_t pos = 0;
        fin.seekg(pos, std::ifstream::end);
        const std::size_t fileSize = static_cast<std::size_t>(fin.tellg());
        fin.seekg(pos, std::ifstream::beg);

        // Check if header is correct
        CheckOverflow(27, pos, fileSize);

        std::string magic(20, '\0');
        fin.read(&magic[0], 20);

        fin.seekg(23);
        unsigned int version = 0;
        fin.read(reinterpret_cast<char*>(&version), 4);

        // Create record stack.
        std::stack<RecordList *> listStack;
        listStack.push(this);
        RecordList * curList = this;
        Record * curRecord = nullptr;

        unsigned int endOffset = 0;
        unsigned int numProperties = 0;
        unsigned int propertyListLen = 0;
        unsigned char nameLen = 0;

        unsigned int arrayLength = 0;
        unsigned int encoding = 0;
        unsigned int compressedLength = 0;


        size_t level = 1;

        while (pos < fileSize && fin.eof() == false)
        {
            CheckOverflow(13, pos, fileSize);

            fin.read(reinterpret_cast<char*>(&endOffset), 4);
            if (endOffset == 0)
            {
                //  std::cout << pos << ": Skip" << std::endl;

                level--;
                if (level == 0)
                {
                    break;
                }

                fin.seekg(pos, std::ifstream::beg);

                listStack.pop();
                curList = listStack.top();
                curRecord = curList->m_recordList.back();

                continue; // Done parsing.
            }

            fin.read(reinterpret_cast<char*>(&numProperties), 4);
            fin.read(reinterpret_cast<char*>(&propertyListLen), 4);
            fin.read(reinterpret_cast<char*>(&nameLen), 1);

            CheckOverflow(nameLen, pos, fileSize);
            std::string name(nameLen, '\0');
            fin.read(&name[0], nameLen);

            if (endOffset > fileSize)
            {
                throw std::runtime_error("Incorrect record(" + name + ") end offset: " + std::to_string(endOffset));
            }

            // Add new record to list
            curRecord = curList->pushBack(name);

            //std::cout << pos << ": Rec(" << level << "): " << name << " - " << pos << ", " << endOffset << std::endl;

            // Parse properties
            size_t nesterListPos = pos + propertyListLen;

            for (unsigned int p = 0; p < numProperties; p++)
            {
                char propType = '\0';
                CheckOverflow(1, pos, fileSize);
                fin.read(&propType, 1);

                //std::cout << "  Prop " << p << ": " << propType << std::endl;;

                /*
                i) Primitive Types

                Y : 2 byte signed Integer
                C : 1 bit boolean(1: true, 0 : false) encoded as the LSB of a 1 Byte value.
                I : 4 byte signed Integer
                F : 4 byte single - precision IEEE 754 number
                D : 8 byte double - precision IEEE 754 number
                L : 8 byte signed Integer

                ii) Array types

                f : Array of 4 byte single - precision IEEE 754 number
                d : Array of 8 byte double - precision IEEE 754 number
                l : Array of 8 byte signed Integer
                i : Array of 4 byte signed Integer
                b : Array of 1 byte Booleans(always 0 or 1)

                iii) Special types

                S: String
                R: raw binary data

                */

                bool isArray = false;


                switch (propType)
                {
                case 'C':
                {
                    CheckOverflow(1, pos, fileSize);
                    bool boolean = false;
                    fin.read(reinterpret_cast<char*>(&boolean), 1);
                    curRecord->pushBack(boolean);
                }
                break;
                case 'Y':
                {
                    CheckOverflow(2, pos, fileSize);
                    int16_t integer16 = 0;
                    fin.read(reinterpret_cast<char*>(&integer16), 2);
                    curRecord->pushBack(integer16);
                }
                break;
                case 'I':
                {
                    CheckOverflow(4, pos, fileSize);
                    int32_t integer32 = 0;
                    fin.read(reinterpret_cast<char*>(&integer32), 4);
                    curRecord->pushBack(integer32);
                }
                break;
                case 'L':
                {
                    CheckOverflow(8, pos, fileSize);
                    int64_t integer64 = 0;
                    fin.read(reinterpret_cast<char*>(&integer64), 8);
                    curRecord->pushBack(integer64);
                }
                break;
                case 'F':
                {
                    CheckOverflow(4, pos, fileSize);
                    float float32 = 0;
                    fin.read(reinterpret_cast<char*>(&float32), 4);
                    curRecord->pushBack<float>(float32);
                }
                break;
                case 'D':
                {
                    CheckOverflow(8, pos, fileSize);
                    double float64 = 0;
                    fin.read(reinterpret_cast<char*>(&float64), 8);
                    curRecord->pushBack(float64);
                }
                break;
                case 'S':
                {
                    CheckOverflow(4, pos, fileSize);
                    unsigned int dataSize = 0;
                    fin.read(reinterpret_cast<char*>(&dataSize), 4);

                    if (dataSize)
                    {
                        CheckOverflow(dataSize, pos, fileSize);
                        Property * prop = curRecord->pushBack(std::string(""));
                        std::string * propStr = prop->m_value.m_pString;
                        propStr->resize(dataSize);
                        fin.read(&(*propStr)[0], dataSize);
                        prop->m_size = dataSize;
                    }
                }
                break;
                case 'R':
                {
                    CheckOverflow(4, pos, fileSize);
                    unsigned int dataSize = 0;
                    fin.read(reinterpret_cast<char*>(&dataSize), 4);

                    if (dataSize)
                    {
                        CheckOverflow(dataSize, pos, fileSize);
                        Property * prop = curRecord->pushBack<uint8_t>(nullptr, dataSize);
                        uint8_t * raw = prop->m_value.m_pRaw;
                        fin.read(reinterpret_cast<char*>(raw), dataSize);
                    }
                }
                break;
                case 'b':
                case 'i':
                case 'l':
                case 'f':
                case 'd':
                {
                    CheckOverflow(12, pos, fileSize);
                    fin.read(reinterpret_cast<char*>(&arrayLength), 4);
                    fin.read(reinterpret_cast<char*>(&encoding), 4);
                    fin.read(reinterpret_cast<char*>(&compressedLength), 4);

                    if (encoding != 0 && encoding != 1)
                    {
                        throw std::runtime_error("Unkown encoding, record(" + curRecord->name() + ".");
                    }

                    isArray = true;
                }
                break;
                default:
                    break;
                }

                if (isArray && arrayLength)
                {
                    switch (propType)
                    {
                    case 'b':
                    {
                        Property * prop = curRecord->pushBack<bool>(nullptr, arrayLength);
                        bool * pArray = prop->m_value.m_pBooleanArray;

                        if (encoding == 1)
                        {
                            CheckOverflow(compressedLength, pos, fileSize);

                            mz_ulong uncomp_len = arrayLength * sizeof(bool);
                            unsigned char * pCmp = new unsigned char[uncomp_len];
                            fin.read(reinterpret_cast<char*>(pCmp), uncomp_len);

                            int cmp_status = uncompress(reinterpret_cast<unsigned char*>(pArray), &uncomp_len, pCmp, compressedLength);
                            delete[] pCmp;
                            if (cmp_status != Z_OK)
                            {
                                throw std::runtime_error("Failed to uncompress arrayy, record(" + curRecord->name() + ".");
                            }
                        }
                        else
                        {
                            CheckOverflow(arrayLength, pos, fileSize);
                            fin.read(reinterpret_cast<char*>(pArray), arrayLength);
                        }
                    }
                    break;
                    case 'i':
                    {
                        Property * prop = curRecord->pushBack<int32_t>(nullptr, arrayLength);
                        int32_t * pArray = prop->m_value.m_pInteger32Array;

                        if (encoding == 1)
                        {
                            CheckOverflow(compressedLength, pos, fileSize);

                            mz_ulong uncomp_len = arrayLength * sizeof(int32_t);
                            unsigned char * pCmp = new unsigned char[uncomp_len];
                            fin.read(reinterpret_cast<char*>(pCmp), uncomp_len);

                            int cmp_status = uncompress(reinterpret_cast<unsigned char*>(pArray), &uncomp_len, pCmp, compressedLength);
                            delete[] pCmp;
                            if (cmp_status != Z_OK)
                            {
                                throw std::runtime_error("Failed to uncompress arrayy, record(" + curRecord->name() + ".");
                            }
                        }
                        else
                        {
                            CheckOverflow(arrayLength * sizeof(int32_t), pos, fileSize);
                            fin.read(reinterpret_cast<char*>(pArray), arrayLength * sizeof(int32_t));
                        }
                    }
                    break;
                    case 'l':
                    {
                        Property * prop = curRecord->pushBack<int64_t>(nullptr, arrayLength);
                        int64_t * pArray = prop->m_value.m_pInteger64Array;

                        if (encoding == 1)
                        {
                            CheckOverflow(compressedLength, pos, fileSize);

                            mz_ulong uncomp_len = arrayLength * sizeof(int64_t);
                            unsigned char * pCmp = new unsigned char[uncomp_len];
                            fin.read(reinterpret_cast<char*>(pCmp), uncomp_len);

                            int cmp_status = uncompress(reinterpret_cast<unsigned char*>(pArray), &uncomp_len, pCmp, compressedLength);
                            delete[] pCmp;
                            if (cmp_status != Z_OK)
                            {
                                throw std::runtime_error("Failed to uncompress arrayy, record(" + curRecord->name() + ".");
                            }
                        }
                        else
                        {
                            CheckOverflow(arrayLength * sizeof(int64_t), pos, fileSize);
                            fin.read(reinterpret_cast<char*>(pArray), arrayLength * sizeof(int64_t));
                        }
                    }
                    break;
                    case 'f':
                    {
                        Property * prop = curRecord->pushBack<float>(nullptr, arrayLength);
                        float * pArray = prop->m_value.m_pFloat32Array;

                        if (encoding == 1)
                        {
                            CheckOverflow(compressedLength, pos, fileSize);

                            mz_ulong uncomp_len = arrayLength * sizeof(float);
                            unsigned char * pCmp = new unsigned char[uncomp_len];
                            fin.read(reinterpret_cast<char*>(pCmp), uncomp_len);

                            int cmp_status = uncompress(reinterpret_cast<unsigned char*>(pArray), &uncomp_len, pCmp, compressedLength);
                            delete[] pCmp;
                            if (cmp_status != Z_OK)
                            {
                                throw std::runtime_error("Failed to uncompress arrayy, record(" + curRecord->name() + ".");
                            }
                        }
                        else
                        {
                            CheckOverflow(arrayLength * sizeof(float), pos, fileSize);
                            fin.read(reinterpret_cast<char*>(pArray), arrayLength * sizeof(float));
                        }
                    }
                    break;
                    case 'd':
                    {
                        Property * prop = curRecord->pushBack<double>(nullptr, arrayLength);
                        double * pArray = prop->m_value.m_pFloat64Array;

                        if (encoding == 1)
                        {
                            CheckOverflow(compressedLength, pos, fileSize);

                            mz_ulong uncomp_len = arrayLength * sizeof(double);
                            unsigned char * pCmp = new unsigned char[uncomp_len];
                            fin.read(reinterpret_cast<char*>(pCmp), uncomp_len);

                            int cmp_status = uncompress(reinterpret_cast<unsigned char*>(pArray), &uncomp_len, pCmp, compressedLength);
                            delete[] pCmp;
                            if (cmp_status != Z_OK)
                            {
                                throw std::runtime_error("Failed to uncompress arrayy, record(" + curRecord->name() + ".");
                            }
                        }
                        else
                        {
                            CheckOverflow(arrayLength * sizeof(double), pos, fileSize);
                            fin.read(reinterpret_cast<char*>(pArray), arrayLength * sizeof(double));
                        }

                    }
                    break;
                    default:
                        break;
                    }
                }

            }

            // Skip properties if skipped any...
            pos = nesterListPos;
            fin.seekg(nesterListPos, std::ifstream::beg);

            // New record level(new nester list).
            if (pos < endOffset)
            {
                level++;

                RecordList * pNewList = new RecordList();
                curRecord->m_pChildList = pNewList;

                listStack.push(pNewList);
                curList = pNewList;
            }

        }

    }

    void RecordList::write(const std::string & filename) const
    {

    }


    size_t RecordList::size() const
    {
        return m_recordList.size();
    }

    Record * RecordList::front() const
    {
        if (m_recordList.size() == 0)
        {
            return nullptr;
        }

        return m_recordList.front();
    }

    Record * RecordList::back() const
    {
        if (m_recordList.size() == 0)
        {
            return nullptr;
        }

        return m_recordList.back();
    }

    Record * RecordList::find(const std::string & name) const
    {
        auto it = m_recordMap.find(name);
        if (it == m_recordMap.end())
        {
            return nullptr;
        }

        return it->second;
    }


    Record * RecordList::pushBack(const std::string & name)
    {
        Record * pOldLast = nullptr;
        if (m_recordList.size())
        {
            pOldLast = m_recordList.back();
        }

        Record * pNewRecord = new Record(this, name);
        m_recordList.push_back(pNewRecord);
        m_recordMap.insert({ name, pNewRecord });

        pNewRecord->m_pPrevRecord = pOldLast;
        if (pOldLast)
        {
            pOldLast->m_pNextRecord = pNewRecord;
        }

        return pNewRecord;
    }

    Record * RecordList::pushFront(const std::string & name)
    {
        Record * pOldLast = nullptr;
        if (m_recordList.size())
        {
            pOldLast = m_recordList.front();
        }

        Record * pNewRecord = new Record(this, name);
        m_recordList.push_front(pNewRecord);
        m_recordMap.insert({ name, pNewRecord });

        pNewRecord->m_pNextRecord = pOldLast;
        if (pOldLast)
        {
            pOldLast->m_pPrevRecord = pNewRecord;
        }

        return pNewRecord;
    }


    /* bool RecordList::erase(Record * record)
    {
    bool found = false;
    for (auto it = m_recordList.begin(); it != m_recordList.end(); it++)
    {
    if (*it == record)
    {
    delete *it;
    it = m_recordList.erase(it);

    if (m_recordList.size() != 0)
    {
    if (m_recordList.size() == 1)
    {
    Record * pRemaining = m_recordList.front();
    pRemaining->m_pPrevRecord = nullptr;
    pRemaining->m_pNextRecord = nullptr;
    }
    else
    {
    if (it == m_recordList.end())
    {
    m_recordList.back()->m_pNextRecord = nullptr;
    }
    else
    {
    Record * pNext = *it;
    it--;
    }
    }
    }

    found = true;
    break;
    }
    }

    if (found == false)
    {
    return false;
    }

    for (auto it = m_recordMap.begin(); it != m_recordMap.end(); it++)
    {
    if (it->second == record)
    {
    m_recordMap.erase(it);
    return true;
    }
    }

    return false;
    }*/

    void RecordList::clear()
    {
        for (auto it = m_recordList.begin(); it != m_recordList.end(); it++)
        {
            delete *it;
        }
        m_recordList.clear();
        m_recordMap.clear();
    }

    void RecordList::setRecordName(Record * record, const std::string & name)
    {
        auto it = m_recordMap.find(record->name());
        if (it == m_recordMap.end())
        {
            throw std::runtime_error("Unkown record: " + record->name());
        }

        m_recordMap.erase(it);
        m_recordMap.insert({ name, record });
        record->m_name = name;
    }


    // Static functions
    void CheckOverflow(const size_t size, size_t & pos, const size_t max)
    {
        if (size + pos > max)
        {
            throw std::runtime_error("File reading overflow.");
        }

        pos += size;
    }

    template<typename T>
    void ToString(std::string & output, const T & in)
    {
        std::stringstream ss;
        ss << in;
        output = ss.str();
    }
}