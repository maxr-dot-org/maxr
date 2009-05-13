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
#ifndef menuitemsH
#define menuitemsH
#include "defines.h"
#include "main.h"
#include "mouse.h"
#include "unifonts.h"
#include "upgradecalculator.h"
#include "sound.h"

class cMenu;
class cHangarMenu;
class cUpgradeHangarMenu;
class cNetworkMenu;

struct sSaveFile
{
	string filename;
	string gamename;
	string type;
	string time;
	int number;
};

class cMenuItem
{
friend class cMenuItemContainer;
protected:
	sSOUND *clickSound;
	sSOUND *releaseSound;

	void (*click)(void *);
	void (*release)(void *);
	void (*hoverOn)(void *);
	void (*hoverAway)(void *);
	void (*moveMouseOver)(void *);
	void (*wasKeyInput)(void *);

	cMenuItem( int x, int y );

	SDL_Rect position;
	bool active;

	bool wasClicked;
	bool isClicked;
	bool locked;

	virtual bool preClicked() { return true; }
	virtual bool preReleased();
	virtual bool preHoveredOn() { return true; }
	virtual bool preHoveredAway() { return true; }
	virtual bool preSetLocked( bool locked_ ) { return true; }

public:
	virtual void draw() = 0;

	virtual void setActivity ( bool active_ ) { active = active_; }
	virtual bool overItem( int x, int y ) const;

	virtual void clicked( void *parent );
	virtual void released( void *parent );
	virtual void hoveredOn( void *parent );
	virtual void hoveredAway( void *parent );
	virtual void movedMouseOver( int lastMouseX, int lastMouseY, void *parent );

	virtual void somewhereReleased();

	virtual void handleKeyInput( SDL_keysym keysym, string ch, void *parent ) {}

	void move ( int x, int y );
	void setLocked( bool locked_ );
	void setClickSound ( sSOUND *clickSound_ );
	void setReleaseSound ( sSOUND *releaseSound_ );

	void setClickedFunction ( void (*click_)(void *) );
	void setReleasedFunction ( void (*release_)(void *) );
	void setMovedOverFunction ( void (*moveMouseOver_)(void *) );
	void setWasKeyInputFunction ( void (*wasKeyInput_)(void *) );
};


class cMenuItemContainer : public cMenuItem
{
protected:
	cList<cMenuItem*> itemList;
public:
	cMenuItemContainer( int x, int y );
	virtual void draw();

	virtual void clicked( void *parent );
	virtual void released( void *parent );
	virtual void hoveredOn( void *parent );
	virtual void hoveredAway( void *parent );

	virtual void movedMouseOver( int lastMouseX, int lastMouseY, void *parent );
	virtual void somewhereReleased();

	void addItem ( cMenuItem* item );
	void removeItem ( cMenuItem* item );
};


class cMenuImage : public cMenuItem
{
protected:
	SDL_Surface *image;

public:
	cMenuImage ( int x, int y, SDL_Surface *image_ = NULL );
	~cMenuImage();
	void setImage( SDL_Surface *image_ );
	void draw();
};

class cMenuLabel : public cMenuItem
{
protected:
	SDL_Rect textPosition;
	string text;
	eUnicodeFontType fontType;

	bool flagCentered;
	bool flagBox;

public:
	cMenuLabel ( int x, int y, string text_ = "", eUnicodeFontType fontType_= FONT_LATIN_NORMAL );
	void setText( string text_ );
	void setFontType ( eUnicodeFontType fontType_ );
	void setCentered( bool centered );
	void setBox( int width, int height );
	void draw();
};

class cMenuButton : public cMenuItem
{
public:
	enum eButtonTypes
	{
		BUTTON_TYPE_STANDARD_BIG,
		BUTTON_TYPE_STANDARD_SMALL,
		BUTTON_TYPE_HUGE,
		BUTTON_TYPE_ARROW_UP_BIG,
		BUTTON_TYPE_ARROW_DOWN_BIG,
		BUTTON_TYPE_ARROW_UP_SMALL,
		BUTTON_TYPE_ARROW_DOWN_SMALL,
		BUTTON_TYPE_ARROW_LEFT_SMALL,
		BUTTON_TYPE_ARROW_RIGHT_SMALL,
		BUTTON_TYPE_ARROW_UP_BAR,
		BUTTON_TYPE_ARROW_DOWN_BAR,
		BUTTON_TYPE_ANGULAR,
	};
protected:

	SDL_Surface* surface;
	string text;
	eUnicodeFontType fontType;
	eButtonTypes buttonType;

	void renewButtonSurface();
	void redraw();
	int getTextYOffset();

	bool preClicked();
	bool preReleased();
	bool preHoveredOn();
	bool preHoveredAway();

