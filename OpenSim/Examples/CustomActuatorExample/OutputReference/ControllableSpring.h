#ifndef _ControllableSpring_h_
#define _ControllableSpring_h_
/* -------------------------------------------------------------------------- *
 *                       OpenSim:  ControllableSpring.h                       *
 * -------------------------------------------------------------------------- *
 * The OpenSim API is a toolkit for musculoskeletal modeling and simulation.  *
 * See http://opensim.stanford.edu and the NOTICE file for more information.  *
 * OpenSim is developed at Stanford University and supported by the US        *
 * National Institutes of Health (U54 GM072970, R24 HD065690) and by DARPA    *
 * through the Warrior Web program.                                           *
 *                                                                            *
 * Copyright (c) 2005-2012 Stanford University and the Authors                *
 * Author(s): Matt S. DeMers                                                  *
 *                                                                            *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may    *
 * not use this file except in compliance with the License. You may obtain a  *
 * copy of the License at http://www.apache.org/licenses/LICENSE-2.0.         *
 *                                                                            *
 * Unless required by applicable law or agreed to in writing, software        *
 * distributed under the License is distributed on an "AS IS" BASIS,          *
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied    *
 * See the License for the specific language governing permissions and        *
 * limitations under the License.                                             *
 * -------------------------------------------------------------------------- */

/*
 * Author: Matt DeMers
 */


#include "PistonActuator.h"

//=============================================================================
//=============================================================================
/**
 * A class that implements a variable stiffness spring actuator acting between 
 * two points on two bodies.
 * This actuator has no states; the control simply scales the optimal force
 * so that control*optimalForce = stiffness.
 *
 * @author Matt DeMers
 * @version 2.0
 */

namespace OpenSim {

class ControllableSpring : public PistonActuator {
OpenSim_DECLARE_CONCRETE_OBJECT(ControllableSpring, PistonActuator);

//=============================================================================
// DATA
//=============================================================================
protected:
	// Additional Properties specific to a controllable spring need to be defined
	/** rest length of the spring */
	PropertyDbl _propRestLength;

	// REFERENCES
	/** rest length */
	double &_restLength;

//=============================================================================
// METHODS
//=============================================================================
	//--------------------------------------------------------------------------
	// CONSTRUCTION
	//--------------------------------------------------------------------------
public:
/* The constructor takes the same form as the PistonActuator constructor.  The 
** _restLength reference must be initialized in the initialization list */
ControllableSpring( std::string aBodyNameA="", std::string aBodyNameB="") :
	PistonActuator(aBodyNameA, aBodyNameB),
	_restLength(_propRestLength.getValueDbl())
{
	setNull();
}
/* The copy constructor must also copy the _restLength since the base class 
** version doesn't know about it. */
ControllableSpring(const ControllableSpring &aControllableSpring) :
	PistonActuator(aControllableSpring),
	_restLength(_propRestLength.getValueDbl())
{
	setNull();
	_restLength = aControllableSpring.getRestLength();
}
/* use the default destructor */
virtual ~ControllableSpring() {};

	//--------------------------------------------------------------------------
	// GET and SET
	//--------------------------------------------------------------------------

/* define private utilities to be used by the constructors. */
private:
void setupProperties()
{
	_propRestLength.setName("rest_length");
	_propRestLength.setValue(1.0);
	_propRestLength.setComment("The equilibrium length of the spring.");
	_propertySet.append( &_propRestLength);
}

void setNull()
{
	setType("ControllableSpring");
	setupProperties();
	setNumStateVariables(0);
	
}

/* since _restLength is a private member, define public methods 
** to get and set it */
public:
// REST LENGTH
void setRestLength(double aLength) { _restLength = aLength; };
double getRestLength() const { return _restLength; };

	//--------------------------------------------------------------------------
	// COMPUTATIONS
	//--------------------------------------------------------------------------

/* The computeForce method is the meat of this simple actuator example.  It 
** computes the direction and distance between the two application points.  It
** then uses the difference between it's current length and rest length to determine
** the force magnitude, then applies the force at the application points, in the
** direction between them. */
void computeForce(const SimTK::State& s, 
							  SimTK::Vector_<SimTK::SpatialVec>& bodyForces, 
							  SimTK::Vector& generalizedForces) const
{
	// make sure the model and bodies are instantiated
	if (_model==NULL) return;
	const SimbodyEngine& engine = getModel().getSimbodyEngine();
	
	if(_bodyA ==NULL || _bodyB ==NULL)
		return;
	
	/* store _pointA and _pointB positions in the global frame.  If not
	** alread in the body frame, transform _pointA and _pointB into their
	** respective body frames. */

	SimTK::Vec3 pointA_inGround, pointB_inGround;

	if (_pointsAreGlobal)
	{
		pointA_inGround = _pointA;
		pointB_inGround = _pointB;
		engine.transformPosition(s, engine.getGroundBody(), _pointA, *_bodyA, _pointA);
		engine.transformPosition(s, engine.getGroundBody(), _pointB, *_bodyB, _pointB);
	}
	else
	{
		engine.transformPosition(s, *_bodyA, _pointA, engine.getGroundBody(), pointA_inGround);
		engine.transformPosition(s, *_bodyB, _pointB, engine.getGroundBody(), pointB_inGround);
	}

	// find the dirrection along which the actuator applies its force
	SimTK::Vec3 r = pointA_inGround - pointB_inGround;
	SimTK::UnitVec3 direction(r);
	double length = sqrt(~r*r);

	/* find the stiffness.  computeActuation is defined in PistonActuator and just returns
	** the product of the actuator's control and it's _optimalForce.  We're using this to mean
	** stiffness. */
	double stiffness = computeActuation(s);

	// find the force magnitude and set it. then form the force vector
	double forceMagnitude = (_restLength - length)*stiffness;
    setForce(s,  forceMagnitude );
	SimTK::Vec3 force = forceMagnitude*direction;

	// appy equal and opposite forces to the bodies
	applyForceToPoint(s, *_bodyA, _pointA, force, bodyForces);
	applyForceToPoint(s, *_bodyB, _pointB, -force, bodyForces);
}

//=============================================================================
};	// END of class ControllableSpring	

} //Namespace
//=============================================================================
//=============================================================================

#endif // _ControllableSpring_h_