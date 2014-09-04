
/*LICENSE_START*/
/*
 *  Copyright (C) 2014 Washington University School of Medicine
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

#define __HELP_VIEWER_DIALOG_DECLARE__
#include "HelpViewerDialog.h"
#undef __HELP_VIEWER_DIALOG_DECLARE__

#include <QAction>
#include <QDesktopServices>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPrintDialog>
#include <QPrinter>
#include <QTextBrowser>
#include <QSplitter>
#include <QTextStream>
#include <QToolBar>
#include <QToolButton>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>

#include "CaretAssert.h"
#include "CaretLogger.h"
#include "CommandOperation.h"
#include "CommandOperationManager.h"
#include "WuQMessageBox.h"
#include "WuQtUtilities.h"

using namespace caret;


    
/**
 * \class caret::HelpViewerDialog 
 * \brief Dialog that displays the applications help information.
 * \ingroup GuiQt
 */

/**
 * Constructor.
 *
 * @param parent
 *    Parent of this dialog.
 * @param f
 *    Qt's window flags.
 */
HelpViewerDialog::HelpViewerDialog(QWidget* parent,
                                   Qt::WindowFlags f)
: WuQDialogNonModal("Help",
                    parent,
                    f)
{
    m_helpBrowser = NULL;
    m_topicSearchLineEditFirstMouseClick = true;
    setApplyButtonText("");
    
    /*
     * Create the tree widget for the help topics
     */
    m_topicIndexTreeWidget = new QTreeWidget;
    m_topicIndexTreeWidget->setColumnCount(1);
    m_topicIndexTreeWidget->setColumnHidden(0, false);
    m_topicIndexTreeWidget->headerItem()->setHidden(true);
    QObject::connect(m_topicIndexTreeWidget, SIGNAL(currentItemChanged(QTreeWidgetItem*,
                                                                QTreeWidgetItem*)),
                     this, SLOT(topicIndexTreeItemChanged(QTreeWidgetItem*,
                                                           QTreeWidgetItem*)));

    /*
     * Search line edit and list widget
     */
    const AString searchText = ("All searches are case insensitive.\n"
                                "\n"
                                "You may use wildcard characters:\n"
                                "    * - Matches any characters.\n"
                                "    ? - Matches a single character.\n");
    const AString topicSearchToolTipText = ("Enter text to search content of ALL help pages.\n"
                                            + searchText);
    m_topicSearchLineEdit = new QLineEdit;
    m_topicSearchLineEdit->setToolTip(topicSearchToolTipText.convertToHtmlPage());
    QObject::connect(m_topicSearchLineEdit, SIGNAL(returnPressed()),
                     this, SLOT(topicSearchLineEditStartSearch()));
    QObject::connect(m_topicSearchLineEdit, SIGNAL(textEdited(const QString&)),
                     this, SLOT(topicSearchLineEditStartSearch()));
    QObject::connect(m_topicSearchLineEdit, SIGNAL(cursorPositionChanged(int,int)),
                     this, SLOT(topicSearchLineEditCursorPositionChanged(int,int)));
    
    /*
     * Collapse All button
     */
    QAction* collapseAllAction = WuQtUtilities::createAction("Collapse All",
                                                           "",
                                                           this,
                                                           this,
                                                           SLOT(topicCollapseAllTriggered()));
    QToolButton* collapseAllToolButton = new QToolButton;
    collapseAllToolButton->setDefaultAction(collapseAllAction);
    
    /*
     * Expand All button
     */
    QAction* expandAllAction = WuQtUtilities::createAction("Expand All",
                                                           "",
                                                           this,
                                                           this,
                                                           SLOT(topicExpandAllTriggered()));
    QToolButton* expandAllToolButton = new QToolButton;
    expandAllToolButton->setDefaultAction(expandAllAction);
    
    
    /*
     * create the back toolbar button
     */
    QToolButton* backwardButton = new QToolButton;
    backwardButton->setArrowType(Qt::LeftArrow);
    backwardButton->setToolTip("Show the previous page");
    
    /*
     * Create the forward toolbar button
     */
    QToolButton* forwardButton = new QToolButton;
    forwardButton->setArrowType(Qt::RightArrow);
    forwardButton->setToolTip("Show the next page");
    
    /*
     * Create the print toolbar button
     */
    QToolButton* printButton = new QToolButton;
    connect(printButton, SIGNAL(clicked()),
            this, SLOT(helpPagePrintButtonClicked()));
    printButton->setText("Print");
    printButton->hide();
    
    /**
     *  Copy button
     */
    QToolButton* copyButton = new QToolButton;
    copyButton->setText("Copy");
    copyButton->setToolTip("Copies selected help text to clipboard.");
    copyButton->setEnabled(false);
    
    /*
     * create the help browser
     */
    m_helpBrowser = new HelpTextBrowser(this);
    m_helpBrowser->setMinimumWidth(400);
    m_helpBrowser->setMinimumHeight(200);
    m_helpBrowser->setOpenExternalLinks(false);
    m_helpBrowser->setOpenLinks(true);
    QObject::connect(forwardButton, SIGNAL(clicked()),
                     m_helpBrowser, SLOT(forward()));
    QObject::connect(backwardButton, SIGNAL(clicked()),
                     m_helpBrowser, SLOT(backward()));
    
    /*
     * Hook up copy button to help browser
     */
    QObject::connect(m_helpBrowser, SIGNAL(copyAvailable(bool)),
                     copyButton, SLOT(setEnabled(bool)));
    QObject::connect(copyButton, SIGNAL(clicked()),
                     m_helpBrowser, SLOT(copy()));
    
    /*
     * Layout for toolbuttons
     */
    QHBoxLayout* toolButtonLayout = new QHBoxLayout;
    toolButtonLayout->addWidget(new QLabel("Navigate:"));
    toolButtonLayout->addWidget(backwardButton);
    toolButtonLayout->addWidget(forwardButton);
    toolButtonLayout->addStretch();
    toolButtonLayout->addWidget(copyButton);
    toolButtonLayout->addWidget(printButton);
    
    /*
     * Layout for help browser and buttons
     */
    QWidget* helpBrowserWidgets = new QWidget;
    QVBoxLayout* helpBrowserLayout = new QVBoxLayout(helpBrowserWidgets);
    helpBrowserLayout->addLayout(toolButtonLayout);
    helpBrowserLayout->addWidget(m_helpBrowser);
    
    /*
     * Layout for collapse/expand all buttons
     */
    QHBoxLayout* collapseExpandLayout = new QHBoxLayout;
    collapseExpandLayout->addStretch();
    collapseExpandLayout->addWidget(collapseAllToolButton);
    collapseExpandLayout->addStretch();
    collapseExpandLayout->addWidget(expandAllToolButton);
    collapseExpandLayout->addStretch();
    
    /*
     * Layout for search line edit and topics
     */
    QWidget* topicWidgets = new QWidget();
    QVBoxLayout* topicLayout = new QVBoxLayout(topicWidgets);
    topicLayout->addWidget(m_topicSearchLineEdit);
    topicLayout->addLayout(collapseExpandLayout);
    topicLayout->addWidget(m_topicIndexTreeWidget);

    /*
     * Create the splitter and add the widgets to the splitter
     */
    m_splitter = new QSplitter;
    m_splitter->setOrientation(Qt::Horizontal);
    m_splitter->addWidget(topicWidgets);
    m_splitter->addWidget(helpBrowserWidgets);
    QList<int> sizeList;
    sizeList << 225 << 375;
    m_splitter->setSizes(sizeList);
    
    setCentralWidget(m_splitter,
                     WuQDialog::SCROLL_AREA_NEVER);

    loadHelpTopicsIntoIndexTree();
}

