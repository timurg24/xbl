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
  xbl::Element& rootEl = doc["root"]["child"];
  auto& v = rootEl.attribute("name").value.data;
  std::cout << std::get<std::string>(v) << "\n";
}
```

# License
This library is licensed under the MIT license.
