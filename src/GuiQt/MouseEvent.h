#ifndef __MouseEvent_H__
#define __MouseEvent_H__

/*LICENSE_START*/ 
/* 
 *  Copyright 1995-2002 Washington University School of Medicine 
 * 
 *  http://brainmap.wustl.edu 
 * 
 *  This file is part of CARET. 
 * 
 *  CARET is free software; you can redistribute it and/or modify 
 *  it under the terms of the GNU General Public License as published by 
 *  the Free Software Foundation; either version 2 of the License, or 
 *  (at your option) any later version. 
 * 
 *  CARET is distributed in the hope that it will be useful, 
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of 
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 *  GNU General Public License for more details. 
 * 
 *  You should have received a copy of the GNU General Public License 
 *  along with CARET; if not, write to the Free Software 
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
 * 
 */ 

#include "CaretObject.h"

#include <stdint.h>

class QMouseEvent;

namespace caret {
    
    class BrainOpenGLViewportContent;
    class BrainOpenGLWidget;
    
    /**
     * Contains information about a mouse event in the OpenGL region.
     */
    class MouseEvent : public CaretObject {
        
    public:
        MouseEvent(BrainOpenGLViewportContent* viewportContent,
                   BrainOpenGLWidget* openGLWidget,
                   const int32_t browserWindowIndex,
                   const int32_t x,
                   const int32_t y,
                   const int32_t dx,
                   const int32_t dy,
                   const int32_t mousePressX,
                   const int32_t mousePressY);
        
        virtual ~MouseEvent();
        
    private:
        void initializeMembersMouseEvent();
        
        MouseEvent(const MouseEvent& o);
        
        MouseEvent& operator=(const MouseEvent& o);
        
    public:
        AString toString() const;
        
        BrainOpenGLViewportContent* getViewportContent() const;
        
        BrainOpenGLWidget* getOpenGLWidget() const;

        int32_t getBrowserWindowIndex() const;
        
        int32_t getDx() const;
        
        int32_t getDy() const;
        
        int32_t getX() const;
        
        int32_t getY() const;
        
        int32_t getPressedX() const;
        
        int32_t getPressedY() const;
        
        int32_t getWheelRotation() const;
        
    private:
        BrainOpenGLViewportContent* m_viewportContent;
        
        BrainOpenGLWidget* m_openGLWidget;
        
        int32_t m_browserWindowIndex;
        
        int32_t m_x;
        
        int32_t m_y;
        
        int32_t m_dx;
        
        int32_t m_dy;

        int32_t m_pressX;
        
        int32_t m_pressY;
        
        int32_t m_wheelRotation;
    };
    
} // namespace

#endif // __MouseEvent_H__
