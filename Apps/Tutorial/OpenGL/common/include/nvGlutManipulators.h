// Adaptors to GLUT native commands for manipulator objects
//
// This code is in part deriver from glh, cross platform glut helper library.
// The copyright for glh follows this notice.
//
// Copyright (c) NVIDIA Corporation. All rights reserved.
////////////////////////////////////////////////////////////////////////////////

/*
    Copyright (c) 2000 Cass Everitt
	Copyright (c) 2000 NVIDIA Corporation
    All rights reserved.

    Redistribution and use in source and binary forms, with or
	without modification, are permitted provided that the following
	conditions are met:

     * Redistributions of source code must retain the above
	   copyright notice, this list of conditions and the following
	   disclaimer.

     * Redistributions in binary form must reproduce the above
	   copyright notice, this list of conditions and the following
	   disclaimer in the documentation and/or other materials
	   provided with the distribution.

     * The names of contributors to this software may not be used
	   to endorse or promote products derived from this software
	   without specific prior written permission. 

       THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
	   ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
	   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
	   FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
	   REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
	   INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
	   BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
	   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
	   CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
	   LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
	   ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
	   POSSIBILITY OF SUCH DAMAGE. 


    Cass Everitt - cass@r3.nu
*/

#ifndef NV_GLUT_MANIPULATORS_H
#define NV_GLUT_MANIPULATORS_H

#include <nvManipulators.h>
#include <GL/glut.h>

namespace nv {

class GlutManipulator {

public:
    
    GlutManipulator( Manipulator &m) : _manip(m) { } 
    virtual ~GlutManipulator() {}

    //
    //  reshape
    //
    //    This function should be called, whenever the region containing the control
    //  has been resized. Typically, this is a window resize event.
    ////////////////////////////////////////////////////////////////////////////
    virtual void reshape( int width, int height) { _manip.reshape( width, height); }

    //
    //  mouse
    //
    //    This function should be invoked whenever the state of a mouse button
    //  changes.
    //    
    ////////////////////////////////////////////////////////////////////////////
    virtual void mouse( int button, int state, int x, int y) {
        _manip.mouse( convertButtonFromGlut(button), convertModifierFromGlut(glutGetModifiers()), state == GLUT_DOWN, x, y);
    }

    //
    //  mouse
    //
    //    This function should be invoked whenever the state of a mouse button
    //  changes.
    //    
    ////////////////////////////////////////////////////////////////////////////
    virtual void mouse( int button, int state, int modifier, int x, int y) {
        _manip.mouse( convertButtonFromGlut(button), convertModifierFromGlut(modifier), state == GLUT_DOWN, x, y);
    }

    //
    //  motion
    //
    //    This function should be called whenever the mouse is moved
    ////////////////////////////////////////////////////////////////////////////
    virtual void motion( int x, int y) { _manip.motion( x, y); }

    //
    //  idle
    //
    //    This function should be called when no input is happening,
    //  but the manipulator is likely to need to update.
    ////////////////////////////////////////////////////////////////////////////
    virtual void idle() { _manip.idle(); }

    //
    //  update
    //
    //    This function is called to apply the latest forces to the
    //  manipulator.
    ////////////////////////////////////////////////////////////////////////////
    virtual void update() { _manip.update(); }

    //
    //  applyTransform
    //
    ////////////////////////////////////////////////////////////////////////////
    virtual void applyTransform() { _manip.applyTransform(); }

    //
    //  getTransform
    //
    //    Compute the current transform based on the latest update, and
    //  return it as a 4x4 matrix.
    ////////////////////////////////////////////////////////////////////////////
    virtual matrix4f getTransform() { return _manip.getTransform(); }

	// get a reference to the manipulator
	virtual Manipulator &getManipulator() { return _manip; }

protected:

    //
    //  convertButtonFromGlut
    //
    //    Converts the integer used by GLUT to identify a mouse button
    //  to the UI independent enums used by the manipulators
    ////////////////////////////////////////////////////////////////////////////
    static Manipulator::Button convertButtonFromGlut( int button) {
        switch (button) {
            case GLUT_LEFT_BUTTON:
                return Manipulator::ELeftButton;
            case GLUT_MIDDLE_BUTTON:
                return Manipulator::EMiddleButton;
            case GLUT_RIGHT_BUTTON:
                return Manipulator::ERightButton;
        }

        //handle fallthrough
        return Manipulator::ENoButton;
    }

