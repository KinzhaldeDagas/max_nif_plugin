
/**********************************************************************
 *<
	FILE: templt.h

	DESCRIPTION:  Defines 2D Template Object

	CREATED BY: Tom Hudson

	HISTORY: created 31 October 1995

 *>	Copyright (c) 1995, All Rights Reserved.
 **********************************************************************/

#pragma once
#include "maxheap.h"
#include "CoreExport.h"
#include <WTypes.h>
#include <Geom/point2.h>
#include <Geom/point3.h>
#include <Geom/box3.h>
// forward declarations
class PolyLine;
class Spline3D;

// Intersection callbacks

class IntersectionCallback2D : public MaxHeapOperators
{
public:
	virtual ~IntersectionCallback2D()
	{
	}
	/*! \remarks This callback may be provided to an intersection-testing
	function in order to receive information on shape intersections. When
	an intersection is found, the function will call this method with the
	2D location of the intersection ('p') along with the indices of the
	two shape segments which intersect.
	\return TRUE to continue finding intersections; FALSE to stop
	intersect tests.*/
	virtual BOOL Intersect(Point2 p, int piece1, int piece2) = 0;
};

class IntersectionCallback3D : public MaxHeapOperators
{
public:
	virtual ~IntersectionCallback3D()
	{
	}
	/*! \remarks This callback may be provided to an intersection-testing
	function in order to receive information on shape intersections. When
	an intersection in 2D space (ignoring the Z component) is found, the
	function will call this method with the 3D location of the intersection
	('p') along with the indices of the two shape segments which intersect.
	\return TRUE to continue finding intersections; FALSE to stop
	intersect tests.*/
	virtual BOOL Intersect(Point3 p, int piece1, int piece2) = 0;
};

// A handy 2D floating-point box class

/*! \sa  Class Point2.\n\n
\par Description:
A 2D floating-point box class. This class has methods and operators to clear
the box, and update its size (bounding rectangle) by specifying additional
points. All methods of this class are implemented by the system.
\par Data Members:
<b>BOOL empty;</b>\n\n
Indicates if the box is empty. When the += operator is used to update the size
of the box, if the box is empty, the box corners are set to the point.\n\n
<b>Point2 min, max;</b>\n\n
The corners of the 2D box.  */
class Box2D : public MaxHeapOperators
{
public:
	BOOL empty;
	Point2 min, max;
	/*! \remarks Constructor. The box is set to empty initially. */
	Box2D()
	{
		empty = TRUE;
	}
	/*! \remarks Sets the box to an empty status.
	\par Operators:
	*/
	void SetEmpty()
	{
		empty = TRUE;
	}
	/*! \remarks Expand this box to include <b>p</b>. If this box is
	empty, the box corners are set to the point <b>p</b>.
	\par Parameters:
	<b>const Point2\& p</b>\n\n
	This box is expanded to include <b>p</b>. */
	CoreExport Box2D& operator+=(const Point2& p); // expand this box to include p
};

// This object is used to test shapes for self-intersection, clockwise status, point
// surrounding and intersection with other templates.  The last and first points will be the
// same if it is closed.

class Template3D;

