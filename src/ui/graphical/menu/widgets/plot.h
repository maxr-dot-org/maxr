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

#ifndef ui_graphical_menu_widgets_plotH
#define ui_graphical_menu_widgets_plotH

#include <functional>
#include <list>

#include "maxrconfig.h"
#include "ui/graphical/widget.h"
#include "utility/color.h"
#include "utility/drawing.h"
#include "utility/string/toString.h"

/**
 *
 * @tparam T
 */
template<typename T>
class cMarker
{
public:
	explicit cMarker (T value);

	void setLabel (std::string label);
	const std::string& getLabel() const;

	void setValue (T value);
	T getValue() const;

	void setColor (cRgbColor color);
	const cRgbColor& getColor() const;
private:
	T value;

	std::string label;

	cRgbColor color;
};

//------------------------------------------------------------------------------
template<typename T>
cMarker<T>::cMarker (T value_) :
	value (value_)
{}

//------------------------------------------------------------------------------
template<typename T>
void cMarker<T>::setLabel (std::string label_)
{
	label = std::move (label_);
}

//------------------------------------------------------------------------------
template<typename T>
const std::string& cMarker<T>::getLabel() const
{
	return label;
}

//------------------------------------------------------------------------------
template<typename T>
void cMarker<T>::setValue (T value_)
{
	value = value_;
}

//------------------------------------------------------------------------------
template<typename T>
T cMarker<T>::getValue() const
{
	return value;
}

//------------------------------------------------------------------------------
template<typename T>
void cMarker<T>::setColor (cRgbColor color_)
{
	color = std::move (color_);
}

//------------------------------------------------------------------------------
template<typename T>
const cRgbColor& cMarker<T>::getColor() const
{
	return color;
}

/**
 *
 * @tparam T
 */
template<typename T>
class cAxis
{
public:
	cAxis();

	void setLabel (std::string label);
	const std::string& getLabel() const;

	void setColor (cRgbColor color);
	const cRgbColor& getColor() const;

	void setInterval (T min, T max);

	void setMinValue (T value);
	T getMinValue() const;

	void setMaxValue (T value);
	T getMaxValue() const;
private:
	T minValue;
	T maxValue;

	std::string label;

	cRgbColor color;
};

//------------------------------------------------------------------------------
template<typename T>
cAxis<T>::cAxis() :
	minValue (0),
	maxValue (0)
{}

//------------------------------------------------------------------------------
template<typename T>
void cAxis<T>::setLabel (std::string label_)
{
	label = std::move (label_);
}

//------------------------------------------------------------------------------
template<typename T>
const std::string& cAxis<T>::getLabel() const
{
	return label;
}

//------------------------------------------------------------------------------
template<typename T>
void cAxis<T>::setColor (cRgbColor color_)
{
	color = std::move (color_);
}

//------------------------------------------------------------------------------
template<typename T>
const cRgbColor& cAxis<T>::getColor() const
{
	return color;
}

//------------------------------------------------------------------------------
template<typename T>
void cAxis<T>::setInterval (T min, T max)
{
	minValue = min;
	maxValue = max;
	assert (minValue <= maxValue);
}

//------------------------------------------------------------------------------
template<typename T>
void cAxis<T>::setMinValue (T value)
{
	minValue = value;
}

//------------------------------------------------------------------------------
template<typename T>
T cAxis<T>::getMinValue() const
{
	return minValue;
}

//------------------------------------------------------------------------------
template<typename T>
void cAxis<T>::setMaxValue (T value)
{
	maxValue = value;
}

//------------------------------------------------------------------------------
template<typename T>
T cAxis<T>::getMaxValue() const
{
	return maxValue;
}

/**
 *
 * @tparam T
 * @tparam U
 */
template<typename T, typename U>
class cGraph
{
public:
	explicit cGraph (std::function<U (T)> function);

	void setLabel (std::string label);
	const std::string& getLabel() const;

	void setColor (cRgbColor color);
	const cRgbColor& getColor() const;

	U evaluate (T x) const;
private:
	std::function<U (T)> function;

	std::string label;

	cRgbColor color;
};

//------------------------------------------------------------------------------
template<typename T, typename U>
cGraph<T, U>::cGraph (std::function<U (T)> function_) :
	function (function_)
{}

//------------------------------------------------------------------------------
template<typename T, typename U>
void cGraph<T, U>::setLabel (std::string label_)
{
	label = std::move (label_);
}

//------------------------------------------------------------------------------
template<typename T, typename U>
const std::string& cGraph<T, U>::getLabel() const
{
	return label;
}

