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
/*LICENSE_END*/

#include <sstream>

#include "CaretAssert.h"
#include "CaretLogger.h"
#include "GiftiXmlElements.h"
#include "SpecFile.h"
#include "SpecFileSaxReader.h"
#include "GiftiMetaDataSaxReader.h"

#include "XmlAttributes.h"
#include "XmlException.h"

using namespace caret;

/**
 * constructor.
 */
SpecFileSaxReader::SpecFileSaxReader(SpecFile* specFileIn)
{
    CaretAssert(specFileIn);
   this->specFile = specFileIn;
   this->state = STATE_NONE;
   this->stateStack.push(this->state);
   this->elementText = "";
   this->metaDataSaxReader = NULL;
}

/**
 * destructor.
 */
SpecFileSaxReader::~SpecFileSaxReader()
{
}


/**
 * start an element.
 */
void 
SpecFileSaxReader::startElement(const AString& namespaceURI,
                                         const AString& localName,
                                         const AString& qName,
                                         const XmlAttributes& attributes)  throw (XmlSaxParserException)
{
   const STATE previousState = this->state;
   switch (this->state) {
      case STATE_NONE:
           if (qName == SpecFile::XML_TAG_SPEC_FILE) {
            this->state = STATE_SPEC_FILE;
            
            //
            // Check version of file being read
            //
             const float version = attributes.getValueAsFloat(SpecFile::XML_ATTRIBUTE_VERSION);
            if (version > SpecFile::getFileVersion()) {
                AString msg =
                   "File version is " 
                + AString::number(version) 
                + " but versions newer than "
                + SpecFile::getFileVersionAsString()
                + " are not supported.  Update your software.";
                XmlSaxParserException e(msg);
                CaretLogThrowing(e);
                throw e;
            }
            else if (version < 1.0) {
                AString msg =
                "File version is " 
                + AString::number(version) 
                + " but versions before"
                + SpecFile::getFileVersionAsString()
                + " are not supported.  Update your software.";
                XmlSaxParserException e(msg);
                CaretLogThrowing(e);
                throw e;
            }
         }
         else {
             const AString msg =
             "Root elements is "
             + qName
             + " but should be "
             + SpecFile::XML_TAG_SPEC_FILE;
             XmlSaxParserException e(msg);
             CaretLogThrowing(e);
             throw e;
         }
         break;
      case STATE_SPEC_FILE:
         if (qName == GiftiXmlElements::TAG_METADATA) {
             this->state = STATE_METADATA;
             this->metaDataSaxReader = new GiftiMetaDataSaxReader(this->specFile->getMetaData());
             this->metaDataSaxReader->startElement(namespaceURI, localName, qName, attributes);
         }
         else if (qName == SpecFile::XML_TAG_DATA_FILE) {
             this->state = STATE_DATA_FILE;
             this->fileAttributeStructureName = attributes.getValue(SpecFile::XML_ATTRIBUTE_STRUCTURE);
             this->fileAttributeTypeName = attributes.getValue(SpecFile::XML_ATTRIBUTE_DATA_FILE_TYPE);
             this->fileAttributeSelectionStatus = attributes.getValueAsBoolean(SpecFile::XML_ATTRIBUTE_SELECTED,
                                                                               false);
         }
         else {
             const AString msg =
             "Invalid child of "
             + SpecFile::XML_TAG_SPEC_FILE
             + " is "
             + qName;
             XmlSaxParserException e(msg);
             CaretLogThrowing(e);
             throw e;
         }
         break;
      case STATE_METADATA:
           this->metaDataSaxReader->startElement(namespaceURI, localName, qName, attributes);
         break;
      case STATE_DATA_FILE:
           break;
   }
   
   //
   // Save previous state
   //
   stateStack.push(previousState);
   
   elementText = "";
}

/**
 * end an element.
 */
void 
SpecFileSaxReader::endElement(const AString& namespaceURI,
                                       const AString& localName,
                                       const AString& qName) throw (XmlSaxParserException)
{
   switch (this->state) {
      case STATE_NONE:
         break;
      case STATE_SPEC_FILE:
         break;
      case STATE_METADATA:
           this->metaDataSaxReader->endElement(namespaceURI, localName, qName);
           if (qName == GiftiXmlElements::TAG_METADATA) {
               delete this->metaDataSaxReader;
               this->metaDataSaxReader = NULL;
           }
         break;
      case STATE_DATA_FILE:
       {
           try {
               const AString filename = this->elementText.trimmed();
               this->specFile->addDataFile(this->fileAttributeTypeName, 
                                           this->fileAttributeStructureName, 
                                           filename,
                                           this->fileAttributeSelectionStatus);
               
               this->fileAttributeTypeName = "";
               this->fileAttributeStructureName = "";
               this->fileAttributeSelectionStatus = false;
           }
           catch (const DataFileException& e) {
               throw XmlSaxParserException(e);
           }
       }
         break;
   }

   //
   // Clear out for new elements
   //
   this->elementText = "";
   
   //
   // Go to previous state
   //
   if (this->stateStack.empty()) {
       throw XmlSaxParserException("State stack is empty while reading XML NiftDataFile.");
   }
   this->state = stateStack.top();
   this->stateStack.pop();
}

/**
 * get characters in an element.
 */
void 
SpecFileSaxReader::characters(const char* ch) throw (XmlSaxParserException)
{
    if (this->metaDataSaxReader != NULL) {
        this->metaDataSaxReader->characters(ch);
    }
    else {
        elementText += ch;
    }
}

/**
 * a fatal error occurs.
 */
void 
SpecFileSaxReader::fatalError(const XmlSaxParserException& e) throw (XmlSaxParserException)
{
/*
   std::ostringstream str;
   str << "Fatal Error at line number: " << e.getLineNumber() << "\n"
       << "Column number: " << e.getColumnNumber() << "\n"
       << "Message: " << e.whatString();
   if (errorMessage.isEmpty() == false) {
      str << "\n"
          << errorMessage;
   }
   errorMessage = str.str();
*/   
   //
   // Stop parsing
   //
   throw e;
}

// a warning occurs
void 
SpecFileSaxReader::warning(const XmlSaxParserException& e) throw (XmlSaxParserException)
{    
    CaretLogWarning("XML Parser Warning: " + e.whatString());
}

// an error occurs
void 
SpecFileSaxReader::error(const XmlSaxParserException& e) throw (XmlSaxParserException)
{   
    CaretLogSevere("XML Parser Error: " + e.whatString());
    throw e;
}

void 
SpecFileSaxReader::startDocument()  throw (XmlSaxParserException)
{    
}

void 
SpecFileSaxReader::endDocument() throw (XmlSaxParserException)
{
}

