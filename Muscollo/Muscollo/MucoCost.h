#ifndef MUSCOLLO_MUCOCOST_H
#define MUSCOLLO_MUCOCOST_H
/* -------------------------------------------------------------------------- *
 * OpenSim Muscollo: MucoCost.h                                               *
 * -------------------------------------------------------------------------- *
 * Copyright (c) 2017 Stanford University and the Authors                     *
 *                                                                            *
 * Author(s): Christopher Dembia                                              *
 *                                                                            *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may    *
 * not use this file except in compliance with the License. You may obtain a  *
 * copy of the License at http://www.apache.org/licenses/LICENSE-2.0          *
 *                                                                            *
 * Unless required by applicable law or agreed to in writing, software        *
 * distributed under the License is distributed on an "AS IS" BASIS,          *
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   *
 * See the License for the specific language governing permissions and        *
 * limitations under the License.                                             *
 * -------------------------------------------------------------------------- */

#include <OpenSim/Common/Object.h>

#include <SimTKcommon/internal/State.h>

#include "osimMuscolloDLL.h"

namespace OpenSim {

// TODO give option to specify gradient and hessian analytically.

/// A term in the cost functional, to be minimized.
/// @ingroup mucocost
class OSIMMUSCOLLO_API MucoCost : public Object {
OpenSim_DECLARE_CONCRETE_OBJECT(MucoCost, Object);
public:
    OpenSim_DECLARE_PROPERTY(weight, double,
            "The cost value is multiplied by this weight (default: 1).");

    MucoCost();

    /// This includes the weight.
    SimTK::Real calcIntegralCost(const SimTK::State& state) const {
        double integrand = 0;
        calcIntegralCostImpl(state, integrand);
        return get_weight() * integrand;
    }
    /// This includes the weight.
    // We use SimTK::Real instead of double for when we support adoubles.
    SimTK::Real calcEndpointCost(const SimTK::State& finalState) const {
        double cost = 0;
        calcEndpointCostImpl(finalState, cost);
        return get_weight() * cost;
    }
protected:
    /// Precondition: state is realized to SimTK::Stage::Position.
    virtual void calcIntegralCostImpl(const SimTK::State& state,
            double& integrand) const;
    /// The endpoint cost cannot depend on actuator controls.
    virtual void calcEndpointCostImpl(const SimTK::State& finalState,
            SimTK::Real& cost) const;
private:
    void constructProperties();
};

inline void MucoCost::calcIntegralCostImpl(const SimTK::State&,
        double&) const {}

inline void MucoCost::calcEndpointCostImpl(const SimTK::State&,
        double&) const {}

/// Endpoint cost for final time.
/// @ingroup mucocost
class OSIMMUSCOLLO_API MucoFinalTimeCost : public MucoCost {
OpenSim_DECLARE_CONCRETE_OBJECT(MucoFinalTimeCost, MucoCost);
protected:
    void calcEndpointCostImpl(const SimTK::State& finalState,
            SimTK::Real& cost) const override {
        cost = finalState.getTime();
    }
};


} // namespace OpenSim

#endif // MUSCOLLO_MUCOCOST_H