//------------------------------------------------------------------------------
template<typename T, typename U>
void cGraph<T, U>::setColor (cRgbColor color_)
{
	color = std::move (color_);
}

//------------------------------------------------------------------------------
template<typename T, typename U>
const cRgbColor& cGraph<T, U>::getColor() const
{
	return color;
}

//------------------------------------------------------------------------------
template<typename T, typename U>
U cGraph<T, U>::evaluate (T x) const
{
	return function (x);
}

/**
 *
 * @tparam T
 * @tparam U
 */
template<typename T, typename U>
class cPlot : public cWidget
{
public:
	typedef cAxis<T> XAxisType;
	typedef cAxis<U> YAxisType;
	typedef cMarker<T> XMarkerType;
	typedef cMarker<U> YMarkerType;
	typedef cGraph<T, U> GraphType;

	explicit cPlot (const cBox<cPosition>& area);

	XAxisType& getXAxis();
	const XAxisType& getXAxis() const;

	YAxisType& getYAxis();
	const YAxisType& getYAxis() const;

	template<typename F>
	GraphType& addGraph (F function);

	template<typename F>
	GraphType& addGraph (std::string label, F function);

	XMarkerType& addXMarker (T value);
	YMarkerType& addYMarker (U value);

	void draw (SDL_Surface& destination, const cBox<cPosition>& clipRect) override;
private:
	const cPosition graphBeginMargin;
	const cPosition graphEndMargin;

	XAxisType xAxis;
	XAxisType yAxis;

	std::list<GraphType> graphs;

	std::list<XMarkerType> xMarkers;
	std::list<YMarkerType> yMarkers;

	T fromPixelX (int pixelX);
	U fromPixelY (int pixelY);

	int toPixelX (T x);
	int toPixelY (U y);
};

//------------------------------------------------------------------------------
template<typename T, typename U>
cPlot<T, U>::cPlot (const cBox<cPosition>& area) :
	cWidget (area),
	graphBeginMargin (20, 15),
	graphEndMargin (10, 10)
{
}

//------------------------------------------------------------------------------
template<typename T, typename U>
typename cPlot<T, U>::XAxisType& cPlot<T, U>::getXAxis()
{
	return xAxis;
}

//------------------------------------------------------------------------------
template<typename T, typename U>
const typename cPlot<T, U>::XAxisType& cPlot<T, U>::getXAxis() const
{
	return xAxis;
}

//------------------------------------------------------------------------------
template<typename T, typename U>
typename cPlot<T, U>::YAxisType& cPlot<T, U>::getYAxis()
{
	return yAxis;
}

//------------------------------------------------------------------------------
template<typename T, typename U>
const typename cPlot<T, U>::YAxisType& cPlot<T, U>::getYAxis() const
{
	return yAxis;
}

//------------------------------------------------------------------------------
template<typename T, typename U>
template<typename F>
typename cPlot<T, U>::GraphType& cPlot<T, U>::addGraph (F function)
{
	graphs.push_back (GraphType (function));
	auto& graph = graphs.back();
	return graph;
}

//------------------------------------------------------------------------------
template<typename T, typename U>
template<typename F>
typename cPlot<T, U>::GraphType& cPlot<T, U>::addGraph (std::string label, F function)
{
	graphs.push_back (GraphType (function));
	auto& graph = graphs.back();
	graph.setLabel (std::move (label));
	return graph;
}

//------------------------------------------------------------------------------
template<typename T, typename U>
typename cPlot<T, U>::XMarkerType&  cPlot<T, U>::addXMarker (T value)
{
	xMarkers.push_back (XMarkerType (value));
	return xMarkers.back();
}

//------------------------------------------------------------------------------
template<typename T, typename U>
typename cPlot<T, U>::YMarkerType&  cPlot<T, U>::addYMarker (U value)
{
	yMarkers.push_back (YMarkerType (value));
	return yMarkers.back();
}

//------------------------------------------------------------------------------
template<typename T, typename U>
T cPlot<T, U>::fromPixelX (int pixelX)
{
	const auto xPixelWidth = getSize().x() - graphBeginMargin.x() - graphEndMargin.x();

	return xAxis.getMinValue() + (xAxis.getMaxValue() - xAxis.getMinValue()) * pixelX / xPixelWidth;
}

//------------------------------------------------------------------------------
template<typename T, typename U>
U cPlot<T, U>::fromPixelY (int pixelY)
{
	const auto yPixelWidth = getSize().y() - graphBeginMargin.y() - graphEndMargin.y();

	return yAxis.getMinValue() + (yAxis.getMaxValue() - yAxis.getMinValue()) * pixelY / yPixelWidth;
}

