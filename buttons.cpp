#include <SDL_video.h>

#include "buttons.h"
#include "fonts.h"
#include "main.h"
#include "mouse.h"


void Button::Draw(bool const down) const
{
	SDL_Rect const& r   = GfxRect();
	SDL_Rect        src = r;
	if (down || locked_) src.y += 30;
	SDL_Rect dst = { x_, y_, 0, 0 };
	SDL_BlitSurface(GraphicsData.gfx_menu_stuff, &src, buffer, &dst);
	font->showTextCentered(x_ + r.w / 2, y_ + 7, lngPack.i18n(text_), LATIN_BIG);
}


bool Button::CheckClick(int const x, int const y, bool const down, bool const up)
{
	if (locked_) return false;

	SDL_Rect const& r = GfxRect();
	if (x_ <= x && x < x_ + r.w &&
			y_ <= y && y < y_ + r.h)
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
				PlayFX(Sound());
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


sSOUND* Button::Sound() const { return SoundData.SNDMenuButton; }


SDL_Rect const& MenuButton::GfxRect() const
{
	static SDL_Rect r = { 0, 0, 200, 29 };
	return r;
}


SDL_Rect const& SmallButton::GfxRect() const
{
	static SDL_Rect r = { 0, 60, 150, 29 };
	return r;
}


sSOUND* SmallButtonHUD::Sound() const { return SoundData.SNDHudButton; }
