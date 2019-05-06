
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

#include "progressbar.h"

namespace appleseed {
namespace studio {

//
// StatusBar class implementation.
//

ProgressBar::ProgressBar()
  :m_layout(this)
{
    // Set format.
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    m_style =
        "QProgressBar\
        {\
            border: 2px solid grey;\
        }\
        QProgressBar::chunk\
        {\
            background: QLinearGradient( x1: 0, y1: 0, x2: 1, y2: 0, stop: 0 #fb9, stop: 0.4999 #f75, stop: 0.5 #f75, stop: 1 #f41 );\
            border: 1px;\
      }";

    m_progress_bar.setOrientation(Qt::Horizontal);
    m_progress_bar.setRange(0, 100);
    m_progress_bar.setFixedSize(150, 20);
    m_progress_bar.setAlignment(Qt::AlignCenter);
    m_progress_bar.setStyleSheet(m_style);
    m_progress_bar.hide();

    m_layout.addWidget(&m_label, 0, 0);
    m_layout.addWidget(&m_progress_bar, 0, 1);
}

void ProgressBar::start_progress_bar_display(std::string text, const double* value)
{
    m_progress_value = value;
    m_label.setText(QString::fromStdString(text));

    m_progress_bar.show();

    m_timer_id = startTimer(100);
}

void ProgressBar::stop_progress_bar_display()
{
    m_progress_bar.hide();
    m_progress_bar.reset();
    m_progress_value = nullptr;

    m_label.clear();

    killTimer(m_timer_id);
}

void ProgressBar::timerEvent(QTimerEvent* event)
{
    m_progress_bar.setValue(*m_progress_value);
}

} // namespace studio
} // namespace appleseed
