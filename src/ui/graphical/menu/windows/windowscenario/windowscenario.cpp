#include "windowscenario.h"

#include "main.h"
#include "pcx.h"
#include "utility/files.h"
#include "ui/graphical/menu/widgets/label.h"
#include "ui/graphical/menu/widgets/pushbutton.h"
#include "ui/graphical/menu/widgets/combobox.h"

cWindowScenario::cWindowScenario() :
    cWindow (LoadPCX (GFXOD_OPTIONS))
{
    addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition () + cPosition (0, 13), getPosition () + cPosition (getArea ().getMaxCorner ().x (), 23)), lngPack.i18n ("Text~Others~Game_NewScenario"), FONT_LATIN_NORMAL, eAlignmentType::CenterHorizontal));

    addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition () + cPosition (40, 60+20), getPosition () + cPosition (40+95, 60+20+10)), "Scenario:", FONT_LATIN_NORMAL, eAlignmentType::Right)); // TODO: translate
    scenarioComboBox = addChild (std::make_unique<cComboBox> (cBox<cPosition> (getPosition () + cPosition (140, 56+20), getPosition () + cPosition (140 + 300, 56+20+17))));

    // TODO_M: Add a field for scenario description that will be updated when selected scenario change!
    // Will require to load the scenario files to read scenario text and descriptions, also use scenario name and not filename in GUI
    // TODO_M: List available scenarios in a list view is better looking that in combobox.

    auto okButton = addChild (std::make_unique<cPushButton> (getPosition () + cPosition (390, 440), ePushButtonType::StandardBig, lngPack.i18n ("Text~Others~OK")));
    signalConnectionManager.connect (okButton->clicked, std::bind (&cWindowScenario::okClicked, this));

    auto backButton = addChild (std::make_unique<cPushButton> (getPosition () + cPosition (50, 440), ePushButtonType::StandardBig, lngPack.i18n ("Text~Others~Back")));
    signalConnectionManager.connect (backButton->clicked, std::bind (&cWindowScenario::backClicked, this));

    loadAvailableScenarios();
}

cWindowScenario::~cWindowScenario()
{
}

std::string cWindowScenario::getSelectedScenario() const
{
    return scenarioComboBox->getSelectedText();
}

void cWindowScenario::okClicked()
{
    done();
}

void cWindowScenario::backClicked()
{
    close();
}

void cWindowScenario::loadAvailableScenarios()
{
    scenarioComboBox->clearItems();

    // List all available scenario files from scenario directory
    std::vector<std::string> scenarios = getFilesOfDirectory(cSettings::getInstance().getScenariosPath());
    for (size_t i = 0; i != scenarios.size(); ++i)
    {
        const std::string& scenario = scenarios[i];
        scenarioComboBox->addItem(scenario);
    }

    scenarioComboBox->setMaxVisibleItems(scenarioComboBox->getItemsCount());
    scenarioComboBox->setSelectedIndex(0);
}
