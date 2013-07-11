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

#include "AlgorithmMetricDilate.h"
#include "AlgorithmException.h"

#include "CaretOMP.h"
#include "GeodesicHelper.h"
#include "MetricFile.h"
#include "PaletteColorMapping.h"
#include "SurfaceFile.h"
#include "TopologyHelper.h"

#include <cmath>

using namespace caret;
using namespace std;

AString AlgorithmMetricDilate::getCommandSwitch()
{
    return "-metric-dilate";
}

AString AlgorithmMetricDilate::getShortDescription()
{
    return "DILATE A METRIC FILE";
}

OperationParameters* AlgorithmMetricDilate::getParameters()
{
    OperationParameters* ret = new OperationParameters();
    ret->addMetricParameter(1, "metric", "the metric to dilate");
    
    ret->addSurfaceParameter(2, "surface", "the surface to compute on");
    
    ret->addDoubleParameter(3, "distance", "distance in mm to dilate");
    
    ret->addMetricOutputParameter(4, "metric-out", "the output metric");
    
    OptionalParameter* badRoiOpt = ret->createOptionalParameter(5, "-bad-vertex-roi", "specify an roi of vertices to overwrite, rather than vertices with value zero");
    badRoiOpt->addMetricParameter(1, "roi-metric", "metric file, positive values denote vertices to have their values replaced");
    
    OptionalParameter* dataRoiOpt = ret->createOptionalParameter(9, "-data-roi", "specify an roi of where there is data");
    dataRoiOpt->addMetricParameter(1, "roi-metric", "metric file, positive values denote vertices that have data");
    
    OptionalParameter* columnSelect = ret->createOptionalParameter(6, "-column", "select a single column to dilate");
    columnSelect->addStringParameter(1, "column", "the column number or name");
    
    ret->createOptionalParameter(7, "-nearest", "use the nearest good value instead of a weighted average");
    
    OptionalParameter* exponentOpt = ret->createOptionalParameter(8, "-exponent", "use a different exponent in the weighting function");
    exponentOpt->addDoubleParameter(1, "exponent", "exponent 'n' to use in (area / (distance ^ n)) as the weighting function (default 2)");
    
    ret->setHelpText(
        AString("For all metric vertices that are designated as bad, if they neighbor a non-bad vertex with data or are within the specified distance of such a vertex, ") +
        "replace the value with a distance weighted average of nearby non-bad vertices that have data, otherwise set the value to zero.  " +
        "No matter how small <distance> is, dilation will always use at least the immediate neighbor vertices.  " +
        "If -nearest is specified, it will use the value from the closest non-bad vertex with data within range instead of a weighted average.\n\n" +
        "If -bad-vertex-roi is specified, only vertices with a positive value in the ROI are bad.  " +
        "If it is not specified, only vertices that have data, with a value of zero, are bad.  " +
        "If -data-roi is not specified, all vertices are assumed to have data."
    );
    return ret;
}

void AlgorithmMetricDilate::useParameters(OperationParameters* myParams, ProgressObject* myProgObj)
{
    MetricFile* myMetric = myParams->getMetric(1);
    SurfaceFile* mySurf = myParams->getSurface(2);
    float distance = (float)myParams->getDouble(3);
    MetricFile* myMetricOut = myParams->getOutputMetric(4);
    OptionalParameter* badRoiOpt = myParams->getOptionalParameter(5);
    MetricFile* badNodeRoi = NULL;
    if (badRoiOpt->m_present)
    {
        badNodeRoi = badRoiOpt->getMetric(1);
    }
    OptionalParameter* dataRoiOpt = myParams->getOptionalParameter(9);
    MetricFile* dataRoi = NULL;
    if (dataRoiOpt->m_present)
    {
        dataRoi = dataRoiOpt->getMetric(1);
    }
    OptionalParameter* columnSelect = myParams->getOptionalParameter(6);
    int columnNum = -1;
    if (columnSelect->m_present)
    {//set up to use the single column
        columnNum = (int)myMetric->getMapIndexFromNameOrNumber(columnSelect->getString(1));
        if (columnNum < 0)
        {
            throw AlgorithmException("invalid column specified");
        }
    }
    bool nearest = myParams->getOptionalParameter(7)->m_present;
    float exponent = 2.0f;
    OptionalParameter* exponentOpt = myParams->getOptionalParameter(8);
    if (exponentOpt->m_present)
    {
        exponent = (float)exponentOpt->getDouble(1);
    }
    AlgorithmMetricDilate(myProgObj, myMetric, mySurf, distance, myMetricOut, badNodeRoi, dataRoi, columnNum, nearest, exponent);
}

