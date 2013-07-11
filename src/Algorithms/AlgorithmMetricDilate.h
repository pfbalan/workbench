#ifndef __ALGORITHM_METRIC_DILATE_H__
#define __ALGORITHM_METRIC_DILATE_H__

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

#include "AbstractAlgorithm.h"

namespace caret {
    
    class AlgorithmMetricDilate : public AbstractAlgorithm
    {
        struct StencilElem
        {
            std::vector<std::pair<int, float> > m_weightlist;
            float m_weightsum;
        };
        AlgorithmMetricDilate();
        void precomputeStencils(std::vector<std::pair<int, StencilElem> >& myStencils, const SurfaceFile* mySurf, const float* myAreas, const MetricFile* badNodeRoi, const MetricFile* dataRoi,
                                const float& distance, const float& exponent);
        void precomputeNearest(std::vector<std::pair<int, int> >& myNearest, const SurfaceFile* mySurf, const MetricFile* badNodeRoi, const MetricFile* dataRoi, const float& distance);
        void processColumn(float* colScratch, const int& numNodes, const float* myInputData, std::vector<std::pair<int, int> > myNearest);
        void processColumn(float* colScratch, const int& numNodes, const float* myInputData, std::vector<std::pair<int, StencilElem> > myStencils);
        void processColumn(float* colScratch, const float* myInputData, const SurfaceFile* mySurf, const float* myAreas, const MetricFile* dataRoi,
                           const float& distance, const bool& nearest, const float& exponent);
    protected:
        static float getSubAlgorithmWeight();
        static float getAlgorithmInternalWeight();
    public:
        AlgorithmMetricDilate(ProgressObject* myProgObj, const MetricFile* myMetric, const SurfaceFile* mySurf, const float& distance,
                              MetricFile* myMetricOut, const MetricFile* badNodeRoi = NULL, const MetricFile* dataRoi = NULL, const int& columnNum = -1, const bool& nearest = false, const float& exponent = 2.0f);
        static OperationParameters* getParameters();
        static void useParameters(OperationParameters* myParams, ProgressObject* myProgObj);
        static AString getCommandSwitch();
        static AString getShortDescription();
    };

    typedef TemplateAutoOperation<AlgorithmMetricDilate> AutoAlgorithmMetricDilate;

}

#endif //__ALGORITHM_METRIC_DILATE_H__
