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

#ifndef FBX_HPP_GUARD
#define FBX_HPP_GUARD

#include <string>
#include <list>
#include <map>
#include <vector>
#include <functional>

namespace Fbx
{

    class Property
    {

    public:

        union Value
        {
            bool    boolean;
            int16_t integer16;
            int32_t integer32;
            int64_t integer64;
            float   float32;
            double  float64;
        };

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

        Property(const bool primitive);
        Property(const int16_t primitive);
        Property(const int32_t primitive);
        Property(const int64_t primitive);
        Property(const float primitive);
        Property(const double primitive);
        Property(const bool * array, const uint32_t count);
        Property(const int32_t * array, const uint32_t count);
        Property(const int64_t * array, const uint32_t count);
        Property(float * array, const uint32_t count);
        Property(const double * array, const uint32_t count);
        Property(const char * string);
        Property(const std::string & string);
        Property(const uint8_t * raw, const uint32_t size);

        Type type() const;
        uint8_t code() const;
        Value & primitive();
        const Value & primitive() const;
        std::vector<Value> & array();
        const std::vector<Value> & array() const;
        std::string string() const;
        std::vector<uint8_t> & raw();
        const std::vector<uint8_t> & raw() const;
        uint32_t size() const;

        bool isPrimitive() const;
        bool isArray() const;
        bool isString() const;
        bool isRaw() const;

    private:

        Type                    m_type;
        Value                   m_primitive;
        std::vector<Value>      m_array;
        std::vector<uint8_t>    m_raw;

    };


    class PropertyList
    {

    public:

        typedef std::list<Property *>::iterator Iterator;
        typedef std::list<Property *>::const_iterator ConstIterator;

        PropertyList();
        ~PropertyList();

        size_t size() const;
        Iterator insert(Property * property);
        Iterator insert(Iterator position, Property * property);
        Iterator begin();
        ConstIterator begin() const;
        Iterator end();
        ConstIterator end() const;
        Property * front();
        const Property * front() const;
        Property * back();
        const Property * back() const;
        Iterator erase(Property * property);
        Iterator erase(Iterator position);
        void clear();

    private:

        PropertyList(PropertyList &);

        std::list<Property *> m_properties;

    };


    class Record
    {

    public:

        typedef std::list<Record *>::iterator Iterator;
        typedef std::list<Record *>::const_iterator ConstIterator;

        Record();
        Record(const std::string & name);
        Record(const std::string & name, Record * parent);
        ~Record();

        void read(const std::string & filename);
        void read(const std::string & filename, std::function<void(std::string, uint32_t)> onHeaderRead);
        void write(const std::string & filename) const;
        void write(const std::string & filename, const uint32_t version) const;

        const std::string & name() const;
        void name(const std::string & name);
        Record * parent();
        const Record * parent() const;
        void parent(Record * parent);
        PropertyList & properties();
        const PropertyList & properties() const;

        size_t size() const;
        Iterator insert(Record * record);
        Iterator insert(Iterator position, Record * record);
        Iterator begin();
        ConstIterator begin() const;
        Iterator end();
        ConstIterator end() const;
        Record * front();
        const Record * front() const;
        Record * back();
        const Record * back() const;
        Iterator find(const std::string & name);
        ConstIterator find(const std::string & name) const;
        Iterator erase(Record * record);
        Iterator erase(Iterator position);
        void clear();

    private:
        
        Record(const Record &);

        std::string         m_name;
        Record *            m_pParent;
        PropertyList        m_properties;
        std::list<Record *> m_nestedList;

    };

}

#endif