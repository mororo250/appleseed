
//
// This source file is part of appleseed.
// Visit http://appleseedhq.net/ for additional information and resources.
//
// This software is released under the MIT license.
//
// Copyright (c) 2015 Francois Beaune, The appleseedhq Organization
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

// Interface header.
#include "bssrdf.h"

// appleseed.renderer headers.
#include "renderer/kernel/shading/shadingpoint.h"
#include "renderer/modeling/frame/frame.h"
#include "renderer/modeling/input/inputevaluator.h"
#include "renderer/modeling/project/project.h"

// appleseed.foundation headers.
#include "foundation/image/colorspace.h"

using namespace foundation;

/*

Quick ref:
----------

    sigma_a     absorption coeff.
    sigma_s     scattering coeff.
    g           anisotropy

    sigma_t         extinction coeff.           -> sigma_a + sigma_s
    sigma_s_prime   reduced scattering coeff.   -> sigma_s * (1 - g)
    sigma_t_prime   reduced extinction coeff.   -> sigma_a + sigma_s_prime
    sigma_tr        effective extinction coeff. -> sqrt( 3 * sigma_a * sigma_t_prime)

    Texture mapping:
    ----------------

    Fdr             fresnel diffuse reflectance
    A                                           -> (1 + Fdr) / (1 - Fdr)
    alpha_prime                                 -> sigma_s_prime / sigma_t_prime
    ld              mean free path              -> 1 / sigma_tr

    sigma_t_prime = sigma_tr / sqrt( 3 * (1 - alpha_prime))
    sigma_s_prime = alpha_prime * sigma_t_prime
    sigma_a = sigma_t_prime - sigma_s_prime
*/

namespace renderer
{


//
// BSSRDF class implementation.
//

namespace
{
    const UniqueID g_class_uid = new_guid();
}

UniqueID BSSRDF::get_class_uid()
{
    return g_class_uid;
}

BSSRDF::BSSRDF(
    const char*             name,
    const ParamArray&       params)
  : ConnectableEntity(g_class_uid, params)
  , m_lighting_conditions(0)
{
    set_name(name);
}

bool BSSRDF::on_frame_begin(
    const Project&          project,
    const Assembly&         assembly,
    IAbortSwitch*           abort_switch)
{
    m_lighting_conditions = &project.get_frame()->get_lighting_conditions();
    return true;
}

void BSSRDF::on_frame_end(
    const Project&          project,
    const Assembly&         assembly)
{
    m_lighting_conditions = 0;
}

size_t BSSRDF::compute_input_data_size(
    const Assembly&         assembly) const
{
    return get_inputs().compute_data_size();
}

void BSSRDF::evaluate_inputs(
    const ShadingContext&   shading_context,
    InputEvaluator&         input_evaluator,
    const ShadingPoint&     shading_point,
    const size_t            offset) const
{
    input_evaluator.evaluate(get_inputs(), shading_point.get_uv(0), offset);
}

// A better dipole, Eugene d’Eon
// http://www.eugenedeon.com/papers/betterdipole.pdf

double BSSRDF::fresnel_moment_1(const double eta)
{
    if (eta >= 1.0)
        return (-9.23372 + eta * (22.2272 + eta * (-20.9292 + eta * (10.2291 + eta * (-2.54396 + 0.254913 * eta))))) * 0.5;
    else
        return (0.919317 + eta * (-3.4793 + eta * (6.75335 + eta * (-7.80989 + eta *(4.98554 - 1.36881 * eta))))) * 0.5;
}

double BSSRDF::fresnel_moment_2(const double eta)
{
    double r = -1641.1 + eta * (1213.67 + eta * (-568.556 + eta * (164.798 + eta * (-27.0181 + 1.91826 * eta))));
    r += (((135.926 / eta) - 656.175) / eta + 1376.53) / eta;
    return r * 0.33333333;
}

}   // namespace renderer
