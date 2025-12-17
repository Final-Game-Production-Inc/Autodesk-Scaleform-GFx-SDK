#ifndef INC_SF_GFX_Gesture_H
#define INC_SF_GFX_Gesture_H

#include "GFxConfig.h"
#ifdef GFX_GESTURE_RECOGNIZE

//#include <map>
#include "Kernel/SF_Types.h"
#include "Kernel/SF_Array.h"
#include "GFx_Types.h"
#include "GFx_Player.h"

#include "Platform/Platform.h"

namespace Scaleform { namespace GFx {

// declarations
class Gesture;
class MovieImpl;

//////////////////////////////////////////
// GFx gesture recognizer
	
class TouchPoint
{
public:
	TouchPoint(UInt32 id,float x,float y);

	bool active;
	UInt32 id;
	float x;
	float y;
	};

class GestureRecognizer : public Platform::GestureManager
{
public:
	GestureRecognizer();
	GestureRecognizer(class Platform::AppBase& app):GestureManager(app) { }
	
	void setup();
	void update();
	void reset();

	void ProcessDown(UInt32 id, const Point<int>& screenPt, const PointF& moviePt);
	void ProcessUp(UInt32 id, const Point<int>& screenPt, const PointF& moviePt);
	void ProcessMove(UInt32 id, const Point<int>& screenPt, const PointF& moviePt);

	void SetMovie(Ptr<Scaleform::GFx::Movie> moviePtr);

	TouchPoint* getTouchPointById(UInt32 id);
		
	Gesture* supportedGestures[6];
	Array<TouchPoint*> touchPointArr;

	int numTouches;
	int numSupportedGestures;
	UInt32 oldTime;
	float deltaSeconds;

	float DPI;
	float minSwipeDist;
	float deltaInches;

	float OldCentroidX;
	float OldCentroidY;
	float NewCentroidX;
	float NewCentroidY;

	Gesture *selected;
	Ptr<Movie> pMovie;

	bool doingComplex;
	
};

class Gesture
{
public:
	enum GesturePhase
	{
		Phase_None,
		Phase_Update
	};

	Gesture();
	virtual void update();
	void reset();

	GestureRecognizer* parent;
	GesturePhase currentPhase;

	UInt32 lastTime;
	float timeCounter;
	bool started;
	bool recognized;
};

// pan
class GesturePan:public Gesture
{
public:
	GesturePan();
	virtual void update();

	float startDist;
	float startCentroidX;
	float startCentroidY;
	float totalCentroidOffsetX;
	float totalCentroidOffsetY;
};

// rotate
class GestureRotate:public Gesture
{
public:
	GestureRotate();
	virtual void update();

	float lastCoordsX[2];
	float lastCoordsY[2];
	float totalAngle;
};

// zoom
class GestureZoom:public Gesture
{
public:
	GestureZoom();
	virtual void update();
	float startDist;
	float startDistX;
	float startDistY;
	float lastDist;
	float lastDistX;
	float lastDistY;
};

// swipe
class GestureSwipe:public Gesture
{
public:
	GestureSwipe();
	virtual void update();
	float startPosX;
	float startPosY;

};

// press and tap
class GesturePressAndTap:public Gesture
{
public:
	GesturePressAndTap();
	virtual void update();

	bool tapping;
};

// two finger tap
class GestureTwoFingerTap:public Gesture
{
public:
	GestureTwoFingerTap();
	virtual void update();

	float lastCentroidX;
	float lastCentroidY;
};


}} // namespace Scaleform::GFx


#endif // GFX_GESTURE_RECOGNIZE
#endif // INC_SF_GFX_Gesture_H
