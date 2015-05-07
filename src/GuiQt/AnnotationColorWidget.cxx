
/*LICENSE_START*/
/*
 *  Copyright (C) 2015 Washington University School of Medicine
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

#define __ANNOTATION_COLOR_WIDGET_DECLARE__
#include "AnnotationColorWidget.h"
#undef __ANNOTATION_COLOR_WIDGET_DECLARE__

#include <QAction>
#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QLabel>
#include <QPainter>
#include <QToolButton>
#include <QVBoxLayout>

#include "AnnotationOneDimensionalShape.h"
#include "AnnotationTwoDimensionalShape.h"
#include "BrainOpenGL.h"
#include "CaretAssert.h"
#include "CaretColorEnumMenu.h"
#include "EventGraphicsUpdateOneWindow.h"
#include "EventManager.h"
#include "WuQFactory.h"
#include "WuQtUtilities.h"

using namespace caret;


    
/**
 * \class caret::AnnotationColorWidget 
 * \brief Widget for annotation color selection.
 * \ingroup GuiQt
 */

/**
 * Constructor.
 *
 * @param parent
 *     Parent for this widget.
 */
AnnotationColorWidget::AnnotationColorWidget(const int32_t browserWindowIndex,
                                             QWidget* parent)
: QWidget(parent),
m_browserWindowIndex(browserWindowIndex)
{
    m_annotation = NULL;
    
    QLabel* fillLabel      = new QLabel("Fill");
    QLabel* fillColorLabel = new QLabel("Color");
    QLabel* lineLabel      = new QLabel("Line");
    QLabel* lineColorLabel = new QLabel("Color");
    QLabel* lineWidthLabel = new QLabel("Width");
    
    const QSize toolButtonSize(16, 16);
    
    /*
     * Background color menu
     */
    m_backgroundColorMenu = new CaretColorEnumMenu((CaretColorEnum::OPTION_INCLUDE_CUSTOM_COLOR
                                                    | CaretColorEnum::OPTION_INCLUDE_NONE_COLOR));
    QObject::connect(m_backgroundColorMenu, SIGNAL(colorSelected(const CaretColorEnum::Enum)),
                     this, SLOT(backgroundColorSelected(const CaretColorEnum::Enum)));
    
    /*
     * Background action and tool button
     */
    m_backgroundColorAction = new QAction("B",
                                          this);
    m_backgroundColorAction->setToolTip("Adjust the fill color");
    m_backgroundColorAction->setMenu(m_backgroundColorMenu);
    m_backgroundToolButton = new QToolButton();
    m_backgroundToolButton->setDefaultAction(m_backgroundColorAction);
    m_backgroundToolButton->setIconSize(toolButtonSize);
    
    /*
     * Foreground color menu
     */
    m_foregroundColorMenu = new CaretColorEnumMenu((CaretColorEnum::OPTION_INCLUDE_CUSTOM_COLOR
                                                    | CaretColorEnum::OPTION_INCLUDE_NONE_COLOR));
    QObject::connect(m_foregroundColorMenu, SIGNAL(colorSelected(const CaretColorEnum::Enum)),
                     this, SLOT(foregroundColorSelected(const CaretColorEnum::Enum)));
    
    /*
     * Foreground color action and toolbutton
     */
    m_foregroundColorAction = new QAction("F",
                                          this);
    m_foregroundColorAction->setToolTip("Adjust the line color");
    m_foregroundColorAction->setMenu(m_foregroundColorMenu);
    m_foregroundToolButton = new QToolButton();
    m_foregroundToolButton->setDefaultAction(m_foregroundColorAction);
    m_foregroundToolButton->setIconSize(toolButtonSize);
    
    /*
     * Foreground thickness
     */
    float minimumLineWidth = 0.0;
    float maximumLineWidth = 1.0;
    
    BrainOpenGL::getMinMaxLineWidth(minimumLineWidth,
                                    maximumLineWidth);
    minimumLineWidth = std::max(minimumLineWidth, 1.0f);
    m_foregroundThicknessSpinBox = WuQFactory::newDoubleSpinBoxWithMinMaxStepDecimalsSignalDouble(minimumLineWidth,
                                                                                                  maximumLineWidth,
                                                                                                  1.0,
                                                                                                  0,
                                                                                                  this,
                                                                                                  SLOT(foregroundThicknessSpinBoxValueChanged(double)));
    WuQtUtilities::setWordWrappedToolTip(m_foregroundThicknessSpinBox,
                                         "Adjust the line thickness");
    m_foregroundThicknessSpinBox->setFixedWidth(45);
    
    /*
     * Layout widgets
     */
    QGridLayout* gridLayout = new QGridLayout(this);
    WuQtUtilities::setLayoutSpacingAndMargins(gridLayout, 2, 0);
    gridLayout->addWidget(lineLabel,
                          0, 0,
                          1, 2,
                          Qt::AlignHCenter);
    gridLayout->addWidget(lineWidthLabel,
                          1, 0,
                          Qt::AlignHCenter);
    gridLayout->addWidget(lineColorLabel,
                          1, 1,
                          Qt::AlignHCenter);
    gridLayout->addWidget(m_foregroundThicknessSpinBox,
                          2, 0,
                          Qt::AlignHCenter);
    gridLayout->addWidget(m_foregroundToolButton,
                          2, 1,
                          Qt::AlignHCenter);
    gridLayout->addWidget(fillLabel,
                          0, 2,
                          Qt::AlignHCenter);
    gridLayout->addWidget(fillColorLabel,
                          1, 2,
                          Qt::AlignHCenter);
    gridLayout->addWidget(m_backgroundToolButton,
                          2, 2,
                          Qt::AlignHCenter);
    
//    QVBoxLayout* layout = new QVBoxLayout(this);
//    WuQtUtilities::setLayoutSpacingAndMargins(gridLayout, 0, 0);
//    layout->addLayout(gridLayout);
//    layout->addStretch();
    
    
//    setSizePolicy(QSizePolicy::Fixed,
//                  QSizePolicy::Fixed);
    
    backgroundColorSelected(CaretColorEnum::WHITE);
    foregroundColorSelected(CaretColorEnum::BLACK);
}

