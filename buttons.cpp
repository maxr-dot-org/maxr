#include <SDL_video.h>

#include "buttons.h"
#include "main.h"
#include "mouse.h"


void Button::Draw(bool const down) const
{
	Button::Gfx const& gfx = down || locked_ ? GfxDown() : GfxUp();
	SDL_Rect           src = gfx.rect;
	SDL_Rect           dst = { x_, y_, 0, 0 };
	SDL_BlitSurface(gfx.surface, &src, buffer, &dst);
	FontInfo           fi  = Font();
	font->showTextCentered(x_ + fi.off_x + gfx.rect.w / 2, y_ + fi.off_y, lngPack.i18n(text_), fi.font);
}


bool Button::CheckClick(int const x, int const y, bool const down, bool const up)
{
	if (locked_) return false;

	SDL_Rect const& r = GfxUp().rect;
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


Button::FontInfo const& Button::Font() const
{
	static FontInfo const fi = { FONT_LATIN_BIG, 0, 7 };
	return fi;
}


sSOUND* Button::Sound() const { return SoundData.SNDMenuButton; }


Button::Gfx const& MenuButton::GfxUp() const
{
	static Gfx const gfx = { GraphicsData.gfx_menu_stuff, { 0, 0, 200, 29 } };
	return gfx;
}


Button::Gfx const& MenuButton::GfxDown() const
{
	static Gfx const gfx = { GraphicsData.gfx_menu_stuff, { 0, 30, 200, 29 } };
	return gfx;
}


Button::Gfx const& SmallButton::GfxUp() const
{
	static Gfx const gfx = { GraphicsData.gfx_menu_stuff, { 0, 60, 150, 29 } };
	return gfx;
}


Button::Gfx const& SmallButton::GfxDown() const
{
	static Gfx const gfx = { GraphicsData.gfx_menu_stuff, { 0, 90, 150, 29 } };
	return gfx;
}


sSOUND* SmallButtonHUD::Sound() const { return SoundData.SNDHudButton; }


Button::FontInfo const& NormalButton::Font() const
{
	static FontInfo const fi = { FONT_LATIN_NORMAL, 0, 4 };
	return fi;
}


Button::Gfx const& NormalButton::GfxUp() const
{
	static Gfx const gfx = { GraphicsData.gfx_hud_stuff, { 308, 455, 77, 23 } };
	return gfx;
}


Button::Gfx const& NormalButton::GfxDown() const
{
	static Gfx const gfx = { GraphicsData.gfx_hud_stuff, { 230, 455, 77, 23 } };
	return gfx;
}


Button::FontInfo const& BigButton::Font() const
{
	static FontInfo const fi = { FONT_LATIN_BIG, 0, 11 };
	return fi;
}


Button::Gfx const& BigButton::GfxUp() const
{
	static Gfx const gfx = { GraphicsData.gfx_hud_stuff, { 0, 370, 106, 40 } };
	return gfx;
}


Button::Gfx const& BigButton::GfxDown() const
{
	static Gfx const gfx = { GraphicsData.gfx_hud_stuff, { 0, 411, 106, 40 } };
	return gfx;
}
