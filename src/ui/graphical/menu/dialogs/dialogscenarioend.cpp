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

#include "ui/graphical/menu/dialogs/dialogscenarioend.h"

#include "ui/graphical/menu/widgets/label.h"
#include "ui/graphical/menu/widgets/pushbutton.h"
#include "pcx.h"
#include "main.h"

cDialogScenarioEnd::cDialogScenarioEnd(bool victory) :
    cWindow (LoadPCX (GFXOD_DIALOG2), eWindowBackgrounds::Alpha)
{
    std::string text;
    if (victory) text = "Victory ! Congratulation general\n";
    else text = "Defeat ! Try again\n";
    text.append("Click on exit to end the scenario, click on continue to continue playing any way");
    auto textLabel = addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition () + cPosition (35, 35), getPosition () + cPosition (267, 173)), text, FONT_LATIN_NORMAL, toEnumFlag(eAlignmentType::CenterHorizontal) | eAlignmentType::Top));
    textLabel->setWordWrap (true);

    auto exitButton = addChild (std::make_unique<cPushButton> (getPosition () + cPosition (155, 185), ePushButtonType::Angular, "Exit", FONT_LATIN_NORMAL));
    exitButton->addClickShortcut (cKeySequence (cKeyCombination (eKeyModifierType::None, SDLK_RETURN)));
    signalConnectionManager.connect (exitButton->clicked, [&]()
    {
        exitClicked ();
        close ();
    });

    auto continueButton = addChild (std::make_unique<cPushButton> (getPosition () + cPosition (67, 185), ePushButtonType::Angular, "Continue", FONT_LATIN_NORMAL));
    continueButton->addClickShortcut (cKeySequence (cKeyCombination (eKeyModifierType::None, SDLK_ESCAPE)));
    signalConnectionManager.connect (continueButton->clicked, [&]()
    {
        continueClicked ();
        close ();
    });
}

cDialogScenarioEnd::~cDialogScenarioEnd()
{
}