	bool preSetLocked( bool locked_ );

public:
	cMenuButton ( int x, int y, string text_ = "", eButtonTypes buttonType_ = BUTTON_TYPE_STANDARD_BIG, eUnicodeFontType fontType_ = FONT_LATIN_BIG, sSOUND *clickSound_ = SoundData.SNDHudButton );
	~cMenuButton();
	void draw();
};

class cMenuCheckButton : public cMenuItem
{
friend class cMenuRadioGroup;

public:
	enum eCheckButtonTypes
	{
		RADIOBTN_TYPE_TEXT_ONLY,
		RADIOBTN_TYPE_BTN_ROUND,
		RADIOBTN_TYPE_ANGULAR_BUTTON,
		CHECKBOX_TYPE_STANDARD,
		CHECKBOX_TYPE_TANK,
		CHECKBOX_TYPE_PLANE,
		CHECKBOX_TYPE_SHIP,
		CHECKBOX_TYPE_BUILD,
		CHECKBOX_TYPE_TNT,
	};

	enum eCheckButtonTextOriantation
	{
		TEXT_ORIENT_RIGHT,
		TEXT_ORIENT_LEFT
	};
protected:
	string text;
	SDL_Surface* surface;
	eUnicodeFontType fontType;
	eCheckButtonTypes buttonType;
	eCheckButtonTextOriantation textOrientation;

	class cMenuRadioGroup *group;

	bool centered;
	bool checked;

	void renewButtonSurface();

	bool preClicked();

public:
	cMenuCheckButton( int x, int y, string text_ = "", bool checked_ = false, bool centered_ = false,eCheckButtonTypes buttonType_ = RADIOBTN_TYPE_BTN_ROUND, eCheckButtonTextOriantation textOrientation = TEXT_ORIENT_RIGHT, eUnicodeFontType fontType_ = FONT_LATIN_NORMAL, sSOUND *clickSound_ = SoundData.SNDObjectMenu);
	void draw();

	void setChecked ( bool checked_ );
	bool isChecked();
};

class cMenuRadioGroup : public cMenuItem
{
friend class cMenuCheckButton;
protected:
	cList<cMenuCheckButton*> buttonList;

	void checkedButton ( cMenuCheckButton* button );
public:
	cMenuRadioGroup () : cMenuItem ( 0, 0 ) {}
	void draw();

	bool overItem( int x, int y ) const;
	void clicked( void *parent );

	void addButton( cMenuCheckButton* button );
	bool buttonIsChecked ( int index );
};

enum eMenuUnitListDisplayTypes
{
	MUL_DIS_TYPE_COSTS,
	MUL_DIS_TYPE_CARGO,
	MUL_DIS_TYPE_NOEXTRA
};

struct sUnitUpgrade
{
	enum eUpgradeTypes
	{
		UPGRADE_TYPE_DAMAGE,
		UPGRADE_TYPE_SHOTS,
		UPGRADE_TYPE_RANGE,
		UPGRADE_TYPE_AMMO,
		UPGRADE_TYPE_ARMOR,
		UPGRADE_TYPE_HITS,
		UPGRADE_TYPE_SCAN,
		UPGRADE_TYPE_SPEED,
		UPGRADE_TYPE_NONE
	};

	bool active; // is this upgrade buyable for the player
	int nextPrice; // what will the next upgrade cost
	int purchased; // how many upgrades of this type has the player purchased
	int curValue; // what is the current value
	int startValue; // the value that this unit would have without all upgrades
	eUpgradeTypes type;

	sUnitUpgrade() : active(false), nextPrice(0), purchased(0), curValue(-1), startValue(0), type(UPGRADE_TYPE_NONE) {}
};

class cMenuUnitListItem : public cMenuItem
{
friend class cMenuUnitsList;
protected:
	eMenuUnitListDisplayTypes displayType;
	class cMenuUnitsList* parentList;

	sID unitID;
	cPlayer *owner;

	SDL_Surface *surface;

	int resValue;
	int minResValue;
	sUnitUpgrade *upgrades;

	bool fixed;
	bool selected;
	bool fixedResValue;
	bool marked;

	int drawName( bool withNumber );
	void drawCargo( int destY );

public:
	cMenuUnitListItem( sID unitID_, cPlayer *owner_, sUnitUpgrade *upgrades_, eMenuUnitListDisplayTypes displayType_, cMenuUnitsList* parent, bool fixedResValue_ );
	~cMenuUnitListItem();
	void draw();

	void released( void *parent );

	sID getUnitID();
	cPlayer *getOwner();
	int getResValue();
	bool getFixedResValue ();
	void setResValue( int resValue_, bool cargoCheck = true );
	void setMinResValue ( int minResValue_ );
	void setMarked ( bool marked_ );
	void setFixedResValue ( bool fixedResValue_ );
	void setFixed ( bool fixed_ );
	bool getFixedStatus();
	sUnitUpgrade *getUpgrades();
};