AlgorithmMetricDilate::AlgorithmMetricDilate(ProgressObject* myProgObj, const MetricFile* myMetric, const SurfaceFile* mySurf, const float& distance, MetricFile* myMetricOut,
                                             const MetricFile* badNodeRoi, const MetricFile* dataRoi, const int& columnNum, const bool& nearest, const float& exponent) : AbstractAlgorithm(myProgObj)
{
    LevelProgress myProgress(myProgObj);
    int numNodes = mySurf->getNumberOfNodes();
    if (numNodes != myMetric->getNumberOfNodes())
    {
        throw AlgorithmException("surface and metric number of vertices do not match");
    }
    if (badNodeRoi != NULL && badNodeRoi->getNumberOfNodes() != numNodes)
    {
        throw AlgorithmException("bad vertex roi number of vertices does not match");
    }
    if (dataRoi != NULL && dataRoi->getNumberOfNodes() != numNodes)
    {
        throw AlgorithmException("data roi number of vertices does not match");
    }
    if (columnNum < -1 || columnNum >= myMetric->getNumberOfColumns())
    {
        throw AlgorithmException("invalid column specified");
    }
    if (distance < 0.0f)
    {
        throw AlgorithmException("invalid distance specified");
    }
    float cutoffRatio = 1.5f, test = pow(0.1f, -1.0f / exponent);//don't use less than a 1.5 * nearest cutoff
    if (test > 1.0f && test < cutoffRatio)//if it is less than 1, the exponent is weird, so simply ignore it and use default
    {
        if (test > 1.1f)
        {
            cutoffRatio = test;
        } else {
            cutoffRatio = 1.1f;
        }
    }
    myMetricOut->setStructure(mySurf->getStructure());
    vector<pair<int, StencilElem> > myStencils;//because we need to iterate over it in parallel
    vector<pair<int, int> > myNearest;
    vector<float> colScratch(numNodes);
    vector<float> myAreas;
    mySurf->computeNodeAreas(myAreas);
    if (badNodeRoi != NULL)
    {
        if (nearest)
        {
            precomputeNearest(myNearest, mySurf, badNodeRoi, dataRoi, distance);
        } else {
            precomputeStencils(myStencils, mySurf, myAreas.data(), badNodeRoi, dataRoi, distance, exponent);
        }
    }
    if (columnNum == -1)
    {
        myMetricOut->setNumberOfNodesAndColumns(numNodes, myMetric->getNumberOfColumns());
        for (int thisCol = 0; thisCol < myMetric->getNumberOfColumns(); ++thisCol)
        {
            *(myMetricOut->getMapPaletteColorMapping(thisCol)) = *(myMetric->getMapPaletteColorMapping(thisCol));
            const float* myInputData = myMetric->getValuePointerForColumn(thisCol);
            myMetricOut->setColumnName(thisCol, myMetric->getColumnName(thisCol));
            if (badNodeRoi == NULL)
            {
                processColumn(colScratch.data(), myInputData, mySurf, myAreas.data(), dataRoi, distance, nearest, exponent);
            } else {
                if (nearest)
                {
                    processColumn(colScratch.data(), numNodes, myInputData, myNearest);
                } else {
                    processColumn(colScratch.data(), numNodes, myInputData, myStencils);
                }
            }
            myMetricOut->setValuesForColumn(thisCol, colScratch.data());
        }
    } else {
        myMetricOut->setNumberOfNodesAndColumns(numNodes, 1);
        *(myMetricOut->getMapPaletteColorMapping(0)) = *(myMetric->getMapPaletteColorMapping(columnNum));
        const float* myInputData = myMetric->getValuePointerForColumn(columnNum);
        myMetricOut->setColumnName(0, myMetric->getColumnName(columnNum));
        if (badNodeRoi == NULL)
        {
            processColumn(colScratch.data(), myInputData, mySurf, myAreas.data(), dataRoi, distance, nearest, exponent);
        } else {
            if (nearest)
            {
                processColumn(colScratch.data(), numNodes, myInputData, myNearest);
            } else {
                processColumn(colScratch.data(), numNodes, myInputData, myStencils);
            }
        }
        myMetricOut->setValuesForColumn(0, colScratch.data());
    }
}