/**
 * Destructor.
 */
HelpViewerDialog::~HelpViewerDialog()
{
}

/**
 * Update the content of the dialog.
 */
void
HelpViewerDialog::updateDialog()
{
}

/**
 * Show the help page with the given name.
 *
 * @param helpPageName
 *    Name of help page.
 */
void
HelpViewerDialog::showHelpPageWithName(const AString& helpPageName)
{
    CaretAssertMessage(0, "Not implmented yet.");
    const AString pageName = QString(helpPageName).replace('_', ' ');
    if (pageName.isEmpty()) {
        return;
    }
    
    //    for (int i = 0; i < m_topicTreeWidget->count(); i++) {
    //        QListWidgetItem* lwi = m_workbenchIndexListWidget->item(i);
    //        if (lwi->text() == pageName) {
    //            m_workbenchIndexListWidget->setCurrentItem(lwi);
    //            workbenchIndexListWidgetItemClicked(lwi);
    //            return;
    //        }
    //    }
    
    CaretLogSevere("Could not find help page \""
                   + helpPageName
                   + "\" for loading.");
}

///**
// * load the index tree with the help topics.
// */
//void
//HelpViewerDialog::loadHelpTopicsIntoIndexTree()
//{
//    m_topicIndexTreeWidget->blockSignals(true);
//    
//    
//    QTreeWidgetItem* workbenchItem = new QTreeWidgetItem(m_topicIndexTreeWidget,
//                                                         TREE_ITEM_NONE);
//    workbenchItem->setText(0, "Workbench");
//    
//
//    QTreeWidgetItem* menuItem = new QTreeWidgetItem(workbenchItem,
//                                                    TREE_ITEM_NONE);
//    menuItem->setText(0, "Menus");
//    
//    QTreeWidgetItem* glossaryItem = new QTreeWidgetItem(workbenchItem,
//                                                        TREE_ITEM_NONE);
//    glossaryItem->setText(0, "Glossary");
//    
//    QTreeWidgetItem* otherItem = new QTreeWidgetItem(workbenchItem,
//                                                        TREE_ITEM_NONE);
//    otherItem->setText(0, "Other");
//    
//    
//    QTreeWidgetItem* menuWbViewItem = NULL;
//    QTreeWidgetItem* menuFileItem = NULL;
//    QTreeWidgetItem* menuViewItem = NULL;
//    QTreeWidgetItem* menuDataItem = NULL;
//    QTreeWidgetItem* menuSurfaceItem = NULL;
//    QTreeWidgetItem* menuConnectItem = NULL;
//    QTreeWidgetItem* menuDevelopItem = NULL;
//    QTreeWidgetItem* menuWindowItem = NULL;
//    QTreeWidgetItem* menuHelpItem = NULL;
//    std::vector<QTreeWidgetItem*> unknownMenuItems;
//    
//    
//    QDir resourceHelpDirectory(":/HelpFiles");
//    
//    // CAN BE SET TO FIND FILES WITHOUT FULL PATH
//    //m_helpBrowser->setSearchPaths(QStringList(":/HelpFiles/Menus/File_Menu"));
//    
//    if (resourceHelpDirectory.exists()) {
//        QStringList htmlFileFilter;
//        htmlFileFilter << "*";
//        //htmlFileFilter << "*.htm" << "*.html";
//        QDirIterator dirIter(":/HelpFiles",
//                             htmlFileFilter,
//                             QDir::NoFilter,
//                             QDirIterator::Subdirectories);
//        
//        while (dirIter.hasNext()) {
//            dirIter.next();
//            const QFileInfo fileInfo = dirIter.fileInfo();
//            const QString name       = fileInfo.baseName();
//            const QString filePath   = fileInfo.filePath();
// 
//            if (filePath.endsWith(".htm")
//                || filePath.endsWith(".html")) {
//                if (name.contains("Menu",
//                                  Qt::CaseInsensitive)) {
//                    QTreeWidgetItem* item = createHelpTreeWidgetItemForHelpPage(NULL,
//                                                                                name,
//                                                                                filePath);
//                    if (name.contains("wb_view")) {
//                        menuWbViewItem = item;
//                    }
//                    else if (name.startsWith("File",
//                                             Qt::CaseInsensitive)) {
//                        menuFileItem = item;
//                    }
//                    else if (name.startsWith("View",
//                                             Qt::CaseInsensitive)) {
//                        menuViewItem = item;
//                    }
//                    else if (name.startsWith("Data",
//                                             Qt::CaseInsensitive)) {
//                        menuDataItem = item;
//                    }
//                    else if (name.startsWith("Surface",
//                                             Qt::CaseInsensitive)) {
//                        menuSurfaceItem = item;
//                    }
//                    else if (name.startsWith("Connect",
//                                             Qt::CaseInsensitive)) {
//                        menuConnectItem = item;
//                    }
//                    else if (name.startsWith("Develop",
//                                             Qt::CaseInsensitive)) {
//                        menuDevelopItem = item;
//                    }
//                    else if (name.startsWith("Window",
//                                             Qt::CaseInsensitive)) {
//                        menuWindowItem = item;
//                    }
//                    else if (name.startsWith("Help",
//                                             Qt::CaseInsensitive)) {
//                        menuHelpItem = item;
//                    }
//                    else {
//                        CaretLogSevere("Unrecognized menu name, has a new menu been added? \""
//                                       + name);
//                        unknownMenuItems.push_back(item);
//                    }
//                }
//                else if (filePath.contains("Glossary")) {
//                    createHelpTreeWidgetItemForHelpPage(glossaryItem,
//                                                        name,
//                                                        filePath);
//                }
//                else {
//                    createHelpTreeWidgetItemForHelpPage(otherItem,
//                                                        name,
//                                                        filePath);
//                }
//            }
//        }
//        
//        glossaryItem->sortChildren(0,
//                                   Qt::AscendingOrder);
//        otherItem->sortChildren(0,
//                                Qt::AscendingOrder);
//        
//    }
//    else {
//        CaretLogSevere("Resource directory "
//                       + resourceHelpDirectory.absolutePath()
//                       + " not found.");
//    }
//    
//    addItemToParentMenu(menuItem,
//                        menuWbViewItem,
//                        "");
//    addItemToParentMenu(menuItem,
//                        menuFileItem,
//                        "");
//    addItemToParentMenu(menuItem,
//                        menuViewItem,
//                        "");
//    addItemToParentMenu(menuItem,
//                        menuDataItem,
//                        "");
//    addItemToParentMenu(menuItem,
//                        menuSurfaceItem,
//                        "");
//    addItemToParentMenu(menuItem,
//                        menuConnectItem,
//                        "");
//    addItemToParentMenu(menuItem,
//                        menuDevelopItem,
//                        "");
//    addItemToParentMenu(menuItem,
//                        menuWindowItem,
//                        "");
//    addItemToParentMenu(menuItem,
//                        menuHelpItem,
//                        "");
//    for (std::vector<QTreeWidgetItem*>::iterator unknownIter = unknownMenuItems.begin();
//         unknownIter != unknownMenuItems.end();
//         unknownIter++) {
//        addItemToParentMenu(menuItem,
//                            *unknownIter,
//                            "");
//    }
//    
//    m_topicIndexTreeWidget->setItemExpanded(menuItem,
//                                       true);
//    m_topicIndexTreeWidget->setItemExpanded(workbenchItem,
//                                       true);
//    
//    /*
//     * Load commands
//     */
//    CommandOperationManager* commandOperationManager = CommandOperationManager::getCommandOperationManager();
//    std::vector<CommandOperation*> commandOperations = commandOperationManager->getCommandOperations();
//    
//    if ( ! commandOperations.empty()) {
//        /*
//         * Use map to sort commands by short description
//         */
//        std::map<QString, CommandOperation*> sortCommandsMap;
//        for (std::vector<CommandOperation*>::iterator vecIter = commandOperations.begin();
//             vecIter != commandOperations.end();
//             vecIter++) {
//            CommandOperation* op = *vecIter;
//            sortCommandsMap.insert(std::make_pair(op->getCommandLineSwitch(),
//                                                  op));
//        }
//        
//        QTreeWidgetItem* wbCommandItem = new QTreeWidgetItem(m_topicIndexTreeWidget,
//                                                             TREE_ITEM_NONE);
//        wbCommandItem->setText(0, "wb_command");
//        
//        QFont commandFont = wbCommandItem->font(0);
//        commandFont.setPointSize(10);
//        
//        for (std::map<QString, CommandOperation*>::iterator mapIter = sortCommandsMap.begin();
//             mapIter != sortCommandsMap.end();
//             mapIter++) {
//            CommandOperation* op = mapIter->second;
//            
//            HelpTreeWidgetItem* item = HelpTreeWidgetItem::newInstanceForCommandOperation(wbCommandItem,
//                                                                 op);
//            item->setFont(0, commandFont);
//            m_allHelpWidgetItems.push_back(item);
//        }
//        
//        m_topicIndexTreeWidget->setItemExpanded(wbCommandItem,
//                                                true);
//    }
//    
//    m_topicIndexTreeWidget->blockSignals(false);
//}

