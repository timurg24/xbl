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
#include <type_traits>
#include <cstring>

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
        //uint32_t nanoseconds;
        //int16_t offsetMinutes;
    };

    using ValueVariant = std::variant<std::string,int32_t,uint32_t,int64_t,uint64_t,float,double,uint8_t,DateTime>;

    enum class ValueType : uint8_t {
        String      = 0x00,
        Int32       = 0x01,
        UInt32      = 0x02,
        Int64       = 0x03,
        UInt64      = 0x04,
        Float32     = 0x05,
        Float64     = 0x06,
        UInt8        = 0x07,
        DateTime    = 0x08
    };

    struct Value {
        ValueType type;
        ValueVariant data;
    };

    struct Attribute {
        std::string name;
        Value       value;

        template <typename T>
        T getValue() const;
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
        DateTime parseDateTime(const std::string& stringDateTime);
        xbl::Attribute parseStandardAttribute(const std::string& name, uint8_t typeByte, std::string value);
        Document parse(const std::vector<uint8_t>& data);

        std::vector<uint8_t> readBinary(const std::string& path);
    };

    struct Serializer {

        template <typename T>
        void writeByte(std::vector<uint8_t>& out, ValueVariant v);

        void writeBinary(const std::string& path, const std::vector<uint8_t>& data);
        std::vector<uint8_t> serializeAttributeValue(const Attribute& at);
        std::vector<uint8_t> serializeAttribute(const Attribute& at);
        std::vector<uint8_t> serializeElement(const Element& el);
        std::vector<uint8_t> serialize(const Document& doc);
    };

    

} // namespace xbl