class cMenuUnitsList : public cMenuItem
{
friend class cMenuUnitListItem;
protected:
	bool (*doubleClicked)(cMenuUnitsList *, void *parent );

	cHangarMenu *parentMenu;

	eMenuUnitListDisplayTypes displayType;

	cList<cMenuUnitListItem*> unitsList;
	cMenuUnitListItem *selectedUnit;
	int offset;
	int maxDisplayUnits;

public:
	cMenuUnitsList( int x, int y, int w, int h, cHangarMenu *parent, eMenuUnitListDisplayTypes displayType_ );
	~cMenuUnitsList();
	void draw();

	void released( void *parent );

	int getSize();
	cMenuUnitListItem* getItem ( int index );
	cMenuUnitListItem* getSelectedUnit();

	void resize ( int w, int h );
	void setDoubleClickedFunction( bool (*doubleClicked_)(cMenuUnitsList *, void *parent ) );
	void scrollUp();
	void scrollDown();
	void setSelection ( cMenuUnitListItem *selectedUnit_ );
	cMenuUnitListItem *addUnit ( sID unitID, cPlayer *owner, sUnitUpgrade *upgrades = NULL, bool scroll = false, bool fixedCargo = false );
	void removeUnit ( cMenuUnitListItem *item );
	void clear();
	void setDisplayType ( eMenuUnitListDisplayTypes displayType_ );
};

class cMenuUnitDetails : public cMenuItem
{
public:
	enum eMenuSymbolsBig
	{
		MENU_SYMBOLS_BIG_SPEED,
		MENU_SYMBOLS_BIG_HITS,
		MENU_SYMBOLS_BIG_AMMO,
		MENU_SYMBOLS_BIG_ATTACK,
		MENU_SYMBOLS_BIG_SHOTS,
		MENU_SYMBOLS_BIG_RANGE,
		MENU_SYMBOLS_BIG_ARMOR,
		MENU_SYMBOLS_BIG_SCAN,
		MENU_SYMBOLS_BIG_METAL,
		MENU_SYMBOLS_BIG_OIL,
		MENU_SYMBOLS_BIG_GOLD,
		MENU_SYMBOLS_BIG_ENERGY,
		MENU_SYMBOLS_BIG_HUMAN
	};
protected:

	cHangarMenu *parentMenu;
	cMenuUnitListItem *selectedUnit;

	void drawBigSymbol ( eMenuSymbolsBig sym, int x, int y, int maxx, int value, int orgvalue );
public:
	cMenuUnitDetails( int x, int y, cHangarMenu *parent );
	void draw();

	void setSelection ( cMenuUnitListItem *selectedUnit_ );
};

class cMenuMaterialBar : public cMenuItem
{
public:
	enum eMaterialBarTypes
	{
		MAT_BAR_TYPE_METAL,
		MAT_BAR_TYPE_OIL,
		MAT_BAR_TYPE_GOLD
	};

protected:
	SDL_Surface *surface;
	eMaterialBarTypes materialType;

	cMenuLabel *valueLabel;

	int maxValue, currentValue;

	void generateSurface();
public:
	cMenuMaterialBar( int x, int y, int labelX, int labelY, int maxValue_, eMaterialBarTypes materialType_ );
	~cMenuMaterialBar();
	void draw();

	void setMaximalValue( int maxValue_ );
	void setCurrentValue( int currentValue_ );
};

class cMenuUpgradeHandler : public cMenuItemContainer
{
	cUpgradeHangarMenu *parentMenu;
	cMenuUnitListItem *selection;

	cMenuButton *decreaseButtons[8];
	cMenuButton *increaseButtons[8];
	cMenuLabel *costsLabel[8];

	cUpgradeCalculator::UpgradeTypes getUpgradeType( sUnitUpgrade upgrade );
	void updateUnitValues ( cMenuUnitListItem *unit ); 
public:
	cMenuUpgradeHandler( int x, int y, cUpgradeHangarMenu *parent );
	~cMenuUpgradeHandler();

	static void buttonReleased( void* parent );

	void setSelection ( cMenuUnitListItem *selection_ );
};

class cMenuScroller : public cMenuItem
{
friend class cMenuScrollBar;
	SDL_Surface *surface;

public:
	cMenuScroller ( int x, int y );
	~cMenuScroller();
	void draw();

	void move ( int y );
};

class cMenuScrollBar : public cMenuItemContainer
{
friend class cMenuListBox;
friend class cMenuPlayersBox;
protected:
	cMenu *parentMenu;
	cMenuItem *parentItem;

	SDL_Surface *surface;

	int maximalScroll;
	int pageSteps;

	int offset;
	int maximalOffset;
	int scrollerSteps;

