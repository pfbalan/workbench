#ifndef __BRAIN_OPEN_G_L_CHART_DRAWING_FIXED_PIPELINE_H__
#define __BRAIN_OPEN_G_L_CHART_DRAWING_FIXED_PIPELINE_H__

/*LICENSE_START*/
/*
 * Copyright 2014 Washington University,
 * All rights reserved.
 *
 * Connectome DB and Connectome Workbench are part of the integrated Connectome 
 * Informatics Platform.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *    * Neither the names of Washington University nor the
 *      names of its contributors may be used to endorse or promote products
 *      derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR  
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE.
 */
/*LICENSE_END*/


#include "BrainOpenGLChartDrawingInterface.h"
#include "CaretColorEnum.h"

namespace caret {

    class ChartAxis;
    class ChartDataCartesian;
    class ChartModelDataSeries;
    class ChartModelMatrix;
    
    class BrainOpenGLChartDrawingFixedPipeline : public BrainOpenGLChartDrawingInterface {
        
    public:
        BrainOpenGLChartDrawingFixedPipeline();
        
        virtual ~BrainOpenGLChartDrawingFixedPipeline();
        
        virtual void drawChart(const int32_t viewport[4],
                               BrainOpenGLTextRenderInterface* textRenderer,
                               ChartModel* chart);
    private:
        BrainOpenGLChartDrawingFixedPipeline(const BrainOpenGLChartDrawingFixedPipeline&);

        BrainOpenGLChartDrawingFixedPipeline& operator=(const BrainOpenGLChartDrawingFixedPipeline&);
        
        void drawChartGraphics(BrainOpenGLTextRenderInterface* textRenderer,
                               ChartModel* chart);
        
        void drawChartGraphicsLineSeries( BrainOpenGLTextRenderInterface* textRenderer,
                                         ChartModelDataSeries* chart);
        
        void drawChartGraphicsMatrix(BrainOpenGLTextRenderInterface* textRenderer,
                                     ChartModelMatrix* chart);

        void drawChartAxesGrid(const float vpX,
                               const float vpY,
                               const float vpWidth,
                               const float vpHeight,
                               const float marginSize,
                               int32_t chartGraphicsDrawingViewportOut[4]);
        
        void drawChartAxis(const float vpX,
                           const float vpY,
                           const float vpWidth,
                           const float vpHeight,
                           const float marginSize,
                           BrainOpenGLTextRenderInterface* textRenderer,
                           const ChartAxis* axis);
        
        void drawChartDataCartesian(const ChartDataCartesian* chartDataCartesian,
                                    const float lineWidth,
                                    const float rgb[3]);
        
        AString axisValueToText(const float axisValue) const;
        
        void restoreStateOfOpenGL();
        
        void saveStateOfOpenGL();
        
        
    public:

        // ADD_NEW_METHODS_HERE

    private:
        float m_foregroundColor[4];
        
        // ADD_NEW_MEMBERS_HERE

    };
    
#ifdef __BRAIN_OPEN_G_L_CHART_DRAWING_FIXED_PIPELINE_DECLARE__
    // <PLACE DECLARATIONS OF STATIC MEMBERS HERE>
#endif // __BRAIN_OPEN_G_L_CHART_DRAWING_FIXED_PIPELINE_DECLARE__

} // namespace
#endif  //__BRAIN_OPEN_G_L_CHART_DRAWING_FIXED_PIPELINE_H__
