
/*LICENSE_START*/
/*
 *  Copyright (C) 2018 Washington University School of Medicine
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

#define __WU_Q_MACRO_SIGNAL_WATCHER_DECLARE__
#include "WuQMacroSignalWatcher.h"
#undef __WU_Q_MACRO_SIGNAL_WATCHER_DECLARE__

#include <QAction>
#include <QActionGroup>
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QPushButton>
#include <QRadioButton>
#include <QSlider>
#include <QSpinBox>
#include <QTabBar>
#include <QTabWidget>
#include <QToolButton>

#include "CaretAssert.h"
#include "CaretLogger.h"
#include "WuQMacroCommand.h"
#include "WuQMacroCommandParameter.h"
#include "WuQMacroManager.h"

using namespace caret;

/**
 * \class caret::WuQMacroSignalWatcher 
 * \brief Watches a QObject instance to observe its "value changed" signal
 * \ingroup WuQMacro
 */

/**
 * Constructor.
 *
 * @param parentMacroManager
 *     Parent macro manager.
 * @param object
 *     Object that is watched for a "value changed" signal
 * @param objectType
 *     The type of the object.
 * @param descriptiveName
 *     Descriptive name shown to user in macro command
 * @param toolTipTextOverride
 *     Used to override tool tip or for when object does not
 *     support a tool tip.
 */
WuQMacroSignalWatcher::WuQMacroSignalWatcher(WuQMacroManager* parentMacroManager,
                                             QObject* object,
                                             const WuQMacroClassTypeEnum::Enum objectType,
                                             const QString& descriptiveName,
                                             const QString& toolTipTextOverride)