/**
 * Destructor.
 */
AnnotationColorWidget::~AnnotationColorWidget()
{
    EventManager::get()->removeAllEventsFromListener(this);
}

/**
 * Update with the given annotation.
 *
 * @param annotation.
 */
void
AnnotationColorWidget::updateContent(Annotation* annotation)
{
    m_annotation = annotation;
    
    if (m_annotation != NULL) {
    }
    else {
    }
    
    updateBackgroundColorButton();
    updateForegroundColorButton();
    updateForegroundThicknessSpinBox();
}

/**
 * Gets called when the background color is changed.
 *
 * @param caretColor
 *     Color that was selected.
 */
void
AnnotationColorWidget::backgroundColorSelected(const CaretColorEnum::Enum caretColor)
{
    if (m_annotation != NULL) {
        m_annotation->setBackgroundColor(caretColor);
    }

    updateBackgroundColorButton();
    EventManager::get()->sendEvent(EventGraphicsUpdateOneWindow(m_browserWindowIndex).getPointer());
}

/**
 * Update the background color.
 */
void
AnnotationColorWidget::updateBackgroundColorButton()
{
    CaretColorEnum::Enum colorEnum = CaretColorEnum::BLACK;
    float rgba[4];
    CaretColorEnum::toRGBFloat(colorEnum, rgba);
    rgba[3] = 1.0;
    
    if (m_annotation != NULL) {
        colorEnum = m_annotation->getBackgroundColor();
        m_annotation->getBackgroundColorRGBA(rgba);
    }
    
//    QPixmap pm(24, 24);
//    pm.fill(QColor::fromRgbF(rgba[0],
//                             rgba[1],
//                             rgba[2]));
    
    QPixmap pm = WuQtUtilities::createCaretColorEnumPixmap(m_backgroundToolButton, 24, 24, colorEnum, rgba, false);
    QIcon icon(pm);
    
    m_backgroundColorAction->setIcon(icon);
    m_backgroundColorMenu->setSelectedColor(colorEnum);
}


/**
 * Update the foreground color.
 */