void AlgorithmMetricDilate::processColumn(float* colScratch, const int& numNodes, const float* myInputData, vector<pair<int, int> > myNearest)
{
    for (int i = 0; i < numNodes; ++i)
    {
        colScratch[i] = myInputData[i];//precopy so that the parallel part doesn't have to worry about vertices that don't get dilated to
    }
    int numStencils = (int)myNearest.size();
#pragma omp CARET_PARFOR schedule(dynamic)
    for (int i = 0; i < numStencils; ++i)//no idea if parallel actually helps here
    {
        const int& node = myNearest[i].first;
        const int& nearest = myNearest[i].second;
        if (nearest != -1)
        {
            colScratch[node] = myInputData[nearest];
        } else {
            colScratch[node] = 0.0f;
        }
    }
}

void AlgorithmMetricDilate::processColumn(float* colScratch, const int& numNodes, const float* myInputData, vector<pair<int, StencilElem> > myStencils)
{
    for (int i = 0; i < numNodes; ++i)
    {
        colScratch[i] = myInputData[i];//precopy so that the parallel part doesn't have to worry about vertices that don't get dilated to
    }
    int numStencils = (int)myStencils.size();
#pragma omp CARET_PARFOR schedule(dynamic)
    for (int i = 0; i < numStencils; ++i)//does parallel help here?
    {
        const int& node = myStencils[i].first;
        const StencilElem& stencil = myStencils[i].second;
        int numWeights = (int)stencil.m_weightlist.size();
        if (numWeights > 0)
        {
            double accum = 0.0;
            for (int j = 0; j < numWeights; ++j)
            {
                accum += myInputData[stencil.m_weightlist[j].first] * stencil.m_weightlist[j].second;
            }
            colScratch[node] = accum / stencil.m_weightsum;
        } else {
            colScratch[node] = 0.0f;
        }
    }
}

void AlgorithmMetricDilate::processColumn(float* colScratch, const float* myInputData, const SurfaceFile* mySurf, const float* myAreas, const MetricFile* dataRoi,
                                          const float& distance, const bool& nearest, const float& exponent)
{
    float cutoffRatio = 1.5f, test = pow(10.0f, 1.0f / exponent);//find what cutoff ratio corresponds to a tenth of weight, but don't use more than a 1.5 * nearest cutoff
    if (test > 1.0f && test < cutoffRatio)//if it is less than 1, the exponent is weird, so simply ignore it and use default
    {
        if (test > 1.1f)
        {
            cutoffRatio = test;
        } else {
            cutoffRatio = 1.1f;
        }
    }
    int numNodes = mySurf->getNumberOfNodes();
    vector<char> charRoi(numNodes);
    const float* dataRoiVals = NULL;
    if (dataRoi != NULL)
    {
        dataRoiVals = dataRoi->getValuePointerForColumn(0);
        for (int i = 0; i < numNodes; ++i)
        {
            if (dataRoiVals[i] > 0.0f && myInputData[i] != 0.0f)
            {
                charRoi[i] = 1;
            } else {
                charRoi[i] = 0;
            }
        }
    } else {
        for (int i = 0; i < numNodes; ++i)
        {
            if (myInputData[i] != 0.0f)
            {
                charRoi[i] = 1;
            } else {
                charRoi[i] = 0;
            }
        }
    }
#pragma omp CARET_PAR
    {
        CaretPointer<TopologyHelper> myTopoHelp = mySurf->getTopologyHelper();
        CaretPointer<GeodesicHelper> myGeoHelp = mySurf->getGeodesicHelper();
#pragma omp CARET_FOR schedule(dynamic)
        for (int i = 0; i < numNodes; ++i)
        {
            if ((dataRoiVals == NULL || dataRoiVals[i] > 0.0f) && myInputData[i] == 0.0f)
            {
                float closestDist;
                int closestNode = myGeoHelp->getClosestNodeInRoi(i, charRoi.data(), distance, closestDist);
                if (closestNode == -1)//check neighbors, to ensure we dilate by at least one node everywhere
                {
                    const vector<int32_t>& nodeList = myTopoHelp->getNodeNeighbors(i);
                    vector<float> distList;
                    myGeoHelp->getGeoToTheseNodes(i, nodeList, distList);//ok, its a little silly to do this
                    const int numInRange = (int)nodeList.size();
                    for (int j = 0; j < numInRange; ++j)
                    {
                        if (charRoi[nodeList[j]] != 0 && (closestNode == -1 || distList[j] < closestDist))
                        {
                            closestNode = nodeList[j];
                            closestDist = distList[j];
                        }
                    }
                }
                if (closestNode == -1)
                {
                    colScratch[i] = 0.0f;
                } else {
                    if (nearest)
                    {
                        colScratch[i] = myInputData[closestNode];
                    } else {
                        vector<int32_t> nodeList;
                        vector<float> distList;
                        myGeoHelp->getNodesToGeoDist(i, closestDist * cutoffRatio, nodeList, distList);
                        int numInRange = (int)nodeList.size();
                        float totalWeight = 0.0f, weightedSum = 0.0f;
                        for (int j = 0; j < numInRange; ++j)
                        {
                            if (charRoi[nodeList[j]] != 0)
                            {
                                float weight;
                                const float tolerance = 0.9f;//distances should NEVER be less than closestDist, for obvious reasons
                                float divdist = distList[j] / closestDist;
                                if (divdist > tolerance)//tricky: if closestDist is zero, this filters between NaN and inf, resulting in a straight average between nodes with 0 distance
                                {
                                    weight = myAreas[nodeList[j]] / pow(divdist, exponent);
                                } else {
                                    weight = myAreas[nodeList[j]] / pow(tolerance, exponent);
                                }
                                totalWeight += weight;
                                weightedSum += myInputData[nodeList[j]] * weight;
                            }
                        }
                        if (totalWeight != 0.0f)
                        {
                            colScratch[i] = weightedSum / totalWeight;
                        } else {
                            colScratch[i] = 0.0f;
                        }
                    }
                }
            } else {
                colScratch[i] = myInputData[i];
            }
        }
    }
}