: QObject(),
m_parentMacroManager(parentMacroManager),
m_object(object),
m_objectType(objectType),
m_descriptiveName(descriptiveName),
m_objectName(object->objectName())
{
    CaretAssert(m_parentMacroManager);
    CaretAssert(m_object);
    
    QWidget* widget = qobject_cast<QWidget*>(m_object);
    if (widget != NULL) {
        m_toolTipText = widget->toolTip();
    }
    
    switch (m_objectType) {
        case WuQMacroClassTypeEnum::ACTION:
        {
            QAction* action = qobject_cast<QAction*>(m_object);
            CaretAssert(action);
            QObject::connect(action, &QAction::triggered,
                             this, &WuQMacroSignalWatcher::actionTriggered);
            m_toolTipText = action->toolTip();
        }
            break;
        case WuQMacroClassTypeEnum::ACTION_CHECKABLE:
        {
            QAction* action = qobject_cast<QAction*>(m_object);
            CaretAssert(action);
            QObject::connect(action, &QAction::triggered,
                             this, &WuQMacroSignalWatcher::actionCheckableTriggered);
            m_toolTipText = action->toolTip();
        }
            break;
        case WuQMacroClassTypeEnum::ACTION_GROUP:
        {
            QActionGroup* actionGroup = qobject_cast<QActionGroup*>(m_object);
            CaretAssert(actionGroup);
            QObject::connect(actionGroup, &QActionGroup::triggered,
                             this, &WuQMacroSignalWatcher::actionGroupTriggered);
        }
            break;
        case WuQMacroClassTypeEnum::BUTTON_GROUP:
        {
            QButtonGroup* buttonGroup = qobject_cast<QButtonGroup*>(m_object);
            CaretAssert(buttonGroup);
            QObject::connect(buttonGroup, static_cast<void (QButtonGroup::*)(QAbstractButton*)>(&QButtonGroup::buttonClicked),
                             this, &WuQMacroSignalWatcher::buttonGroupButtonClicked);
        }
            break;
        case WuQMacroClassTypeEnum::CHECK_BOX:
        {
            QCheckBox* checkBox = qobject_cast<QCheckBox*>(m_object);
            CaretAssert(checkBox);
            QObject::connect(checkBox, &QCheckBox::clicked,
                             this, &WuQMacroSignalWatcher::checkBoxClicked);
        }
            break;
        case WuQMacroClassTypeEnum::COMBO_BOX:
        {
            QComboBox* comboBox = qobject_cast<QComboBox*>(m_object);
            CaretAssert(comboBox);
            QObject::connect(comboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::activated),
                             this, &WuQMacroSignalWatcher::comboBoxActivated);
        }
            break;
        case WuQMacroClassTypeEnum::DOUBLE_SPIN_BOX:
        {
            QDoubleSpinBox* spinBox = qobject_cast<QDoubleSpinBox*>(m_object);
            CaretAssert(spinBox);
            QObject::connect(spinBox, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
                             this, &WuQMacroSignalWatcher::doubleSpinBoxValueChanged);
        }
            break;
        case WuQMacroClassTypeEnum::INVALID:
            CaretAssert(0);
            break;
        case WuQMacroClassTypeEnum::LINE_EDIT:
        {
            QLineEdit* lineEdit = qobject_cast<QLineEdit*>(m_object);
            CaretAssert(lineEdit);
            QObject::connect(lineEdit, &QLineEdit::textEdited,
                             this, &WuQMacroSignalWatcher::lineEditTextEdited);
        }
            break;
        case WuQMacroClassTypeEnum::LIST_WIDGET:
        {
            QListWidget* listWidget = qobject_cast<QListWidget*>(m_object);
            CaretAssert(listWidget);
            QObject::connect(listWidget, &QListWidget::itemActivated,
                             this, &WuQMacroSignalWatcher::listWidgetItemActivated);
        }
            break;
        case WuQMacroClassTypeEnum::MENU:
        {
            QMenu* menu = qobject_cast<QMenu*>(m_object);
            CaretAssert(menu);
            QObject::connect(menu, &QMenu::triggered,
                             this, &WuQMacroSignalWatcher::menuTriggered);
        }
            break;
        case WuQMacroClassTypeEnum::MOUSE_USER_EVENT:
            CaretAssertToDoFatal();
            break;
        case WuQMacroClassTypeEnum::PUSH_BUTTON:
        {
            QPushButton* pushButton = qobject_cast<QPushButton*>(m_object);
            CaretAssert(pushButton);
            QObject::connect(pushButton, &QPushButton::clicked,
                             this, &WuQMacroSignalWatcher::pushButtonClicked);
        }
            break;
        case WuQMacroClassTypeEnum::PUSH_BUTTON_CHECKABLE:
        {
            QPushButton* pushButton = qobject_cast<QPushButton*>(m_object);
            CaretAssert(pushButton);
            QObject::connect(pushButton, &QPushButton::clicked,
                             this, &WuQMacroSignalWatcher::pushButtonClicked);
        }
            break;
        case WuQMacroClassTypeEnum::RADIO_BUTTON:
        {
            QRadioButton* radioButton = qobject_cast<QRadioButton*>(m_object);
            CaretAssert(radioButton);
            QObject::connect(radioButton, &QRadioButton::clicked,
                             this, &WuQMacroSignalWatcher::radioButtonClicked);
        }
            break;
        case WuQMacroClassTypeEnum::SLIDER:
        {
            QSlider* slider = qobject_cast<QSlider*>(m_object);
            CaretAssert(slider);
            QObject::connect(slider, &QSlider::valueChanged,
                             this, &WuQMacroSignalWatcher::sliderValueChanged);
        }
            break;
        case WuQMacroClassTypeEnum::SPIN_BOX:
        {
            QSpinBox* spinBox = qobject_cast<QSpinBox*>(m_object);
            CaretAssert(spinBox);
            QObject::connect(spinBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
                             this, &WuQMacroSignalWatcher::spinBoxValueChanged);
        }
            break;
        case WuQMacroClassTypeEnum::TAB_BAR:
        {
            QTabBar* tabBar = qobject_cast<QTabBar*>(m_object);
            CaretAssert(tabBar);
            QObject::connect(tabBar, &QTabBar::tabBarClicked,
                             this, &WuQMacroSignalWatcher::tabBarCurrentChanged);
        }
            break;
        case WuQMacroClassTypeEnum::TAB_WIDGET:
        {
            QTabWidget* tabWidget = qobject_cast<QTabWidget*>(m_object);
            CaretAssert(tabWidget);
            QObject::connect(tabWidget, &QTabWidget::tabBarClicked,
                             this, &WuQMacroSignalWatcher::tabWidgetCurrentChanged);
        }
            break;
        case WuQMacroClassTypeEnum::TOOL_BUTTON:
        {
            QToolButton* toolButton = qobject_cast<QToolButton*>(m_object);
            CaretAssert(toolButton);
            QObject::connect(toolButton, &QCheckBox::clicked,
                             this, &WuQMacroSignalWatcher::toolButtonClicked);
        }
            break;
        case WuQMacroClassTypeEnum::TOOL_BUTTON_CHECKABLE:
        {
            QToolButton* toolButton = qobject_cast<QToolButton*>(m_object);
            CaretAssert(toolButton);
            QObject::connect(toolButton, &QCheckBox::clicked,
                             this, &WuQMacroSignalWatcher::toolButtonClicked);
        }
            break;
    }
    
    /*
     * Override the tool tip text
     */
    if ( ! toolTipTextOverride.isEmpty()) {
        m_toolTipText = toolTipTextOverride;
    }
    
    QObject::connect(m_object, &QObject::destroyed,
                     this, &WuQMacroSignalWatcher::objectWasDestroyed);
    QObject::connect(m_object, &QObject::objectNameChanged,
                     this, &WuQMacroSignalWatcher::objectNameWasChanged);
}

