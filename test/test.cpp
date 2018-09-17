#include "gtest/gtest.h"
#include "fbx.hpp"

using namespace Fbx;

TEST(Property, Value)
{
    {
        {
            Property p((bool)false);
            EXPECT_EQ(p.type(), Property::Type::Boolean);
            EXPECT_TRUE(p.isPrimitive());
            EXPECT_FALSE(p.isArray());
            EXPECT_FALSE(p.isString());
            EXPECT_FALSE(p.isRaw());
            EXPECT_EQ(p.primitive().boolean, (int16_t)false);
        }
        {
            Property p((int16_t)1234);
            EXPECT_EQ(p.type(), Property::Type::Integer16);
            EXPECT_TRUE(p.isPrimitive());
            EXPECT_FALSE(p.isArray());
            EXPECT_FALSE(p.isString());
            EXPECT_FALSE(p.isRaw());
            EXPECT_EQ(p.primitive().integer16, (int16_t)1234);
        }
        {
            Property p((int32_t)1234567);
            EXPECT_EQ(p.type(), Property::Type::Integer32);
            EXPECT_TRUE(p.isPrimitive());
            EXPECT_FALSE(p.isArray());
            EXPECT_FALSE(p.isString());
            EXPECT_FALSE(p.isRaw());
            EXPECT_EQ(p.primitive().integer32, (int32_t)1234567);
        }
        {
            Property p((int64_t)123456789012);
            EXPECT_EQ(p.type(), Property::Type::Integer64);
            EXPECT_TRUE(p.isPrimitive());
            EXPECT_FALSE(p.isArray());
            EXPECT_FALSE(p.isString());
            EXPECT_FALSE(p.isRaw());
            EXPECT_EQ(p.primitive().integer64, (int64_t)123456789012);
        }
        {
            Property p((float)1234567.0f);
            EXPECT_EQ(p.type(), Property::Type::Float32);
            EXPECT_TRUE(p.isPrimitive());
            EXPECT_FALSE(p.isArray());
            EXPECT_FALSE(p.isString());
            EXPECT_FALSE(p.isRaw());
            EXPECT_EQ(p.primitive().float32, (float)1234567.0f);
        }
        {
            Property p((double)123456789.0f);
            EXPECT_EQ(p.type(), Property::Type::Float64);
            EXPECT_TRUE(p.isPrimitive());
            EXPECT_FALSE(p.isArray());
            EXPECT_FALSE(p.isString());
            EXPECT_FALSE(p.isRaw());
            EXPECT_EQ(p.primitive().float64, (double)123456789.0f);
        }
        {
            bool array[5] = { true, false, false, true, false };
            Property p(array, 5);
            EXPECT_EQ(p.type(), Property::Type::BooleanArray);
            EXPECT_FALSE(p.isPrimitive());
            EXPECT_TRUE(p.isArray());
            EXPECT_FALSE(p.isString());
            EXPECT_FALSE(p.isRaw());
            EXPECT_EQ(p.size(), 5);
            for (size_t i = 0; i < 5; ++i)
            {
                EXPECT_EQ(p.array()[i].boolean, array[i]);
            }
        }
        {
            int32_t array[5] = { 100000, 200000, 300000, 400000, 500000 };
            Property p(array, 5);
            EXPECT_EQ(p.type(), Property::Type::Integer32Array);
            EXPECT_FALSE(p.isPrimitive());
            EXPECT_TRUE(p.isArray());
            EXPECT_FALSE(p.isString());
            EXPECT_FALSE(p.isRaw());
            EXPECT_EQ(p.size(), 5);
            for (size_t i = 0; i < 5; ++i)
            {
                EXPECT_EQ(p.array()[i].integer32, array[i]);
            }
        }
        {
            int64_t array[5] = { 100000, 200000, 300000, 400000, 500000 };
            Property p(array, 5);
            EXPECT_EQ(p.type(), Property::Type::Integer64Array);
            EXPECT_FALSE(p.isPrimitive());
            EXPECT_TRUE(p.isArray());
            EXPECT_FALSE(p.isString());
            EXPECT_FALSE(p.isRaw());
            EXPECT_EQ(p.size(), 5);
            for (size_t i = 0; i < 5; ++i)
            {
                EXPECT_EQ(p.array()[i].integer64, array[i]);
            }
        }
        {
            float array[5] = { 100000.0f, 200000.0f, 300000.0f, 400000.0f, 500000.0f };
            Property p(array, 5);
            EXPECT_EQ(p.type(), Property::Type::Float32Array);
            EXPECT_FALSE(p.isPrimitive());
            EXPECT_TRUE(p.isArray());
            EXPECT_FALSE(p.isString());
            EXPECT_FALSE(p.isRaw());
            EXPECT_EQ(p.size(), 5);
            for (size_t i = 0; i < 5; ++i)
            {
                EXPECT_EQ(p.array()[i].float32, array[i]);
            }
        }
        {
            double array[5] = { 1000001.0f, 2000002.0f, 3000003.0f, 4000004.0f, 5000005.0f };
            Property p(array, 5);
            EXPECT_EQ(p.type(), Property::Type::Float64Array);
            EXPECT_FALSE(p.isPrimitive());
            EXPECT_TRUE(p.isArray());
            EXPECT_FALSE(p.isString());
            EXPECT_FALSE(p.isRaw());
            EXPECT_EQ(p.size(), 5);
            for (size_t i = 0; i < 5; ++i)
            {
                EXPECT_EQ(p.array()[i].float64, array[i]);
            }
        }
        {
            Property p("Hello world!");
            EXPECT_EQ(p.type(), Property::Type::String);
            EXPECT_FALSE(p.isPrimitive());
            EXPECT_FALSE(p.isArray());
            EXPECT_TRUE(p.isString());
            EXPECT_FALSE(p.isRaw());
            EXPECT_STREQ(p.string().c_str(), "Hello world!");
        }
        {
            uint8_t raw[5] = { 1, 2, 3, 4, 5 };
            Property p(raw, 5);
            EXPECT_EQ(p.type(), Property::Type::Raw);
            EXPECT_FALSE(p.isPrimitive());
            EXPECT_FALSE(p.isArray());
            EXPECT_FALSE(p.isString());
            EXPECT_TRUE(p.isRaw());
            EXPECT_EQ(p.size(), 5);
            EXPECT_TRUE(memcmp(raw, &p.raw()[0], 5) == 0);
        }
    }
}

TEST(Record, ReaderWriter)
{
    Record file1;
    Record file2;

    EXPECT_NO_THROW(file1.read("../models/blender-default.fbx"));
    EXPECT_NO_THROW(file1.write("../bin/blender-default-test.fbx"));
    EXPECT_NO_THROW(file2.read("../bin/blender-default-test.fbx"));
}

int main(int argc, char ** argv)
{
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
