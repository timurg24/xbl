# xbl
Extendable Binary Language is a binary data-interchange format designed to be similar to XML. The code in this repo is a lightweight reader for XBL in C++.

# Documentation
To see the documentation, including how XBL works, please see the repo wiki.

# Example
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