/**
 * Destructor.
 */
WuQMacroSignalWatcher::~WuQMacroSignalWatcher()
{
}

/**
 * Called if the object whose signal is being monitored is destroyed
 *
 * @obj
 *    Pointer to object that was destroyed
 */
void
WuQMacroSignalWatcher::objectWasDestroyed(QObject* /*obj*/)
{
    /*
     * Log object destroyed only when NOT debug
     */
#ifndef NDEBUG
    /* disable as need way to disallow this while a window is closing or application exiting */
    const bool allowMessageFlag(false);
    if (allowMessageFlag) {
        CaretLogWarning("Object was destroyed: "
                        + m_objectName);
    }
#endif
}

/**
 * Called if the object whose signal is being monitored 
 * has its name changed
 *
 * @name
 *    New name
 */
void
WuQMacroSignalWatcher::objectNameWasChanged(const QString& name)
{
    std::cout << "Object name changed from "
    << m_objectName << " to " << name << std::endl;
}

/**
 * Create an new instance of a widget signal watcher for
 * the given object.
 *
 * @param parentMacroManager
 *     Parent macro manager.
 * @param object
 *     Object that will have a widget signal watcher.
 * @param descriptiveName
 *     Descriptive name shown to user in macro command
 * @param toolTipTextOverride
 *     Used to override tool tip or for when object does not
 *     support a tool tip.
 * @param errorMessageOut
 *     Output containing error information if failure.
 * @return
 *     Pointer to widget watcher or NULL if there was an error.
 */
