
//
// This source file is part of appleseed.
// Visit https://appleseedhq.net/ for additional information and resources.
//
// This software is released under the MIT license.
//
// Copyright (c) 2019 João Marcos Costa, The appleseedhq Organization
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

// appleseed.renderer headers.
#include "renderer/api/frame.h"
#include "renderer/kernel/rendering/renderprogress.h"

// appleseed.foundation headers.
#include "foundation/image/canvasproperties.h"
#include "foundation/image/image.h"

using namespace foundation;

namespace renderer
{
    RenderProgress::RenderProgress(const Frame* frame)
      : m_max_average_spp(frame->get_parameters().get_path_optional("progressive_frame_renderer.max_average_spp", std::numeric_limits<std::size_t>::max()))
      , m_time_limit(frame->get_parameters().get_path_optional("progressive_frame_renderer.time_limit", std::numeric_limits < std::size_t > ::max()))
      , m_total_pixels(frame->image().properties().m_pixel_count)
      , m_sample_count(0)
    {
    }

    void RenderProgress::update_progress()
    {
        double sample_per_pixel = static_cast<double>(m_sample_count) / m_total_pixels;

        m_stopwatch.measure();

        double remeaning_time_limit = m_time_limit - m_stopwatch.get_seconds();
        double remeaning_time_samples = (sample_per_pixel / m_stopwatch.get_seconds()) * (m_max_average_spp - sample_per_pixel);

        if (remeaning_time_limit < remeaning_time_samples)
        {
            m_remeaning_time = remeaning_time_limit;
            m_progress = m_stopwatch.get_seconds() / m_time_limit * 100;
        }
        else
        {
            m_remeaning_time = remeaning_time_samples;
            m_progress = sample_per_pixel / m_max_average_spp * 100.0;
        }
    }
}   // namespace renderer
