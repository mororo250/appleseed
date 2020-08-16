
//
// This source file is part of appleseed.
// Visit https://appleseedhq.net/ for additional information and resources.
//
// This software is released under the MIT license.
//
// Copyright (c) 2010-2013 Francois Beaune, Jupiter Jazz Limited
// Copyright (c) 2014-2018 Francois Beaune, The appleseedhq Organization
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

#pragma once

// appleseed.renderer headers.
#include "renderer/modeling/light/ilightfactory.h"
#include "renderer/modeling/light/light.h"

// appleseed.foundation headers.
#include "foundation/memory/autoreleaseptr.h"
#include "foundation/platform/compiler.h"

// appleseed.main headers.
#include "main/dllsymbol.h"

// Forward declarations.
namespace foundation    { class Dictionary; }
namespace foundation    { class DictionaryArray; }
namespace renderer      { class ParamArray; }
namespace renderer      { class EnvironmentEDF; }

namespace renderer
{

    //
    // Physically-based Sun light.
    //

    class SunLight
        :public Light
    {
    public:
        SunLight(
            const char*                     name,
            const ParamArray&               params);

        bool on_frame_begin(
            const Project&                  project,
            const BaseGroup*                parent,
            OnFrameBeginRecorder&           recorder,
            foundation::IAbortSwitch*       abort_switch = nullptr) override;

        virtual void evaluate(
            const foundation::Vector3d&     outgoing,
            Spectrum&                       value) const = 0;

        float compute_distance_attenuation(
            const foundation::Vector3d&                 target,
            const foundation::Vector3d&                 position) const override;

    protected:

        float compute_limb_darkening(const float squared_distance_to_center) const;

        APPLESEED_DECLARE_INPUT_VALUES(InputValues)
        {
            float           m_turbidity;                // atmosphere turbidity
            float           m_radiance_multiplier;      // emitted radiance multiplier
            float           m_size_multiplier;          // Sun size multiplier
            float           m_distance;                 // distance between Sun and scene, in millions of km
        };

        InputValues             m_values;

        bool                    m_visible;                  // visible of the sun
        foundation::Vector3d    m_scene_center;             // world space
        double                  m_scene_radius;             // world space
        double                  m_safe_scene_diameter;      // world space
        float                   m_sun_solid_angle;          // Sun's solid angle, in steradians
        float                   m_sun_size;

      private:

        void apply_env_edf_overrides(EnvironmentEDF* env_edf);
    };

}   // namespace renderer
