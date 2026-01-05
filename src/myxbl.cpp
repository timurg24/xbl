#include "myxbl.h"

//==========
// ATTRIBUTE
//==========

/**
 * Returns the attribute value
 * @param None
 * @returns Value of attribute
 * @throws None
*/
template <typename T>
T xbl::Attribute::getValue() const {
    return std::get<T>(value.data); 
}

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

//==========
// SERIALIZER
//==========


/*
    Steps
    Write length of attribute
    Write data type of attribute
    Write attribute data

*/

/**
 * Writes vector of bytes into binary file
 * 
 * @param path Path to the output binary file
 * @param data Vector of bytes
 * @return None
 * @throws std::runtime_error If file cannot be opened or written to
 */
void xbl::Serializer::writeBinary(const std::string& path, const std::vector<uint8_t>& data) {
    std::ofstream file(path, std::ios::binary | std::ios::out | std::ios::trunc);
    if(!file) ERROR("File cannot be opened/written: " + path);
    file.write(reinterpret_cast<const char*>(data.data()), data.size());
}


/**
 * Turns the attribute object value into bytes
 * @param at Attribute Object
 * @returns Vector of bytes
 * @throws runtime_error If a string is longer than 255
 * @throws runtime_error If the value type is invalid
 */
std::vector<uint8_t> xbl::Serializer::serializeAttributeValue(const Attribute& at) {
    std::vector<uint8_t> result;
    result.push_back(static_cast<uint8_t>(at.value.type));

    switch (at.value.type) {
        case xbl::ValueType::String: {
            std::string s = at.getValue<std::string>();
            if(s.size() > 255) ERROR(std::string("String too long with size: ") + std::to_string(s.size()));
            result.push_back(static_cast<uint8_t>(s.size()));
            result.insert(result.end(),
                reinterpret_cast<const uint8_t*>(s.data()),
                reinterpret_cast<const uint8_t*>(s.data() + s.size())
            );
            break;
        }

        case xbl::ValueType::Int32: {
            int32_t u = at.getValue<int32_t>();
            uint32_t x = static_cast<uint32_t>(u);
            result.push_back(static_cast<uint8_t>(sizeof(int32_t)));
            for (int i = 0; i < sizeof(int32_t); ++i) { 
                result.push_back(static_cast<uint8_t>((x >> (8 * i)) & 0xFF));
            }
            break;
        }

        case xbl::ValueType::UInt32: {
            uint32_t x = at.getValue<uint32_t>();
            result.push_back(static_cast<uint8_t>(sizeof(uint32_t)));
            for (int i = 0; i < sizeof(uint32_t); ++i) { 
                result.push_back(static_cast<uint8_t>((x >> (8 * i)) & 0xFF));
            }
            break;
        }

        case xbl::ValueType::Int64: {
            int64_t u = at.getValue<int64_t>();
            uint64_t x = static_cast<uint64_t>(u);
            result.push_back(static_cast<uint8_t>(sizeof(int64_t)));
            for (size_t i = 0; i < sizeof(int64_t); ++i) {
                result.push_back(static_cast<uint8_t>((x >> (8 * i)) & 0xFF));
            }
            break;
        }

        case xbl::ValueType::UInt64: {
            uint64_t x = at.getValue<uint64_t>();
            result.push_back(static_cast<uint8_t>(sizeof(uint64_t)));
            for (size_t i = 0; i < sizeof(uint64_t); ++i) {
                result.push_back(static_cast<uint8_t>((x >> (8 * i)) & 0xFF));
            }
            break;
        }

        case xbl::ValueType::Float32: {
            float x = at.getValue<float>();
            result.push_back(static_cast<uint8_t>(sizeof(float)));
            static_assert(sizeof(float) == 4);

            uint32_t u;
            std::memcpy(&u, &x, 4); // reinterpret bits safely

            for (size_t i = 0; i <  sizeof(uint32_t); ++i) {
                result.push_back(static_cast<uint8_t>((u >> (8 * i)) & 0xFF));
            }
            break;
        }

        case xbl::ValueType::Float64: {
            double x = at.getValue<double>();
            result.push_back(static_cast<uint8_t>(sizeof(double)));
            static_assert(sizeof(double) == 8);

            uint64_t u;
            std::memcpy(&u, &x, 8); // reinterpret bits safely

            for (size_t i = 0; i < sizeof(uint64_t); ++i) {
                result.push_back(static_cast<uint8_t>((u >> (8 * i)) & 0xFF));
            }
            break;
        }

        case xbl::ValueType::UInt8: {
            uint8_t x = std::get<uint8_t>(at.value.data);
            result.push_back(sizeof(uint8_t));
            result.push_back(x); // 1 byte
            break;
        }

        default:
            ERROR("Uknown Value Type");
    }

    return result;
}

/**
 * Serializes a (full) Attribute Object into vector of bytes
 * @param at Attribute
 * @returns Vector of bytes
 * @throws None
 */
std::vector<uint8_t> xbl::Serializer::serializeAttribute(const Attribute& at) {
    std::vector<uint8_t> result;
    result.push_back(at.name.size());                           // Length of name
    result.insert(                                              // Name
        result.end(), 
        reinterpret_cast<const uint8_t*>(at.name.data()),
        reinterpret_cast<const uint8_t*>(at.name.data()) + at.name.size()
    );
    std::vector<uint8_t> value = serializeAttributeValue(at);
    result.insert(result.end(), value.begin(), value.end());    // Value
    return result;
}

/**
 * Serializes Element object
 * @param el Element
 * @returns Vector of bytes
 * @throws None
 */
std::vector<uint8_t> xbl::Serializer::serializeElement(const Element& el) {
    std::vector<uint8_t> result;
    result.push_back(ElementStart);                         // ElementStart
    if(el.name.size() > 255) ERROR(std::string("Element name too long with size: ") + std::to_string(el.name.size()));
    result.push_back(static_cast<uint8_t>(el.name.size())); // Size of name
    result.insert(                                          // Name
        result.end(), 
        reinterpret_cast<const uint8_t*>(el.name.data()),
        reinterpret_cast<const uint8_t*>(el.name.data()) + el.name.size()
    );
    size_t attributeCount = el.attributes.size();
    if(attributeCount > 255) ERROR(std::string("Too many attributes with attribute count of: ") + std::to_string(attributeCount));
    result.push_back(static_cast<uint8_t>(attributeCount)); // Attribute Count
    // Loop through the attributes
    for(size_t attributeIndex = 0; attributeIndex < attributeCount; attributeIndex++) {
        std::vector<uint8_t> attribute = serializeAttribute(el.attributes[attributeIndex]);
        result.insert(result.end(), attribute.begin(), attribute.end());
    }
    // Loop through child objects
    for(const auto& child : el.children) {
        const Element& ref = *child;
        auto childElement = serializeElement(ref);
        result.insert(result.end(), childElement.begin(), childElement.end());
    }

    result.push_back(ElementEnd);

    return result;
}

/**
 * Serializes document object into bytes
 * @param doc Document
 * @returns Vector of bytes
 * @throws None
 */
std::vector<uint8_t> xbl::Serializer::serialize(const Document& doc) {
    std::vector<uint8_t> result;
    for(const auto& root : doc.elements) {
        const Element& ref = *root;
        auto rootElementSerialized = serializeElement(ref);
        result.insert(result.end(), rootElementSerialized.begin(), rootElementSerialized.end());
    }
    return result;
}
