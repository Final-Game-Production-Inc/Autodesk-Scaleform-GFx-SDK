// Simple manipulator objects, for applying user input to a model
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

#ifndef NV_MANIPULATORS_H
#define NV_MANIPULATORS_H


#include <nvMath.h>

namespace nv {

//
//
//  Base manipulator class
//
//////////////////////////////////////////////////////////////////////
class Manipulator{

public:

    enum Button {
        ENoButton = 0x0,
        ELeftButton = 0x1,
        EMiddleButton = 0x2,
        ERightButton = 0x4
    };

    enum Modifier {
        ENoModifier = 0x0,
        EShiftModifier = 0x1,
        EControlModifier = 0x2,
        EAltModifier = 0x3
    };

    Manipulator() : _width(0), _height(0) {}
    virtual ~Manipulator() {}

    //
    //  reshape
    //
    //    This function should be called, whenever the region containing the control
    //  has been resized. Typically, this is a window resize event.
    ////////////////////////////////////////////////////////////////////////////
    virtual void reshape( int width, int height) { _width = width; _height = height; }

    //
    //  mouse
    //
    //    This function should be invoked whenever the state of a mouse button
    //  changes.
    //    
    ////////////////////////////////////////////////////////////////////////////
    virtual void mouse( Button button, Modifier mod, bool depressed, int x, int y) = 0;

    //
    //  motion
    //
    //    This function should be called whenever the mouse is moved
    ////////////////////////////////////////////////////////////////////////////
    virtual void motion( int x, int y) = 0;

    //
    //  idle
    //
    //    This function should be called when no input is happening,
    //  but the manipulator is likely to need to update.
    ////////////////////////////////////////////////////////////////////////////
    virtual void idle() = 0;

    //
    //  update
    //
    //    This function is called to apply the latest forces to the
    //  manipulator.
    ////////////////////////////////////////////////////////////////////////////
    virtual void update() = 0;

    //
    //  applyTransform
    //
    ////////////////////////////////////////////////////////////////////////////
    virtual void applyTransform() = 0;

    //
    //  getTransform
    //
    //    Compute the current transform based on the latest update, and
    //  return it as a 4x4 matrix.
    ////////////////////////////////////////////////////////////////////////////
    virtual matrix4f getTransform() = 0;

protected:

    int _width, _height;
};


//
// Trackball Manipulator
//
//////////////////////////////////////////////////////////////////////
class TrackballManipulator : public Manipulator {

public:

    //
    //
    //////////////////////////////////////////////////////////////////
    TrackballManipulator() : _activateButton(ELeftButton), _activateModifiers(ENoModifier),  _active(false) {
        _r = quaternionf( 0.0f, 1.0f, 0.0f, 0.0f);
        _scale = 1.0f;
    }

    //
    //
    //////////////////////////////////////////////////////////////////
    virtual void mouse( Button button, Modifier mod, bool depressed, int x, int y) {
        
        //check to see if this click changes our activation state
        if ( button == _activateButton && depressed && ( _activateModifiers != ENoModifier || (_activateModifiers == mod)) ) {
            
            _active = true;
            _x = _x0 = x;
            _y = _y0 = y;
            _dx = _dy = 0;
        }
        else if ( button == _activateButton && !depressed) {
            if(_dx == 0 && _dy == 0)
                update();
            _active = false;
            _dx = _dy = 0;
        }
    }

    //
    //
    //////////////////////////////////////////////////////////////////
    virtual void motion( int x, int y) {
        if ( _active) {
            _dx = x - _x;   _dy = _y - y;
            _x = x;   _y = y;
            update();
        }
    }

    //
    //
    //////////////////////////////////////////////////////////////////
    virtual void idle() {
        //simply increment the rotation
        _r = _incr*_r;
    }

    //
    //
    //////////////////////////////////////////////////////////////////
    virtual void update() {
        float min = _width < _height ? _width : _height;
        min *= 0.5f;
        vec3f offset(_width/2.f, _height/2.f, 0);
        vec3f a(_x-_dx, _height - (_y+_dy), 0);
        vec3f b(   _x, _height -     _y , 0);
        a -= offset;
        b -= offset;
        a /= min;
        b /= min;

        a[2] = pow(2.0f, -0.5f * length(a));
        a = normalize(a);
        b[2] = pow(2.0f, -0.5f * length(b));
        b = normalize(b);

        vec3f axis = cross( a, b);
        axis = normalize(axis);

        float angle = acos( dot( a, b));

        //original glh version had an invert flag and a parent frame, do we need one?

        _incr.set_value(axis, angle * _scale);

        _r = _incr*_r;        
        
    }

    //
    //
    //////////////////////////////////////////////////////////////////
    void applyTransform() {
        glTranslatef( _centroid[0], _centroid[1], _centroid[2]);
        applyRotation(_r);
        glTranslatef( -_centroid[0], -_centroid[1], -_centroid[2]);
    }

