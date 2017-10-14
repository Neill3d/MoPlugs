/****************************************************************************/
/* Copyright (c) 2011, Markus Billeter, Ola Olsson, Erik Sintorn and Ulf Assarsson
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
/****************************************************************************/

//--- SDK include
#include <fbsdk/fbsdk.h>

#include "graphics\CheckGLError_MOBU.h"

#include <gl\glew.h>
//#include <GL/glut.h>
#include <sstream>
#include <iostream>


bool checkGLErrorMoBu(const char *file, int line)
{
  bool wasError = false;

  std::stringstream ss;
  for (GLenum glErr = glGetError(); glErr != GL_NO_ERROR; glErr = glGetError())
  {
    wasError = true;
    const GLubyte* sError = gluErrorString(glErr);
    
    if (!sError)
    {
      sError = reinterpret_cast<const GLubyte *>(" (no message available)");
    }

    ss  << "  GL Error #" << glErr << "(" << sError << ") " << std::endl;
  }

  if (wasError)
  {

	  FBTrace( ss.str().c_str() );

    //outputFailure(file, line, ss.str().c_str());
  }
  return wasError;
}
