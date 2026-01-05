# myxbl
myxbl is a reader/writer for XBL in C++. XBL is a lightweight binary data-interchange format designed to resemble XML.

# Documentation
To see the documentation, including how XBL works, please see the repo wiki.

# Example
Reading:
```cpp
#include <iostream>
#include "xbl.h"

int main() {
  xbl::Document doc;
  xbl::Parser parser;
  doc = parser.parse(xbl::readBinary("test.bin"));
  xbl::Element& rootElement = doc["root"]["child"];
  std::string name = rootElement.attribute("name").getValue<std::string>();
  std::cout << name << "\n";
}
```

# License
This library is licensed under the MIT license.

# Upcoming Features
- Document serialization into binary file
