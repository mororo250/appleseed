
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

#pragma once

#include "foundation/utility/stopwatch.h"
#include "foundation/platform/defaulttimers.h"
#include "renderer/api/rendering.h"

#include <memory>
#include <string>

using namespace foundation;

namespace renderer
{


 class RenderProgress
{
  public:
    RenderProgress(const Frame* frame);

    void start_progess() { m_stopwatch.start(); }
    void pouse_progress() { m_stopwatch.pause(); };
    void end_progress()
    {
        m_progress = 0;
        m_sample_count = 0;
    }

    void update_progress();

    void add_samples(std::size_t samples) { m_sample_count = samples; }

    const double get_progress() const { return m_progress; }
    const double get_time() const { return m_stopwatch.get_seconds(); }

  private:
    double m_remeaning_time;

    //const std::size_t m_pass_count;
    const std::size_t m_max_average_spp;
    const std::size_t m_time_limit;
    const std::size_t m_total_pixels;

    //std::size_t m_rendered_tiles_count;
    //std::size_t m_total_tiles;
    std::size_t m_sample_count;
    double m_progress;
    Stopwatch<DefaultWallclockTimer> m_stopwatch;
};
}
