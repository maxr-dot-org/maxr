# spiritless_po ![badge](https://github.com/oo13/spiritless_po/workflows/ci-workflow/badge.svg)

spiritless_po is a kind of gettext library in C++11 and inspired by [spirit-po](https://github.com/cbeck88/spirit-po), but I don't intend to be compatible with spirit-po.

spirit-po depend on Boost library, but this library can be compiled by C++11.

Spiritless_po has some features (as same as spirit-po):
- A catalog handles only one textdomain and only one language, doesn't handle multiple textdomains and multiple languages.
- The catalog can read the messages from multiple PO files, instead of a single MO file. You can add new messages to a single catalog any number of times.
- The catalog doesn't care the locale.
- The catalog doesn't handle the character encoding.

If you would use multiple textdomains and/or multiple languages, you need to use multiple catalogs.

You need only to use [Catalog](@ref spiritless_po::Catalog) class and not to use other classes directly. The "public" interfaces that Catalog doesn't publish by even indirect are considered as the internal interfaces in the spiritless_po module.

Example:
```c++
#include <fstream>
#include <iostream>

#include "spiritless_po.h"

using namespace std;

int main(int argc, char *argv[])
{
    if (argc <= 1) {
        cerr << "This program needs one filename." << endl;
        return 1;
    }

    spiritless_po::Catalog catalog;
    for (size_t i = 0; i < static_cast<size_t>(argc) - 1; i++) {
        ifstream f(argv[i + 1]);
        catalog.ClearError();
        if (!catalog.Add(f)) {
            for (const auto &s : catalog.GetError()) {
                cerr << argv[i + 1] << ": " << s << endl;
            }
        }
    }

    cout << "Apple"
         << ": " << catalog.gettext("Apple") << endl;
    for (size_t i = 0; i < 30; i++) {
        cout << i << ": Bean"
             << ": " << catalog.ngettext("Bean", "Beans", i) << endl;
    }

    auto index = catalog.GetIndex();
    cout << "Number of msgid: " << index.size() << endl;
    return 0;
}
```

# To Generate the Documents
Use doxygen. I tested the generation in doxygen 1.9.4.

```
% cd spiritless_po
% doxygen spiritless_po.doxygen
# Open spiritless_po/html/index.html with your HTML browser.
```

# Unit Test
This library includes some unit test codes. If you want to run it, the following programs are needed:

- Catch2 (Tested in version 2.13.10)
- cmake  (Tested in Version 3.24.3) or meson (Tested in Version 1.0.1)

cmake:
```
% cd spiritless_po/test
% cmake -DCMAKE_BUILD_TYPE=Release -B build .
% cd build
% make
% ./test_spiritless_po
% ./test_spiritless_po '[!benchmark]' ; # For benchmark
```

meson:
```
% cd spiritless_po/test
% meson setup build
% cd build
% meson compile
% meson test ; # or ninja test
% meson test --benchmark ; # or ninja benchmark
```
