#ifndef BUTTONS_H
#define BUTTONS_H

#include "sound.h"


class Button
{
	public:
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

		virtual SDL_Rect const& GfxRect() const = 0;

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
		SDL_Rect const& GfxRect() const;
};


class SmallButton : public Button
{
	public:
		SmallButton(int const x, int const y, char const* const text) : Button(x, y, text) {}
		SDL_Rect const& GfxRect() const;
};


class SmallButtonHUD : public SmallButton
{
	public:
		SmallButtonHUD(int const x, int const y, char const* const text) : SmallButton(x, y, text) {}
		sSOUND* Sound() const;
};

#endif