/**
 * Load Workbench help from the given directory and add it to the
 * the given parent.  Also process any subdirectories
 *
 * @param parent
 *    Parent tree widget item
 * @param dirInfo
 *    The directory examined for HTML pages and subdirectories
 * @return
 *    Tree widget item that was created.
 */
QTreeWidgetItem*
HelpViewerDialog::loadWorkbenchHelpInfoFromDirectory(QTreeWidgetItem* parent,
                                                     const QFileInfo& dirInfo)
{
    QDir directory(dirInfo.absoluteFilePath());
    
    /*
     * Get all HTML pages in the directory
     * and file an HTML page that is the same
     * name as the directory.
     */
    QStringList htmlNameFilter;
    htmlNameFilter << "*.htm" << "*.html";
    QFileInfoList htmlFileList = directory.entryInfoList(htmlNameFilter,
                                                QDir::Files,
                                                QDir::Name);
    
    QString dirHtmlPageName;
    QFileInfoList otherHtmlPagesList;
    QListIterator<QFileInfo> htmlFileIter(htmlFileList);
    while (htmlFileIter.hasNext()) {
        const QFileInfo htmlFileInfo = htmlFileIter.next();
        if (htmlFileInfo.baseName() == dirInfo.baseName()) {
            dirHtmlPageName = htmlFileInfo.absoluteFilePath();
        }
        else {
            otherHtmlPagesList.append(htmlFileInfo.absoluteFilePath());
        }
    }
    
    /*
     * Create a tree widget item for this directory
     * that may have a help page.
     */
    QTreeWidgetItem* treeItem = NULL;
    if ( ! dirHtmlPageName.isEmpty()) {
        treeItem = createHelpTreeWidgetItemForHelpPage(parent,
                                                       dirInfo.baseName(),
                                                       dirHtmlPageName);
    }
    else {
        AString text = dirInfo.baseName();
        text = text.replace('_', ' ');
        treeItem = new QTreeWidgetItem(parent);
        treeItem->setText(0, text);
    }
    
    /*
     * Add items for any other HTML pages found in the directory
     */
    QListIterator<QFileInfo> otherHtmlPageIter(otherHtmlPagesList);
    while (otherHtmlPageIter.hasNext()) {
        const QFileInfo pageInfo = otherHtmlPageIter.next();
        createHelpTreeWidgetItemForHelpPage(treeItem,
                                            pageInfo.baseName(),
                                            pageInfo.absoluteFilePath());
    }
    
    /*
     * Add any subdirectories as children
     */
    QFileInfoList subDirList = directory.entryInfoList((QDir::AllDirs | QDir::NoDotAndDotDot),
                                                       QDir::Name);
    QListIterator<QFileInfo> subDirIter(subDirList);
    while (subDirIter.hasNext()) {
        const QFileInfo subDirInfo = subDirIter.next();
        loadWorkbenchHelpInfoFromDirectory(treeItem,
                                           subDirInfo);
    }
    
    return treeItem;
}