WuQMacroSignalWatcher*
WuQMacroSignalWatcher::newInstance(WuQMacroManager* parentMacroManager,
                                   QObject* object,
                                   const QString& descriptiveName,
                                   const QString& toolTipTextOverride,
                                   QString& errorMessageOut)
{
    errorMessageOut.clear();
    
    QString objectClassName = object->metaObject()->className();
    
    bool validFlag(false);
    WuQMacroClassTypeEnum::Enum objectType = WuQMacroClassTypeEnum::fromGuiName(objectClassName,
                                                                                  &validFlag);

    /*
     * Some Qt Widgets may have a 'checkable' state
     * and the 'checkable' and 'non-checkable' states
     * must be handled differently.
     */
    switch (objectType) {
        case WuQMacroClassTypeEnum::ACTION:
        {
            /*
             * Actions may have a 'checkable' status enabled
             * Actions may also be in a QActionGroup and the
             * QActionGroup may have an 'exclusive' status.
             *
             * NOTE: For this logic to work, the actions must have
             * macro support added after the actions are placed
             * in an exclusive action group
             */
            QAction* action = qobject_cast<QAction*>(object);
            CaretAssert(action);
            if (action->isCheckable()) {
                /*
                 * Probably checkable
                 */
                objectType = WuQMacroClassTypeEnum::ACTION_CHECKABLE;
                
                const QActionGroup* actionGroup = action->actionGroup();
                if (actionGroup != NULL) {
                    if (actionGroup->isExclusive()) {
                        /*
                         * In an exclusive group, actions CANNOT be
                         * uchecked so treat as non-checkable action
                         */
                        objectType = WuQMacroClassTypeEnum::ACTION;
                    }
                }
            }
        }
            break;
        case WuQMacroClassTypeEnum::ACTION_CHECKABLE:
            CaretAssertMessage(0, "ACTION_CHECKABLE is created by ACTION above");
            break;
        case WuQMacroClassTypeEnum::ACTION_GROUP:
            break;
        case WuQMacroClassTypeEnum::BUTTON_GROUP:
            break;
        case WuQMacroClassTypeEnum::CHECK_BOX:
            break;
        case WuQMacroClassTypeEnum::COMBO_BOX:
            break;
        case WuQMacroClassTypeEnum::DOUBLE_SPIN_BOX:
            break;
        case WuQMacroClassTypeEnum::INVALID:
            break;
        case WuQMacroClassTypeEnum::LINE_EDIT:
            break;
        case WuQMacroClassTypeEnum::LIST_WIDGET:
            break;
        case WuQMacroClassTypeEnum::MENU:
            break;
        case WuQMacroClassTypeEnum::MOUSE_USER_EVENT:
            break;
        case WuQMacroClassTypeEnum::PUSH_BUTTON:
        {
            /*
             * Buttons may have a 'checkable' status enabled
             * Buttons may also be in a QButtonGroup and the
             * QButtonGroup may have an 'exclusive' status.
             *
             * NOTE: For this logic to work, the buttons must have
             * macro support added after the buttons are placed
             * in an exclusive button group
             */
            QAbstractButton* button = qobject_cast<QAbstractButton*>(object);
            CaretAssert(button);
            if (button->isCheckable()) {
                /*
                 * Probably checkable
                 */
                objectType = WuQMacroClassTypeEnum::PUSH_BUTTON_CHECKABLE;
                
                const QButtonGroup* buttonGroup = button->group();
                if (buttonGroup != NULL) {
                    if (buttonGroup->exclusive()) {
                        /*
                         * In an exclusive group, buttons CANNOT be
                         * uchecked so treat as non-checkable button
                         */
                        objectType = WuQMacroClassTypeEnum::PUSH_BUTTON;
                    }
                }
            }
        }
            break;
        case WuQMacroClassTypeEnum::PUSH_BUTTON_CHECKABLE:
            CaretAssertMessage(0, "PUSH_BUTTON_CHECKABLE is created by PUSH_BUTTON case above");
            break;
        case WuQMacroClassTypeEnum::RADIO_BUTTON:
            break;
        case WuQMacroClassTypeEnum::SLIDER:
            break;
        case WuQMacroClassTypeEnum::SPIN_BOX:
            break;
        case WuQMacroClassTypeEnum::TAB_BAR:
            break;
        case WuQMacroClassTypeEnum::TAB_WIDGET:
            break;
        case WuQMacroClassTypeEnum::TOOL_BUTTON:
        {
            /*
             * Buttons may have a 'checkable' status enabled
             * Buttons may also be in a QButtonGroup and the
             * QButtonGroup may have an 'exclusive' status.
             *
             * NOTE: For this logic to work, the buttons must have
             * macro support added after the buttons are placed
             * in an exclusive button group
             */
            QAbstractButton* button = qobject_cast<QAbstractButton*>(object);
            CaretAssert(button);
            if (button->isCheckable()) {
                /*
                 * Probably checkable
                 */
                objectType = WuQMacroClassTypeEnum::TOOL_BUTTON_CHECKABLE;
                
                const QButtonGroup* buttonGroup = button->group();
                if (buttonGroup != NULL) {
                    if (buttonGroup->exclusive()) {
                        /*
                         * In an exclusive group, buttons CANNOT be
                         * uchecked so treat as non-checkable button
                         */
                        objectType = WuQMacroClassTypeEnum::TOOL_BUTTON;
                    }
                }
            }
        }
            break;
        case WuQMacroClassTypeEnum::TOOL_BUTTON_CHECKABLE:
            CaretAssertMessage(0, "TOOL_BUTTON_CHECKABLE is created by TOOL_BUTTON case above");
            break;
    }

    if ((objectType == WuQMacroClassTypeEnum::INVALID)
        || ( ! validFlag)) {
        errorMessageOut = ("Widget named \""
                           + object->objectName()
                           + "\" of class \""
                           + object->metaObject()->className()
                           + "\" is not supported for macros");
        return NULL;
    }
    
    WuQMacroSignalWatcher* ww = new WuQMacroSignalWatcher(parentMacroManager,
                                                          object,
                                                          objectType,
                                                          descriptiveName,
                                                          toolTipTextOverride);
    return ww;
}

/**
 * @return Tooltip for this signal watcher
 */
