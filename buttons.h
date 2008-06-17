#ifndef BUTTONS_H
#define BUTTONS_H

#include "sound.h"


class SmallButton
{
	public:
		SmallButton(int const x, int const y, char const* const text, sSOUND* sound = SoundData.SNDMenuButton) :
			x_(x),
			y_(y),
			text_(text),
			sound_(sound),
			down_(false)
		{}

		void Draw(bool down = false) const;

		bool CheckClick(int x, int y, bool down, bool up);

	private:
		int         x_;
		int         y_;
		char const* text_;
		sSOUND*     sound_;
		bool        down_;
};

#endif