/*! \sa  Class PolyShape, Class Box2D, Class Spline3D, Class PolyLine.\n\n
\par Description:
This class defines a 2D template object. This object is used to test shapes for
self-intersection, clockwise status, point surrounding and intersection with
other templates. The last and first points will be the same if it is closed.
All methods of this class are implemented by the system.\n\n
<b>Note:</b> Developers should normally use the PolyShape class instead of this
class. It provides methods for the same purposes; the Template class additionally
provides the ability to optionally use more precise double-precision math in the
SurroundsPoint(), SelfIntersects() and Intersects() methods.
\par Data Members:
<b>int points;</b>\n\n
The number of points in the template.\n\n
<b>BOOL closed;</b>\n\n
Indicates if the template is closed.\n\n
<b>Point2 *pts;</b>\n\n
The template points.  */
class Template : public MaxHeapOperators
{
public:
	int points;
	BOOL closed;
	Point2* pts;
	/*! \remarks Constructor.
	\par Parameters:
	<b>Spline3D *spline</b>\n\n
	Builds the template from this spline. */
	CoreExport Template(Spline3D* spline);
	/*! \remarks Constructor.
	\par Parameters:
	<b>PolyLine *line</b>\n\n
	Builds the template from this polyline. */
	CoreExport Template(PolyLine* line);
	CoreExport Template(Template3D* t3);
	/*! \remarks Updates the template with the data from the specified
	<b>PolyLine</b>.
	\par Parameters:
	<b>PolyLine *line</b>\n\n
	Builds the template from this polyline. */
	CoreExport void Create(PolyLine* line);
	/*! \remarks Destructor. */
	CoreExport ~Template();
	/*! \remarks Returns the number of points. */
	int Points()
	{
		return points;
	}
	/*! \remarks Returns TRUE if the specified point is surrounded
	(contained within) this Template. (Legacy single-precision version)
	\par Parameters:
	<b>Point2\& point</b>\n\n
	The point to check. */
	CoreExport BOOL SurroundsPoint(Point2& point);
	/*! \remarks Returns TRUE if the specified point is surrounded
	(contained within) this Template, optionally using double-precision
	math for improved accuracy.
	\par Parameters:
	<b>BOOL useDoubleMath</b>\n\n
	When TRUE, uses double-precision math for calculations for
	more precise results, at some additional computational cost.\n\n
	<b>Point2\& point</b>\n\n
	The point to check. */
	CoreExport BOOL SurroundsPoint(BOOL useDoubleMath, Point2& point);
	/*! \remarks Returns TRUE if the Template is clockwise in the XY plane
	(it ignores Z); otherwise FALSE. If the Template self intersects, the
	results from this method are meaningless. */
	CoreExport BOOL IsClockWise();
	/*! \remarks Returns TRUE if the Template intersects itself in the XY
	plane (it ignores Z); otherwise FALSE. (Legacy single-precision version)
	\par Parameters:
	<b>BOOL findAll = FALSE</b>\n\n
	TRUE to find all self intersections. FALSE to find only the first self
	intersection.\n\n
	<b>IntersectionCallback2D *cb = NULL</b>\n\n
	A pointer to an IntersectionCallback2D class. */
	CoreExport BOOL SelfIntersects(
			BOOL findAll = FALSE, IntersectionCallback2D* cb = NULL) const;
	/*! \remarks Returns TRUE if the Template intersects itself in the XY
	plane (it ignores Z); otherwise FALSE, optionally using double-precision
	math for improved accuracy.
	\par Parameters:
	<b>BOOL useDoubleMath</b>\n\n
	When TRUE, uses double-precision math for intersection calculations for
	more precise results, at some additional computational cost.\n\n
	<b>BOOL findAll = FALSE</b>\n\n
	TRUE to find all self intersections. FALSE to find only the first self
	intersection.\n\n
	<b>IntersectionCallback2D *cb = NULL</b>\n\n
	A pointer to an IntersectionCallback2D class. */
	CoreExport BOOL SelfIntersects(BOOL useDoubleMath, BOOL findAll = FALSE, IntersectionCallback2D* cb = NULL) const;
	/*! \remarks Returns TRUE if the specified Template intersects this
	Template in the XY plane (it ignores Z); otherwise FALSE. (Legacy
	single-precision version)
	\par Parameters:
	<b>Template \&t</b>\n\n
	The Template to check.\n\n
	<b>BOOL findAll = FALSE</b>\n\n
	TRUE to find all self intersections. FALSE to find only the first self
	intersection.\n\n
	<b>IntersectionCallback2D *cb = NULL</b>\n\n
	A pointer to an IntersectionCallback2D class. */
	CoreExport BOOL Intersects(
			Template& t, BOOL findAll = FALSE, IntersectionCallback2D* cb = NULL);
	/*! \remarks Returns TRUE if the specified Template intersects this
	Template in the XY plane (it ignores Z); otherwise FALSE, optionally using
	double-precision math for improved accuracy.
	\par Parameters:
	<b>BOOL useDoubleMath</b>\n\n
	When TRUE, uses double-precision math for intersection calculations for
	more precise results, at some additional computational cost.\n\n
	<b>Template \&t</b>\n\n
	The Template to check.\n\n
	<b>BOOL findAll = FALSE</b>\n\n
	TRUE to find all self intersections. FALSE to find only the first self
	intersection.\n\n
	<b>IntersectionCallback2D *cb = NULL</b>\n\n
	A pointer to an IntersectionCallback2D class. */
	CoreExport BOOL Intersects(
			BOOL useDoubleMath, Template& t, BOOL findAll = FALSE, IntersectionCallback2D* cb = NULL);
	/*! \remarks Returns an instance of the <b>Box2D</b> class that
	contains two corner points defining the bounding box size of this
	template. */
	CoreExport Box2D Bound();
};

// This is a version of the above for 3D use -- the various tests (SurroundsPoint,
// SelfIntersects, etc. are all performed on the X and Y coordinates only,
// discarding Z.  The IntersectionCallback returns the intersection point on the
// template in 3D.
//
// As with the Template class, the SurroundsPoint(), SelfIntersects() and Intersects()
// methods optionally allow the tests to be performed using double-precision math.

class Template3D : public MaxHeapOperators
{
private:
	Template* template2D;

public:
	int points;
	BOOL closed;
	Point3* pts;
	CoreExport Template3D(Spline3D* spline);
	CoreExport Template3D(PolyLine* line);
	CoreExport void Create(PolyLine* line);
	CoreExport ~Template3D();
	int Points()
	{
		return points;
	}
	CoreExport BOOL SurroundsPoint(Point2& point); // 2D test!
	// Version of the above which optionally performs the check with double-precision math */
	CoreExport BOOL SurroundsPoint(BOOL useDoubleMath, Point2& point); // 2D test!
	CoreExport BOOL IsClockWise(); // 2D test!
	CoreExport BOOL SelfIntersects(BOOL findAll = FALSE, IntersectionCallback3D* cb = NULL) const; // 2D test!
	// Version of the above which optionally performs the check with double-precision math */
	CoreExport BOOL SelfIntersects(
			BOOL useDoubleMath, BOOL findAll = FALSE, IntersectionCallback3D* cb = NULL) const; // 2D test!
	CoreExport BOOL Intersects(Template3D& t, BOOL findAll = FALSE, IntersectionCallback3D* cb = NULL); // 2D test!
	// Version of the above which optionally performs the check with double-precision math */
	CoreExport BOOL Intersects(
			BOOL useDoubleMath, Template3D& t, BOOL findAll = FALSE, IntersectionCallback3D* cb = NULL); // 2D test!
	CoreExport Box2D Bound();
	CoreExport Box3 Bound3D();
	CoreExport void Ready2DTemplate();
};