QString
WuQMacroSignalWatcher::getToolTip() const
{
    return m_toolTipText;
}


/**
 * If recording mode is enabled, create and send a macro command
 * to the macro manager.
 *
 * @param parameters
 *    Parameters for the command
 */
void
WuQMacroSignalWatcher::createAndSendMacroCommand(std::vector<WuQMacroCommandParameter*>& parameters)
{
    if (m_parentMacroManager->isModeRecording()) {
        WuQMacroCommand* mc = new WuQMacroCommand(m_objectType,
                                                  m_objectName,
                                                  m_descriptiveName,
                                                  m_toolTipText);
        for (auto p : parameters) {
            mc->addParameter(p);
        }
        
        if ( ! m_parentMacroManager->addMacroCommandToRecording(mc)) {
            delete mc;
        }
    }

}

/**
 * Called when a action group has an item triggered
 *
 * @param action
 *     ActionGroup action that was triggered
 */
void
WuQMacroSignalWatcher::actionGroupTriggered(QAction* action)
{
    QActionGroup* actionGroup = qobject_cast<QActionGroup*>(m_object);
    CaretAssert(actionGroup);
    
    int actionIndex(-1);
    QList<QAction*> actionList = actionGroup->actions();
    for (int32_t i = 0; i < actionList.size(); i++) {
        if (actionList.at(i) == action) {
            actionIndex = i;
            break;
        }
    }
    
    const QString actionText((action != NULL)
                             ? action->text()
                             : "");
    
    std::vector<WuQMacroCommandParameter*> params;
    params.push_back(new WuQMacroCommandParameter(WuQMacroDataValueTypeEnum::STRING,
                                                  "Select name",
                                                  actionText));
    params.push_back(new WuQMacroCommandParameter(WuQMacroDataValueTypeEnum::INTEGER,
                                                  "Select index",
                                                  actionIndex));

    createAndSendMacroCommand(params);
}


/**
 * Called when an action is triggered
 *
 * @param checked
 *     New checked status
 */
void
WuQMacroSignalWatcher::actionTriggered(bool /*checked*/)
{
    std::vector<WuQMacroCommandParameter*> params;
    params.push_back(new WuQMacroCommandParameter(WuQMacroDataValueTypeEnum::NONE,
                                                  "Click/Select",
                                                  ""));

    createAndSendMacroCommand(params);
}

/**
 * Called when a checkable action is triggered
 *
 * @param checked
 *     New checked status
 */
void
WuQMacroSignalWatcher::actionCheckableTriggered(bool checked)
{
    std::vector<WuQMacroCommandParameter*> params;
    params.push_back(new WuQMacroCommandParameter(WuQMacroDataValueTypeEnum::BOOLEAN,
                                                  "On/Off",
                                                  checked));
    
    createAndSendMacroCommand(params);
}

/**
 * Called when a button group button is clicked
 *
 * @param button
 *     Button that was clicked
 */
void
WuQMacroSignalWatcher::buttonGroupButtonClicked(QAbstractButton* button)
{
    QButtonGroup* buttonGroup = qobject_cast<QButtonGroup*>(m_object);
    CaretAssert(buttonGroup);
    
    int buttonIndex(-1);
    QList<QAbstractButton*> buttonList = buttonGroup->buttons();
    for (int32_t i = 0; i < buttonList.size(); i++) {
        if (buttonList.at(i) == button) {
            buttonIndex = i;
            break;
        }
    }

    const QString buttonText((button != NULL)
                       ? button->text()
                       : "");
    std::vector<WuQMacroCommandParameter*> params;
    params.push_back(new WuQMacroCommandParameter(WuQMacroDataValueTypeEnum::STRING,
                                                  "Select button with name",
                                                  buttonText));
    params.push_back(new WuQMacroCommandParameter(WuQMacroDataValueTypeEnum::INTEGER,
                                                  "Select button at index",
                                                  buttonIndex));

    createAndSendMacroCommand(params);
}


/**
 * Called when a check box is clicked
 *
 * @param checked
 *     New checked status
 */
void
WuQMacroSignalWatcher::checkBoxClicked(bool checked)
{
    std::vector<WuQMacroCommandParameter*> params;
    params.push_back(new WuQMacroCommandParameter(WuQMacroDataValueTypeEnum::BOOLEAN,
                                                  "On/Off",
                                                  checked));

    createAndSendMacroCommand(params);
}

