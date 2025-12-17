#include "GFxConfig.h"
#ifdef GFX_GESTURE_RECOGNIZE

#include "GFx/GFx_Gesture.h"
#include "GFx/GFx_Input.h"
#include "GFx/GFx_PlayerImpl.h"
//#include <iostream>

namespace Scaleform { namespace GFx {


//////////////////////////////////////////////////////////////////////////
// Gesture-related utils

float Slope(float deltaX, float deltaY)
{
	return deltaY/deltaX;
}

float angleBetweenLines(float line1x, float line1y, float line1x2, float line1y2, float line2x, float line2y, float line2x2, float line2y2) 
{
	float a = line1x2 - line1x;
	float b = line1y2 - line1y;
	float c = line2x2 - line2x;
	float d = line2y2 - line2y;

	float atanA = atan2(a, b) ;
	float atanB = atan2(c, d);

	float angle = (atanA - atanB)*(180/3.14159265359f);

	// TODO: fix range mismatch

	return angle;
}

float AngleBetweenSlopes(float m1, float m2)
{
	return atan(abs( (m1 - m2)/(1 + m1*m2) ));
}


float Angle(float x1, float y1, float x2, float y2)
{
	float deltaY = y2 - y1;
	float deltaX = x2 - x1;

	float angleInDegrees = atan2(deltaY, deltaX) * 180 / 3.14159265359f;
	return angleInDegrees;
	}

float Dot(PointF v1, PointF v2)
{
	return (v1.x*v2.x + v1.y*v2.y);
}

float signedAngle(PointF v1, PointF v2)
{
	float perpDot = v1.x * v2.y - v1.y * v2.x;
	
	return (float)atan2(perpDot, Dot(v1, v2)) * 180 / 3.14159265359f;
}

float Distance(float x1, float y1, float x2, float y2)
{
	float dx = x2 - x1;
	float dy = y2 - y1;
	return sqrtf(dx * dx + dy * dy);
}

//////////////////////////////////////////////////////////////////////////
// Gesture-related class declarations

TouchPoint::TouchPoint(UInt32 id, float x, float y):id(id),x(x),y(y){}

void GestureRecognizer::setup()
{
	oldTime = Timer::GetTicksMs();
	DPI = 72;
	minSwipeDist = DPI*.65f;
	selected=0;

	supportedGestures[0] = new GestureRotate();
	supportedGestures[0]->parent = this;

	supportedGestures[1] = new GesturePan();
	supportedGestures[1]->parent = this;

	supportedGestures[2] = new GestureZoom();
	supportedGestures[2]->parent = this;

	supportedGestures[3] = new GestureSwipe();
	supportedGestures[3]->parent = this;

	supportedGestures[4] = new GesturePressAndTap();
	supportedGestures[4]->parent = this;

	supportedGestures[5] = new GestureTwoFingerTap();
	supportedGestures[5]->parent = this;

	numSupportedGestures = 6;

	doingComplex = false;

	numTouches=0;

	for(int a=0; a< 5;a++)
	{
		TouchPoint *tp = new TouchPoint(0, 0.0f, 0.0f);
		tp->active = false;
		touchPointArr.PushBack(tp);
	}
}

void GestureRecognizer::SetMovie(Ptr<Scaleform::GFx::Movie> moviePtr)
{
	pMovie = moviePtr;
}

TouchPoint *GestureRecognizer::getTouchPointById(UInt32 id)
{
	for(UPInt a=0;a<touchPointArr.GetSize();a++)
	{
		if(touchPointArr[a]->id==id)
			return touchPointArr[a];
	}
	
	return 0;
}

void GestureRecognizer::ProcessDown(UInt32 id, const Point<int>& screenPt, const PointF& moviePt)
{
	SF_UNUSED(moviePt);

	for(UPInt a=0;a<touchPointArr.GetSize();a++)
	{
		if(!touchPointArr[a]->active)
		{
			touchPointArr[a]->active = true;
			touchPointArr[a]->id = id;
			touchPointArr[a]->x = (float)screenPt.x;
			touchPointArr[a]->y = (float)screenPt.y;
			numTouches++;
			break;
		}
	}


	update();
}

void GestureRecognizer::ProcessUp(UInt32 id, const Point<int>& screenPt, const PointF& moviePt)
{
	SF_UNUSED(moviePt);

	for(UPInt a=0;a<touchPointArr.GetSize();a++)
	{
		if(touchPointArr[a]->id==id)
		{
			touchPointArr[a]->active = false;
			numTouches--;
			break;
		}
	}

	update();
}

void GestureRecognizer::ProcessMove(UInt32 id, const Point<int>& screenPt, const PointF& moviePt)
{
	SF_UNUSED(moviePt);

	TouchPoint* tp = getTouchPointById(id); 
	if(tp==0)
		return;

	tp->x = (float)screenPt.x;
	tp->y = (float)screenPt.y;

	update();
}


void GestureRecognizer::update()
{
	// calculating deltaTime
	UInt32 newTime = Timer::GetTicksMs(); 
	deltaSeconds = (float)(newTime - oldTime)/1000.0f;
	oldTime = newTime;

	// calculating centroid of current points
	if(numTouches>0)
	{
		float sumX=0;
		float sumY=0;

		for(UPInt a=0;a<numTouches;a++ )
		{
			sumX+=touchPointArr[a]->x;
			sumY+=touchPointArr[a]->y;
		}

		NewCentroidX = sumX / (float)numTouches;
		NewCentroidY = sumY / (float)numTouches;
	}

	// calculating distance of centroid change
	deltaInches = Distance(OldCentroidX, OldCentroidY, NewCentroidX, NewCentroidY) / DPI;

	//
	for(int a=0; a<numSupportedGestures; a++)
	{
//#ifndef GFX_GESTURE_RECOGNIZE_ALL
		//if(selected==0 || selected==supportedGestures[a])
//#endif
			supportedGestures[a]->update();

	}

	//
	OldCentroidX=NewCentroidX;
	OldCentroidY=NewCentroidY;
}

void GestureRecognizer::reset()
{
	selected=0;
	for(int a=0; a<numSupportedGestures; a++)
	{
		supportedGestures[a]->reset();
	}

}

// Gesture
Gesture::Gesture()
{
	currentPhase = Phase_None;
	started=false;
}

GestureRotate::GestureRotate(){}
GesturePan::GesturePan(){}
GestureZoom::GestureZoom(){}
GestureSwipe::GestureSwipe(){ }
GesturePressAndTap::GesturePressAndTap(){}
GestureTwoFingerTap::GestureTwoFingerTap(){}

void Gesture::reset()
{
	currentPhase=Gesture::Phase_None;
	timeCounter=0;
	started=false;
	recognized=false;
}

void Gesture::update(){}

// AS3 usage
//	event.currentTarget.x += event.offsetX;
//	event.currentTarget.y += event.offsetY;
void GesturePan::update()
{
	Array<TouchPoint*> touchPoints = parent->touchPointArr;

	float dist;

	int count=0;
	float coordsX[2];
	float coordsY[2];

	if(parent->numTouches==2)
	{
		for(int a=0;a<2;a++ )
		{
			coordsX[count] = touchPoints[a]->x;
			coordsY[count] = touchPoints[a]->y;
			count++;
		}

		dist = Distance(coordsX[0], coordsY[0], coordsX[1], coordsY[1]);
	}

	switch(currentPhase)
	{
	case Phase_None:

		if(parent->numTouches==2)
		{
			if(!started)
			{
				timeCounter = 0;
				startDist = Distance(coordsX[0], coordsY[0], coordsX[1], coordsY[1]);

				totalCentroidOffsetX=0;
				totalCentroidOffsetY=0;

				startCentroidX=parent->NewCentroidX;
				startCentroidY=parent->NewCentroidY;

				currentPhase=Phase_Update;

				started=true;
				recognized=false;
			}
		}

		break;
	case Phase_Update:

		if(parent->numTouches!=2)
		{
			if(recognized)
			{
				recognized=false;

				GestureEvent evt(GFx::Event::GestureEnd, GestureEvent::GestureBit_Pan, parent->OldCentroidX, parent->OldCentroidY, 0, 0, 1, 1, 0);
				parent->pMovie->HandleEvent(evt);

				reset();

				parent->selected=0;
				parent->doingComplex=false;
			}
			else
			{
				started=false;
				currentPhase=Phase_None;
			}
		}
		else if(!(parent->NewCentroidX==parent->OldCentroidX && parent->NewCentroidY==parent->OldCentroidY))
		{
			timeCounter+=parent->deltaSeconds;
			totalCentroidOffsetX+=(parent->NewCentroidX-parent->OldCentroidX);
			totalCentroidOffsetY+=(parent->NewCentroidY-parent->OldCentroidY);

			if(recognized)
			{
				GestureEvent evt(GFx::Event::Gesture, GestureEvent::GestureBit_Pan, parent->NewCentroidX, parent->NewCentroidY, (parent->NewCentroidX-parent->OldCentroidX), (parent->NewCentroidY-parent->OldCentroidY), 1, 1, 0);
				parent->pMovie->HandleEvent(evt);
			}		
			else if(Distance(parent->NewCentroidX, parent->NewCentroidY, startCentroidX, startCentroidY) > 40) // threshold
			{
				recognized=true;

				GestureEvent evt(GFx::Event::GestureBegin, GestureEvent::GestureBit_Pan, parent->NewCentroidX, parent->NewCentroidY, 0, 0, 1, 1, 0);
				parent->pMovie->HandleEvent(evt);

				parent->doingComplex=true;
				parent->selected=this;
			}
		}

		break;
	}
}

// AS3 usage
// event.target.rotation += event.rotation;
void GestureRotate::update()
{
	Array<TouchPoint*> touchPoints = parent->touchPointArr;

	int count;
	float coordsX[2];
	float coordsY[2];
	float dist;

	count=0;
	if(parent->numTouches==2)
	{
		for(int a=0;a<2;a++ )
		{
			coordsX[count] = touchPoints[a]->x;
			coordsY[count] = touchPoints[a]->y;
			count++;
		}

		dist = Distance(coordsX[0], coordsY[0], coordsX[1], coordsY[1]);
	}

	switch(currentPhase)
	{
	case Phase_None:
		if(parent->numTouches==2)
		{
			if(!started)
			{
				timeCounter = 0;
				currentPhase=Phase_Update;
				totalAngle=0;
				started=true;
				recognized=false;
			}

		}
		break;
	case Phase_Update:
		if(parent->numTouches!=2)
		{
			started=false;
			if(recognized)
			{
				recognized=false;

				GestureEvent evt(GFx::Event::GestureEnd, GestureEvent::GestureBit_Rotate, parent->OldCentroidX, parent->OldCentroidY, 0, 0, 1, 1, 0);
				parent->pMovie->HandleEvent(evt);

				reset();

				parent->doingComplex=false;
				parent->selected=0;
			}
		}
		else if(!(coordsX[1]==lastCoordsX[1] && coordsX[0]==lastCoordsX[0] && coordsY[1]==lastCoordsY[1] && coordsY[0]==lastCoordsY[0]))
		{
			timeCounter+=parent->deltaSeconds;
			float deltaAngle = -signedAngle(PointF(coordsX[1] - coordsX[0], coordsY[1] - coordsY[0]) , PointF(lastCoordsX[1] - lastCoordsX[0], lastCoordsY[1] - lastCoordsY[0]));

			totalAngle+=deltaAngle;

			if(recognized)
			{
				GestureEvent evt(GFx::Event::Gesture, GestureEvent::GestureBit_Rotate, parent->NewCentroidX, parent->NewCentroidY, 0, 0, 1, 1, deltaAngle);
				parent->pMovie->HandleEvent(evt);
			}
			else if(abs(totalAngle) > 7.5) // threshold
			{
				recognized=true;

				GestureEvent evt(GFx::Event::GestureBegin, GestureEvent::GestureBit_Rotate, parent->NewCentroidX, parent->NewCentroidY, 0, 0, 1, 1, 0);
				parent->pMovie->HandleEvent(evt);

				parent->doingComplex=true;
				parent->selected=this;
			}
			
		}

		break;
	}

	if(parent->numTouches==2)
	{
		count=0;
		for(int a=0;a<2;a++ )
		{
			lastCoordsX[count] = touchPoints[a]->x;
			lastCoordsY[count] = touchPoints[a]->y;
			count++;
		}
	}
}

// AS3 usage
// event.offsetX = 1 | -1;
// event.offsetY = 1 | -1;
void GestureSwipe::update()
{
	if(parent->doingComplex)
		return;

	Array<TouchPoint*> touchPoints = parent->touchPointArr;

	switch(currentPhase)
	{
	case Phase_None:

		if(parent->numTouches==1 && !started)
		{
			this->startPosX = parent->NewCentroidX;
			this->startPosY = parent->NewCentroidY;
			currentPhase = Phase_Update;
			timeCounter = 0;
			started=true;
			lastTime = Timer::GetTicksMs();
		}

		if(parent->numTouches==0)
		{
			started=false;
		}

		break;
	case Phase_Update:
		if(parent->numTouches==1)
		{
			UInt32 newTime = Timer::GetTicksMs();
			timeCounter = (float)(newTime - lastTime)/1000.0f;

			if(timeCounter<.5f)
			{
				PointF posChange = PointF(parent->NewCentroidX - startPosX, parent->NewCentroidY - startPosY);
				if(Distance(0,0,posChange.x,posChange.y)>parent->minSwipeDist)
				{
					float HorizontalAngle;
					
					HorizontalAngle = signedAngle(posChange, PointF(1,0));
					if(abs(HorizontalAngle)<15 && posChange.x>0)	// swipe right
					{
						GestureEvent evt(GFx::Event::GestureSimple, GestureEvent::GestureBit_Swipe, parent->NewCentroidX, parent->NewCentroidY, 1, 0, 1, 1, 0);
						parent->pMovie->HandleEvent(evt);
						//currentPhase = Phase_None;
						reset();
					}

					HorizontalAngle = signedAngle(posChange, PointF(-1,0));
					if(abs(HorizontalAngle)<15 && posChange.x<0)	// swipe left
					{
						GestureEvent evt(GFx::Event::GestureSimple, GestureEvent::GestureBit_Swipe, parent->NewCentroidX, parent->NewCentroidY, -1, 0, 1, 1, 0);
						parent->pMovie->HandleEvent(evt);
						//currentPhase = Phase_None;
						reset();
					}

					///

					float VerticalAngle;

					VerticalAngle = signedAngle(posChange, PointF(0,1));
					if(abs(VerticalAngle)<15 && posChange.y>0)	//swipe down
					{
						GestureEvent evt(GFx::Event::GestureSimple, GestureEvent::GestureBit_Swipe, parent->NewCentroidX, parent->NewCentroidY, 0, 1, 1, 1, 0);
						parent->pMovie->HandleEvent(evt);
						//currentPhase = Phase_None;
						reset();
					}

					VerticalAngle = signedAngle(posChange, PointF(0,-1));
					if(abs(VerticalAngle)<15 && posChange.y<0)	//swipe up
					{
						GestureEvent evt(GFx::Event::GestureSimple, GestureEvent::GestureBit_Swipe, parent->NewCentroidX, parent->NewCentroidY, 0, -1, 1, 1, 0);
						parent->pMovie->HandleEvent(evt);
						//currentPhase = Phase_None;
						reset();
					}

				}
			}
		}
		else
		{
			//currentPhase = Phase_None;
			reset();
		}
		break;
	}
}

//instance_name_here.scaleX *= event.scaleX;
//instance_name_here.scaleY *= event.scaleY;
void GestureZoom::update()
{
	Array<TouchPoint*> touchPoints = parent->touchPointArr;

	float dist=0;

	int count=0;
	float coordsX[2];
	float coordsY[2];

	if(parent->numTouches==2)
	{
		for(int a=0;a<2;a++ )
		{
			coordsX[count] = touchPoints[a]->x;
			coordsY[count] = touchPoints[a]->y;
			count++;
		}

		dist = Distance(coordsX[0], coordsY[0], coordsX[1], coordsY[1]);
	}

	float distX = abs(coordsX[0] - coordsX[1]);
	float distY = abs(coordsY[0] - coordsY[1]);

	switch(currentPhase)
	{
	case Phase_None:
		if(parent->numTouches==2)
		{
			if(!started)
			{
				timeCounter = 0;
				startDist = dist;

				started=true;
				recognized=false;

				currentPhase=Phase_Update;

				lastDist = dist;
				startDist = dist;

				startDistX = distX;
				startDistY = distY;
			}
		}

		break;
	case Phase_Update:

		if(parent->numTouches!=2)
		{
			started=false;
			if(recognized)
			{
				//parent->pMovie->InputEventsQueue.AddGestureEvent(InputEventsQueueEntry::Phase_End, GestureEvent::GestureBit_Zoom, PointF(parent->OldCentroidX, parent->OldCentroidY), PointF(0,0), 1,1,0);
				GestureEvent evt(GFx::Event::GestureEnd, GestureEvent::GestureBit_Zoom, parent->OldCentroidX, parent->OldCentroidY, 0, 0, 1, 1, 0);
				parent->pMovie->HandleEvent(evt);

				reset();

				parent->selected=0;
				parent->doingComplex=false;
			}
		}
		else if(!(distX==lastDistX && distY==lastDistY))
		{
			timeCounter+=parent->deltaSeconds;

			if(recognized)
			{
				float ratioX = distX/lastDistX;
				float ratioY = distY/lastDistY;

				if(distX==0.0f || lastDistX==0.0f)
					ratioX = 1.0f;
				if(distY==0.0f || lastDistY==0.0f)
					ratioY = 1.0f;


				GestureEvent evt(GFx::Event::Gesture, GestureEvent::GestureBit_Zoom, parent->NewCentroidX, parent->NewCentroidY, 0, 0, ratioX, ratioY, 0);
				parent->pMovie->HandleEvent(evt);
			}
			else
			{
				if(abs(dist - startDist) > parent->DPI) // threshold
				{
					GestureEvent evt(GFx::Event::GestureBegin, GestureEvent::GestureBit_Zoom, parent->NewCentroidX, parent->NewCentroidY, 0, 0, 1, 1, 0);
					parent->pMovie->HandleEvent(evt);

					recognized=true;

					parent->selected=this;
					parent->doingComplex=true;

				}
			}
		}

		break;
	}

	lastDist = dist;
	lastDistX = distX;
	lastDistY = distY;
}

//event.tapLocalX;
//event.tapLocalY;
//event.tapStageX;
//event.tapStageY;
void GesturePressAndTap::update()
{
	if(parent->doingComplex)
		return;

	Array<TouchPoint*> touchPoints = parent->touchPointArr;

	switch(currentPhase)
	{
	case Phase_None:
		if(parent->numTouches==1)
		{
			currentPhase = Phase_Update;
			timeCounter = 0;
			tapping = false;
		}
		break;
	case Phase_Update:

		timeCounter+=parent->deltaSeconds;

		if(parent->numTouches==2 && timeCounter>.35 && !tapping)
		{
			GestureEvent evt(GFx::Event::GestureSimple, GestureEvent::GestureBit_PressAndTap, parent->NewCentroidX, parent->NewCentroidY, 0, 0, 1, 1, 0);
			
			parent->pMovie->HandleEvent(evt);

			tapping = true;
			currentPhase = Phase_None;
		}
		
		if(parent->numTouches==0 || parent->numTouches>2)
		{
			tapping = false;
			currentPhase = Phase_None;
		}

		break;
	}
	
}

// null
void GestureTwoFingerTap::update()
{
	if(parent->doingComplex)
		return;

	Array<TouchPoint*> touchPoints = parent->touchPointArr;

	int count=0;

	float dist=0.0f;
	float coordsX[2];
	float coordsY[2];

	if(parent->numTouches==2)
	{
		for(int a=0;a<2;a++)
		{
			coordsX[count] = touchPoints[a]->x;
			coordsY[count] = touchPoints[a]->y;
			count++;
		}

		dist = Distance(coordsX[0], coordsY[0], coordsX[1], coordsY[1]);
	}

	switch(currentPhase)
	{
	case Phase_None:

		if(!started && parent->numTouches==2 && dist<8.0f*parent->DPI)
		{
			currentPhase = Phase_Update;
			started=true;
			timeCounter=0;
			lastCentroidX = parent->NewCentroidX;
			lastCentroidY = parent->NewCentroidY;
			lastTime = Timer::GetTicksMs();
		}

		break;
	case Phase_Update:

		UInt32 newTime = Timer::GetTicksMs();
		timeCounter = (float)(newTime - lastTime)/1000.0f;

		if(started)
		{
			if(parent->numTouches==0)
			{
				if(timeCounter<=.25f)
				{
					GestureEvent evt(GFx::Event::GestureSimple, GestureEvent::GestureBit_TwoFingerTap, lastCentroidX, lastCentroidY, 0, 1, 1, 0);
					parent->pMovie->HandleEvent(evt);

				}

				reset();
			}

		}

		break;
	}

}

}} // namespace Scaleform::GFx

#endif // GFX_GESTURE_RECOGNIZE