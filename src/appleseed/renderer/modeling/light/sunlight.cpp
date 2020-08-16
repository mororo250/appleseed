
//
// This source file is part of appleseed.
// Visit https://appleseedhq.net/ for additional information and resources.
//
// This software is released under the MIT license.
//
// Copyright (c) 2020 Joao Marcos Costa , Jupiter Jazz Limited
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
#include "sunlight.h"

// appleseed.renderer headers.
#include "renderer/global/globallogger.h"
#include "renderer/modeling/environmentedf/environmentedf.h"
#include "renderer/modeling/input/source.h"
#include "renderer/modeling/project/project.h"
#include "renderer/modeling/scene/scene.h"

// appleseed.foundation headers.
#include "foundation/utility/api/apistring.h"

// Standard headers.
#include <string>


using namespace foundation;

namespace renderer
{

    //
    // SunLight class implementation.
    //

    namespace
    {
        // Sun's radius, in millions of km.
        // Reference: https://en.wikipedia.org/wiki/Solar_radius
        constexpr float SunRadius = 0.6957f;
    }

    SunLight::SunLight(
        const char*                 name,
        const ParamArray&           params)
      : Light(name, params)
    {
    }

    bool SunLight::on_frame_begin(
        const Project&              project,
        const BaseGroup*            parent,
        OnFrameBeginRecorder&       recorder,
        foundation::IAbortSwitch*   abort_switch)
    {
        if (!Light::on_frame_begin(project, parent, recorder, abort_switch))
            return false;

        // Check if sun disk is visible.
        m_visible = m_params.get_optional<bool>("visible", true);

        // Evaluate uniform inputs.
        m_inputs.evaluate_uniforms(&m_values);

        // Warn if distance input is not uniform.
        Source* distance_src = get_inputs().source("distance");
        assert(distance_src != nullptr);
        if (!distance_src->is_uniform())
        {
            RENDERER_LOG_WARNING(
                "distance between sun and scene \"%s\" is not uniform, using default value of 149.6 million km.",
                get_path().c_str());
            m_values.m_distance = 149.6f;
        }

        // Warn if size multiplier input is not uniform.
        const Source* size_multiplier_src = get_inputs().source("size_multiplier");
        assert(size_multiplier_src != nullptr);
        if (!size_multiplier_src->is_uniform())
        {
            RENDERER_LOG_WARNING(
                "size multiplier of the sun light \"%s\" is not uniform.",
                get_path().c_str());
            m_values.m_size_multiplier = 1.0f;
        }

        m_sun_size = SunRadius * m_values.m_size_multiplier;

        // Compute the Sun's solid angle.
        // Reference: https://en.wikipedia.org/wiki/Solid_angle#Sun_and_Moon
        m_sun_solid_angle = TwoPi<float>() * (1.0f - std::cos(std::atan(SunRadius * m_values.m_size_multiplier / m_values.m_distance)));

        // If the Sun light is bound to an environment EDF, let it override the Sun's direction and turbidity.
        EnvironmentEDF* env_edf = dynamic_cast<EnvironmentEDF*>(m_inputs.get_entity("environment_edf"));
        if (env_edf != nullptr)
            apply_env_edf_overrides(env_edf);

        const Scene::RenderData& scene_data = project.get_scene()->get_render_data();
        m_scene_center = Vector3d(scene_data.m_center);
        m_scene_radius = scene_data.m_radius;
        m_safe_scene_diameter = scene_data.m_safe_diameter;

        return true;
    }

    float SunLight::compute_distance_attenuation(
        const Vector3d&             target,
        const Vector3d&             position) const
    {
        return 1.0f;
    }

    float SunLight::compute_limb_darkening(const float squared_distance_to_center) const
    {

        // Reference:
        //
        //   Lintu, Andrei & Haber, Jörg & Magnor, Marcus.
        //   (2005). Realistic Solar Disc Rendering.
        //   http://wscg.zcu.cz/wscg2005/Papers_2005/Full/F17-full.pdf
        //

        constexpr float LimbDarkeningCoeficent = 0.6f; // Limb darkening coefficient for the sun for visible HosekSunLight.
        float limb_darkening = 1.0f;
        if (squared_distance_to_center > 0.0f)
        {
            limb_darkening = (1.0f - LimbDarkeningCoeficent *
                (1.0f - std::sqrt(1.0f - squared_distance_to_center / square(m_sun_size))));
        }

        return limb_darkening;
    }

    void SunLight::apply_env_edf_overrides(EnvironmentEDF* env_edf)
    {
        env_edf->get_inputs().find("sun_light").bind(this);

        // Use the Sun direction from the EDF if it has one.
        const Source* sun_theta_src = env_edf->get_inputs().source("sun_theta");
        const Source* sun_phi_src = env_edf->get_inputs().source("sun_phi");
        const Source* sun_shift_src = env_edf->get_inputs().source("horizon_shift");
        if (sun_theta_src != nullptr &&
            sun_theta_src->is_uniform() &&
            sun_phi_src != nullptr &&
            sun_phi_src->is_uniform() &&
            sun_shift_src != nullptr &&
            sun_shift_src->is_uniform())
        {
            float sun_theta, sun_phi, sun_shift;
            sun_theta_src->evaluate_uniform(sun_theta);
            sun_phi_src->evaluate_uniform(sun_phi);
            sun_shift_src->evaluate_uniform(sun_shift);

            Transformd scratch;
            const Transformd& env_edf_transform = env_edf->transform_sequence().evaluate(0.0f, scratch);

            set_transform(
                Transformd::from_local_to_parent(
                    Matrix4d::make_translation(Vector3d(0.0, sun_shift, 0.0)) *
                    Matrix4d::make_rotation(
                        Quaterniond::make_rotation(
                            Vector3d(0.0, 0.0, -1.0),   // default emission direction of this light
                            -Vector3d::make_unit_vector(deg_to_rad(sun_theta), deg_to_rad(sun_phi))))) *
                env_edf_transform);
        }

        // Use the Sun turbidity from the EDF if it has one.
        const Source* turbidity_src = env_edf->get_inputs().source("turbidity");
        const Source* turbidity_multiplier_src = env_edf->get_inputs().source("turbidity_multiplier");
        if (turbidity_src != nullptr &&
            turbidity_src->is_uniform() &&
            turbidity_multiplier_src != nullptr &&
            turbidity_multiplier_src->is_uniform())
        {
            float turbidity_multiplier;
            turbidity_multiplier_src->evaluate_uniform(turbidity_multiplier);
            turbidity_src->evaluate_uniform(m_values.m_turbidity);
            m_values.m_turbidity *= turbidity_multiplier;
        }
    }

}   // namespace renderer