/**
 * load the index tree with the help topics.
 */
void
HelpViewerDialog::loadHelpTopicsIntoIndexTree()
{
    m_topicIndexTreeWidget->blockSignals(true);
    
    
    QTreeWidgetItem* workbenchItem = new QTreeWidgetItem(m_topicIndexTreeWidget,
                                                         TREE_ITEM_NONE);
    workbenchItem->setText(0, "wb_view");
    
    QDir resourceHelpDirectory(":/HelpFiles");
    
    QTreeWidgetItem* glossaryItem = NULL;
    
    // CAN BE SET TO FIND FILES WITHOUT FULL PATH
    //m_helpBrowser->setSearchPaths(QStringList(":/HelpFiles/Menus/File_Menu"));
    
    QFileInfoList subDirList = resourceHelpDirectory.entryInfoList((QDir::AllDirs | QDir::NoDotAndDotDot),
                                                                   QDir::Name);
    QListIterator<QFileInfo> subDirIter(subDirList);
    while (subDirIter.hasNext()) {
        const QFileInfo subDirInfo = subDirIter.next();
        
        QTreeWidgetItem* item = loadWorkbenchHelpInfoFromDirectory(workbenchItem,
                                                                   subDirInfo);
        
        /*
         * Is this the GLOSSARY?
         * If so, move it so that it is a top level item.
         */
        if (subDirInfo.baseName().toLower() == "glossary") {
            if (glossaryItem != NULL) {
                CaretAssertMessage(0, "There should be only one glossary subdirectory !!!!");
            }
            glossaryItem = item;
            workbenchItem->removeChild(glossaryItem);
            m_topicIndexTreeWidget->addTopLevelItem(glossaryItem);
        }
    }
    
//    if (resourceHelpDirectory.exists()) {
//        QStringList htmlFileFilter;
//        htmlFileFilter << "*";
//        //htmlFileFilter << "*.htm" << "*.html";
//        QDirIterator dirIter(":/HelpFiles",
//                             htmlFileFilter,
//                             QDir::NoFilter,
//                             QDirIterator::Subdirectories);
//        
//        while (dirIter.hasNext()) {
//            dirIter.next();
//            const QFileInfo fileInfo = dirIter.fileInfo();
//            const QString name       = fileInfo.baseName();
//            const QString filePath   = fileInfo.filePath();
//            
//            if (filePath.endsWith(".htm")
//                || filePath.endsWith(".html")) {
//                if (name.contains("Menu",
//                                  Qt::CaseInsensitive)) {
//                    QTreeWidgetItem* item = createHelpTreeWidgetItemForHelpPage(NULL,
//                                                                                name,
//                                                                                filePath);
//                }
//            }
//        }
////        otherItem->sortChildren(0,
////                                Qt::AscendingOrder);
//        
//    }
//    else {
//        CaretLogSevere("Resource directory "
//                       + resourceHelpDirectory.absolutePath()
//                       + " not found.");
//    }
//    
//    
////    m_topicIndexTreeWidget->setItemExpanded(menuItem,
////                                            true);
//    m_topicIndexTreeWidget->setItemExpanded(workbenchItem,
//                                            true);
    
    /*
     * Load commands
     */
    CommandOperationManager* commandOperationManager = CommandOperationManager::getCommandOperationManager();
    std::vector<CommandOperation*> commandOperations = commandOperationManager->getCommandOperations();
    
    QTreeWidgetItem* wbCommandItem = NULL;
    if ( ! commandOperations.empty()) {
        /*
         * Use map to sort commands by short description
         */
        std::map<QString, CommandOperation*> sortCommandsMap;
        for (std::vector<CommandOperation*>::iterator vecIter = commandOperations.begin();
             vecIter != commandOperations.end();
             vecIter++) {
            CommandOperation* op = *vecIter;
            sortCommandsMap.insert(std::make_pair(op->getCommandLineSwitch(),
                                                  op));
        }
        
        wbCommandItem = new QTreeWidgetItem(m_topicIndexTreeWidget,
                                                             TREE_ITEM_NONE);
        wbCommandItem->setText(0, "wb_command");
        
        QFont commandFont = wbCommandItem->font(0);
        commandFont.setPointSize(10);
        
        for (std::map<QString, CommandOperation*>::iterator mapIter = sortCommandsMap.begin();
             mapIter != sortCommandsMap.end();
             mapIter++) {
            CommandOperation* op = mapIter->second;
            
            HelpTreeWidgetItem* item = HelpTreeWidgetItem::newInstanceForCommandOperation(wbCommandItem,
                                                                                          op);
            item->setFont(0, commandFont);
            m_allHelpWidgetItems.push_back(item);
        }
    }

    /*
     * Using setExpanded on a QTreeWidgetItem only expands its immediate children.
     * So, expand everything and then collapse Glossary and wb_command items so
     * that only wb_view items are expanded.
     */
    m_topicIndexTreeWidget->expandAll();
    if (glossaryItem != NULL) {
        glossaryItem->setExpanded(false);
    }
    if (wbCommandItem != NULL) {
        wbCommandItem->setExpanded(false);
    }
    
    m_topicIndexTreeWidget->sortItems(0, Qt::AscendingOrder);
    
    m_topicIndexTreeWidget->blockSignals(false);
}