/**
 * Called when a combo box is activated
 *
 * @param index
 *     Index of activated item
 */
void
WuQMacroSignalWatcher::comboBoxActivated(int index)
{
    QComboBox* comboBox = qobject_cast<QComboBox*>(m_object);
    CaretAssert(comboBox);

    QString text;
    if ((index >= 0)
        && (index < comboBox->count())) {
        text = comboBox->itemText(index);
    }
    
    std::vector<WuQMacroCommandParameter*> params;
    params.push_back(new WuQMacroCommandParameter(WuQMacroDataValueTypeEnum::STRING,
                                                  "Select item with name",
                                                  text));
    params.push_back(new WuQMacroCommandParameter(WuQMacroDataValueTypeEnum::INTEGER,
                                                  "Select item at index",
                                                  index));

    createAndSendMacroCommand(params);
}

/**
 * Called when a spin box value is changed
 *
 * @param value
 *     New value in double spin box
 */
void
WuQMacroSignalWatcher::doubleSpinBoxValueChanged(double value)
{
    std::vector<WuQMacroCommandParameter*> params;
    params.push_back(new WuQMacroCommandParameter(WuQMacroDataValueTypeEnum::FLOAT,
                                                  "New value",
                                                  value));

    createAndSendMacroCommand(params);
}

/**
 * Called when a line edit has text edited
 *
 * @param text
 *     New value of text in the line edit
 */
void
WuQMacroSignalWatcher::lineEditTextEdited(const QString& text)
{
    std::vector<WuQMacroCommandParameter*> params;
    params.push_back(new WuQMacroCommandParameter(WuQMacroDataValueTypeEnum::STRING,
                                                  "New text",
                                                  text));

    createAndSendMacroCommand(params);
}

/**
 * Called when a list widget item is activated
 *
 * @param item
 *     List widget item that was selected
 */
void
WuQMacroSignalWatcher::listWidgetItemActivated(QListWidgetItem* item)
{
    QListWidget* listWidget = qobject_cast<QListWidget*>(m_object);
    CaretAssert(listWidget);
    
    const int rowIndex = listWidget->row(item);
    
    const QString text((item != NULL)
                       ? item->text()
                       : "");
    std::vector<WuQMacroCommandParameter*> params;
    params.push_back(new WuQMacroCommandParameter(WuQMacroDataValueTypeEnum::STRING,
                                                  "Select item with name",
                                                  text));
    params.push_back(new WuQMacroCommandParameter(WuQMacroDataValueTypeEnum::INTEGER,
                                                  "Select item at index",
                                                  rowIndex));

    createAndSendMacroCommand(params);
}

/**
 * Called when a menu has an item triggered
 *
 * @param action
 *     Menu action that was triggered
 */
void
WuQMacroSignalWatcher::menuTriggered(QAction* action)
{
    QMenu* menu = qobject_cast<QMenu*>(m_object);
    CaretAssert(menu);
    
    int actionIndex(-1);
    QList<QAction*> actionList = menu->actions();
    for (int32_t i = 0; i < actionList.size(); i++) {
        if (actionList.at(i) == action) {
            actionIndex = i;
            break;
        }
    }
    
    const QString text((action != NULL)
                       ? action->text()
                       : "");
    std::vector<WuQMacroCommandParameter*> params;
    params.push_back(new WuQMacroCommandParameter(WuQMacroDataValueTypeEnum::STRING,
                                                  "Select item with name",
                                                  text));
    params.push_back(new WuQMacroCommandParameter(WuQMacroDataValueTypeEnum::INTEGER,
                                                  "Select item at index",
                                                  actionIndex));
    
    createAndSendMacroCommand(params);
}

/**
 * Called when a push button is clicked
 *
 * @param checked
 *     New checked status
 */
void
WuQMacroSignalWatcher::pushButtonClicked(bool /*checked*/)
{
    std::vector<WuQMacroCommandParameter*> params;
    params.push_back(new WuQMacroCommandParameter(WuQMacroDataValueTypeEnum::NONE,
                                                  "Click button",
                                                  ""));

    createAndSendMacroCommand(params);
}

/**
 * Called when a checkable push button is clicked
 *
 * @param checked
 *     New checked status
 */