void
AnnotationColorWidget::updateForegroundColorButton()
{
    CaretColorEnum::Enum colorEnum = CaretColorEnum::WHITE;
    float rgba[4];
    CaretColorEnum::toRGBFloat(colorEnum, rgba);
    rgba[3] = 1.0;
    
    if (m_annotation != NULL) {
        colorEnum = m_annotation->getForegroundColor();
        m_annotation->getForegroundColorRGBA(rgba);
    }
    
    
    
//    /*
//     * Get the toolbutton's background color
//     */
//    const QPalette tbPalette = m_foregroundToolButton->palette();
//    const QPalette::ColorRole backgroundRole = m_foregroundToolButton->backgroundRole();
//    const QBrush brush = tbPalette.brush(backgroundRole);
//    const QColor toolButtonBackgroundColor = brush.color();
//    
//    /*
//     * Create a small, square pixmap that will contain
//     * the foreground color around the pixmap's perimeter.
//     */
//    const int width  = 24;
//    const int height = 24;
//    QPixmap pm(width,
//               height);
//    
//    /*
//     * Create a painter and fill the pixmap with
//     * the background color
//     */
//    QPainter painter(&pm);
//    painter.setBackgroundMode(Qt::OpaqueMode);
//    painter.fillRect(pm.rect(), toolButtonBackgroundColor);
//    
//    /*
//     * Draw lines (rectangle) around the perimeter of
//     * the pixmap
//     */
//    painter.setPen(QColor::fromRgbF(rgba[0],
//                                    rgba[1],
//                                    rgba[2]));
//    for (int32_t i = 0; i < 3; i++) {
//        painter.drawRect(i, i,
//                         width - 1 - i * 2, height - 1 - i * 2);
//    }

    
    QPixmap pm = WuQtUtilities::createCaretColorEnumPixmap(m_foregroundToolButton, 24, 24, colorEnum, rgba, true);
    m_foregroundColorAction->setIcon(QIcon(pm));
    
    
    
    
    m_foregroundColorMenu->setSelectedColor(colorEnum);
}

/**
 * Gets called when the foreground color is changed.
 *
 * @param caretColor
 *     Color that was selected.
 */
void
AnnotationColorWidget::foregroundColorSelected(const CaretColorEnum::Enum caretColor)
{
    if (m_annotation != NULL) {
        m_annotation->setForegroundColor(caretColor);
    }
    updateForegroundColorButton();
    EventManager::get()->sendEvent(EventGraphicsUpdateOneWindow(m_browserWindowIndex).getPointer());
}

/**
 * Gets called when the foreground thickness value changes.
 *
 * @param value
 *     New value for foreground thickness.
 */
void
AnnotationColorWidget::foregroundThicknessSpinBoxValueChanged(double value)
{
    bool updateGraphicsFlag = false;
    if (m_annotation != NULL) {
        AnnotationOneDimensionalShape* oneDimAnn = dynamic_cast<AnnotationOneDimensionalShape*>(m_annotation);
        if (oneDimAnn != NULL) {
            oneDimAnn->setLineWidth(value);
            updateGraphicsFlag = true;
        }
        else {
            AnnotationTwoDimensionalShape* twoDimAnn = dynamic_cast<AnnotationTwoDimensionalShape*>(m_annotation);
            if (twoDimAnn != NULL) {
                twoDimAnn->setOutlineWidth(value);
                updateGraphicsFlag = true;
            }
        }
    }
    
    if (updateGraphicsFlag) {
        EventManager::get()->sendEvent(EventGraphicsUpdateOneWindow(m_browserWindowIndex).getPointer());
    }
}

/**
 * Update the foreground thickness spin box.
 */
void
AnnotationColorWidget::updateForegroundThicknessSpinBox()
{
    float value = 0.0;
    bool widgetEnabled = false;
    if (m_annotation != NULL) {
        AnnotationOneDimensionalShape* oneDimAnn = dynamic_cast<AnnotationOneDimensionalShape*>(m_annotation);
        if (oneDimAnn != NULL) {
            value = oneDimAnn->getLineWidth();
            widgetEnabled = true;
        }
        else {
            AnnotationTwoDimensionalShape* twoDimAnn = dynamic_cast<AnnotationTwoDimensionalShape*>(m_annotation);
            if (twoDimAnn != NULL) {
                if (twoDimAnn->getType() != AnnotationTypeEnum::TEXT) {
                    value = twoDimAnn->getOutlineWidth();
                    widgetEnabled = true;
                }
            }
        }
    }
    
    m_foregroundThicknessSpinBox->blockSignals(true);
    m_foregroundThicknessSpinBox->setValue(value);
    m_foregroundThicknessSpinBox->blockSignals(false);
    m_foregroundThicknessSpinBox->setEnabled(widgetEnabled);
    
}
/**
 * Receive an event.
 *
 * @param event
 *    An event for which this instance is listening.
 */
void
AnnotationColorWidget::receiveEvent(Event* event)
{
//    if (event->getEventType() == EventTypeEnum::) {
//        <EVENT_CLASS_NAME*> eventName = dynamic_cast<EVENT_CLASS_NAME*>(event);
//        CaretAssert(eventName);
//
//        event->setEventProcessed();
//    }
}