/**
 * Add an item to the menu's item.
 * If the given item is NULL, it was not found an an error message will
 * be logged.
 *
 * @param parentMenu
 *    The parent menu item.
 * @param item
 *    The item that is added to the parent menu.
 * @param itemName
 *    Name for item.
 */
void
HelpViewerDialog::addItemToParentMenu(QTreeWidgetItem* parentMenu,
                                      QTreeWidgetItem* item,
                                      const AString& itemName)
{
    CaretAssert(parentMenu);
    
    if (item != NULL) {
        if ( ! itemName.isEmpty()) {
            item->setText(0,
                          itemName);
        }
        parentMenu->addChild(item);
    }
    else {
        CaretLogSevere("Did not find help for menu: "
                       + itemName);
    }
}

/**
 * Create a help tree widget item for a help page URL.
 *
 * @param parent
 *    Parent for item in index.
 * @param itemText
 *    Text for the item shown in the topic index.
 * @param helpPageURL
 *    URL for the help page.
 */
HelpTreeWidgetItem*
HelpViewerDialog::createHelpTreeWidgetItemForHelpPage(QTreeWidgetItem* parent,
                                                      const AString& itemText,
                                                      const AString& helpPageURL)
{
    HelpTreeWidgetItem* helpItem = HelpTreeWidgetItem::newInstanceForHtmlHelpPage(parent,
                                                                                  itemText,
                                                                                  helpPageURL);
    
    m_allHelpWidgetItems.push_back(helpItem);
    
    return helpItem;
}

