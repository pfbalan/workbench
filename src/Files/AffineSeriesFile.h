#ifndef __AFFINE_SERIES_FILE_H__
#define __AFFINE_SERIES_FILE_H__

/*LICENSE_START*/
/*
 *  Copyright (C) 2020  Washington University School of Medicine
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
/*LICENSE_END*/

#include "AString.h"
#include "FloatMatrix.h"

#include <vector>

namespace caret {

    class AffineSeriesFile
    {
        std::vector<FloatMatrix> m_matrixList;
    public:
        void readWorld(const AString& filename, const bool inverse = false);//forward nifti coordinate transform, optionally reverse
        void writeWorld(const AString& filename, const bool inverse = false) const;
        void readFlirt(const AString& filename, const AString& sourceName, const AString& targetName);//flirt convention matrix, requires source/target volumes
        void writeFlirt(const AString& filename, const AString& sourceName, const AString& targetName) const;
        //no ITK format, they have an extra 3 parameters in their affine
        const std::vector<FloatMatrix>& getMatrixList() const { return m_matrixList; }
        std::vector<FloatMatrix> getInverseMatrixList() const;//convenience function, because it is a little annoying to do when needed
        void setMatrixList(const std::vector<FloatMatrix>& matrixList);//needs to do sanity checking, so don't inline
    };

}

#endif //__AFFINE_SERIES_FILE_H__
