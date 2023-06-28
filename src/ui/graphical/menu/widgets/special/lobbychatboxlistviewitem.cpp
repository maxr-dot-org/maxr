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

#include "ui/graphical/menu/widgets/special/lobbychatboxlistviewitem.h"

#include "ui/widgets/label.h"
#include "utility/language.h"

//------------------------------------------------------------------------------
cLobbyChatBoxListViewItem::cLobbyChatBoxListViewItem (const std::string& text) :
	cAbstractListViewItem (cPosition (50, 0))
{
	prefixLabel = nullptr;

	messageLabel = emplaceChild<cLabel> (cBox<cPosition> (getPosition(), getPosition() + cPosition (getSize().x() - 1, 0)), text);
	messageLabel->setWordWrap (true);
	messageLabel->resizeToTextHeight();
	messageLabel->setConsumeClick (false);

	fitToChildren();
}

//------------------------------------------------------------------------------
cLobbyChatBoxListViewItem::cLobbyChatBoxListViewItem (const std::string& prefix, const std::string& text, bool addColon) :
	cLobbyChatBoxListViewItem (prefix, 0, text, addColon)
{}

//------------------------------------------------------------------------------
cLobbyChatBoxListViewItem::cLobbyChatBoxListViewItem (const std::string& prefix, int desiredPrefixTextWidth, const std::string& text, bool addColon) :
	cAbstractListViewItem (cPosition (50, 0))
{
	int prefixTextWidth;
	auto font = cUnicodeFont::font.get();
	if (!prefix.empty())
	{
		const auto prefixText = addColon ? prefix + lngPack.i18n ("Punctuation~Colon") : prefix;
		prefixTextWidth = std::max (desiredPrefixTextWidth, font->getTextWide (prefixText));
		prefixLabel = emplaceChild<cLabel> (cBox<cPosition> (getPosition(), getPosition() + cPosition (prefixTextWidth, 10)), prefixText);
		prefixLabel->setConsumeClick (false);
	}
	else
	{
		prefixTextWidth = 0;
		prefixLabel = nullptr;
	}

	const cPosition messageLabelBeginPos = getPosition() + cPosition (prefixTextWidth, 0);
	const cPosition messageLabelEndPos (std::max (getPosition().x() + getSize().x() - 1, messageLabelBeginPos.x() + 1), getPosition().y() + 5);

	messageLabel = emplaceChild<cLabel> (cBox<cPosition> (messageLabelBeginPos, messageLabelEndPos), text);
	messageLabel->setWordWrap (true);
	messageLabel->resizeToTextHeight();
	messageLabel->setConsumeClick (false);

	fitToChildren();
}

//------------------------------------------------------------------------------
void cLobbyChatBoxListViewItem::handleResized (const cPosition& oldSize)
{
	cAbstractListViewItem::handleResized (oldSize);

	if (oldSize.x() == getSize().x()) return;

	auto prefixTextWidth = prefixLabel ? prefixLabel->getSize().x() - 1 : 0;
	messageLabel->resize (cPosition (getSize().x() - prefixTextWidth, messageLabel->getSize().y()));
	messageLabel->resizeToTextHeight();

	fitToChildren();
}

//------------------------------------------------------------------------------
int cLobbyChatBoxListViewItem::getPrefixLabelWidth() const
{
	return prefixLabel ? prefixLabel->getSize().x() : 0;
}

//------------------------------------------------------------------------------
void cLobbyChatBoxListViewItem::setDesiredPrefixLabelWidth (int width)
{
	int prefixTextWidth;
	if (prefixLabel != nullptr)
	{
		prefixTextWidth = std::max (prefixLabel->getSize().x(), width);
		prefixLabel->resize (cPosition (prefixTextWidth, prefixLabel->getSize().y()));
		prefixTextWidth -= 1;
	}
	else
	{
		prefixTextWidth = std::max (0, width);
	}
	messageLabel->resize (cPosition (getSize().x() - prefixTextWidth, messageLabel->getSize().y()));
	messageLabel->moveTo (getPosition() + cPosition (prefixTextWidth, 0));
	messageLabel->resizeToTextHeight();
	messageLabel->setConsumeClick (false);

	fitToChildren();
}
