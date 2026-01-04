#include "xbl.h"


// ELEMENT
void xbl::Element::addAttribute(const std::string& name, const xbl::Value& value) {
    attributes.push_back({name, value});
}

void xbl::Element::addAttributesVec(std::vector<xbl::Attribute> el) {
    for (auto& a : el) {
        attributes.push_back(std::move(a));
    }
}

// Returns attribute by name of `name`
xbl::Attribute& xbl::Element::attribute(const std::string& name) {
    for(auto& attribute : attributes) {
        if(attribute.name == name) return attribute;
    }
    ERROR("Element does not have attribute: " + name);
}

xbl::Element& xbl::Element::createChild(const std::string& elementName) {
    auto el = std::make_unique<Element>();
    el->name = elementName;

    children.push_back(std::move(el));
    return *children.back();
}

// Returns child element
xbl::Element& xbl::Element::operator[](const std::string& childName) {
    for(auto& child : children) {
        if(child->name == childName) return *child;
    }
    ERROR("Child element not found: " + childName);
}

// DOCUMENT
xbl::Element& xbl::Document::createElement(const std::string& elementName) {
    auto el = std::make_unique<Element>();
    el->name = elementName;

    elements.push_back(std::move(el));
    return *elements.back();
}

xbl::Element& xbl::Document::operator[](const std::string& elementName) {
    for(auto& element : elements) {
        if(element->name == elementName) return *element;
    }
    ERROR("Element not found: " + elementName);
}

// PARSER
uint8_t xbl::Parser::nextByte(size_t& i, const std::vector<uint8_t>& data) {
    if(i >= data.size()) ERROR("Unexpected EOF while getting next byte");
    return data[i++];
}

// Input: `i` should be pointing to length byte
// Output: `i` is pointing to the byte after the end of the string
// [length byte] [string bytes... ]
std::string xbl::Parser::parseStandardString(size_t& i, const std::vector<uint8_t>& data) {
    if (i >= data.size())
        ERROR("Index is out of range: " + std::to_string(i));

    uint8_t length = nextByte(i, data);   // consume length; now i points at first char

    if (i + length > data.size())
        ERROR("Unexpected EOF while reading string");

    std::string s(reinterpret_cast<const char*>(&data[i]), length);
    i += length;                          // consume string bytes
    return s;
}
// Parse string value into correct serializable data
xbl::Attribute xbl::Parser::parseStandardAttribute(const std::string& name, uint8_t typeByte, std::string value) {
    xbl::Attribute result;
    result.name = name;

    xbl::ValueType type = static_cast<xbl::ValueType>(typeByte);
    result.value.type = type;

    switch(type) {
    case(xbl::ValueType::String): {
        result.value.data = std::string(value);
        break;
    }
    case(xbl::ValueType::Int32): {
        result.value.data = int32_t(std::stoi(value));
        break;
    }
    case(xbl::ValueType::UInt32): {
        result.value.data = uint32_t(std::stoi(value));
        break;
    }
    case(xbl::ValueType::Int64): {
        result.value.data = int64_t(std::stoll(value));
        break;
    }
    case(xbl::ValueType::UInt64): {
        result.value.data = uint32_t(std::stoull(value));
        break;
    }
    case(xbl::ValueType::Float32): {
        result.value.data = float(std::stof(value));
        break;
    }
    case(xbl::ValueType::Float64): {
        result.value.data = double(std::stod(value));
        break;
    }
    case(xbl::ValueType::UInt8): {
        int v = std::stoi(value);
        if (v < 0 || v > 255)
            ERROR("UInt8 out of range: " + value);
        result.value.data = static_cast<uint8_t>(v);
        break;
    }
    default: {
        ERROR(std::string("Invalid data type: ") + std::to_string((int)typeByte));
        break;
    }
    }
    return result;
}

xbl::Document xbl::Parser::parse(const std::vector<uint8_t>& data) {

    xbl::Document result;

    std::vector<xbl::Element*> stack;

    for(size_t i = 0; i < data.size();) {
        uint8_t byte = data[i];

        // Element Start
        if(byte == ElementStart) {
            // Element Name
            nextByte(i, data); // Move index to length byte
            std::string name = parseStandardString(i, data);
            // Attribute Count
            size_t attributeCount = data[i];
            nextByte(i, data); // now points to attribute name length
            std::vector<xbl::Attribute> attributes; // attributes of the element
            // Get all attributes
            for(int j = 0; j < attributeCount; j++) {
                std::string attributeName = parseStandardString(i, data);
                uint8_t attributeType = data[i];
                nextByte(i, data);
                std::string attributeValue = parseStandardString(i, data);
                attributes.push_back(parseStandardAttribute(attributeName, attributeType, attributeValue));
            }

            // Create element
            xbl::Element* node = nullptr;
            if(stack.empty()) { // Root element
                xbl::Element& root = result.createElement(name);
                root.addAttributesVec(std::move(attributes));
                node = &root;
            } else {
                xbl::Element& child = stack.back()->createChild(name);
                child.addAttributesVec(std::move(attributes));
                node = &child;
            }

            stack.push_back(node);
            continue;
        
        }
        // Element End
        if(byte == ElementEnd) {
            stack.pop_back();
            ++i;
            continue;
        }

        // Invalid byte
        ERROR(std::string("Unrecognized byte: ") + std::to_string((int)byte));
    }

    if(!stack.empty()) {
        ERROR("Incomplete elements present");
    }
    return result;
}

// FILE
std::vector<uint8_t> xbl::readBinary(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if(!file) ERROR(std::string("Failed to find file: ") + path);
    return std::vector<uint8_t>(
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>()
    );
}
