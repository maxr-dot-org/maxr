/* sample/dump.cpp

Copyright (c) 2022 OOTA, Masato

This is published under CC0 1.0.
For more information, see CC0 1.0 Universal (CC0 1.0) at <https://creativecommons.org/publicdomain/zero/1.0/legalcode>.
*/
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
#if 0
    /* Add version */
    for (size_t i = 0; i < static_cast<size_t>(argc) - 1; i++) {
        ifstream f(argv[i + 1]);
        catalog.ClearError();
        if (!catalog.Add(f)) {
            for (const auto &s : catalog.GetError()) {
                cerr << argv[i + 1] << ": " << s << endl;
            }
        }
    }
#else
    /* Merge version */
    for (size_t i = 0; i < static_cast<size_t>(argc) - 1; i++) {
        spiritless_po::Catalog newCatalog;
        ifstream f(argv[i + 1]);
        newCatalog.ClearError();
        if (!newCatalog.Add(f)) {
            for (const auto &s : newCatalog.GetError()) {
                cerr << argv[i + 1] << ": " << s << endl;
            }
        }
        catalog.Merge(newCatalog);
    }
#endif

    cout << "String Table:" << endl;
    size_t no = 0;
    for (auto s : catalog.GetStringTable()) {
        cout << no << ": " << s << endl;
        ++no;
    }
    cout << "Index:" << endl;
    for (auto idx : catalog.GetIndex()) {
        auto base = idx.second.stringTableIndex;
        auto total = idx.second.totalPlurals;
        if (total == 1) {
            cout << idx.first << ": " << base << endl;
        } else {
            cout << idx.first << ": " << base << " - " << (base + total - 1) << endl;
        }
    }
    cout << "Metadata:" << endl;
    for (auto entry : catalog.GetMetadata()) {
        cout << entry.first << ": " << entry.second << endl;
    }
    cout << "Errors:" << endl;
    for (const auto &s : catalog.GetError()) {
        cerr << s << endl;
    }
    return 0;
}