    //
    //
    //////////////////////////////////////////////////////////////////
    matrix4f getTransform() {
        matrix4f trans, invTrans, rotation;
        _r.get_value( rotation);
        trans.set_translate( _centroid);
        invTrans.set_translate( -_centroid);
        return trans * rotation * invTrans;
    }

    //
    //  setTrackballActivate
    //
    //    Set the mouse button and optional modifiers that activate
    //  the trackball.
    //////////////////////////////////////////////////////////////////
    void setTrackballActivate( Button b, Modifier m = ENoModifier) {
        _activateButton = b;
        _activateModifiers = m;
    }

    //
    //  setTrackballScale
    //
    //    Set the speed for the trackball.
    //////////////////////////////////////////////////////////////////
    void setTrackballScale( float scale) {
        _scale = scale;
    }

    //
    //  setCenterOfRotation
    //
    //    Set the point around which the trackball will rotate.
    //////////////////////////////////////////////////////////////////
    void setCenterOfRotation( const vec3f& c) {
        _centroid = c;
    }

protected:

    int _x, _y;
    int _x0, _y0;
    int _dx, _dy;

    // UI commands that this trackball responds to (defaults to left mouse button with no modifier key)
    Button _activateButton;
    Modifier _activateModifiers;

    // Variable used to determine if the manipulator is presently tracking the maouse
    bool _active;

    quaternionf _r;
    
    float _scale;
    quaternionf _incr;
    vec3f _centroid;
};


//
// Examine Manipulator
//
//  Combines a trackball interface with pan and dolly operations
//
//////////////////////////////////////////////////////////////////////
class ExamineManipulator : public Manipulator {

public:

    //
    //
    //////////////////////////////////////////////////////////////////
    ExamineManipulator() : _tbActivateButton(ELeftButton), _dActivateButton(ERightButton), _pActivateButton(EMiddleButton),
            _tbActivateModifiers(ENoModifier), _dActivateModifiers(ENoModifier), _pActivateModifiers(ENoModifier), _tbActive(false), _dActive(false), _pActive(false) {
        _r = quaternionf( 0.0f, 1.0f, 0.0f, 0.0f);
        _incr = quaternionf( 0.0f, 0.0f, 0.0f, 1.0f); //no rotation
        _tbScale = 1.0f;
        _dScale = 0.01f;
        _pScale = 0.01f;
    }

    //
    //
    //////////////////////////////////////////////////////////////////
    virtual void mouse( Button button, Modifier mod, bool depressed, int x, int y) {
        
        if ( button == _tbActivateButton && depressed && (_tbActivateModifiers == mod) ) {
            
            _tbActive = true;
            _x = _x0 = x;
            _y = _y0 = y;
            _dx = _dy = 0;
        }
        else if ( button == _tbActivateButton && !depressed) {
            if(_dx == 0 && _dy == 0)
                update();
            _tbActive = false;
            _dx = _dy = 0;
        }
        
        if ( button == _dActivateButton && depressed && (_dActivateModifiers == mod) ) {
            
            _dActive = true;
            _x = _x0 = x;
            _y = _y0 = y;
            _dx = _dy = 0;
        }
        else if ( button == _dActivateButton && !depressed) {
            if(_dx == 0 && _dy == 0)
                update();
            _dActive = false;
            _dx = _dy = 0;
        }
        
        if ( button == _pActivateButton && depressed && (_pActivateModifiers == mod) ) {
            
            _pActive = true;
            _x = _x0 = x;
            _y = _y0 = y;
            _dx = _dy = 0;
        }
        else if ( button == _pActivateButton && !depressed) {
            if(_dx == 0 && _dy == 0)
                update();
            _pActive = false;
            _dx = _dy = 0;
        }
    }

    //
    //
    //////////////////////////////////////////////////////////////////
    virtual void motion( int x, int y) {
        if ( _tbActive || _dActive || _pActive) {
            _dx = x - _x;   _dy = _y - y;
            _x = x;   _y = y;
            update();
        }
    }

    //
    //
    //////////////////////////////////////////////////////////////////
    virtual void idle() {
        //simply increment the rotation
        _r = _incr*_r;
    }

    //
    //
    //////////////////////////////////////////////////////////////////
    void updateTrackball() {
        float min = _width < _height ? _width : _height;
        min *= 0.5f;
        vec3f offset(_width/2.f, _height/2.f, 0);
        vec3f a(_x-_dx, _height - (_y+_dy), 0);
        vec3f b(   _x, _height -     _y , 0);
        a -= offset;
        b -= offset;
        a /= min;
        b /= min;

        a[2] = pow(2.0f, -0.5f * length(a));
        a = normalize(a);
        b[2] = pow(2.0f, -0.5f * length(b));
        b = normalize(b);

        vec3f axis = cross( a, b);
        axis = normalize(axis);

        float angle = acos( dot( a, b));

        //original glh version had an invert flag and a parent frame, do we need one?

        _incr.set_value(axis, angle * _tbScale);

        _r = _incr*_r;
    }

