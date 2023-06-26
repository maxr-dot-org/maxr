/** Metadata parser.
    \file MetadataParser.h
    \author OOTA, Masato
    \copyright Copyright © 2019, 2022 OOTA, Masato
    \par License Boost
    \parblock
      This program is distributed under the Boost Software License Version 1.0.
      You can get the license file at “https://www.boost.org/LICENSE_1_0.txt”.
    \endparblock
*/

#ifndef SPIRITLESS_PO_METADATA_PARSER_H_
#define SPIRITLESS_PO_METADATA_PARSER_H_

#include <string>
#include <unordered_map>

namespace spiritless_po {
    namespace MetadataParser {
        /** The type of the metadata.

            map[key] == value
        */
        typedef std::unordered_map<std::string, std::string> MapT;

        /** Parse a metadata text.
            \param [in] metadataString The source text of the metadata.
            \return The map of the metadata.

            This function parses a metadata text and set the keys and the values to the map.

            The metadata text consists of the lines that have a key and a value, such as "key: value\n", more exactly it can be expressed the regex "^(.+): *(.+)\n?$" (key = $1, value = $2), of course, '\\n' is necessary except for the last line. It's compatible with po_header_field() in GNU libgettextpo.

            If some lines have the same keys, the first line is registered.
        */
        inline MapT Parse(const std::string &metadataString)
        {
            MapT map;
            enum { KEY, SPACE, VALUE } stat = KEY;
            std::string key;
            std::string value;
            for (char c : metadataString) {
                if (stat == KEY) {
                    if (c == ':') {
                        stat = SPACE;
                    } else {
                        key += c;
                    }
                } else if ((stat == SPACE && c != ' ') || stat == VALUE) {
                    stat = VALUE;
                    if (c == '\n') {
                        stat = KEY;
                        if (!key.empty()) {
                            map.emplace(key, value);
                            key.clear();
                        }
                        value.clear();
                    } else {
                        value += c;
                    }
                }
            }
            if (!key.empty()) {
                map.emplace(key, value);
            }
            return map;
        }
    } // namespace MetadataParser
} // namespace spiritless_po

#endif // SPIRITLESS_PO_METADATA_PARSER_H_