void AlgorithmMetricDilate::precomputeStencils(vector<pair<int, StencilElem> >& myStencils, const SurfaceFile* mySurf, const float* myAreas,
                                               const MetricFile* badNodeRoi, const MetricFile* dataRoi,
                                               const float& distance, const float& exponent)
{
    CaretAssert(badNodeRoi != NULL);//because it should never be called if we don't know exactly what nodes we are replacing
    const float* badNodeData = badNodeRoi->getValuePointerForColumn(0);
    float cutoffRatio = 1.5f, test = pow(10.0f, 1.0f / exponent);//find what cutoff ratio corresponds to a tenth of weight, but don't use more than a 1.5 * nearest cutoff
    if (test > 1.0f && test < cutoffRatio)//if it is less than 1, the exponent is weird, so simply ignore it and use default
    {
        if (test > 1.1f)
        {
            cutoffRatio = test;
        } else {
            cutoffRatio = 1.1f;
        }
    }
    int numNodes = mySurf->getNumberOfNodes();
    vector<char> charRoi(numNodes);
    const float* dataRoiVals = NULL;
    int badCount = 0;
    if (dataRoi != NULL)
    {
        dataRoiVals = dataRoi->getValuePointerForColumn(0);
        for (int i = 0; i < numNodes; ++i)
        {
            if (!(badNodeData[i] > 0.0f))//in case some clown uses NaN as "bad" in the ROI
            {
                if (dataRoiVals[i] > 0.0f)
                {
                    charRoi[i] = 1;
                } else {
                    charRoi[i] = 0;
                }
            } else {
                ++badCount;
                charRoi[i] = 0;
            }
        }
    } else {
        for (int i = 0; i < numNodes; ++i)
        {
            if (!(badNodeData[i] > 0.0f))
            {
                charRoi[i] = 1;
            } else {
                ++badCount;
                charRoi[i] = 0;
            }
        }
    }
    myStencils.resize(badCount);//initializes all stencils to have empty lists
    badCount = 0;
#pragma omp CARET_PAR
    {
        CaretPointer<TopologyHelper> myTopoHelp = mySurf->getTopologyHelper();
        CaretPointer<GeodesicHelper> myGeoHelp = mySurf->getGeodesicHelper();
#pragma omp CARET_FOR schedule(dynamic)
        for (int i = 0; i < numNodes; ++i)
        {
            if (badNodeData[i] > 0.0f)
            {
                int myIndex;
#pragma omp critical
                {
                    myIndex = badCount;
                    ++badCount;
                }
                myStencils[myIndex].first = i;
                StencilElem& myElem = myStencils[myIndex].second;
                float closestDist;
                int closestNode = myGeoHelp->getClosestNodeInRoi(i, charRoi.data(), distance, closestDist);
                if (closestNode == -1)//check neighbors, to ensure we dilate by at least one node everywhere
                {
                    const vector<int32_t>& nodeList = myTopoHelp->getNodeNeighbors(i);
                    vector<float> distList;
                    myGeoHelp->getGeoToTheseNodes(i, nodeList, distList);//ok, its a little silly to do this
                    const int numInRange = (int)nodeList.size();
                    for (int j = 0; j < numInRange; ++j)
                    {
                        if (charRoi[nodeList[j]] != 0 && (closestNode == -1 || distList[j] < closestDist))
                        {
                            closestNode = nodeList[j];
                            closestDist = distList[j];
                        }
                    }
                }
                if (closestNode != -1)
                {
                    vector<int32_t> nodeList;
                    vector<float> distList;
                    myGeoHelp->getNodesToGeoDist(i, closestDist * cutoffRatio, nodeList, distList);
                    int numInRange = (int)nodeList.size();
                    myElem.m_weightsum = 0.0f;
                    for (int j = 0; j < numInRange; ++j)
                    {
                        if (charRoi[nodeList[j]] != 0)
                        {
                            float weight;
                            const float tolerance = 0.9f;//distances should NEVER be less than closestDist, for obvious reasons
                            float divdist = distList[j] / closestDist;
                            if (divdist > tolerance)//tricky: if closestDist is zero, this filters between NaN and inf, resulting in a straight average between nodes with 0 distance
                            {
                                weight = myAreas[nodeList[j]] / pow(divdist, exponent);
                            } else {
                                weight = myAreas[nodeList[j]] / pow(tolerance, exponent);
                            }
                            myElem.m_weightsum += weight;
                            myElem.m_weightlist.push_back(pair<int, float>(nodeList[j], weight));
                        }
                    }
                    if (myElem.m_weightsum == 0.0f)//set list to empty instead of making NaNs
                    {
                        myElem.m_weightlist.clear();
                    }
                }
            }
        }
    }
}