/**
 * Called when selected index tree item changes.
 *
 * @param currentItem
 *    The selected item
 * @param previousItem
 *    The previously selected item
 */
void
HelpViewerDialog::topicIndexTreeItemChanged(QTreeWidgetItem* currentItem,
                                            QTreeWidgetItem* /*previousItem*/)
{
    if (currentItem != NULL) {
        /*
         * Note not all items are castable to HelpTreeWidgetItem.
         * Items not castable are category items that have an arrow to
         * expand/collapse its children.
         */
        HelpTreeWidgetItem* helpItem = dynamic_cast<HelpTreeWidgetItem*>(currentItem);
        if (helpItem != NULL) {
            displayHelpTextForHelpTreeWidgetItem(helpItem);
            m_topicIndexTreeWidget->scrollToItem(helpItem,
                                                 QTreeWidget::EnsureVisible);
        }
        else {
            const AString html = AString(currentItem->text(0)).convertToHtmlPage();
            m_helpBrowser->setHtml(html);
        }
    }
}

/**
 * Display the help information for the given help item.
 *
 * @param helpItem
 *    Item for which help text is loaded.
 */
void
HelpViewerDialog::displayHelpTextForHelpTreeWidgetItem(HelpTreeWidgetItem* helpItem)
{
    CaretAssert(helpItem);
    m_helpBrowser->setSource(helpItem->m_helpPageURL);
}

/**
 * Called when search text is changed or return pressed to start
 * searching the help topics
 */
