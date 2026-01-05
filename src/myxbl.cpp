#include "myxbl.h"

//==========
// ELEMENT
//==========

/**
 * Creates a new attribute inside an element
 * @param name Name of the attribute to be created
 * @param value Value object containing the value
 * @returns None
 * @throws None
 */
void xbl::Element::addAttribute(const std::string& name, const Value& value) {
    attributes.push_back({name, value});
}

/**
 * Creates new attributes based on a vector
 * @param el Vector of elements
 * @returns None
 * @throws None
 */
void xbl::Element::addAttributesVec(std::vector<Attribute> el) {
    for (auto& a : el) {
        attributes.push_back(std::move(a));
    }
}

/**
 * Returns attribute object by name of `name`
 * @param name Name of attribute
 * @returns Reference to attribute
 * @throws std::runtime_error If element does not have an attribute by name of `name`
 */
xbl::Attribute& xbl::Element::attribute(const std::string& name) {
    for(auto& attribute : attributes) {
        if(attribute.name == name) return attribute;
    }
    ERROR("Element does not have attribute: " + name);
}

/**
 * Creates a child element inside another element
 * @param elementName Name of new child element
 * @returns Reference to newly created child element
 * @throws None
 */
xbl::Element& xbl::Element::createChild(const std::string& elementName) {
    auto el = std::make_unique<Element>();
    el->name = elementName;

    children.push_back(std::move(el));
    return *children.back();
}

/**
 * Returns the child element by name of `childName`
 * @param childName Name of child element to be returned
 * @returns Reference to child element
 * @throws std::runtime_error If child element is not found
 */
xbl::Element& xbl::Element::operator[](const std::string& childName) {
    for(auto& child : children) {
        if(child->name == childName) return *child;
    }
    ERROR("Child element not found: " + childName);
}

//==========
// DOCUMENT
//==========

/**
 * Creates new root element inside the Document object
 * @param elementName Name of the new element
 * @returns Reference to newly created element
 * @throws None
 */
xbl::Element& xbl::Document::createElement(const std::string& elementName) {
    auto el = std::make_unique<Element>();
    el->name = elementName;

    elements.push_back(std::move(el));
    return *elements.back();
}

/**
 * Returns the root element with name of `elementName`
 * @param elementName Name of element to be returned
 * @returns Reference to element by the name of `elementName`
 * @throws std::runtime_error If `elementName` is not in the Document object
 */
xbl::Element& xbl::Document::operator[](const std::string& elementName) {
    for(auto& element : elements) {
        if(element->name == elementName) return *element;
    }
    ERROR("Element not found: " + elementName);
}

/**
 * Advances index to the next byte and returns the new byte
 * @param i Reference to the index
 * @param data Vector containing the bytes
 * @returns Next byte
 * @throws std::runtime_error If next byte is out of range for data
 */
uint8_t xbl::Parser::nextByte(size_t& i, const std::vector<uint8_t>& data) {
    if(i >= data.size()) ERROR("Unexpected EOF while getting next byte");
    return data[i++];
}

/**
 * Parses binary data into a string
 * @param i Index pointing to the length byte
 * @param data Vector containing the length and string
 * @returns Deserialized string
 * @throws std::runtime_error If `i` is out of range for `data`
 * @throws std::runtime_error If length defined in `data` is bigger than data
 */
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

/**
 * Deserializes string containing date and time into DateTime object
 * @param stringDateTime String representation of the date and time
 * @returns Deserialized DateTime object
 * @throws None
 */
xbl::DateTime xbl::Parser::parseDateTime(const std::string& stringDateTime) {
    xbl::DateTime result;

    // Parse into seperate strings
    std::string year = stringDateTime.substr(0, 4);
    std::string month = stringDateTime.substr(5,2);
    std::string day = stringDateTime.substr(8,2);
    // Skip char 9 (literal seperator)
    std::string hour = stringDateTime.substr(11,2);
    std::string minute = stringDateTime.substr(14,2);
    std::string second = stringDateTime.substr(17,2);
    // TODO: Implement full RFC 3339 spec

    // TODO: Replace stoi with something else
    result.year =   static_cast<uint16_t>(std::stoi(year));
    result.month =  static_cast<uint8_t>(std::stoi(month));
    result.day =    static_cast<uint8_t>(std::stoi(day));
    result.hour =   static_cast<uint8_t>(std::stoi(hour));
    result.minute = static_cast<uint8_t>(std::stoi(minute));
    result.second = static_cast<uint8_t>(std::stoi(second));

    return result;
}

/**
 * Deserializes string containing value into an Attribute object
 * @param name Name of the attribute
 * @param typeByte Byte containing the ValueType (eg String will be 0x00)
 * @param value String representation of the attribute value
 * @returns Deserialized Attribute object
 * @throws std::runtime_error If value for UInt8 is larger than 255
 * @throws std::runtime_error If typeByte is incorrect
 */
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
    case(xbl::ValueType::DateTime): {
        result.value.data = parseDateTime(value);
    }
    default: {
        ERROR(std::string("Invalid data type: ") + std::to_string((int)typeByte));
        break;
    }
    }
    return result;
}

/**
 * Parses (deserializes) binary data into a Document object
 * @param data Binary bytes of the XBL file
 * @returns Deserialized Document object containing file data and structure
 * @throws std::runtime_error If an invalid byte is read
 * @throws std::runtime_error If elements are not closed (missing ElementEnd)
 */
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

/**
 * Reads binary file
 * @param path Path to the file that is read
 * @return Vector of bytes
 * @throws std::runtime_error If file is not read
 */
std::vector<uint8_t> xbl::Parser::readBinary(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if(!file) ERROR(std::string("Failed to find file: ") + path);
    return std::vector<uint8_t>(
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>()
    );
}
