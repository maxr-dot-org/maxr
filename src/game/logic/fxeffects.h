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

#ifndef game_logic_fxeffectsH
#define game_logic_fxeffectsH

#include <vector>
#include <memory>
#include "maxrconfig.h"
#include "utility/autosurface.h"
#include "utility/position.h"

class cSoundManager;

class cFx
{
protected:
	cFx (bool bottom_, const cPosition& position);

	cPosition position;
	int tick;
	int length;

public:
	virtual ~cFx();

	const bool bottom;

	const cPosition& getPosition ();

	virtual bool isFinished() const;
	int getLength() const;

	virtual void draw (float zoom, const cPosition& destination) const = 0;
	virtual void playSound (cSoundManager& soundManager) const;
	virtual void run();
};

class cFxContainer
{
public:
	void push_back (std::shared_ptr<cFx> fx);
	void push_front (std::shared_ptr<cFx> fx);
	size_t size () const;
	void run();
private:
	std::vector<std::shared_ptr<cFx>> fxs;
};

class cFxMuzzle : public cFx
{
protected:
	cFxMuzzle (const cPosition& position, int dir_);
	virtual void draw (float zoom, const cPosition& destination) const MAXR_OVERRIDE_FUNCTION;

	AutoSurface (*pImages) [2];
	int dir;
};

class cFxMuzzleBig : public cFxMuzzle
{
public:
	cFxMuzzleBig (const cPosition& position, int dir_);
};

class cFxMuzzleMed : public cFxMuzzle
{
public:
	cFxMuzzleMed (const cPosition& position, int dir_);
};

class cFxMuzzleMedLong : public cFxMuzzle
{
public:
	cFxMuzzleMedLong (const cPosition& position, int dir_);
};

class cFxMuzzleSmall : public cFxMuzzle
{
public:
	cFxMuzzleSmall (const cPosition& position, int dir_);
};

class cFxExplo : public cFx
{
protected:
	cFxExplo (const cPosition& position, int frames_);
	virtual void draw (float zoom, const cPosition& destination) const MAXR_OVERRIDE_FUNCTION;

protected:
	AutoSurface (*pImages) [2];
	// TODO: frames could be calculated (frames = w / h),
	// if the width and height of the grapics for one frame would be equal
	// (which they aren't yet).
	const int frames;
};

class cFxExploSmall : public cFxExplo
{
public:
	cFxExploSmall (const cPosition& position); // x, y is the center of the explosion
    virtual void playSound (cSoundManager& soundManager) const MAXR_OVERRIDE_FUNCTION;
};

class cFxExploBig : public cFxExplo
{
public:
	cFxExploBig (const cPosition& position, bool onWater);
    virtual void playSound (cSoundManager& soundManager) const MAXR_OVERRIDE_FUNCTION;

private:
	bool onWater;
};

class cFxExploAir : public cFxExplo
{
public:
	cFxExploAir (const cPosition& position);
    virtual void playSound (cSoundManager& soundManager) const MAXR_OVERRIDE_FUNCTION;
};

class cFxExploWater : public cFxExplo
{
public:
	cFxExploWater (const cPosition& position);
    virtual void playSound (cSoundManager& soundManager) const MAXR_OVERRIDE_FUNCTION;
};

class cFxHit : public cFxExplo
{
public:
	cFxHit (const cPosition& position);
    virtual void playSound (cSoundManager& soundManager) const MAXR_OVERRIDE_FUNCTION;
};

class cFxAbsorb : public cFxExplo
{
public:
	cFxAbsorb (const cPosition& position);
    virtual void playSound (cSoundManager& soundManager) const MAXR_OVERRIDE_FUNCTION;
};

class cFxFade : public cFx
{
protected:
	cFxFade (const cPosition& position, bool bottom, int start, int end);
	virtual void draw (float zoom, const cPosition& destination) const MAXR_OVERRIDE_FUNCTION;

	AutoSurface (*pImages) [2];
	const int alphaStart;
	const int alphaEnd;
};

class cFxSmoke : public cFxFade
{
public:
	cFxSmoke (const cPosition& position, bool bottom); // x, y is the center of the effect
};

class cFxCorpse : public cFxFade
{
public:
	cFxCorpse (const cPosition& position);
};

class cFxTracks : public cFx
{
private:
	AutoSurface (*pImages) [2];
	const int alphaStart;
	const int alphaEnd;
	const int dir;
public:
	cFxTracks (const cPosition& position, int dir_);
	virtual void draw (float zoom, const cPosition& destination) const MAXR_OVERRIDE_FUNCTION;
};


class cFxRocket : public cFx
{
private:
	const int speed;
	std::vector<cFx*> subEffects;
	AutoSurface (*pImages) [2];
	int dir;
	int distance;
	const cPosition startPosition;
	const cPosition endPosition;
public:
	cFxRocket (const cPosition& startPosition, const cPosition& endPosition, int dir_, bool bottom);
	~cFxRocket();
	virtual void draw (float zoom, const cPosition& destination) const MAXR_OVERRIDE_FUNCTION;
	void run();
	// return true, when the last smoke effect is finished.
	// getLength() returns only the time until
	// the rocket has reached the destination
	virtual bool isFinished () const MAXR_OVERRIDE_FUNCTION;
};

class cFxDarkSmoke : public cFx
{
private:
	float dx;
	float dy;
	const int alphaStart;
	const int alphaEnd;
	const int frames;
	AutoSurface (*pImages) [2];
public:
	cFxDarkSmoke (const cPosition& position, int alpha, float windDir);
	virtual void draw (float zoom, const cPosition& destination) const MAXR_OVERRIDE_FUNCTION;
};

#endif // game_logic_fxeffectsH
