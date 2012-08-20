//
// This source file is part of appleseed.
// Visit http://appleseedhq.net/ for additional information and resources.
//
// This software is released under the MIT license.
//
// Copyright (c) 2012 Esteban Tovagliari.
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

#ifndef APPLESEED_PY_BIND_AUTO_RELEASE_PTR_H
#define APPLESEED_PY_BIND_AUTO_RELEASE_PTR_H

// Has to be first, to avoid redifinition warnings.
#include <Python.h>

#include <boost/python.hpp>

#include "foundation/utility/autoreleaseptr.h"

namespace boost
{
namespace python
{

template<class T> struct pointee<foundation::auto_release_ptr<T> >
{
    typedef T type;
};

} // python
} // boost

namespace foundation
{

template<class T>
T *get_pointer( const auto_release_ptr<T>& p)
{
    return p.get();
}

} // foundation

#endif // APPLESEED_PY_BIND_AUTO_RELEASE_PTR_H