	cMenuButton *upButton;
	cMenuButton *downButton;
	cMenuScroller *scroller;

	void createSurface();
public:
	cMenuScrollBar ( int x, int y, int h, int pageSteps_, cMenu *parentMenu_, cMenuItem *parentItem_ );
	~cMenuScrollBar();
	void draw();

	void setMaximalScroll ( int maximalScroll_ );

	static void upButtonReleased( void* parent );
	static void downButtonReleased( void* parent );
};


class cMenuListBox : public cMenuItemContainer
{
protected:
	cMenu *parentMenu;

	cList<string> lines;
	int maxLines;
	int maxDrawLines;

	cMenuScrollBar *scrollBar;
public:
	cMenuListBox ( int x, int y, int w, int h, int maxLines_, cMenu *parentMenu_ );
	~cMenuListBox();
	void draw();

	void addLine ( string line );
};

class cMenuLineEdit : public cMenuItem
{
	void (*returnPressed)(void *);

	cMenu *parentMenu;
	string text;
	int cursorPos;
	int startOffset, endOffset;

	bool readOnly;
	bool takeChars, takeNumerics;

	void doPosIncrease( int &value, int pos );
	void doPosDecrease( int &pos );
	void scrollLeft( bool changeCursor = true );
	void scrollRight();
	void deleteLeft();
	void deleteRight();
public:
	cMenuLineEdit ( int x, int y, int w, int h, cMenu *parentMenu_ );
	void draw();

	bool preClicked();

	void setReadOnly ( bool readOnly_ );
	void setTaking ( bool takeChars_, bool takeNumerics_ );
	void setText ( string text_ );
	string getText ();
	void handleKeyInput( SDL_keysym keysym, string ch, void *parent );

	void setReturnPressedFunc( void (*returnPressed_)(void *) );
};

struct sMenuPlayer
{
	string name;
	int color;
	bool ready;

	int nr;
	int socket;

	sMenuPlayer (string name_ = "", int color_ = 0, bool ready_ = false, int nr_ = 0, int socket_ = -1)
		: name(name_), color(color_), ready(ready_), nr(nr_), socket(socket_) {}
};

class cMenuPlayersBox : public cMenuItemContainer
{
	cList<sMenuPlayer*> *players;

	cNetworkMenu *parentMenu;
	int maxDrawPlayers;

	cList<cMenuImage*> playerColors;
	cList<cMenuLabel*> playerNames;
	cList<cMenuImage*> playerReadys;

	cMenuScrollBar *scrollBar;
public:
	cMenuPlayersBox ( int x, int y, int w, int h, cNetworkMenu *parentMenu_ );
	~cMenuPlayersBox();
	void draw();

	bool preClicked();

	void setPlayers ( cList<sMenuPlayer*> *player_ );
};

class cMenuSaveSlot : public cMenuItem
{
	cMenuLabel *saveNumber;
	cMenuLabel *saveType;
	cMenuLabel *saveTime;
	cMenuLineEdit *saveName;
public:
	cMenuSaveSlot( int x, int y, cMenu *parent );
	~cMenuSaveSlot();
	void draw();

	void setActivity ( bool active_ ) {}

	void setSaveData ( sSaveFile saveFile, bool selected );
	void reset( int number, bool selected );
	cMenuLineEdit *getNameEdit();
};

class cMenuBuildSpeedHandler : public cMenuItemContainer
{
	cMenuCheckButton *speedButtons[3];
	cMenuRadioGroup *speedGroup;

	cMenuLabel *turnsLabels[3];
	cMenuLabel *costsLabels[3];

public:
	cMenuBuildSpeedHandler( int x, int y );
	~cMenuBuildSpeedHandler();

	void setValues ( int *turboBuildTurns, int *turboBuildCosts );
	void setBuildSpeed( int buildSpeed );
	int getBuildSpeed();
};

class cMenuUpgradeFilter : public cMenuItemContainer
{
	cHangarMenu *parentMenu;

	cMenuCheckButton* checkButtonTank;
	cMenuCheckButton* checkButtonPlane;
	cMenuCheckButton* checkButtonShip;
	cMenuCheckButton* checkButtonBuilding;
	cMenuCheckButton* checkButtonTNT;

	static void buttonChanged( void *parent );
public:
	cMenuUpgradeFilter( int x, int y, cHangarMenu *parentMenu );
	~cMenuUpgradeFilter();

	void setTankChecked ( bool checked );
	void setPlaneChecked ( bool checked );
	void setShipChecked ( bool checked );
	void setBuildingChecked ( bool checked );
	void setTNTChecked ( bool checked );

	bool TankIsChecked();
	bool PlaneIsChecked();
	bool ShipIsChecked();
	bool BuildingIsChecked();
	bool TNTIsChecked();
};

#endif // menuitemsH
