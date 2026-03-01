/**********************************************************************
 *<
	FILE: box2.h

	DESCRIPTION:

	CREATED BY: Dan Silva

	HISTORY:

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/
#pragma once
#include "ipoint2.h"
#include "point2.h"
#include "Platform.h"

#include <Geom/GeomExport.h>
#include <cmath>
#include <cstdint>
#include <type_traits>

#if defined(GEOM_WINDOWS)
#include <wtypes.h>
#endif

// We use this base class on other platforms.
// When building on windows, we confirm it is identical.
namespace geom_detail {
struct UnixRect
{
	int32_t left;
	int32_t top;
	int32_t right;
	int32_t bottom;
};

// On windows, select the appropriate RECT base class but also check abi.
#if defined(GEOM_WINDOWS)
static_assert(sizeof(RECT) == sizeof(UnixRect), "Geom/box2.h : UnixRect ABI mismatch");
static_assert(alignof(RECT) == alignof(UnixRect), "Geom/box2.h : UnixRect ABI mismatch");
static_assert(std::is_signed<decltype(RECT::left)>::value == std::is_signed<decltype(UnixRect::left)>::value,
		"Geom/box2.h : UnixRect ABI mismatch");

using BaseRect = RECT;
#else
// Use UnixRect equivalent of windows RECT.
using BaseRect = UnixRect;
#endif
} // namespace detail


/*! \sa  Class IPoint2.\n\n
\par Description:
This class describes a 2D rectangular region using integer coordinates. This
class is sub-classed from RECT (from the Windows API). Box2 provides methods
that return individual coordinates of the box, scale and translate it, retrieve
its center, modify its size, expand it to include points or other boxes, and
determine if points are inside the box. All methods are implemented by the
system.  */
class Box2
		: public geom_detail::BaseRect
{
	// Warning - instances of this class are saved as a binary blob to scene file
	// Adding/removing members will break file i/o
public:
	/*! \remarks Constructs a Box2 object. The box is initialized such that it
	is 'empty'. See <b>IsEmpty()</b> below. */
	GEOM_EXPORT Box2();
	/*! \remarks Constructs a Box2 object from the specified corners.
	\par Parameters:
	<b>const IPoint2 a</b>\n\n
	The upper left corner of the box.\n\n
	<b>const IPoint2 b</b>\n\n
	The lower right corner of the box. */
	GEOM_EXPORT Box2(const IPoint2& a, const IPoint2& b);
	/*! \remarks Determines whether the box has been 'Set Empty' (see below).
	When a box is created using the default constructor it is set to 'empty'.
	\return  true if the box is empty; false otherwise. */
	GEOM_EXPORT bool IsEmpty();
	/*! \remarks Sets the box to 'empty'. This indicates the box has not had
	specific values set by the developer. */
	GEOM_EXPORT void SetEmpty();
	/*! \remarks Adjusts the coordinates of the box such that
	<b>top\<bottom</b> and <b>left\<right</b>. */
	GEOM_EXPORT void Rectify(); // makes top<bottom, left<right
	/*! \remarks Scales the coordinates of the box about the center of the
	box.
	\par Parameters:
	<b>float f</b>\n\n
	Specifies the scale factor. */
	GEOM_EXPORT void Scale(float f);
	/*! \remarks Translate the box by the distance specified.
	\par Parameters:
	<b>IPoint2 t</b>\n\n
	The distance to translate the box. */
	GEOM_EXPORT void Translate(IPoint2 t);

	/*! \remarks Returns the center of the box (the midpoint between the box
	corners). */
	IPoint2 GetCenter() const
	{
		return IPoint2((left + right) / 2, (top + bottom) / 2);
	}
	/*! \remarks Returns the minimum x coordinate of the box. */
	int x() const
	{
		return ((left < right) ? left : right);
	}
	/*! \remarks Returns the minimum y coordinate. */
	int y() const
	{
		return ((top < bottom) ? top : bottom);
	}
	/*! \remarks Returns the width of the box. */
	int w() const
	{
		return abs(right - left) + 1;
	}
	/*! \remarks Returns the height of the box. */
	int h() const
	{
		return abs(bottom - top) + 1;
	}

	/*! \remarks Sets the box width to the width specified. The 'right'
	coordinate is adjusted such that:\n\n
	<b>right = left + w -1</b>
	\par Parameters:
	<b>int w</b>\n\n
	The new width for the box. */
	void SetW(int w)
	{
		right = left + w - 1;
	}
	/*! \remarks Sets the height of the box to the height specified. The
	'bottom' coordinate is adjusted such that:\n\n
	<b>bottom = top + h -1;</b>
	\par Parameters:
	<b>int h</b>\n\n
	The new height for the box. */
	void SetH(int h)
	{
		bottom = top + h - 1;
	}
	/*! \remarks Sets the left coordinate of the box to x.
	\par Parameters:
	<b>int x</b>\n\n
	The new value for the left coordinate. */
	void SetX(int x)
	{
		left = x;
	}
	/*! \remarks Set the top coordinate to y.
	\par Parameters:
	<b>int y</b>\n\n
	The new value for the top coordinate. */
	void SetY(int y)
	{
		top = y;
	}
	/*! \remarks Sets both the width and height of the box.
	\par Parameters:
	<b>int w</b>\n\n
	The new width for the box.\n\n
	<b>int h</b>\n\n
	The new height of the box. */
	void SetWH(int w, int h)
	{
		SetW(w);
		SetH(h);
	}
	/*! \remarks Sets both the left and top coordinates of the box.
	\par Parameters:
	<b>int x</b>\n\n
	The new left coordinate.\n\n
	<b>int y</b>\n\n
	The new top coordinate. */
	void SetXY(int x, int y)
	{
		SetX(x);
		SetY(y);
	}

	/*! \remarks Assignment operators. Copies the specified source RECT into this
	Box2 object.
	*/
	GEOM_EXPORT Box2& operator=(const geom_detail::BaseRect& r);
	/*! \remarks Assignment operators. Copies the specified source RECT into this
	Box2 object.
	*/
	GEOM_EXPORT Box2& operator=(geom_detail::BaseRect& r);

	/*! \remarks Expands this <b>Box2</b> to completely include box <b>b</b>.
	 */
	GEOM_EXPORT Box2& operator+=(const Box2& b);
	/*! \remarks Expands this <b>Box2</b> to include point <b>p</b>. */
	GEOM_EXPORT Box2& operator+=(const IPoint2& p);
	/*! \remarks Equality operator. Determines whether b is equal to this box.
	Returns true if the boxes are equal; false otherwise. */
	bool operator==(const Box2& b) const
	{
		return (left == b.left && right == b.right && top == b.top && bottom == b.bottom);
	}
	/*! \remarks Inequality operator. Determines whether b is different from this box.
	Returns true if the boxes are different. */
	bool operator!=(const Box2& b) const
	{
		return !(operator==(b));
	}
	/*! \remarks Determines if the point passed is contained within the box.
	Returns true if the point is inside the box; otherwise false.
	*/
	GEOM_EXPORT bool Contains(const IPoint2& p) const; // is point in this box?
	/*! \remarks Intersection test between a line and a rectangle. Returns true if they intersect.
	 * \par Parameters:
	<b>int x0</b>\n\n
	Line start x coordinate.\n\n
	<b>int y0</b>\n\n
	Line start y coordinate.
	<b>int x1</b>\n\n
	Line end x coordinate.\n\n
	<b>int y1</b>\n\n
	Line end y coordinate.\n\n */
	GEOM_EXPORT bool IntersectsLine(int x0, int y0, int x1, int y1) const;
};

typedef Box2 Rect;


struct FBox2
{
	// Warning - instances of this struct are saved as a binary blob to scene file
	// Adding/removing members will break file i/o
	Point2 pmin;
	Point2 pmax;
	bool IsEmpty()
	{
		return pmin.x > pmax.x ? 1 : 0;
	}
	void SetEmpty()
	{
		pmin = Point2(1E30, 1E30);
		pmax = -pmin;
	}
	FBox2& operator=(const FBox2& r)
	{
		pmin = r.pmin;
		pmax = r.pmax;
		return *this;
	}
	GEOM_EXPORT FBox2& operator+=(const Point2& p);
	GEOM_EXPORT FBox2& operator+=(const FBox2& b);
	GEOM_EXPORT bool Contains(const Point2& p) const; // is point in this box?
};
