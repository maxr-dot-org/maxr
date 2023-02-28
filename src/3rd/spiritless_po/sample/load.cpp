/* sample/load.cpp

Copyright (c) 2019 OOTA, Masato

This is published under CC0 1.0.
For more information, see CC0 1.0 Universal (CC0 1.0) at <https://creativecommons.org/publicdomain/zero/1.0/legalcode>.
*/
#include <chrono>
#include <fstream>
#include <iostream>

#include "spiritless_po.h"

using namespace std;
using namespace std::chrono;

int main(int argc, char *argv[])
{
    if (argc <= 1) {
        cerr << "This program needs one filename." << endl;
        return 1;
    }

    const auto start_time = high_resolution_clock::now();
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
    const auto end_time = high_resolution_clock::now();
    const microseconds d = duration_cast<microseconds>(end_time - start_time);
    std::cout << "Loading elapse time: " << d.count() << " us" << endl;

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