//------------------------------------------------------------------------------
template<typename T, typename U>
int cPlot<T, U>::toPixelX (T x)
{
	const auto xPixelWidth = getSize().x() - graphBeginMargin.x() - graphEndMargin.x();

	return xPixelWidth * (x - xAxis.getMinValue()) / (xAxis.getMaxValue() - xAxis.getMinValue());
}

//------------------------------------------------------------------------------
template<typename T, typename U>
int cPlot<T, U>::toPixelY (U y)
{
	const auto yPixelWidth = getSize().y() - graphBeginMargin.y() - graphEndMargin.y();

	return yPixelWidth * (y - yAxis.getMinValue()) / (yAxis.getMaxValue() - yAxis.getMinValue());
}

//------------------------------------------------------------------------------
template<typename T, typename U>
void cPlot<T, U>::draw (SDL_Surface& destination, const cBox<cPosition>& clipRect)
{
	auto* font = cUnicodeFont::font.get();
	const cPosition origin (graphBeginMargin.x(), getSize().y() - graphBeginMargin.y());

	const auto xPixelWidth = getSize().x() - graphBeginMargin.x() - graphEndMargin.x();
	const auto yPixelWidth = getSize().y() - graphBeginMargin.y() - graphEndMargin.y();

	// draw graphs
	for (auto graph = graphs.cbegin(); graph != graphs.cend(); ++graph)
	{
		auto lastEvaluationPoint = fromPixelX (0);
		cPosition lastPoint (0, -toPixelY (graph->evaluate (lastEvaluationPoint)));

		for (int pixelX = 1; pixelX < xPixelWidth; ++pixelX)
		{
			const auto x = fromPixelX (pixelX);
			const auto y = graph->evaluate (x);
			const auto pixelY = toPixelY (y);

			const cPosition newPoint (pixelX, -pixelY);

			if (x != lastEvaluationPoint /*|| pixelX == xPixelWidth-1*/)
			{
				drawLine (&destination, getPosition() + origin + lastPoint, getPosition() + origin + newPoint, graph->getColor());

				lastPoint = newPoint;
			}

			lastEvaluationPoint = x;
		}
	}

	// draw axes
	drawLine (&destination, getPosition() + origin, getPosition() + origin + cPosition (xPixelWidth, 0), xAxis.getColor());

	font->showTextCentered (getPosition() + origin + cPosition (0, 2), iToStr (xAxis.getMinValue()), FONT_LATIN_SMALL_WHITE);
	font->showTextCentered (getPosition() + origin + cPosition (xPixelWidth, 2), iToStr (xAxis.getMaxValue()), FONT_LATIN_SMALL_WHITE);

	drawLine (&destination, getPosition() + origin, getPosition() + origin + cPosition (0, -yPixelWidth), yAxis.getColor());

	font->showText (getPosition() + origin + cPosition (-16, -font->getFontHeight (FONT_LATIN_SMALL_WHITE) / 2), iToStr (yAxis.getMinValue()), FONT_LATIN_SMALL_WHITE);
	font->showText (getPosition() + origin + cPosition (-16, -yPixelWidth - font->getFontHeight (FONT_LATIN_SMALL_WHITE) / 2), iToStr (yAxis.getMaxValue()), FONT_LATIN_SMALL_WHITE);

	// draw markers
	for (auto marker = xMarkers.begin(); marker != xMarkers.end(); ++marker)
	{
		if (marker->getValue() < xAxis.getMinValue() || marker->getValue() > xAxis.getMaxValue()) continue;

		const auto pixelX = toPixelX (marker->getValue());
		drawLine (&destination, getPosition() + origin + cPosition (pixelX, 0), getPosition() + origin + cPosition (pixelX, -yPixelWidth), marker->getColor());

		font->showTextCentered (getPosition() + origin + cPosition (pixelX, 2), iToStr (marker->getValue()), FONT_LATIN_SMALL_WHITE);
	}

	for (auto marker = yMarkers.begin(); marker != yMarkers.end(); ++marker)
	{
		if (marker->getValue() < yAxis.getMinValue() || marker->getValue() > yAxis.getMaxValue()) continue;

		const auto pixelY = toPixelY (marker->getValue());
		drawLine (&destination, getPosition() + origin + cPosition (0, -pixelY), getPosition() + origin + cPosition (xPixelWidth, -pixelY), marker->getColor());

		font->showText (getPosition() + origin + cPosition (-16, -pixelY - font->getFontHeight (FONT_LATIN_SMALL_WHITE) / 2), iToStr (marker->getValue()), FONT_LATIN_SMALL_WHITE);
	}
	cWidget::draw (destination, clipRect);
}

#endif // ui_graphical_menu_widgets_plotH