    //
    //  convertModifierFromGlut
    //
    //    Converts the integer used by GLUT to identify modifier keys
    //  to the UI independent enums used by the manipulators
    ////////////////////////////////////////////////////////////////////////////
    static Manipulator::Modifier convertModifierFromGlut( int modifier) {
        int val = (int)Manipulator::ENoModifier;

        //need to use a bitwise or, because it could be multiple keys
        if (modifier & GLUT_ACTIVE_ALT)
            val |= Manipulator::EAltModifier;
        
        if (modifier & GLUT_ACTIVE_CTRL)
            val |= Manipulator::EControlModifier;
        
        if (modifier & GLUT_ACTIVE_SHIFT)
            val |= Manipulator::EShiftModifier;

        return (Manipulator::Modifier)val;
    }


    //
    //  Manipulator reference
    //
    //    This reference is used such that the GLUT manipulator adaptors
    //  can all share the same UI message passing code.
    ////////////////////////////////////////////////////////////////////////////
    Manipulator &_manip;
};

//
//
//////////////////////////////////////////////////////////////////////
class GlutTrackball : public GlutManipulator{

public:

    //
    //
    /////////////////////////////////////////////////////////////////
    GlutTrackball() : GlutManipulator(_trackball) {}

    //
    //  setTrackballActivate
    //
    //    Set the mouse button and optional modifiers that activate
    //  the trackball.
    //////////////////////////////////////////////////////////////////
    void setTrackballActivate( int button, int modifier = 0) {
        _trackball.setTrackballActivate( convertButtonFromGlut(button), convertModifierFromGlut(modifier));
    }

    //
    //  setTrackballScale
    //
    //    Set the speed for the trackball.
    //////////////////////////////////////////////////////////////////
    void setTrackballScale( float scale) {
        _trackball.setTrackballScale( scale);
    }

    //
    //  setCenterOfRotation
    //
    //    Set the point around which the trackball will rotate.
    //////////////////////////////////////////////////////////////////
    void setCenterOfRotation( const vec3f& c) {
        _trackball.setCenterOfRotation( c);
    }


protected:
    TrackballManipulator _trackball;
};

//
//
//////////////////////////////////////////////////////////////////////
class GlutExamine : public GlutManipulator{

public:

    //
    //
    //////////////////////////////////////////////////////////////////
    GlutExamine() : GlutManipulator(_examine) {}

    //
    //  setTrackballActivate
    //
    //    Set the mouse button and optional modifiers that activate
    //  the trackball.
    //////////////////////////////////////////////////////////////////
    void setTrackballActivate( int button, int modifier = 0) {
        _examine.setTrackballActivate( convertButtonFromGlut(button), convertModifierFromGlut(modifier));
    }

    //
    //  setTrackballScale
    //
    //    Set the speed for the trackball.
    //////////////////////////////////////////////////////////////////
    void setTrackballScale( float scale) {
        _examine.setTrackballScale( scale);
    }

    //
    //  setDollyActivate
    //
    //    Set the mouse button and optional modifiers that activate
    //  the dolly operation.
    //////////////////////////////////////////////////////////////////
    void setDollyActivate( int button, int modifier = 0) {
        _examine.setDollyActivate( convertButtonFromGlut(button), convertModifierFromGlut(modifier));
    }

    //
    //  setDollyScale
    //
    //    Set the speed for the dolly operation.
    //////////////////////////////////////////////////////////////////
    void setDollyScale( float scale) {
        _examine.setDollyScale( scale);
    }

    //
    //  setDollyPosition
    //
    //    Set the Dolly to a specified distance.
    //////////////////////////////////////////////////////////////////
    void setDollyPosition( float pos) {
        _examine.setDollyPosition( pos);
    }

    //
    //  setPanActivate
    //
    //    Set the mouse button and optional modifiers that activate
    //  the pan operation.
    //////////////////////////////////////////////////////////////////
    void setPanActivate( int button, int modifier = 0) {
        _examine.setPanActivate( convertButtonFromGlut(button), convertModifierFromGlut(modifier));
    }

    //
    //  setPanScale
    //
    //    Set the speed for panning.
    //////////////////////////////////////////////////////////////////
    void setPanScale( float scale) {
        _examine.setPanScale( scale);
    }

    //
    //  setCenterOfRotation
    //
    //    Set the point around which the trackball will rotate.
    //////////////////////////////////////////////////////////////////
    void setCenterOfRotation( const vec3f& c) {
        _examine.setCenterOfRotation( c);
    }

protected:
    ExamineManipulator _examine;
};


};

#endif