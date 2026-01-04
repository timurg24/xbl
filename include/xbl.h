#pragma once
#include <cstdint>
#include <variant>
#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include <stdexcept>
#include <fstream>
#include <iterator>

#define ERROR(msg)  throw std::runtime_error(msg);


#define ElementStart    0x0A
#define ElementEnd      0x0B

namespace xbl {

    struct DateTime {
        uint16_t year;
        uint8_t month;
        uint8_t day;
        uint8_t hour;
        uint8_t minute;
        uint8_t second;
        uint32_t nanoseconds;
        int16_t offsetMinutes;
    };

    enum class ValueType : uint8_t {
        String      = 0x00,
        Int32       = 0x01,
        UInt32      = 0x02,
        Int64       = 0x03,
        UInt64      = 0x04,
        Float32     = 0x05,
        Float64     = 0x06,
        UInt8        = 0x07,
        //DateTime    = 0x08
    };

    struct Value {
        ValueType type;
        std::variant<std::string,int32_t,uint32_t,int64_t,uint64_t,float,double,uint8_t,DateTime> data;

        // Value(std::string v) : type(ValueType::String), data(std::move(v)) {}
        // Value(int32_t v)     : type(ValueType::Int32),  data(v) {}
        // Value(uint32_t v)    : type(ValueType::UInt32), data(v) {}
        // Value(int64_t v)     : type(ValueType::Int64),  data(v) {}
        // Value(uint64_t v)    : type(ValueType::UInt64), data(v) {}
        // Value(float v)       : type(ValueType::Float32),data(v) {}
        // Value(double v)      : type(ValueType::Float64),data(v) {}
        // Value(uint8_t v)     : type(ValueType::UInt8),   data(v) {}     // (rename Bool if you mean UInt8)
        // Value(DateTime v)    : type(ValueType::DateTime), data(v) {}
    };

    struct Attribute {
        std::string name;
        Value       value;
    };

    struct Element {
        std::string name;
        std::vector<Attribute> attributes;
        Element* parent;
        std::vector<std::unique_ptr<Element>> children;

        void addAttribute(const std::string& name, const Value& value);
        void addAttributesVec(std::vector<xbl::Attribute> el);
        Attribute& attribute(const std::string& name);
        Element& createChild(const std::string& elementName);
        Element& operator[](const std::string& childName);
    };

    struct Document {
        std::vector<std::unique_ptr<Element>> elements; // root elements
        Element& createElement(const std::string& elementName);
        Element& operator[](const std::string& elementName);
    };

    struct Parser {
        uint8_t nextByte(size_t& i, const std::vector<uint8_t>& data);
        std::string parseStandardString(size_t& i, const std::vector<uint8_t>& data);
        xbl::Attribute parseStandardAttribute(const std::string& name, uint8_t typeByte, std::string value);
        Document parse(const std::vector<uint8_t>& data);
    };

    std::vector<uint8_t> readBinary(const std::string& path);

}
