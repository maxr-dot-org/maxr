/***************************************************************************
 *      Mechanized Assault and Exploration Reloaded Projectfile            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef EXTENDEDTINYXML_H
#define EXTENDEDTINYXML_H

#include "tinyxml2.h"
#include <stdarg.h>
#include <string>


tinyxml2::XMLElement* XmlGetFirstElement (tinyxml2::XMLDocument& xmlDoc, const char* first, ...);

/**
 * Tries to find a element from a path in a xml file.
 * If the element does not exist, it will be generated (and all parent nodes that do not exist as well).
 * If the configuration file does not exist it tries to generate a new one.
 * @param path The path to the node to get. Nodes should be devided by '~'.
 *             e.g.: "Options~Game~Net~PlayerName"
 * @param configFile The XML file to search in.
 * @return The found or generated node at the specific path or NULL if the config file could not be read and generated.
 */
tinyxml2::XMLElement* getOrCreateXmlElement (tinyxml2::XMLDocument& xmlDoc, const std::string& path);

int         getXMLAttributeInt    (tinyxml2::XMLDocument& document, const char* first, ...);
float       getXMLAttributeFloat  (tinyxml2::XMLDocument& document, const char* first, ...);
std::string getXMLAttributeString (tinyxml2::XMLDocument& document, const char* attribut, const char* first, ...);
bool        getXMLAttributeBool   (tinyxml2::XMLDocument& document, const char* first, ...);


#endif