void AlgorithmMetricDilate::precomputeNearest(vector<pair<int, int> >& myNearest, const SurfaceFile* mySurf, const MetricFile* badNodeRoi, const MetricFile* dataRoi, const float& distance)
{
    CaretAssert(badNodeRoi != NULL);//because it should never be called if we don't know exactly what nodes we are replacing
    const float* badNodeData = badNodeRoi->getValuePointerForColumn(0);
    int numNodes = mySurf->getNumberOfNodes();
    vector<char> charRoi(numNodes);
    const float* dataRoiVals = NULL;
    int badCount = 0;
    if (dataRoi != NULL)
    {
        dataRoiVals = dataRoi->getValuePointerForColumn(0);
        for (int i = 0; i < numNodes; ++i)
        {
            if (!(badNodeData[i] > 0.0f))//in case some clown uses NaN as "bad" in the ROI
            {
                if (dataRoiVals[i] > 0.0f)
                {
                    charRoi[i] = 1;
                } else {
                    charRoi[i] = 0;
                }
            } else {
                ++badCount;
                charRoi[i] = 0;
            }
        }
    } else {
        for (int i = 0; i < numNodes; ++i)
        {
            if (!(badNodeData[i] > 0.0f))
            {
                charRoi[i] = 1;
            } else {
                ++badCount;
                charRoi[i] = 0;
            }
        }
    }
    myNearest.resize(badCount);
    badCount = 0;
#pragma omp CARET_PAR
    {
        CaretPointer<TopologyHelper> myTopoHelp = mySurf->getTopologyHelper();
        CaretPointer<GeodesicHelper> myGeoHelp = mySurf->getGeodesicHelper();
#pragma omp CARET_FOR schedule(dynamic)
        for (int i = 0; i < numNodes; ++i)
        {
            if (badNodeData[i] > 0.0f)
            {
                int myIndex;
#pragma omp critical
                {
                    myIndex = badCount;
                    ++badCount;
                }
                myNearest[myIndex].first = i;
                float closestDist;
                int closestNode = myGeoHelp->getClosestNodeInRoi(i, charRoi.data(), distance, closestDist);
                if (closestNode == -1)//check neighbors, to ensure we dilate by at least one node everywhere
                {
                    const vector<int32_t>& nodeList = myTopoHelp->getNodeNeighbors(i);
                    vector<float> distList;
                    myGeoHelp->getGeoToTheseNodes(i, nodeList, distList);//ok, its a little silly to do this
                    const int numInRange = (int)nodeList.size();
                    for (int j = 0; j < numInRange; ++j)
                    {
                        if (charRoi[nodeList[j]] != 0 && (closestNode == -1 || distList[j] < closestDist))
                        {
                            closestNode = nodeList[j];
                            closestDist = distList[j];
                        }
                    }
                }
                myNearest[myIndex].second = closestNode;
            }
        }
    }
}

float AlgorithmMetricDilate::getAlgorithmInternalWeight()
{
    return 1.0f;//override this if needed, if the progress bar isn't smooth
}

float AlgorithmMetricDilate::getSubAlgorithmWeight()
{
    //return AlgorithmInsertNameHere::getAlgorithmWeight();//if you use a subalgorithm
    return 0.0f;
}
