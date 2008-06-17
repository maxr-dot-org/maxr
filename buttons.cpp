#include "buttons.h"
#include "fonts.h"
#include "main.h"
#include "mouse.h"


void SmallButton::Draw(bool const down) const
{
	SDL_Rect src = { 0, down ? 90 : 60, 150, 29 };
	SDL_Rect dst = { x_, y_, 0, 0 };
	SDL_BlitSurface(GraphicsData.gfx_menu_stuff, &src, buffer, &dst);
	font->showTextCentered(x_ + 150 / 2, y_ + 7, lngPack.i18n(text_), LATIN_BIG);
}


bool SmallButton::CheckClick(int const x, int const y, bool const down, bool const up)
{
	if (x_ <= x && x < x_ + 150 &&
			y_ <= y && y < y_ +  29)
	{
		if (down_)
		{
			if (up)
			{
				down_ = false;
				Draw(false);
				return true;
			}
		}
		else
		{
			if (down)
			{
				down_ = true;
				PlayFX(sound_);
				Draw(true);
				SHOW_SCREEN
				mouse->draw(false, screen);
			}
		}
	}
	else
	{
		if (down_)
		{
			down_ = false;
			Draw(false);
			SHOW_SCREEN
			mouse->draw(false, screen);
		}
	}
	return false;
}
