#ifndef BUTTONS_H
#define BUTTONS_H

#include "fonts.h"
#include "sound.h"


class Button
{
	public:
		struct Gfx
		{
			SDL_Surface* surface;
			SDL_Rect     rect;
		};

		struct FontInfo
		{
			eBitmapFontType font;
			int             off_x;
			int             off_y;
		};

		Button(int const x, int const y, char const* const text) :
			x_(x),
			y_(y),
			text_(text),
			down_(false),
			locked_(false)
		{}

		void Lock()   { locked_ = true;  Draw(); }
		void Unlock() { locked_ = false; Draw(); }

		void Draw(bool down = false) const;

		bool CheckClick(int x, int y, bool down, bool up);

		virtual Gfx const& GfxUp()   const = 0;
		virtual Gfx const& GfxDown() const = 0;

		virtual FontInfo const& Font() const;

		virtual sSOUND* Sound() const;

	private:
		int         x_;
		int         y_;
		char const* text_;
		bool        down_:1;
		bool        locked_:1;
};


class MenuButton : public Button
{
	public:
		MenuButton(int const x, int const y, char const* const text) : Button(x, y, text) {}
		Gfx const& GfxUp()   const;
		Gfx const& GfxDown() const;
};


class SmallButton : public Button
{
	public:
		SmallButton(int const x, int const y, char const* const text) : Button(x, y, text) {}
		Gfx const& GfxUp()   const;
		Gfx const& GfxDown() const;
};


class SmallButtonHUD : public SmallButton
{
	public:
		SmallButtonHUD(int const x, int const y, char const* const text) : SmallButton(x, y, text) {}
		sSOUND* Sound() const;
};


class NormalButton : public Button
{
	public:
		NormalButton(int const x, int const y, char const* const text) : Button(x, y, text) {}
		FontInfo const& Font()    const;
		Gfx      const& GfxUp()   const;
		Gfx      const& GfxDown() const;
};

class BigButton : public Button
{
	public:
		BigButton(int const x, int const y, char const* const text) : Button(x, y, text) {}
		FontInfo const& Font()    const;
		Gfx      const& GfxUp()   const;
		Gfx      const& GfxDown() const;
};

#endif
