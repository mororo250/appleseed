
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

// Interface header.
#include "progresstilecallback.h"

// appleseed.renderer headers.
#include "renderer/api/frame.h"

// appleseed.foundation headers.
#include "foundation/image/canvasproperties.h"
#include "foundation/image/image.h"
#include "foundation/image/tile.h"
#include "foundation/platform/defaulttimers.h"
#include "foundation/platform/thread.h"
#include "foundation/utility/log.h"
#include "foundation/utility/stopwatch.h"
#include "foundation/utility/string.h"

// Standard headers.
#include <memory>
#include <string>

using namespace foundation;
using namespace renderer;
using namespace std;

namespace appleseed {
namespace shared {

namespace
{
    //
    // ProgressTileCallback.
    //

    class FinalProgressTileCallback
      : public TileCallbackBase
    {
      public:
        FinalProgressTileCallback(Logger& logger, const size_t pass_count, double& progress)
          : m_logger(logger)
          , m_rendered_tiles(0)
          , m_pass_count(pass_count)
          , m_progress_track(progress)
        {
        }

        void release() override
        {
            m_progress_track = 0.0;
            // The factory always return the same tile callback instance.
            // Prevent this instance from being destroyed by doing nothing here.
        }

        void on_tiled_frame_begin(const Frame* frame) override
        {
            // Do not restart the stopwatch if it is already running.
            if (m_rendered_tiles == 0)
                m_stopwatch.start();
        }

        void on_tile_end(
            const Frame*    frame,
            const size_t    tile_x,
            const size_t    tile_y) override
        {
            boost::mutex::scoped_lock lock(m_mutex);

            // Retrieve the total number of tiles.
            const size_t total_tiles = frame->image().properties().m_tile_count * m_pass_count;

            // Keep track of the total number of rendered tiles.
            ++m_rendered_tiles;

            // Estimate remaining render time.
            m_stopwatch.measure();
            const double elapsed_time = m_stopwatch.get_seconds();
            const double remaining_time = (elapsed_time / m_rendered_tiles) * (total_tiles - m_rendered_tiles);

            // Print a progress message.
            if (m_rendered_tiles <= total_tiles)
            {
                LOG_INFO(
                    m_logger,
                    "rendering, %s done; about %s remaining...",
                    pretty_percent(m_rendered_tiles, total_tiles).c_str(),
                    pretty_time(remaining_time).c_str());
                m_progress_track = static_cast<double>(m_rendered_tiles) / static_cast<double>(total_tiles) * 100.0;
            }   
        }

      private:
        Logger&                             m_logger;
        const size_t                        m_pass_count;
        boost::mutex                        m_mutex;
        size_t                              m_rendered_tiles;
        Stopwatch<DefaultWallclockTimer>    m_stopwatch;
        double&                             m_progress_track;
    };

    class InteractiveProgressTileCallback
      : public TileCallbackBase
    {
      public:
        InteractiveProgressTileCallback(Logger& logger, uint64 max_average_spp, uint64 time_limit, double& progress)
          : m_logger(logger)
          , m_sample_count(0)
          , m_progress_track(progress)
          , m_max_average_spp(max_average_spp)
          , m_time_limit(time_limit)
        {
        }
    
        void release() override
        {
            m_progress_track = 0.0;
            // The factory always return the same tile callback instance.
            // Prevent this instance from being destroyed by doing nothing here.
        }
    
        void on_tiled_frame_begin(const Frame* frame) override
        {
            // Do not restart the stopwatch if it is already running.
            const size_t total_pixels = frame->image().properties().m_pixel_count;

            if (m_sample_count == 0)
                m_stopwatch.start();
        }       
    
        void on_progressive_frame_update(const Frame* frame) override
        {

            m_stopwatch.measure();

            m_progress_track = m_stopwatch.get_seconds() / m_time_limit * 100;
        }
    
      private:
        Logger&                             m_logger;
        size_t                              m_sample_count;
        uint64                              m_max_average_spp;
        size_t                              m_total_pixel;
        uint64                              m_time_limit;
        Stopwatch<DefaultWallclockTimer>    m_stopwatch;
        double&                             m_progress_track;
    };
}

//
// ProgressTileCallbackFactory class implementation.
//

struct ProgressTileCallbackFactory::Impl
{
    unique_ptr<ITileCallback> m_callback;
};

ProgressTileCallbackFactory::ProgressTileCallbackFactory(
    Logger&             logger,
    const ParamArray&   params,
    double&             progress)
  : impl(new Impl())
{
    const string rendering_mode = params.get_required<string>("frame_renderer", "generic");

    if (rendering_mode == "generic")
        impl->m_callback = unique_ptr<ITileCallback>(
            new FinalProgressTileCallback(logger, params.get_optional<size_t>("passes", 1), progress));
    else
        impl->m_callback = unique_ptr<ITileCallback>(
            new InteractiveProgressTileCallback(logger,
                params.get_path_optional("progressive_frame_renderer.max_average_spp", numeric_limits<uint64>::max()),
                params.get_path_optional("progressive_frame_renderer.time_limit", numeric_limits<uint64>::max()),
                progress));
}   

ProgressTileCallbackFactory::~ProgressTileCallbackFactory()
{
    delete impl;
}

void ProgressTileCallbackFactory::release()
{
    delete this;
}

ITileCallback* ProgressTileCallbackFactory::create()
{
    return impl->m_callback.get();
}

}   // namespace shared
}   // namespace appleseed
