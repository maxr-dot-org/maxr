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

#include "game/data/units/unitdata.h"
#include "utility/position.h"

#include <memory>
#include <vector>

class cFxMuzzleBig;
class cFxMuzzleMed;
class cFxMuzzleMedLong;
class cFxMuzzleSmall;
class cFxExploAir;
class cFxExploBig;
class cFxExploSmall;
class cFxExploWater;
class cFxAbsorb;
class cFxRocket;
class cFxFade;
class cFxDarkSmoke;
class cFxCorpse;
class cFxSmoke;
class cFxHit;
class cFxTracks;

class IFxVisitor
{
public:
	virtual ~IFxVisitor() = default;

	virtual void visit (const cFxMuzzleBig&) = 0;
	virtual void visit (const cFxMuzzleMed&) = 0;
	virtual void visit (const cFxMuzzleMedLong&) = 0;
	virtual void visit (const cFxMuzzleSmall&) = 0;
	virtual void visit (const cFxExploAir&) = 0;
	virtual void visit (const cFxExploBig&) = 0;
	virtual void visit (const cFxExploSmall&) = 0;
	virtual void visit (const cFxExploWater&) = 0;
	virtual void visit (const cFxAbsorb&) = 0;
	virtual void visit (const cFxRocket&) = 0;
	virtual void visit (const cFxSmoke&) = 0;
	virtual void visit (const cFxCorpse&) = 0;
	virtual void visit (const cFxDarkSmoke&) = 0;
	virtual void visit (const cFxHit&) = 0;
	virtual void visit (const cFxTracks&) = 0;
};

class cFx
{
protected:
	cFx (bool bottom_, const cPosition& pixelPosition);

	cPosition position;
	int tick = 0;
	int length = -1;

public:
	virtual ~cFx() = default;

	virtual void accept (IFxVisitor&) const = 0;
	virtual bool isFinished() const;
	virtual void run();

	// TODO: Should not be pixel related
	const cPosition& getPixelPosition() const { return position; }

	int getTick() const { return tick; }
	int getLength() const { return length; }

	const bool bottom;
};

class cFxContainer
{
public:
	void push_back (std::shared_ptr<cFx> fx);
	void push_front (std::shared_ptr<cFx> fx);
	size_t size() const;
	void run();

private:
	std::vector<std::shared_ptr<cFx>> fxs;
};

class cFxMuzzle : public cFx
{
public:
	int getDir() const { return dir; }
	const sID& getId() const { return id; }

protected:
	cFxMuzzle (const cPosition& position, int dir_, sID id);

	int dir;
	const sID id;
};

class cFxMuzzleBig : public cFxMuzzle
{
public:
	cFxMuzzleBig (const cPosition& position, int dir, sID id);
	void accept (IFxVisitor& visitor) const override { visitor.visit (*this); }
};

class cFxMuzzleMed : public cFxMuzzle
{
public:
	cFxMuzzleMed (const cPosition& position, int dir, sID id);
	void accept (IFxVisitor& visitor) const override { visitor.visit (*this); }
};

class cFxMuzzleMedLong : public cFxMuzzle
{
public:
	cFxMuzzleMedLong (const cPosition& position, int dir, sID id);
	void accept (IFxVisitor& visitor) const override { visitor.visit (*this); }
};

class cFxMuzzleSmall : public cFxMuzzle
{
public:
	cFxMuzzleSmall (const cPosition& position, int dir, sID id);
	void accept (IFxVisitor& visitor) const override { visitor.visit (*this); }
};

class cFxExplo : public cFx
{
protected:
	cFxExplo (const cPosition& position, int frames_);

public:
	int getFrames() const { return frames; }

protected:
	// TODO: frames could be calculated (frames = w / h),
	// if the width and height of the graphics for one frame would be equal
	// (which they aren't yet).
	const int frames;
};

class cFxExploSmall : public cFxExplo
{
public:
	cFxExploSmall (const cPosition& position); // x, y is the center of the explosion
	void accept (IFxVisitor& visitor) const override { visitor.visit (*this); }
};

class cFxExploBig : public cFxExplo
{
public:
	cFxExploBig (const cPosition& position, bool onWater);
	void accept (IFxVisitor& visitor) const override { visitor.visit (*this); }

	bool isOnWater() const { return onWater; }

private:
	bool onWater;
};

class cFxExploAir : public cFxExplo
{
public:
	cFxExploAir (const cPosition& position);
	void accept (IFxVisitor& visitor) const override { visitor.visit (*this); }
};

class cFxExploWater : public cFxExplo
{
public:
	cFxExploWater (const cPosition& position);
	void accept (IFxVisitor& visitor) const override { visitor.visit (*this); }
};

class cFxHit : public cFxExplo
{
private:
	bool targetHit;
	bool big;

public:
	cFxHit (const cPosition& position, bool targetHit, bool big);
	void accept (IFxVisitor& visitor) const override { visitor.visit (*this); }

	bool isBig() const { return big; }
	bool isTargetHit() const { return targetHit; }
};

class cFxAbsorb : public cFxExplo
{
public:
	cFxAbsorb (const cPosition& position);
	void accept (IFxVisitor& visitor) const override { visitor.visit (*this); }
};

class cFxFade : public cFx
{
protected:
	cFxFade (const cPosition& position, bool bottom, int start, int end);

public:
	const int alphaStart;
	const int alphaEnd;
};

class cFxSmoke : public cFxFade
{
public:
	cFxSmoke (const cPosition& position, bool bottom); // x, y is the center of the effect
	void accept (IFxVisitor& visitor) const override { visitor.visit (*this); }
};

class cFxCorpse : public cFxFade
{
public:
	cFxCorpse (const cPosition& position);
	void accept (IFxVisitor& visitor) const override { visitor.visit (*this); }
};

class cFxTracks : public cFx
{
public:
	static constexpr int alphaStart = 100;
	static constexpr int alphaEnd = 0;
	const int dir;

public:
	cFxTracks (const cPosition& position, int dir_);
	void accept (IFxVisitor& visitor) const override { visitor.visit (*this); }
};

class cFxRocket : public cFx
{
private:
	static constexpr int speed = 8;
	std::vector<std::unique_ptr<cFx>> subEffects;
	int dir;
	int distance;
	const cPosition startPosition;
	const cPosition endPosition;
	const sID id;

public:
	cFxRocket (const cPosition& startPosition, const cPosition& endPosition, int dir_, bool bottom, sID id);
	void accept (IFxVisitor& visitor) const override { visitor.visit (*this); }
	void run() override;
	// return true, when the last smoke effect is finished.
	// getLength() returns only the time until
	// the rocket has reached the destination
	bool isFinished() const override;
	const sID getId() const { return id; }
	int getDir() const { return dir; }
	const std::vector<std::unique_ptr<cFx>>& getSubEffects() const { return subEffects; }
};

class cFxDarkSmoke : public cFx
{
private:
	float dx;
	float dy;

public:
	const int alphaStart;
	static constexpr int alphaEnd = 0;
	static constexpr int frames = 50;

public:
	cFxDarkSmoke (const cPosition& position, int alpha, float windDir);
	void accept (IFxVisitor& visitor) const override { visitor.visit (*this); }

	float getDx() const { return dx; }
	float getDy() const { return dy; }
};

#endif // game_logic_fxeffectsH