    //
    //
    //////////////////////////////////////////////////////////////////
    void updatePan() {
        vec3f v(_dx, _dy, 0);
	  
        _pan += v * _pScale;
	}

    //
    //
    //////////////////////////////////////////////////////////////////
    void updateDolly() {
        vec3f v(0,0,_dy);

        _dolly -= v * _dScale;
	}

    //
    //
    //////////////////////////////////////////////////////////////////
    virtual void update() {
        if (_tbActive)
            updateTrackball();
        if (_dActive)
            updateDolly();
        if (_pActive)
            updatePan();
    }

    //
    //
    //////////////////////////////////////////////////////////////////
    void applyTransform() {
        glTranslatef( _pan[0], _pan[1], _pan[2]);
        glTranslatef( _dolly[0], _dolly[1], _dolly[2]);
        glTranslatef( _centroid[0], _centroid[1], _centroid[2]);
        applyRotation(_r);
        glTranslatef( -_centroid[0], -_centroid[1], -_centroid[2]);
    }

    //
    //
    //////////////////////////////////////////////////////////////////
    matrix4f getTransform() {
        matrix4f trans, invTrans, rotation;
        matrix4f pan, dolly;

        //create pan and dolly translations
        pan.set_translate( _pan);
        dolly.set_translate( _dolly);

        //create the trackball rotation
        _r.get_value( rotation);

        //create the translations to move the center of rotation to the origin and back
        trans.set_translate( _centroid);
        invTrans.set_translate( -_centroid);

        //concatenate all the tranforms
        return pan * dolly * trans * rotation * invTrans;
    }

    //
    //  setTrackballActivate
    //
    //    Set the mouse button and optional modifiers that activate
    //  the trackball.
    //////////////////////////////////////////////////////////////////
    void setTrackballActivate( Button b, Modifier m = ENoModifier) {
        _tbActivateButton = b;
        _tbActivateModifiers = m;
    }

    //
    //  setTrackballScale
    //
    //    Set the speed for the trackball.
    //////////////////////////////////////////////////////////////////
    void setTrackballScale( float scale) {
        _tbScale = scale;
    }

    //
    //  setDollyActivate
    //
    //    Set the mouse button and optional modifiers that activate
    //  the dolly operation.
    //////////////////////////////////////////////////////////////////
    void setDollyActivate( Button b, Modifier m = ENoModifier) {
        _dActivateButton = b;
        _dActivateModifiers = m;
    }

    //
    //  setDollyScale
    //
    //    Set the speed for the dolly operation.
    //////////////////////////////////////////////////////////////////
    void setDollyScale( float scale) {
        _dScale = scale;
    }

    //
    //  setDollyPosition
    //
    //    Set the Dolly to a specified distance.
    //////////////////////////////////////////////////////////////////
    void setDollyPosition( float pos) {
        _dolly.z = pos;
    }

    //
    //  setPanActivate
    //
    //    Set the mouse button and optional modifiers that activate
    //  the pan operation.
    //////////////////////////////////////////////////////////////////
    void setPanActivate( Button b, Modifier m = ENoModifier) {
        _pActivateButton = b;
        _pActivateModifiers = m;
    }

    //
    //  setPanScale
    //
    //    Set the speed for panning.
    //////////////////////////////////////////////////////////////////
    void setPanScale( float scale) {
        _pScale = scale;
    }

    //
    //  setCenterOfRotation
    //
    //    Set the point around which the trackball will rotate.
    //////////////////////////////////////////////////////////////////
    void setCenterOfRotation( const vec3f& c) {
        _centroid = c;
    }

	// get the rotation quaternion
	quaternionf &getRotation() { return _r; }

	// get the rotation increment
	quaternionf &getIncrement() { return _incr; }

protected:

    int _x, _y;
    int _x0, _y0;
    int _dx, _dy;

    // UI commands that this trackball responds to (defaults to left mouse button with no modifier key)
    Button _tbActivateButton, _dActivateButton, _pActivateButton;
    Modifier _tbActivateModifiers, _dActivateModifiers, _pActivateModifiers;

    // Variable used to determine if the manipulator is presently tracking the mouse
    bool _tbActive;
    bool _dActive;
    bool _pActive;

    quaternionf _r;
    vec3f _pan;
    vec3f _dolly;
    
    float _tbScale; //trackball scale
    float _dScale;  //dolly scale
    float _pScale;   //pan scale
    quaternionf _incr;
    vec3f _centroid;
};

};

#endif