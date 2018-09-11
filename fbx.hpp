#pragma once

#include <string>
#include <list>
#include <map>
#include <vector>

namespace fbx
{

    class Record;
    class RecordList;


    class Property
    {

    public:

        enum class Type
        {
            Boolean,
            Integer16,
            Integer32,
            Integer64,
            Float32,
            Float64,

            BooleanArray,
            Integer32Array,
            Integer64Array,
            Float32Array,
            Float64Array,

            String,
            Raw

        };

        Type type() const;
        uint32_t size() const;
        Record * record() const;

        bool asBoolean() const;
        int16_t asInteger16() const;
        int32_t asInteger32() const;
        int64_t asInteger64() const;
        float asFloat32() const;
        double asFloat64() const;

        bool * asBooleanArray() const;
        int32_t * asInteger32Array() const;
        int64_t * asInteger64Array() const;
        float * asFloat32Array() const;
        double * asFloat64Array() const;

        const std::string & asString() const;
        const uint8_t * asRaw() const;

        std::string typeString() const;
        static std::string typeString(const Type type);
        

    private:

        friend class Record;
        friend class RecordList;

        Property(Record * record, const bool value);
        Property(Record * record, const int16_t value);
        Property(Record * record, const int32_t value);
        Property(Record * record, const int64_t value);
        Property(Record * record, const float value);
        Property(Record * record, const double value);
        Property(Record * record, const std::string & value);

        Property(Record * record, const bool * value, const size_t size);
        Property(Record * record, const int32_t * value, const size_t size);
        Property(Record * record, const int64_t * value, const size_t size);
        Property(Record * record, const float * value, const size_t size);
        Property(Record * record, const double * value, const size_t size);

        Property(Record * record, const uint8_t * value, const size_t size); // Raw.
        ~Property();

        Type        m_Type;
        uint32_t    m_Size;
        Record *    m_pRecord;

        union
        {
            bool        m_Boolean;
            int16_t     m_Integer16;
            int32_t     m_Integer32;
            int64_t     m_Integer64;
            float       m_Float32;
            double      m_Float64;

            bool *      m_pBooleanArray;
            int32_t *   m_pInteger32Array;
            int64_t *   m_pInteger64Array;
            float *     m_pFloat32Array;
            double *    m_pFloat64Array;

            std::string *   m_pString;
            uint8_t *       m_pRaw;
        } m_Value;

    };


    class Record
    {

    public:

        const std::string & name() const;
        size_t size() const;
        Property * at(const size_t index) const;
        template<typename T> Property * push_back(const T value)
        {
            Property * prop = new Property(this, value);
            m_Properties.push_back(prop);
            return prop;
        }
        template<typename T> Property * push_back(const T * value, const size_t size)
        {
            Property * prop = new Property(this, value, size);
            m_Properties.push_back(prop);
            return prop;
        }

        RecordList * parentList() const;
        RecordList * childList() const;
        Record * prev() const;
        Record * next() const;

    private:

        friend class RecordList;

        Record(RecordList * parent, const std::string & name);
        ~Record();

        std::string             m_Name;
        std::vector<Property*>  m_Properties;
        RecordList *            m_pParentList;
        RecordList *            m_pChildList;
        Record *                m_pPrevRecord;
        Record *                m_pNextRecord;

    };

    class RecordList
    {

    public:

        RecordList();
        RecordList(const std::string & filename);
        ~RecordList();

        Record * push_back(const std::string & name);
        Record * push_front(const std::string & name);
        //bool erase(Record * record
        void clear();

        size_t size() const;
        Record * front() const;
        Record * back() const;
        Record * find(const std::string & name) const;

        void read(const std::string & filename);
        void write(const std::string & filename) const;

    private:

        std::multimap<std::string, Record *>    m_RecordMap;
        std::list<Record *>                     m_RecordList;

    };

}