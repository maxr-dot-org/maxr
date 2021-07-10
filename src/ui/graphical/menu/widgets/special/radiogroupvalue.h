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

#ifndef ui_graphical_menu_widgets_special_radiogroupvalueH
#define ui_graphical_menu_widgets_special_radiogroupvalueH

#include "ui/graphical/menu/widgets/checkbox.h"
#include "ui/graphical/widget.h"
#include "utility/signal/signalconnectionmanager.h"

#include "config/workaround/cpp17/optional.h"

/*
** Similar to regular radioGroup, but handles natively an association
** between checkbox and value.
*/
template <typename T>
class cRadioGroupValue: public cWidget
{
public:
	cRadioGroupValue() : cRadioGroupValue (false) {}
	explicit cRadioGroupValue (bool allowUncheckAll) : allowUncheckAll (allowUncheckAll) {}

	template <typename ... Ts>
	cCheckBox* emplaceCheckBox (T value, Ts&&... args) { return addCheckBox (value, std::make_unique<cCheckBox> (std::forward<Ts> (args)...)); }

	template <typename TCheckBox>
	TCheckBox* addCheckBox (T value, std::unique_ptr<TCheckBox> button);

	bool selectValue (T value);

	std::optional<T> getSelectedValue() const { if (currentlyCheckedButton) return value; return std::nullopt;}

	void handleMoved (const cPosition& offset);

private:
	cSignalConnectionManager signalConnectionManager;

	void buttonToggled (cCheckBox&, T);

private:
	cCheckBox* currentlyCheckedButton = nullptr;
	T value{};

	std::map<T, cCheckBox*> checkBoxes;
	bool allowUncheckAll = false;
	bool internalMoving = false;
};

//------------------------------------------------------------------------------
template <typename T>
template <typename CheckBox>
CheckBox* cRadioGroupValue<T>::addCheckBox (T value, std::unique_ptr<CheckBox> newCheckBox)
{
	const bool hadButtons = hasChildren();
	CheckBox* res = addChild (std::move (newCheckBox));
	cCheckBox* checkBox = &static_cast<cCheckBox&> (*res);

	if (currentlyCheckedButton == nullptr && !allowUncheckAll && !checkBox->isChecked()) checkBox->setChecked (true);

	signalConnectionManager.connect (checkBox->toggled, [=](){ buttonToggled (*checkBox, value); });
	buttonToggled (*checkBox, value);

	checkBoxes.emplace (value, checkBox);

	// resize own area to include the new button
	internalMoving = true;
	if (hadButtons)
	{
		auto area = getArea();
		area.add (checkBox->getArea());
		setArea (area);
	}
	else
	{
		setArea (checkBox->getArea());
	}
	internalMoving = false;

	return res;
}

//------------------------------------------------------------------------------
template <typename T>
bool cRadioGroupValue<T>::selectValue (T value)
{
	auto it = checkBoxes.find (value);

	if (it != checkBoxes.end())
	{
		it->second->setChecked (true);
		return true;
	}
	if (currentlyCheckedButton && allowUncheckAll)
	{
		currentlyCheckedButton->setChecked (false);
		currentlyCheckedButton = nullptr;
	}
	return false;
}

//------------------------------------------------------------------------------
template <typename T>
void cRadioGroupValue<T>::buttonToggled (cCheckBox& button, T value)
{
	if (&button == currentlyCheckedButton)
	{
		if (!button.isChecked())
		{
			if (!allowUncheckAll) button.setChecked (true);
			else currentlyCheckedButton = nullptr;
		}
	}
	else
	{
		if (button.isChecked())
		{
			auto oldCheckedButton = currentlyCheckedButton;
			currentlyCheckedButton = &button;
			if (oldCheckedButton) oldCheckedButton->setChecked (false);
		}
	}
	if (button.isChecked()) { this->value = value; }
}

//------------------------------------------------------------------------------
template <typename T>
void cRadioGroupValue<T>::handleMoved (const cPosition& offset)
{
	if (internalMoving) return;

	cWidget::handleMoved (offset);
}

#endif