void
HelpViewerDialog::topicSearchLineEditStartSearch()
{
    const QString searchText = m_topicSearchLineEdit->text().trimmed();
    const bool haveSearchTextFlag = ( ! searchText.isEmpty());
    
    QRegExp regEx;
    bool haveWildcardSearchFlag = false;
    if (haveSearchTextFlag) {
        if (searchText.contains('*')
            || searchText.contains('?')) {
            haveWildcardSearchFlag = true;
            regEx.setPatternSyntax(QRegExp::Wildcard);
            regEx.setPattern(searchText);
            regEx.setCaseSensitivity(Qt::CaseInsensitive);
        }
    }
    
    for (std::vector<HelpTreeWidgetItem*>::iterator iter = m_allHelpWidgetItems.begin();
         iter != m_allHelpWidgetItems.end();
         iter++) {
        HelpTreeWidgetItem* helpItem = *iter;
        
        bool showItemFlag = true;
        if (haveSearchTextFlag) {
            showItemFlag = false;
            
            if (haveWildcardSearchFlag) {
                if (regEx.exactMatch(helpItem->m_helpText)) {
                    showItemFlag = true;
                }
            }
            else if (helpItem->m_helpText.contains(searchText,
                                                      Qt::CaseInsensitive)) {
                showItemFlag = true;
            }
        }
        helpItem->setHidden( ! showItemFlag);
    }
}

/**
 * called to print currently displayed page.
 */
void
HelpViewerDialog::helpPagePrintButtonClicked()
{
    QPrinter printer;
    QPrintDialog* printDialog = new QPrintDialog(&printer, this);
    if (printDialog->exec() == QPrintDialog::Accepted) {
        m_helpBrowser->document()->print(&printer);
    }
}

/**
 * Called when the cursor position is changed
 */
void
HelpViewerDialog::topicSearchLineEditCursorPositionChanged(int,int)
{
    if (m_topicSearchLineEditFirstMouseClick) {
        m_topicSearchLineEditFirstMouseClick = false;
        m_topicSearchLineEdit->clear();
        topicSearchLineEditStartSearch();
    }
}

/**
 * Expand all help topics
 */
void
HelpViewerDialog::topicExpandAllTriggered()
{
    m_topicIndexTreeWidget->expandAll();
}

/**
 * Collapse all help topics
 */
void
HelpViewerDialog::topicCollapseAllTriggered()
{
    m_topicIndexTreeWidget->collapseAll();
}


// ========================================================================= //

/**
 * Create a help viewer widget.
 *
 * @param parentHelpViewerDialog
 *    The parent help viewer dialog.
 */
HelpTextBrowser::HelpTextBrowser(HelpViewerDialog* parentHelpViewerDialog)
: QTextBrowser(parentHelpViewerDialog),
m_parentHelpViewerDialog(parentHelpViewerDialog)
{
    CaretAssert(parentHelpViewerDialog);
}

/**
 * Destructor.
 */
HelpTextBrowser::~HelpTextBrowser()
{
    
}

/**
 * Overrides superclass version so that images get loaded properly.
 * Setting search paths may eliminate need for this method.
 *
 * @param type
 *    Type of resource.
 * @param url
 *    URL of resource.
 * @return
 *    QVariant containing content for display in the help viewer.
 */
QVariant
HelpTextBrowser::loadResource(int type, const QUrl& url)
{
    const QString urlText = url.toString();
    
//    std::cout << "   Current Source: " << qPrintable(source().toString()) << std::endl;
//    std::cout << "Loading resource: " << qPrintable(url.toString()) << std::endl;
    
    QVariant result;
    
        for (std::vector<HelpTreeWidgetItem*>::iterator iter = m_parentHelpViewerDialog->m_allHelpWidgetItems.begin();
             iter != m_parentHelpViewerDialog->m_allHelpWidgetItems.end();
             iter++) {
            HelpTreeWidgetItem* treeItem = *iter;
            if (treeItem->m_helpPageURL == urlText) {
//                std::cout << "Found tree item " << qPrintable(treeItem->m_helpPageURL) << std::endl;
                result = treeItem->m_helpText;
                break;
            }
        }
        
        if ( ! result.isValid()) {
            result = QTextBrowser::loadResource(type, url);
            if (result.isValid()) {
//                std::cout << "  WAS LOADED BY QTextBrowser::loadResource()" << std::endl;
            }
            else {
                QString typeName("Unknown");
                if ( ! result.isValid()) {
                    switch (type) {
                        case QTextDocument::HtmlResource:
                            typeName = "Html Resource";
                            break;
                        case QTextDocument::ImageResource:
                            typeName = "Image Resource";
                            break;
                        case QTextDocument::StyleSheetResource:
                            typeName = "Style Sheet Resource";
                            break;
                    }
                }
                CaretLogSevere("Failed to load type: "
                               + typeName
                               + " file: "
                               + url.toString());
            }
        }
        
    
        return result;
}

/**
 * Set the source of the help browser.
 *
 * @param url
 *     URL directing to content.
 */
