#ifndef __STRUCTURE_ENUM__H_
#define __STRUCTURE_ENUM__H_

/*LICENSE_START*/
/* 
 *  Copyright 1995-2011 Washington University School of Medicine 
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


#include <stdint.h>
#include <vector>
#include "AString.h"

namespace caret {

/**
 * \brief Enumerated type for a structure in a brain.
 *
 * Enumerated types for the individual structures in a brain.
 */
class StructureEnum {

public:
    /**
     * Enumerated values.
     */
    enum Enum {
        /** Invalid */
        INVALID,
        /** All Strucures */
        ALL,
        /** Left Nucleus Accumbens */
        ACCUMBENS_LEFT,
        /** Right Nucleus Accumbens */
        ACCUMBENS_RIGHT,
        /** Left Amygdala */
        AMYGDALA_LEFT,
        /** Right Amygdala */
        AMYGDALA_RIGHT,
        /** Brain Stem */
        BRAIN_STEM,
        /** Left Caudate */
        CAUDATE_LEFT,
        /** Right Caudate */
        CAUDATE_RIGHT,
        /** Cerebellum */
        CEREBELLUM,
        /** Left Cerebellum */
        CEREBELLUM_LEFT,
        /** Right Cerebellum */
        CEREBELLUM_RIGHT,
        /** Left Cerebral Cortex */
        CORTEX_LEFT,
        /** Right Cerebral Cortex*/
        CORTEX_RIGHT,
        /** Left Ventral Diencephalon */
        DIENCEPHALON_VENTRAL_LEFT,
        /** Right Ventral Diencephalon */
        DIENCEPHALON_VENTRAL_RIGHT,
        /** Left Hippocampus */
        HIPPOCAMPUS_LEFT,
        /** Right Hippocampus */
        HIPPOCAMPUS_RIGHT,
        /** Left Pallidum */
        PALLIDUM_LEFT,
        /** Right Pallidum */
        PALLIDUM_RIGHT,
        /** Other structure not specified */
        OTHER,
        /** Left Putamen */
        PUTAMEN_LEFT,
        /** Right Putamen */
        PUTAMEN_RIGHT,
        /** Left Subcortical White Matter */
        SUBCORTICAL_WHITE_MATTER_LEFT,
        /** Right Subcortical White Matter */
        SUBCORTICAL_WHITE_MATTER_RIGHT,
        /** Left Thalamus */
        THALAMUS_LEFT,
        /** Right Thalamus */
        THALAMUS_RIGHT
    };


    ~StructureEnum();

    static AString toName(Enum enumValue);
    
    static Enum fromName(const AString& name, bool* isValidOut);
    
    static AString toGuiName(Enum enumValue);
    
    static Enum fromGuiName(const AString& guiName, bool* isValidOut);
    
    static int32_t toIntegerCode(Enum enumValue);
    
    static Enum fromIntegerCode(const int32_t integerCode, bool* isValidOut);

    static void getAllEnums(std::vector<Enum>& allEnums);

private:
    StructureEnum(const Enum enumValue, 
                 const AString& name,
                 const AString& guiName);

    static const StructureEnum* findData(const Enum enumValue);

    /** Holds all instance of enum values and associated metadata */
    static std::vector<StructureEnum> enumData;

    /** Initialize instances that contain the enum values and metadata */
    static void initialize();

    /** Indicates instance of enum values and metadata have been initialized */
    static bool initializedFlag;
    
    static int32_t integerCodeGenerator;
    
    /** The enumerated type value for an instance */
    Enum enumValue;

    /** The integer code associated with an enumerated value */
    int32_t integerCode;

    /** The name, a text string that is identical to the enumerated value */
    AString name;
    
    /** A user-friendly name that is displayed in the GUI */
    AString guiName;
};

#ifdef __STRUCTURE_ENUM_DECLARE__
std::vector<StructureEnum> StructureEnum::enumData;
bool StructureEnum::initializedFlag = false;
    int32_t StructureEnum::integerCodeGenerator = 0;
#endif // __STRUCTURE_ENUM_DECLARE__

} // namespace
#endif  //__STRUCTURE_ENUM__H_