void
WuQMacroSignalWatcher::pushButtonCheckableClicked(bool checked)
{
    std::vector<WuQMacroCommandParameter*> params;
    params.push_back(new WuQMacroCommandParameter(WuQMacroDataValueTypeEnum::NONE,
                                                  "On/Off",
                                                  checked));
    
    createAndSendMacroCommand(params);
}


/**
 * Called when a radio button is clicked
 *
 * @param checked
 *     New checked status
 */
void
WuQMacroSignalWatcher::radioButtonClicked(bool /*checked*/)
{
    std::vector<WuQMacroCommandParameter*> params;
    params.push_back(new WuQMacroCommandParameter(WuQMacroDataValueTypeEnum::NONE,
                                                  "Select button",
                                                  ""));
    
    createAndSendMacroCommand(params);
}

/**
 * Called when a slider value is changed
 *
 * @param value
 *     New value
 */
void
WuQMacroSignalWatcher::sliderValueChanged(int value)
{
    std::vector<WuQMacroCommandParameter*> params;
    
    params.push_back(new WuQMacroCommandParameter(WuQMacroDataValueTypeEnum::INTEGER,
                                                  "Move slider to",
                                                  value));
    
    createAndSendMacroCommand(params);
}

/**
 * Called when a spin box value is changed
 *
 * @param value
 *     New value
 */
void
WuQMacroSignalWatcher::spinBoxValueChanged(int value)
{
    std::vector<WuQMacroCommandParameter*> params;
    
    params.push_back(new WuQMacroCommandParameter(WuQMacroDataValueTypeEnum::INTEGER,
                                                  "Enter value",
                                                  value));

    createAndSendMacroCommand(params);
}

/**
 * Called when a tab bar current tab is changed
 *
 * @param index
 *     Index of the new selected tab
 */
void
WuQMacroSignalWatcher::tabBarCurrentChanged(int index)
{
    QTabBar* tabBar = qobject_cast<QTabBar*>(m_object);
    CaretAssert(tabBar);
    const QString tabText = tabBar->tabText(index);

    std::vector<WuQMacroCommandParameter*> params;
    
    params.push_back(new WuQMacroCommandParameter(WuQMacroDataValueTypeEnum::STRING,
                                                  "Select tab with name",
                                                  tabText));
    params.push_back(new WuQMacroCommandParameter(WuQMacroDataValueTypeEnum::INTEGER,
                                                  "Select tab at index",
                                                  index));
    
    createAndSendMacroCommand(params);
}

/**
 * Called when a tab widget current tab is changed
 *
 * @param index
 *     Index of the new selected tab
 */
void
WuQMacroSignalWatcher::tabWidgetCurrentChanged(int index)
{
    QTabWidget* tabWidget = qobject_cast<QTabWidget*>(m_object);
    CaretAssert(tabWidget);
    const QString tabText = tabWidget->tabText(index);
    
    std::vector<WuQMacroCommandParameter*> params;
    params.push_back(new WuQMacroCommandParameter(WuQMacroDataValueTypeEnum::STRING,
                                                  "Select tab with name",
                                                  tabText));
    params.push_back(new WuQMacroCommandParameter(WuQMacroDataValueTypeEnum::INTEGER,
                                                  "Select tab at index",
                                                  index));
    
    createAndSendMacroCommand(params);
}

/**
 * Called when a tool button is clicked
 *
 * @param checked
 *     New check status
 */
void
WuQMacroSignalWatcher::toolButtonClicked(bool /*checked*/)
{
    std::vector<WuQMacroCommandParameter*> params;
    params.push_back(new WuQMacroCommandParameter(WuQMacroDataValueTypeEnum::NONE,
                                                  "Select button",
                                                  ""));

    createAndSendMacroCommand(params);
}

/**
 * Called when a tool button is clicked
 *
 * @param checked
 *     New check status
 */
void
WuQMacroSignalWatcher::toolButtonCheckableClicked(bool checked)
{
    std::vector<WuQMacroCommandParameter*> params;
    params.push_back(new WuQMacroCommandParameter(WuQMacroDataValueTypeEnum::BOOLEAN,
                                                  "On/Off",
                                                  checked));

    createAndSendMacroCommand(params);
}

/**
 * @return String containing description of this signal watcher
 */
QString
WuQMacroSignalWatcher::toString() const
{
    QString s(m_objectName
              + " type="
              + WuQMacroClassTypeEnum::toGuiName(m_objectType));
    return s;
}