void
HelpTextBrowser::setSource(const QUrl& url)
{
    const AString urlText = url.toString();
    if (urlText.startsWith("http")) {
        if (WuQMessageBox::warningOkCancel(this,
                                           "The link clicked will be displayed in your web browser.")) {
            if ( ! QDesktopServices::openUrl(urlText)) {
                WuQMessageBox::errorOk(this,
                                       ("Failed to load "
                                        + urlText));
            }
        }
    }
    else {
        QTextBrowser::setSource(url);
    }
}


// ========================================================================= //


/**
 * Create a new help tree widget item for a wb_command item.
 *
 * @param parent
 *    Parent for item in index.
 * @param commandOperation
 *    The command.
 */
HelpTreeWidgetItem*
HelpTreeWidgetItem::newInstanceForCommandOperation(QTreeWidgetItem* parent,
                                                   CommandOperation* commandOperation)
{
    const AString itemText = commandOperation->getCommandLineSwitch();
    const AString helpInfoCopy = commandOperation->getHelpInformation("wb_command");
    const AString helpText = helpInfoCopy.convertToHtmlPageWithFontHeight(-1); //10);
    const AString helpPageURL("command:/"
                              + commandOperation->getOperationShortDescription().replace(' ', '_'));
    
    HelpTreeWidgetItem* instance = new HelpTreeWidgetItem(parent,
                                                          TREE_ITEM_HELP_TEXT,
                                                          itemText,
                                                          helpPageURL,
                                                          helpText);
    return instance;
}

/**
 * Create a new help tree widget item for a help page URL.
 *
 * @param parent
 *    Parent for item in index.
 * @param itemText
 *    Text for the item shown in the topic index.
 * @param helpPageURL
 *    URL for the help page.
 */
HelpTreeWidgetItem*
HelpTreeWidgetItem::newInstanceForHtmlHelpPage(QTreeWidgetItem* parent,
                                           const AString& itemText,
                                           const AString& helpPageURL)
{
    CaretAssertMessage( ( ! itemText.startsWith(":/")),
                       "All help pages must be resources (page name starts with \":/\")");
    
    QString helpText;
    
    QFile file(helpPageURL);
    if (file.exists()) {
        if (file.open(QFile::ReadOnly)) {
            QTextStream stream(&file);
            helpText = stream.readAll();
            file.close();
        }
        else {
            AString msg = ("Help file exists but unable to open for reading: "
                           + helpPageURL);
            CaretLogSevere(msg);
            helpText = msg.convertToHtmlPage();
        }
    }
    else {
        AString msg = ("HTML Help file missing: "
                       + helpPageURL);
        CaretLogSevere(msg);
        helpText = msg.convertToHtmlPage();
    }
    
    HelpTreeWidgetItem* instance = NULL;
    
    AString text = itemText;
    text = text.replace('_', ' ');
    if (parent != NULL) {
        instance = new HelpTreeWidgetItem(parent,
                                          TREE_ITEM_HELP_TEXT,
                                          text,
                                          "qrc" + helpPageURL,
                                          helpText);
    }
    else {
        instance = new HelpTreeWidgetItem(TREE_ITEM_HELP_TEXT,
                                          text,
                                          "qrc" + helpPageURL,
                                          helpText);
    }
    
    return instance;
}

/**
 * Constructor for item with parent
 *
 * @param parent
 *    Parent for item in index.
 * @param treeItemType
 *    Type of tree item.
 * @param itemText
 *    Text for the item shown in the topic index.
 * @param helpPageURL
 *    URL for external help pages
 * @param helpText
 *    Text displayed in help browser.
 */
HelpTreeWidgetItem::HelpTreeWidgetItem(QTreeWidgetItem* parent,
                                       const TreeItemType treeItemType,
                                       const AString& itemText,
                                       const AString& helpPageURL,
                                       const AString& helpText)
: QTreeWidgetItem(parent),
m_treeItemType(treeItemType),
m_helpPageURL(helpPageURL),
m_helpText(helpText)
{
    setText(0, itemText);
}

/**
 * Constructor for item WITHOUT parent
 *
 * @param treeItemType
 *    Type of tree item.
 * @param itemText
 *    Text for the item shown in the topic index.
 * @param helpPageURL
 *    URL for external help pages
 * @param helpText
 *    Text displayed in help browser.
 */
HelpTreeWidgetItem::HelpTreeWidgetItem(const TreeItemType treeItemType,
                                       const AString& itemText,
                                       const AString& helpPageURL,
                                       const AString& helpText)
: QTreeWidgetItem(),
m_treeItemType(treeItemType),
m_helpPageURL(helpPageURL),
m_helpText(helpText)
{
    setText(0, itemText);
}

/**
 * Destructor.
 */
HelpTreeWidgetItem::~HelpTreeWidgetItem()
{
    
}

