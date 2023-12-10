/*
** Copyright (c) 2006 - present, Alexis Megas.
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 3. The name of the author may not be used to endorse or promote products
**    derived from BiblioteQ without specific prior written permission.
**
** BIBLIOTEQ IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
** IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
** OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
** IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
** INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
** NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
** BIBLIOTEQ, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <QActionGroup>
#include <QClipboard>
#include <QFileDialog>
#include <QFontDialog>
#include <QPrintPreviewDialog>
#include <QScrollBar>
#include <QSettings>
#include <QTranslator>
#include <QtDebug>
#include <QSortFilterProxyModel>

#include <limits>

#ifdef Q_OS_ANDROID
#if (QT_VERSION >= QT_VERSION_CHECK(6, 1, 0))
#include <QJniObject>
#endif
#endif

#ifdef Q_OS_MACOS
#include "CocoaInitializer.h"
#endif

#ifdef BIBLIOTEQ_POPPLER_VERSION_DEFINED
#include <poppler-version.h>
#endif

extern "C"
{
#if defined(Q_OS_ANDROID) || defined(Q_OS_WIN)
#include <sqlite3/sqlite3.h>
#else
#include <sqlite3.h>
#endif
#ifdef BIBLIOTEQ_LINKED_WITH_YAZ
#include <yaz/yaz-version.h>
#endif
}

#include "biblioteq.h"
#include "biblioteq_architecture.h"
#include "biblioteq_bgraphicsscene.h"
#include "biblioteq_otheroptions.h"
#include "biblioteq_sqlite_create_schema.h"

/*
** -- Global Variables --
*/

QString biblioteq::s_locale = "";
QString biblioteq::s_unknown = QObject::tr("UNKNOWN");
QTranslator *biblioteq::s_appTranslator = nullptr;
QTranslator *biblioteq::s_qtTranslator = nullptr;

int main(int argc, char *argv[])
{
#if defined(Q_OS_MACOS) || defined(Q_OS_WIN)
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling, true);
  QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps, true);
#endif
#endif

  QApplication qapplication(argc, argv);
  auto font(qapplication.font());

#if QT_VERSION >= 0x050700
  qapplication.setAttribute(Qt::AA_DontUseNativeDialogs);
#endif
  font.setStyleStrategy(QFont::StyleStrategy(QFont::PreferAntialias | QFont::PreferQuality));
  qapplication.setFont(font);

#ifdef Q_OS_MACOS
  /*
  ** Eliminate warnings.
  */

  CocoaInitializer ci;
#endif

  qapplication.setWindowIcon(QIcon(":/book.png"));

  /*
  ** Prepare configuration settings.
  */

  QCoreApplication::setOrganizationName("BiblioteQ");
  QCoreApplication::setOrganizationDomain("biblioteq.sourceforge.net");
  QCoreApplication::setApplicationName("BiblioteQ");
  QSettings::setPath(QSettings::IniFormat,
                     QSettings::UserScope,
                     biblioteq::homePath());
  QSettings::setDefaultFormat(QSettings::IniFormat);
  QDir().mkdir(biblioteq::homePath());

  /*
  ** Remove old configuration settings.
  */

  QSettings settings;

  settings.remove("SQLITECustom_header_state");
  settings.remove("automatically_resize_columns");
  settings.remove("column_settings_cleared_v6_51");
  settings.remove("entries_per_page");
  settings.remove("sqlite_db");

  biblioteq::s_appTranslator = new QTranslator(nullptr);
  biblioteq::s_locale = settings.value("locale").toString();
  biblioteq::s_qtTranslator = new QTranslator(nullptr);

  if (!(biblioteq::s_locale == "ar_JO" ||
        biblioteq::s_locale == "cs_CZ" ||
        biblioteq::s_locale == "de_DE" ||
        biblioteq::s_locale == "el_GR" ||
        biblioteq::s_locale == "en_US" ||
        biblioteq::s_locale == "es_AR" ||
        biblioteq::s_locale == "fr_FR" ||
        biblioteq::s_locale == "hu_HU" ||
        biblioteq::s_locale == "nl_BE" ||
        biblioteq::s_locale == "nl_NL" ||
        biblioteq::s_locale == "pl_PL" ||
        biblioteq::s_locale == "pt_PT" ||
        biblioteq::s_locale == "ru_RU"))
    biblioteq::s_locale = QLocale::system().name();

  if (biblioteq::s_appTranslator->load(":/biblioteq_" + biblioteq::s_locale + ".qm"))
    qapplication.installTranslator(biblioteq::s_appTranslator);

  if (biblioteq::s_qtTranslator->load(":/qtbase_" + biblioteq::s_locale + ".qm"))
    qapplication.installTranslator(biblioteq::s_qtTranslator);

  biblioteq biblioteq;

  biblioteq.showMain();
  return qapplication.exec();
}

biblioteq::biblioteq(void) : QMainWindow()
{
  ui.setupUi(this);
  QScreen *screen = QGuiApplication::primaryScreen();
  QRect screenGeometry = screen->geometry();
  int height = screenGeometry.height();
  int width = screenGeometry.width();

  this->resize(width * 0.8, height * 0.8);
  ui.table->setQMain(this);

  if (menuBar())
    menuBar()->setNativeMenuBar(true);

  m_allSearchShown = false;
  m_connected_bar_label = nullptr;
  m_error_bar_label = nullptr;
  m_files = nullptr;
  m_idCt = 0;
  m_lastSearchType = POPULATE_ALL;
  m_pages = 0;
  m_previousTypeFilter = "";
  m_queryOffset = 0;
  m_status_bar_label = nullptr;
  m_branch_diag = new QDialog(this);
  m_menuCategoryActionGroup = new QActionGroup(this);
  m_menuCategoryActionGroup->setExclusive(true);
  m_otheroptions = new biblioteq_otheroptions(this);
  m_pass_diag = new QDialog(this);
  m_printPreview = new QTextBrowser(this);
  m_printPreview->setVisible(false);
#ifdef Q_OS_ANDROID
  m_customquery_diag = new QMainWindow(this);
#else
  m_customquery_diag = new QMainWindow();
#endif
#ifdef Q_OS_ANDROID
  m_error_diag = new QMainWindow(this);
#else
  m_error_diag = new QMainWindow();
#endif
  m_import = new biblioteq_import(this);
  connect(QCoreApplication::instance(), SIGNAL(lastWindowClosed()), this, SLOT(slotLastWindowClosed()));
#ifndef BIBLIOTEQ_LINKED_WITH_POPPLER
  ui.action_Open_PDF_File->setEnabled(false);
#endif
  connect(ui.action_Book, SIGNAL(triggered()), this, SLOT(slotInsertBook()));
  connect(ui.action_Contributors, SIGNAL(triggered()), this, SLOT(slotContributors()));
  connect(ui.action_English, SIGNAL(triggered()), this, SLOT(slotShowDocumentation()));
  connect(ui.action_English_Release_Notes, SIGNAL(triggered()), this, SLOT(slotShowReleaseNotes()));
  connect(ui.action_French, SIGNAL(triggered()), this, SLOT(slotShowDocumentation()));
  connect(ui.action_French_Release_Notes, SIGNAL(triggered()), this, SLOT(slotShowReleaseNotes()));
  connect(ui.action_Online_Documentation, SIGNAL(triggered()), this, SLOT(slotOpenOnlineDocumentation()));
  connect(ui.action_Open_PDF_File, SIGNAL(triggered()), this, SLOT(slotOpenPDFFiles()));
  connect(ui.action_Print_Icons_View, SIGNAL(triggered()), this, SLOT(slotPrintIconsView()));
  connect(ui.actionImportCSV, SIGNAL(triggered()), this, SLOT(slotShowImport()));
  connect(ui.actionOther_Options, SIGNAL(triggered()), this, SLOT(slotShowOtherOptions()));
  connect(ui.action_Full_Screen, SIGNAL(triggered()), this, SLOT(slotViewFullOrNormalScreen()));
  connect(ui.action_Merge_SQLite_Databases, SIGNAL(triggered()), this, SLOT(slotMergeSQLiteDatabases()));
  connect(ui.action_Photograph_Collection, SIGNAL(triggered()), this, SLOT(slotInsertPhotograph()));
  connect(ui.action_Upgrade_SQLite_Schema, SIGNAL(triggered()), this, SLOT(slotUpgradeSqliteScheme()));
  connect(ui.action_VacuumDatabase, SIGNAL(triggered()), this, SLOT(slotVacuum()));
  connect(ui.resetAllSearch, SIGNAL(clicked()), this, SLOT(slotResetAllSearch()));

  auto scene = new biblioteq_bgraphicsscene(ui.graphicsView);

  connect(scene, SIGNAL(selectionChanged()), this, SLOT(slotSceneSelectionChanged()));
  ui.graphicsView->setScene(scene);
  ui.graphicsView->setDragMode(QGraphicsView::RubberBandDrag);
  ui.graphicsView->setRubberBandSelectionMode(Qt::IntersectsItemShape);

  br.setupUi(m_branch_diag);
  cq.setupUi(m_customquery_diag);
  er.setupUi(m_error_diag);
#ifdef Q_OS_ANDROID
  ui.action_Full_Screen->setEnabled(false);
#endif
#ifdef Q_OS_MACOS
  ui.actionSetGlobalFonts->setVisible(false);
#endif
  m_pass_diag->setModal(true);
  m_branch_diag->setModal(true);
  connect(ui.graphicsView->scene(), SIGNAL(enterKeyPressed()), this, SLOT(slotGraphicsSceneEnterKeyPressed()));
  connect(ui.table, SIGNAL(enterKeyPressed()), this, SLOT(slotMainTableEnterKeyPressed()));
  connect(ui.table->horizontalHeader(), SIGNAL(sectionPressed(int)), this, SLOT(slotResizeColumnsAfterSort()));
  connect(ui.table->horizontalHeader(), SIGNAL(sectionClicked(int)), this, SLOT(slotUpdateIndicesAfterSort(int)));
  connect(ui.table->horizontalHeader(), SIGNAL(sectionResized(int, int, int)), this, SLOT(slotSectionResized(int, int, int)));
  connect(er.table->horizontalHeader(), SIGNAL(sectionClicked(int)), this, SLOT(slotResizeColumnsAfterSort()));
  connect(er.copyButton, SIGNAL(clicked()), this, SLOT(slotCopyError()));
  connect(ui.table, SIGNAL(itemSelectionChanged()), this, SLOT(slotDisplaySummary()));
  connect(ui.exitTool, SIGNAL(triggered()), this, SLOT(slotExit()));
  connect(ui.actionExit, SIGNAL(triggered()), this, SLOT(slotExit()));
  connect(ui.actionSetGlobalFonts, SIGNAL(triggered()), this, SLOT(slotSetFonts()));
  connect(ui.deleteTool, SIGNAL(triggered()), this, SLOT(slotDelete()));
  connect(ui.duplicateTool, SIGNAL(triggered()), this, SLOT(slotDuplicate()));
  connect(ui.actionDeleteEntry, SIGNAL(triggered()), this, SLOT(slotDelete()));
  connect(ui.actionDuplicateEntry, SIGNAL(triggered()), this, SLOT(slotDuplicate()));
  connect(ui.refreshTool, SIGNAL(triggered()), this, SLOT(slotRefresh()));
  connect(ui.actionRefreshTable, SIGNAL(triggered()), this, SLOT(slotRefresh()));
  connect(ui.actionReload_biblioteq_conf, SIGNAL(triggered()), this, SLOT(slotReloadBiblioteqConf()));
  connect(ui.menu_Category, SIGNAL(triggered(QAction *)), this, SLOT(slotAutoPopOnFilter(QAction *)));
  connect(ui.modifyTool, SIGNAL(triggered()), this, SLOT(slotModify()));
  connect(ui.actionModifyEntry, SIGNAL(triggered()), this, SLOT(slotModify()));
  connect(ui.actionShowErrorDialog, SIGNAL(triggered()), this, SLOT(slotShowErrorDialog()));
  connect(ui.actionAbout, SIGNAL(triggered()), this, SLOT(slotAbout()));
  connect(ui.actionShowGrid, SIGNAL(triggered()), this, SLOT(slotShowGrid()));
  connect(ui.actionResizeColumns, SIGNAL(triggered()), this, SLOT(slotResizeColumns()));
  connect(ui.actionSaveSettings, SIGNAL(triggered()), this, SLOT(slotSaveConfig()));
  connect(ui.connectTool, SIGNAL(triggered()), this, SLOT(slotShowConnectionDB()));
  connect(ui.actionConnect, SIGNAL(triggered()), this, SLOT(slotShowConnectionDB()));
  connect(ui.disconnectTool, SIGNAL(triggered()), this, SLOT(slotDisconnect()));
  connect(ui.actionDisconnect, SIGNAL(triggered()), this, SLOT(slotDisconnect()));
  connect(br.okButton, SIGNAL(clicked()), this, SLOT(slotConnectDB()));
  connect(br.branch_name, SIGNAL(activated(int)), this, SLOT(slotBranchChanged()));
  connect(ui.filesTool, SIGNAL(triggered()), this, SLOT(slotShowFiles()));
  connect(ui.searchTool, SIGNAL(triggered()), this, SLOT(slotShowMenu()));
  connect(ui.customQueryTool, SIGNAL(triggered()), this, SLOT(slotShowCustomQuery()));
  connect(ui.actionDatabaseSearch, SIGNAL(triggered()), this, SLOT(slotSearch()));
  connect(ui.actionViewDetails, SIGNAL(triggered()), this, SLOT(slotViewDetails()));
  connect(ui.detailsTool, SIGNAL(triggered()), this, SLOT(slotViewDetails()));
  connect(ui.createTool, SIGNAL(triggered()), this, SLOT(slotShowMenu()));
  connect(ui.search, SIGNAL(returnPressed()), this, SLOT(slotSearchBasic()));
  connect(er.resetButton, SIGNAL(clicked()), this, SLOT(slotResetErrorLog()));
#ifdef Q_OS_ANDROID
  connect(er.cancelButton, SIGNAL(clicked()), m_error_diag, SLOT(hide()));
#else
  connect(er.cancelButton, SIGNAL(clicked()), m_error_diag, SLOT(close()));
#endif
  connect(ui.configTool, SIGNAL(triggered()), this, SLOT(slotShowMenu()));
  connect(ui.printTool, SIGNAL(triggered()), this, SLOT(slotShowMenu()));
  connect(ui.previousPageButton, SIGNAL(clicked()), this, SLOT(slotPreviousPage()));
  connect(ui.nextPageButton, SIGNAL(clicked()), this, SLOT(slotNextPage()));
  connect(ui.pagesLabel, SIGNAL(linkActivated(QString)), this, SLOT(slotPageClicked(QString)));
  connect(cq.close_pb, SIGNAL(clicked()), this, SLOT(slotCloseCustomQueryDialog()));
  connect(cq.execute_pb, SIGNAL(clicked()), this, SLOT(slotExecuteCustomQuery()));
  connect(cq.refresh_pb, SIGNAL(clicked()), this, SLOT(slotRefreshCustomQuery()));
  connect(m_otheroptions, SIGNAL(mainWindowCanvasBackgroundColorChanged(QColor)), this, SLOT(slotMainWindowCanvasBackgroundColorChanged(QColor)));
  connect(m_otheroptions, SIGNAL(mainWindowCanvasBackgroundColorPreview(QColor)), this, SLOT(slotPreviewCanvasBackgroundColor(QColor)));
  connect(m_otheroptions, SIGNAL(saved()), this, SLOT(slotOtherOptionsSaved()));
  connect(br.resetButton, SIGNAL(clicked()), this, SLOT(slotResetLoginDialog()));
  connect(br.fileButton, SIGNAL(clicked()), this, SLOT(slotSelectDatabaseFile()));
#ifdef Q_OS_ANDROID
  connect(br.cancelButton, SIGNAL(clicked()), m_branch_diag, SLOT(hide()));
#else
  connect(br.cancelButton, SIGNAL(clicked()), m_branch_diag, SLOT(close()));
#endif
  connect(ui.action_New_SQLite_Database, SIGNAL(triggered()), this, SLOT(slotDisplayNewSqliteDialog()));
  connect(ui.actionDatabase_Enumerations, SIGNAL(triggered()), this, SLOT(slotShowDbEnumerations()));
  connect(ui.actionExport_Current_View, SIGNAL(triggered()), this, SLOT(slotExportAsCSV()));
  connect(ui.actionExport_View_as_PNG, SIGNAL(triggered()), this, SLOT(slotExportAsPNG()));
  connect(ui.action_Database_Enumerations, SIGNAL(triggered()), this, SLOT(slotShowDbEnumerations()));
  ui.table->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(ui.table, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotContextMenu(QPoint)));

#ifdef Q_OS_MACOS
  foreach (auto tool_button, m_all_diag->findChildren<QToolButton *>())
#if (QT_VERSION < QT_VERSION_CHECK(5, 10, 0))
    tool_button->setStyleSheet("QToolButton {border: none; padding-right: 10px}"
                               "QToolButton::menu-button {border: none;}");
#else
    tool_button->setStyleSheet("QToolButton {border: none; padding-right: 15px}"
                               "QToolButton::menu-button {border: none; width: 15px;}");
#endif
#endif

  ui.actionAutoPopulateOnCreation->setEnabled(false);
  ui.actionDatabaseSearch->setEnabled(false);
  ui.actionDatabase_Enumerations->setEnabled(false);
  ui.actionDeleteEntry->setEnabled(false);
  ui.actionDisconnect->setEnabled(false);
  ui.actionDuplicateEntry->setEnabled(false);
  ui.actionImportCSV->setEnabled(false);
  ui.actionModifyEntry->setEnabled(false);
  ui.actionPopulate_Database_Enumerations_Browser_on_Display->setEnabled(false);
  ui.actionRefreshTable->setEnabled(false);
  ui.actionViewDetails->setEnabled(false);
  ui.action_Database_Enumerations->setEnabled(false);
  ui.action_Merge_SQLite_Databases->setEnabled(false);
  ui.action_Upgrade_SQLite_Schema->setEnabled(false);
  ui.action_VacuumDatabase->setEnabled(false);
  ui.configTool->setEnabled(false);
  ui.createTool->setEnabled(false);
  ui.customQueryTool->setEnabled(false);
  ui.deleteTool->setEnabled(false);
  ui.detailsTool->setEnabled(false);
  ui.disconnectTool->setEnabled(false);
  ui.duplicateTool->setEnabled(false);
  ui.filesTool->setEnabled(false);
  ui.menuEntriesPerPage->setEnabled(false);
  ui.menu_Add_Item->setEnabled(false);
  ui.modifyTool->setEnabled(false);
  ui.nextPageButton->setEnabled(false);
  ui.previousPageButton->setEnabled(false);
  ui.printTool->setEnabled(false);
  ui.refreshTool->setEnabled(false);
  ui.searchTool->setEnabled(false);

  QString typefilter("");
  QSettings settings;

  typefilter = m_lastCategory =
      settings.value("last_category", "All").toString();
  typefilter.replace(" ", "_");
  ui.graphicsView->scene()->clear();
  ui.summary->setVisible(false);
  ui.about->setVisible(false);
  ui.table->resetTable(dbUserName(), m_lastCategory);

  if (m_otheroptions->showMainTableImages())
    ui.table->setIconSize(QSize(64, 94));
  else
    ui.table->setIconSize(QSize(0, 0));

  m_previousTypeFilter = m_lastCategory;
  prepareFilter();

  auto found = false;

  for (int i = 0; i < ui.menu_Category->actions().size(); i++)
    if (m_lastCategory ==
        ui.menu_Category->actions().at(i)->data().toString())
    {
      found = true;
      ui.categoryLabel->setText(ui.menu_Category->actions().at(i)->text());
      ui.menu_Category->actions().at(i)->setChecked(true);
      ui.menu_Category->setDefaultAction(ui.menu_Category->actions().at(i));
      break;
    }

  if (!found)
  {
    ui.categoryLabel->setText(tr("All"));

    if (!ui.menu_Category->actions().isEmpty())
      ui.menu_Category->actions().at(0)->setChecked(true);

    ui.menu_Category->setDefaultAction(ui.menu_Category->actions().value(0));
  }

  addConfigOptions(m_lastCategory);
  setUpdatesEnabled(true);
  // userinfo_diag->m_userinfo.expirationdate->setMaximumDate(QDate(3000, 1, 1));

  QActionGroup *group1 = nullptr;
  int end = 21;

  group1 = new QActionGroup(this);

  for (int i = 1; i <= end; i++)
  {
    QAction *action = nullptr;

    if (i == end)
      action = group1->addAction(QString(tr("&Unlimited Entries per Page")));
    else
      action = group1->addAction(QString(tr("&%1")).arg(5 * i));

    if (!action)
      continue;

    connect(action, SIGNAL(triggered()), this, SLOT(slotRefresh()));

    if (i == end)
      action->setData(-1);
    else
      action->setData(5 * i);

    action->setCheckable(true);

    if (i == 1)
      action->setChecked(true);

    ui.menuEntriesPerPage->addAction(action);
  }

  preparePhotographsPerPageMenu();

  QAction *action = nullptr;
  QActionGroup *group2 = nullptr;

  group2 = new QActionGroup(this);
  group2->setObjectName("ViewModeMenu");
  group2->setExclusive(true);
  (action = group2->addAction(tr("Icons Mode")))->setCheckable(true);
  action->setData(0);
  connect(action,
          SIGNAL(triggered(bool)),
          this,
          SLOT(slotChangeView(bool)));
  ui.menu_View->addAction(action);
  (action = group2->addAction(tr("Table Mode")))->setCheckable(true);
  action->setData(1);
  action->setChecked(true);
  connect(action,
          SIGNAL(triggered(bool)),
          this,
          SLOT(slotChangeView(bool)));
  ui.menu_View->addAction(action);

  auto group3 = new QActionGroup(this);

  group3->setExclusive(true);
  (action = group3->addAction(tr("&Arabic")))->setCheckable(true);
  action->setData("ar_JO");
  ui.menu_Language->addAction(action);
  (action = group3->addAction(tr("&Czech")))->setCheckable(true);
  action->setData("cs_CZ");
  ui.menu_Language->addAction(action);
  (action = group3->addAction(tr("Dutch (&Belgium)")))->setCheckable(true);
  action->setData("nl_BE");
  ui.menu_Language->addAction(action);
  (action = group3->addAction(tr("Dutch (&Netherlands)")))->setCheckable(true);
  action->setData("nl_NL");
  ui.menu_Language->addAction(action);
  (action = group3->addAction(tr("&English")))->setCheckable(true);
  action->setData("en_US");
  ui.menu_Language->addAction(action);
  (action = group3->addAction(tr("&French")))->setCheckable(true);
  action->setData("fr_FR");
  ui.menu_Language->addAction(action);
  (action = group3->addAction(tr("&German")))->setCheckable(true);
  action->setData("de_DE");
  ui.menu_Language->addAction(action);
  (action = group3->addAction(tr("Gree&k")))->setCheckable(true);
  action->setData("el_GR");
  ui.menu_Language->addAction(action);
  (action = group3->addAction(tr("&Hebrew")))->setCheckable(true);
  action->setData("he_IL");
  ui.menu_Language->addAction(action);
  (action = group3->addAction(tr("H&ungarian")))->setCheckable(true);
  action->setData("hu_HU");
  ui.menu_Language->addAction(action);
  (action = group3->addAction(tr("&Polish")))->setCheckable(true);
  action->setData("pl_PL");
  ui.menu_Language->addAction(action);
  (action = group3->addAction(tr("Por&tuguese")))->setCheckable(true);
  action->setData("pt_PT");
  ui.menu_Language->addAction(action);
  (action = group3->addAction(tr("&Russian")))->setCheckable(true);
  action->setData("ru_RU");
  ui.menu_Language->addAction(action);
  (action = group3->addAction(tr("&Spanish (Argentina)")))->setCheckable(true);
  action->setData("es_AR");
  ui.menu_Language->addAction(action);

  foreach (auto action, ui.menu_Language->actions())
  {
    if (s_locale == action->data().toString())
      action->setChecked(true);

    connect(action, SIGNAL(triggered()), this, SLOT(slotLanguageChanged()));
  }

  ui.menuPreferredSRUSite->setStyleSheet("QMenu {menu-scrollable: 1;}");
  ui.menuPreferredZ3950Server->setStyleSheet("QMenu {menu-scrollable: 1;}");
  ui.menu_Language->setStyleSheet("QMenu {menu-scrollable: 1;}");
  ui.splitter->restoreState(settings.value("main_splitter_state").toByteArray());
  ui.splitter->setCollapsible(1, false);
  ui.splitter->setStretchFactor(0, 0);
  ui.splitter->setStretchFactor(1, 1);
}

biblioteq::~biblioteq()
{
}

QHash<QString, QString> biblioteq::getAmazonHash(void) const
{
  return m_amazonImages;
}

QHash<QString, QString> biblioteq::getSRUHash(const QString &name) const
{
  QMapIterator<QString, QHash<QString, QString>> it(m_sruMaps);

  while (it.hasNext())
  {
    it.next();

    if (QString(it.key()).remove("&") == QString(name).remove("&"))
      return it.value();
  }

  return QHash<QString, QString>();
}

QHash<QString, QString> biblioteq::getZ3950Hash(const QString &name) const
{
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
  QMapIterator<QString, QHash<QString, QString>> it(m_z3950Maps);
#else
  QMultiMapIterator<QString, QHash<QString, QString>> it(m_z3950Maps);
#endif

  while (it.hasNext())
  {
    it.next();

    if (QString(it.key()).remove("&") == QString(name).remove("&"))
      return it.value();
  }

  return QHash<QString, QString>();
}

QSqlDatabase biblioteq::getDB(void) const
{
  return m_db;
}

QString biblioteq::getPreferredSRUSite(void) const
{
  for (int i = 0; i < ui.menuPreferredSRUSite->actions().size(); i++)
    if (ui.menuPreferredSRUSite->actions().at(i)->isChecked())
      return ui.menuPreferredSRUSite->actions().at(i)->text();

  return "";
}

QString biblioteq::getPreferredZ3950Site(void) const
{
  for (int i = 0; i < ui.menuPreferredZ3950Server->actions().size(); i++)
    if (ui.menuPreferredZ3950Server->actions().at(i)->isChecked())
      return ui.menuPreferredZ3950Server->actions().at(i)->text();

  return "";
}

QString biblioteq::getTypeFilterString(void) const
{
  if (ui.menu_Category->defaultAction())
    return ui.menu_Category->defaultAction()->data().toString();
  else
    return "All";
}

Ui_mainWindow biblioteq::getUI(void) const
{
  return ui;
}

void biblioteq::addConfigOptions(const QString &typefilter)
{
  QAction *action = nullptr;
  int i = 0;

  createConfigToolMenu();
  m_configToolMenu->clear();

  for (i = 0; i < ui.table->columnCount(); i++)
  {
    if (typefilter != "Custom")
    {
      if (typefilter != "All")
      {
        if (ui.table->columnNames().value(i) == "MYOID" ||
            ui.table->columnNames().value(i) == "Type")
          continue;
      }
      else if (ui.table->columnNames().value(i) == "MYOID" ||
               ui.table->columnNames().value(i) == "REQUESTOID")
        continue;
    }

    action = new QAction(ui.table->horizontalHeaderItem(i)->text(), ui.configTool);
    action->setCheckable(true);
    action->setChecked(!ui.table->isColumnHidden(i, typefilter, dbUserName()));
    action->setData(typefilter);
    m_configToolMenu->addAction(action);
    connect(action, SIGNAL(triggered()), this, SLOT(slotSetColumns()));
  }
}

void biblioteq::addError(const QString &type,
                         const QString &summary,
                         const QString &error,
                         const char *file,
                         const int line)
{
  if (error.trimmed().isEmpty())
    return;

  QString str = "";
  QTableWidgetItem *item = nullptr;
  auto now = QDateTime::currentDateTime();
  int i = 0;

  if (m_error_bar_label != nullptr)
  {
    m_error_bar_label->setIcon(QIcon(":/16x16/log.png"));
    m_error_bar_label->setToolTip(tr("Error Log Active"));
  }

  er.table->setSortingEnabled(false);
  er.table->setRowCount(er.table->rowCount() + 1);

  for (i = 0; i < er.table->columnCount(); i++)
  {
    item = new QTableWidgetItem();

    if (i == EVENT_TIME)
      item->setText(now.toString("yyyy/MM/dd hh:mm:ss"));
    else if (i == EVENT_TYPE)
      item->setText(type.trimmed());
    else if (i == SUMMARY)
      item->setText(summary.trimmed());
    else if (i == FULL_DESCRIPTION)
    {
      if (error.simplified().isEmpty())
        item->setText(summary);
      else
        item->setText(error.simplified());
    }
    else if (i == FILE)
    {
      if (file)
        item->setText(file);
    }
    else
    {
      str.setNum(line);
      item->setText(str);
    }

    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    er.table->setItem(er.table->rowCount() - 1, i, item);
  }

  for (int i = 0; i < er.table->columnCount() - 1; i++)
    er.table->resizeColumnToContents(i);

  er.table->setSortingEnabled(true);
}

void biblioteq::adminSetup(void)
{
  ui.action_VacuumDatabase->setEnabled(true);
  ui.detailsTool->setEnabled(true);
  ui.actionViewDetails->setEnabled(true);

  if (m_status_bar_label != nullptr)
  {
    m_status_bar_label->setPixmap(QPixmap(":/16x16/unlock.png"));
    m_status_bar_label->setToolTip(tr("Privileged Mode"));
  }

  if (m_roles.contains("administrator") || m_roles.contains("librarian"))
  {
    ui.table->disconnect(SIGNAL(itemDoubleClicked(QTableWidgetItem *)));
    connect(ui.table, SIGNAL(itemDoubleClicked(QTableWidgetItem *)), this, SLOT(slotModify()));
    ui.graphicsView->scene()->disconnect(SIGNAL(itemDoubleClicked()));
    connect(ui.graphicsView->scene(), SIGNAL(itemDoubleClicked()), this, SLOT(slotModify()));
    updateItemWindows();
  }

  if (m_roles.contains("administrator") || m_roles.contains("librarian"))
    ui.deleteTool->setEnabled(true);

  if (m_roles.contains("administrator") || m_roles.contains("librarian"))
  {
    ui.actionDeleteEntry->setEnabled(true);
    ui.actionDuplicateEntry->setEnabled(true);
    ui.actionImportCSV->setEnabled(true);
    ui.createTool->setEnabled(true);
    ui.duplicateTool->setEnabled(true);
    ui.menu_Add_Item->setEnabled(true);
    ui.modifyTool->setEnabled(true);
  }

  if (m_roles.contains("administrator") || m_roles.contains("librarian"))
  {
    ui.detailsTool->setEnabled(false);
    ui.actionViewDetails->setEnabled(false);
  }

  if (m_roles.contains("administrator") || m_roles.contains("librarian"))
    ui.actionModifyEntry->setEnabled(true);

  if (m_roles.contains("administrator") || m_roles.contains("librarian"))
    ui.actionAutoPopulateOnCreation->setEnabled(true);

  ui.actionDatabase_Enumerations->setEnabled(true);
  ui.actionPopulate_Database_Enumerations_Browser_on_Display->setEnabled(true);
  ui.action_Database_Enumerations->setEnabled(true);
}

void biblioteq::changeEvent(QEvent *event)
{
  if (event)
    switch (event->type())
    {
    case QEvent::LanguageChange:
    {
      br.retranslateUi(m_branch_diag);
      cq.retranslateUi(m_customquery_diag);
      er.retranslateUi(m_error_diag);
      ui.retranslateUi(this);
      ui.graphicsView->scene()->clear();
      ui.graphicsView->resetTransform();
      ui.graphicsView->horizontalScrollBar()->setValue(0);
      ui.graphicsView->verticalScrollBar()->setValue(0);
      ui.nextPageButton->setEnabled(false);
      ui.pagesLabel->setText(tr("1"));
      ui.previousPageButton->setEnabled(false);
      ui.table->resetTable(dbUserName(),
                           ui.menu_Category->defaultAction() ? ui.menu_Category->defaultAction()->data().toString() : "All");
      ui.itemsCountLabel->setText(tr("0 Results"));
      addConfigOptions(m_lastCategory);
      prepareFilter();
      QMessageBox::information(this,
                               tr("BiblioteQ: Information"),
                               tr("You may have selected a new language. Please restart "
                                  "BiblioteQ after saving your settings."));
      QApplication::processEvents();
      break;
    }
    default:
    {
      break;
    }
    }

  QMainWindow::changeEvent(event);
}

void biblioteq::cleanup(void)
{
  if (m_db.isOpen())
    m_db.close();
}

void biblioteq::closeEvent(QCloseEvent *e)
{
  slotExit();
  Q_UNUSED(e);
}

void biblioteq::createSqliteMenuActions(void)
{
  QSettings settings;
  QStringList dups;
  auto allKeys(settings.allKeys());

  ui.menu_Recent_SQLite_Files->clear();

  for (int i = 0; i < allKeys.size(); i++)
  {
    if (!allKeys[i].startsWith("sqlite_db_"))
      continue;

    auto str(settings.value(allKeys[i], "").toString().trimmed());

    if (str.isEmpty())
    {
      settings.remove(allKeys[i]);
      continue;
    }

    QFileInfo fileInfo(str);

    if (!dups.contains(str) && fileInfo.isReadable() && fileInfo.isWritable())
      dups.append(str);
    else
    {
      settings.remove(allKeys[i]);
      continue;
    }

    auto action = new QAction(str, ui.menu_Recent_SQLite_Files);

    action->setData(str);
    connect(action, SIGNAL(triggered(bool)), this,
            SLOT(slotSqliteFileSelected(bool)));
    ui.menu_Recent_SQLite_Files->addAction(action);
  }

  dups.clear();
  allKeys.clear();

  auto action = new QAction(tr("&Clear Menu"), ui.menu_Recent_SQLite_Files);

  connect(action, SIGNAL(triggered(bool)), this, SLOT(slotClearSqliteMenu(bool)));

  if (!ui.menu_Recent_SQLite_Files->actions().isEmpty())
    ui.menu_Recent_SQLite_Files->addSeparator();

  ui.menu_Recent_SQLite_Files->addAction(action);
}

void biblioteq::initialUpdate(void)
{
  /*
  ** Read the configuration file.
  */

  readConfig();

  /*
  ** Act upon the contents of the settings file.
  */

  slotShowGrid();
}

void biblioteq::prepareFilter(void)
{
  QStringList tmplist1;
  QStringList tmplist2;

  tmplist1 << "All"
           << "Books"
           << "Photograph Collections";
  tmplist2 << tr("All")
           << tr("Books")
           << tr("Photograph Collections");

  disconnect(ui.menu_Category, SIGNAL(triggered(QAction *)), this, SLOT(slotAutoPopOnFilter(QAction *)));
  ui.menu_Category->clear();

  for (int i = 0; i < m_menuCategoryActionGroup->actions().size(); i++)
    m_menuCategoryActionGroup->removeAction(m_menuCategoryActionGroup->actions().at(i));

  for (int i = 0; i < tmplist1.size(); i++)
  {
    auto action = ui.menu_Category->addAction(tmplist2[i]);

    if (action)
    {
      action->setCheckable(true);
      action->setData(tmplist1[i]);
      m_menuCategoryActionGroup->addAction(action);
    }
  }

  connect(ui.menu_Category, SIGNAL(triggered(QAction *)), this, SLOT(slotAutoPopOnFilter(QAction *)));
  tmplist1.clear();
  tmplist2.clear();
}

void biblioteq::quit(void)
{
  QCoreApplication::quit();
#ifdef Q_OS_ANDROID
#if (QT_VERSION >= QT_VERSION_CHECK(6, 1, 0))
  auto activity = QJniObject(QNativeInterface::QAndroidApplication::context());

  activity.callMethod<void>("finishAndRemoveTask");
#endif
#endif
}

void biblioteq::removeBook(biblioteq_book *book)
{
  if (book)
    book->deleteLater();
}

void biblioteq::removePhotographCollection(biblioteq_photographcollection *pc)
{
  if (pc)
    pc->deleteLater();
}

void biblioteq::replaceBook(const QString &id, biblioteq_book *book)
{
  Q_UNUSED(id);
  Q_UNUSED(book);
}

void biblioteq::replacePhotographCollection(const QString &id, biblioteq_photographcollection *photograph)
{
  Q_UNUSED(id);
  Q_UNUSED(photograph);
}

void biblioteq::setGlobalFonts(const QFont &font)
{
  QApplication::setOverrideCursor(Qt::WaitCursor);
  QApplication::setFont(font);

  foreach (auto widget, QApplication::allWidgets())
  {
    widget->setFont(font);
    widget->update();
  }

  auto mb = menuBar();

  if (mb)
  {
    mb->setFont(font);

    foreach (auto menu, mb->findChildren<QMenu *>())
      foreach (auto action, menu->actions())
        action->setFont(font);

    mb->update();
  }

  emit fontChanged(font);
  QApplication::restoreOverrideCursor();
}

void biblioteq::showMain(void)
{
  if (statusBar())
    statusBar()->setStyleSheet("QStatusBar::item{border: 0px;}");

  if (m_connected_bar_label)
    m_connected_bar_label->deleteLater();

  if (m_error_bar_label)
    m_error_bar_label->deleteLater();

  if (m_status_bar_label)
    m_status_bar_label->deleteLater();

  if (statusBar())
  {
    m_connected_bar_label = new QLabel();
    m_connected_bar_label->setPixmap(QPixmap(":/16x16/disconnected.png"));
    m_connected_bar_label->setToolTip(tr("Disconnected"));
    statusBar()->addPermanentWidget(m_connected_bar_label);
    m_status_bar_label = new QLabel();
    m_status_bar_label->setPixmap(QPixmap(":/16x16/lock.png"));
    m_status_bar_label->setToolTip(tr("Standard User Mode"));
    statusBar()->addPermanentWidget(m_status_bar_label);
    m_error_bar_label = new QToolButton();
    disconnect(m_error_bar_label, SIGNAL(clicked()), this, SLOT(slotShowErrorDialog()));
    connect(m_error_bar_label, SIGNAL(clicked()), this, SLOT(slotShowErrorDialog()));
    m_error_bar_label->setAutoRaise(true);
    m_error_bar_label->setIcon(QIcon(":/16x16/ok.png"));
    m_error_bar_label->setToolTip(tr("Empty Error Log"));
    statusBar()->addPermanentWidget(m_error_bar_label);
  }

#ifdef Q_OS_MACOS
  if (m_error_bar_label)
    m_error_bar_label->setStyleSheet("QToolButton {border: none;}"
                                     "QToolButton::menu-button {border: none;}");
#endif

  ui.itemsCountLabel->setText(tr("0 Results"));

  QSettings settings;

  if (settings.contains("mainwindowState"))
    restoreState(settings.value("mainwindowState").toByteArray());

  readGlobalSetup();

  /*
  ** Perform additional user interface duties.
  */

  auto group1 = new QActionGroup(this);
  auto list(m_sruMaps.keys());

  for (int i = 0; i < list.size(); i++)
  {
    auto action = group1->addAction(list.at(i));

    if (!action)
      continue;

    action->setCheckable(true);

    if (i == 0)
      action->setChecked(true);

    ui.menuPreferredSRUSite->addAction(action);
  }

  if (ui.menuPreferredSRUSite->actions().isEmpty())
  {
    group1->deleteLater();
    ui.menuPreferredSRUSite->addAction(tr("None"));
  }

  auto group2 = new QActionGroup(this);

  list = m_z3950Maps.keys();

  for (int i = 0; i < list.size(); i++)
  {
    auto action = group2->addAction(list.at(i));

    if (!action)
      continue;

    action->setCheckable(true);

    if (i == 0)
      action->setChecked(true);

    ui.menuPreferredZ3950Server->addAction(action);
  }

  if (ui.menuPreferredZ3950Server->actions().isEmpty())
  {
    group2->deleteLater();
    ui.menuPreferredZ3950Server->addAction(tr("None"));
  }

#ifndef BIBLIOTEQ_LINKED_WITH_YAZ
  ui.menuPreferredZ3950Server->setEnabled(false);
#endif

  /*
  ** Initial update.
  */

  initialUpdate();
  show();
#ifndef Q_OS_MACOS
  setGlobalFonts(QApplication::font());
#endif
  slotResizeColumns();

#if defined(Q_OS_ANDROID)
  QFileInfo fileInfo("assets:/biblioteq.conf");
#elif defined(Q_OS_MACOS)
  QFileInfo fileInfo(QCoreApplication::applicationDirPath() + "/../../../biblioteq.conf");
#elif defined(Q_OS_OS2)
  QFileInfo fileInfo(qgetenv("unixroot") + "/usr/local/biblioteq.conf");
#elif defined(Q_OS_WIN)
  QFileInfo fileInfo(QCoreApplication::applicationDirPath() +
                     QDir::separator() +
                     "biblioteq.conf");
#else
  QFileInfo fileInfo(BIBLIOTEQ_CONFIGFILE);
#endif

  if (!fileInfo.isReadable())
  {
    QMessageBox::warning(this, tr("BiblioteQ: Warning"),
                         tr("BiblioteQ was not able to discover the biblioteq.conf "
                            "file. Default values will be assumed. The expected absolute "
                            "path of biblioteq.conf is %1.")
                             .arg(fileInfo.absolutePath()));
    QApplication::processEvents();
  }

  if (!(QSqlDatabase::isDriverAvailable("QSQLITE")))
  {
    QFileInfo fileInfo("qt.conf");
    QString str("");

    if (fileInfo.isReadable() && fileInfo.size() > 0)
      str = tr("Please verify that the "
               "SQLite driver is installed. "
               "The file qt.conf is present in BiblioteQ's "
               "current working directory. Perhaps a plugin conflict "
               "exists. Please resolve!");
    else
      str = tr("Please verify that the "
               "SQLite driver is installed.");

    QMessageBox::critical(this, tr("BiblioteQ: Error"), str);
    QApplication::processEvents();
  }
  else
  {
    auto list(QApplication::arguments());

    for (int i = 0; i < list.size(); i++)
      if (list.at(i) == "--open-sqlite-database")
      {
        i += 1;

        if (i >= list.size())
          continue;

        br.filename->setText(QFileInfo(list.at(i)).absoluteFilePath());

        for (int j = 0; j < br.branch_name->count(); j++)
          if (m_branches.contains(br.branch_name->itemText(j)))
            if (m_branches[br.branch_name->itemText(j)].value("database_type") == "sqlite")
            {
              br.branch_name->setCurrentIndex(j);
              slotConnectDB();
              break;
            }

        break;
      }
  }
}

void biblioteq::slotAbout(void)
{
  QMessageBox mb(this);
  QString qversion("");
  const auto tmp = qVersion();

  if (tmp)
    qversion = tmp;

  qversion = qversion.trimmed();

  if (qversion.isEmpty())
    qversion = "unknown";

#ifndef Q_OS_MACOS
  mb.setFont(QApplication::font());
#endif
  mb.setWindowIcon(windowIcon());
  mb.setWindowTitle(tr("BiblioteQ: About"));
  mb.setTextFormat(Qt::RichText);
  mb.setText(tr("<html>BiblioteQ Version %1<br>"
                "Architecture %4.<br>"
                "Compiled on %2, %3.<br>"
                "Copyright (c) 2005 - present, X.<br>"
                "Faenza icons.<br>"
#ifdef BIBLIOTEQ_POPPLER_VERSION_DEFINED
                "Poppler version %5.<br>"
#else
                "%5<br>"
#endif
                "Qt version %6 (runtime %7).<br>"
                "SQLite version %9.<br>"
                "This is a heavily modified version of the original Biblioteq Version,<br>"
                "It was created for the use during a PhD Thesis on lithography."
                "Modifications performed by Florian Kleemiss, see <br>"
                "<a href=\"https://github.com/FlorianKleemiss/biblioteq/tree/Strixner_Project\">https://github.com/FlorianKleemiss/biblioteq/tree/Strixner_Project<\a> for changes details.<br>"
                "Please visit <a href=\"https://biblioteq.sourceforge.io\">"
                "https://biblioteq.sourceforge.io</a> or "
                "<a href=\"https://textbrowser.github.io/biblioteq/\">"
                "https://textbrowser.github.io/biblioteq</a> "
                "for project information.</html>")
                 .arg(BIBLIOTEQ_VERSION)
                 .arg(__DATE__)
                 .arg(__TIME__)
                 .arg(BIBLIOTEQ_ARCHITECTURE_STR)
                 .
#ifdef BIBLIOTEQ_POPPLER_VERSION_DEFINED
             arg(POPPLER_VERSION)
                 .
#else
             arg(tr("0.1."))
                 .
#endif
             arg(QT_VERSION_STR)
                 .arg(qversion)
                 .
#ifdef BIBLIOTEQ_LINKED_WITH_YAZ
             arg(YAZ_VERSION)
                 .
#else
             arg(tr("is not available"))
                 .
#endif
             arg(SQLITE_VERSION));
  mb.setStandardButtons(QMessageBox::Ok);
  mb.setIconPixmap(QPixmap(":/book.png").scaled(QSize(128, 128), Qt::KeepAspectRatio, Qt::SmoothTransformation));
  mb.exec();
  QApplication::processEvents();
}

void biblioteq::slotAutoPopOnFilter(QAction *action)
{
  if (!action)
    return;

  disconnect(ui.menu_Category, SIGNAL(triggered(QAction *)), this, SLOT(slotAutoPopOnFilter(QAction *)));
  action->setChecked(true);
  ui.menu_Category->setDefaultAction(action);
  connect(ui.menu_Category, SIGNAL(triggered(QAction *)), this, SLOT(slotAutoPopOnFilter(QAction *)));
  ui.categoryLabel->setText(action->text());

  QSettings settings;

  m_lastCategory = getTypeFilterString();
  settings.setValue("last_category", m_lastCategory);

  /*
  ** Populate the main table only if we're connected to a database.
  */

  if (m_db.isOpen())
    slotRefresh();
  else
  {
    QString typefilter("");

    typefilter = action->data().toString();
    ui.graphicsView->scene()->clear();
    ui.graphicsView->resetTransform();
    ui.graphicsView->verticalScrollBar()->setValue(0);
    ui.graphicsView->horizontalScrollBar()->setValue(0);
    ui.nextPageButton->setEnabled(false);
    ui.pagesLabel->setText(tr("1"));
    ui.previousPageButton->setEnabled(false);
    ui.table->resetTable(dbUserName(), typefilter);
    ui.itemsCountLabel->setText(tr("0 Results"));
  }
}

void biblioteq::slotBranchChanged(void)
{
  QHash<QString, QString> tmphash;

  tmphash = m_branches[br.branch_name->currentText()];

  if (tmphash.value("database_type") == "sqlite")
  {
    br.stackedWidget->setCurrentIndex(0);
    br.fileButton->setFocus();
  }

  tmphash.clear();
  m_branch_diag->update();
  m_branch_diag->resize(m_branch_diag->width(),
                        m_branch_diag->minimumSize().height());
  m_branch_diag->show();
}

void biblioteq::slotChangeView(bool checked)
{
  Q_UNUSED(checked);

  auto action = qobject_cast<QAction *>(sender());

  if (action)
  {
    ui.stackedWidget->setCurrentIndex(action->data().toInt());

    if (ui.stackedWidget->currentIndex() == 0)
      ui.table->setSelectionMode(QAbstractItemView::MultiSelection);
    else
      ui.table->setSelectionMode(QAbstractItemView::ExtendedSelection);

    QSettings settings;

    settings.setValue("view_mode_index", action->data().toInt());
  }
}

void biblioteq::slotClearSqliteMenu(bool state)
{
  Q_UNUSED(state);
  br.filename->clear();
  ui.menu_Recent_SQLite_Files->clear();

  QSettings settings;
  auto allKeys(settings.allKeys());

  for (int i = 0; i < allKeys.size(); i++)
    if (allKeys[i].startsWith("sqlite_db_"))
      settings.remove(allKeys[i]);

  allKeys.clear();
  createSqliteMenuActions();
}

void biblioteq::slotCloseCustomQueryDialog(void)
{
#ifdef Q_OS_ANDROID
  m_customquery_diag->hide();
#else
  m_customquery_diag->close();
#endif
}

void biblioteq::slotCopyError(void)
{
  QString text = "";
  auto clipboard = QApplication::clipboard();
  auto list(er.table->selectionModel()->selectedRows());
  int i = 0;
  int j = 0;

  if (list.isEmpty())
  {
    QMessageBox::critical(m_error_diag,
                          tr("BiblioteQ: User Error"),
                          tr("To copy the contents of the Error Log "
                             "into the clipboard buffer, you must first "
                             "select at least one entry."));
    QApplication::processEvents();
    return;
  }

  QApplication::setOverrideCursor(Qt::WaitCursor);

  foreach (const auto &index, list)
  {
    i = index.row();

    for (j = 0; j < er.table->columnCount(); j++)
    {
      text += er.table->item(i, j)->text();

      if (er.table->columnCount() - 1 != j)
        text += ",";
    }

    text = text.trimmed();
    text += "\n";
  }

  if (!text.isEmpty())
    clipboard->setText(text);

  list.clear();
  QApplication::restoreOverrideCursor();
}

void biblioteq::slotDelete(void)
{
  if (!m_db.isOpen())
    return;

  QSqlQuery query(m_db);
  QString itemType = "";
  QString oid = "";
  QString str = "";
  auto error = false;
  auto list(ui.table->selectionModel()->selectedRows());
  int col = -1;
  int i = 0;
  int numdeleted = 0;

  if (list.isEmpty())
  {
    QMessageBox::critical(this, tr("BiblioteQ: User Error"),
                          tr("Please select an item to delete."));
    QApplication::processEvents();
    return;
  }

  col = ui.table->columnNumber("MYOID");

  foreach (const auto &index, list)
  {
    i = index.row();

    if (ui.table->item(i, col) == nullptr)
      continue;

    oid = biblioteq_misc_functions::getColumnString(ui.table, i, ui.table->columnNumber("MYOID"));
    itemType = biblioteq_misc_functions::getColumnString(ui.table, i, ui.table->columnNumber("Type"));

    if (oid.isEmpty() || itemType.isEmpty())
    {
      addError(QString(tr("Error")),
               QString(tr("The main table does not contain enough "
                          "information for item deletion.")),
               QString(tr("The main table does not contain enough "
                          "information for item deletion.")),
               __FILE__, __LINE__);
      QMessageBox::critical(this, tr("BiblioteQ: Error"),
                            tr("The main table does not contain enough "
                               "information for item deletion."));
      QApplication::processEvents();
      list.clear();
      return;
    }
  }

  if (!list.isEmpty())
  {
    if (QMessageBox::question(this,
                              tr("BiblioteQ: Question"),
                              tr("Are you sure that you wish to permanently "
                                 "delete the selected item(s)?"),
                              QMessageBox::Yes | QMessageBox::No,
                              QMessageBox::No) == QMessageBox::No)
    {
      QApplication::processEvents();
      list.clear();
      return;
    }
    else
      QApplication::processEvents();
  }

  QProgressDialog progress(this);

  progress.setCancelButton(nullptr);
  progress.setModal(true);
  progress.setWindowTitle(tr("BiblioteQ: Progress Dialog"));
  progress.setLabelText(tr("Deleting the selected item(s)..."));
  progress.setMaximum(list.size());
  progress.setMinimum(0);
  progress.show();
  progress.repaint();
  QApplication::processEvents();

  foreach (const auto &index, list)
  {
    i = index.row();

    if (i + 1 <= progress.maximum())
      progress.setValue(i + 1);

    progress.repaint();
    QApplication::processEvents();

    if (ui.table->item(i, col) == nullptr)
      continue;

    str = ui.table->item(i, col)->text();
    itemType = biblioteq_misc_functions::getColumnString(ui.table, i, ui.table->columnNumber("Type")).toLower();

    if (itemType == "photograph collection")
      itemType = itemType.replace(" ", "_");
    else
      itemType = itemType.remove(" ");

    if (itemType == "book" ||
        itemType == "photograph_collection")
      query.prepare(QString("DELETE FROM %1 WHERE myoid = ?").arg(itemType));

    query.bindValue(0, str);

    if (!query.exec())
    {
      error = true;
      addError(QString(tr("Database Error")),
               QString(tr("Unable to delete the item.")),
               query.lastError().text(), __FILE__, __LINE__);
    }
    else
    {
      deleteItem(str, itemType);
      numdeleted += 1;

      /*
      ** SQL errors are ignored.
      */

      if (itemType == "book")
      {
        query.prepare(QString("DELETE FROM %1_copy_info WHERE item_oid = ?").arg(itemType));
        query.bindValue(0, str);
        query.exec();

        query.prepare(QString("DELETE FROM %1_files WHERE item_oid = ?").arg(itemType));
        query.bindValue(0, str);
        query.exec();
      }
      else if (itemType == "photograph_collection")
      {
        query.prepare("DELETE FROM photograph WHERE collection_oid = ?");
        query.bindValue(0, str);
        query.exec();
      }
    }
  }

  progress.close();

  /*
  ** Provide some fancy messages.
  */

  if (error)
  {
    QMessageBox::critical(this, tr("BiblioteQ: Database Error"),
                          tr("Unable to delete all or some of the selected "
                             "items."));
    QApplication::processEvents();
  }

  if (numdeleted > 0)
    slotRefresh();

  list.clear();
}

void biblioteq::slotDisplayNewSqliteDialog(void)
{
  QFileDialog dialog(this);
  auto error = true;

  dialog.setFileMode(QFileDialog::AnyFile);
  dialog.setDirectory(QDir::homePath());
  dialog.setNameFilter("SQLite Database (*.sqlite)");
  dialog.setDefaultSuffix("sqlite");
  dialog.setAcceptMode(QFileDialog::AcceptSave);
  dialog.setOption(QFileDialog::DontUseNativeDialog);
  dialog.setWindowTitle(tr("BiblioteQ: New SQLite Database"));
  dialog.exec();
  dialog.close();
  QApplication::processEvents();

  if (dialog.result() == QDialog::Accepted)
  {
    repaint();
    QApplication::processEvents();

    int rc = 0;
    sqlite3 *ppDb = nullptr;

    QApplication::setOverrideCursor(Qt::WaitCursor);
    QFile::remove(dialog.selectedFiles().value(0));
    rc = sqlite3_open_v2(dialog.selectedFiles().value(0).toUtf8(),
                         &ppDb,
                         SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE,
                         nullptr);

    if (rc == SQLITE_OK)
    {
      char *errorstr = nullptr;

      if (sqlite3_exec(ppDb,
                       sqlite_create_schema_text,
                       nullptr,
                       nullptr,
                       &errorstr) == SQLITE_OK)
        error = false;
      else
        addError(tr("Database Error"),
                 "Unable to create the specified SQLite database.",
                 errorstr, __FILE__, __LINE__);

      sqlite3_free(errorstr);
    }
    else
      addError(tr("Database Error"),
               tr("Unable to create the specified SQLite database."),
               "sqlite3_open_v2() failure.", __FILE__, __LINE__);

    sqlite3_close(ppDb);
    QApplication::restoreOverrideCursor();

    if (!error)
    {
      /*
      ** The user may not wish to open the new database, so let's not
      ** connect automatically.
      */

      if (m_db.isOpen())
      {
        /*
        ** Attempt to locate an SQLite branch.
        */

        auto found = false;

        for (int i = 0; i < br.branch_name->count(); i++)
        {
          if (m_branches.contains(br.branch_name->itemText(i)))
            if (m_branches[br.branch_name->itemText(i)].value("database_type") == "sqlite")
            {
              found = true;
              br.branch_name->setCurrentIndex(i);
              break;
            }
        }

        if (found)
        {
          if (QMessageBox::question(this,
                                    tr("BiblioteQ: Question"),
                                    tr("It appears that you are already "
                                       "connected to a database. Do you "
                                       "want to terminate the current connection "
                                       "and connect to the new SQLite database?"),
                                    QMessageBox::Yes | QMessageBox::No,
                                    QMessageBox::No) == QMessageBox::Yes)
          {
            QApplication::processEvents();
            br.filename->setText(dialog.selectedFiles().value(0));
            slotConnectDB();
          }
          else
            QApplication::processEvents();
        }
      }
      else
      {
        /*
        ** Attempt to locate an SQLite branch.
        */

        auto found = false;

        for (int i = 0; i < br.branch_name->count(); i++)
        {
          if (m_branches.contains(br.branch_name->itemText(i)))
            if (m_branches[br.branch_name->itemText(i)].value("database_type") == "sqlite")
            {
              found = true;
              br.branch_name->setCurrentIndex(i);
              break;
            }
        }

        if (found)
        {
          br.filename->setText(dialog.selectedFiles().value(0));
          slotConnectDB();
        }
      }
    }
    else
    {
      QMessageBox::critical(this, tr("BiblioteQ: Database Error"),
                            tr("An error occurred while attempting "
                               "to create the specified SQLite database."));
      QApplication::processEvents();
    }
  }
}

void biblioteq::slotDuplicate(void)
{
  if (!m_db.isOpen())
    return;

  biblioteq_main_table *table = ui.table;
  QString oid = "";
  QString type = "";
  auto error = false;
  auto list(table->selectionModel()->selectedRows());
  biblioteq_book *book = nullptr;
  biblioteq_photographcollection *photograph = nullptr;
  int i = 0;

  if (list.isEmpty())
  {
    QMessageBox::critical(this, tr("BiblioteQ: User Error"),
                          tr("Please select at least one item to "
                             "duplicate."));
    QApplication::processEvents();
    return;
  }
  else if (list.size() >= MAXIMUM_DEVICES_CONFIRMATION)
  {
    if (QMessageBox::question(this, tr("BiblioteQ: Question"),
                              tr("Are you sure that you wish to duplicate "
                                 "the ") +
                                  QString::number(list.size()) +
                                  tr(" selected items? BiblioteQ will exit if "
                                     "it's unable "
                                     "to acquire resources."),
                              QMessageBox::Yes | QMessageBox::No,
                              QMessageBox::No) == QMessageBox::No)
    {
      QApplication::processEvents();
      list.clear();
      return;
    }

    QApplication::processEvents();
  }

  QString id("");

  QApplication::setOverrideCursor(Qt::WaitCursor);
  std::stable_sort(list.begin(), list.end());

  foreach (const auto &index, list)
  {
    i = index.row();
    oid = biblioteq_misc_functions::getColumnString(table, i, table->columnNumber("MYOID"));
    type = biblioteq_misc_functions::getColumnString(table, i, table->columnNumber("Type"));
    m_idCt += 1;
    id = QString("duplicate_%1").arg(m_idCt);

    if (type.toLower() == "book")
    {
      book = new biblioteq_book(this, oid, index);
      book->duplicate(id, EDITABLE);
    }
    else if (type.toLower() == "photograph collection")
    {
      photograph = new biblioteq_photographcollection(this, oid, index);
      photograph->duplicate(id, EDITABLE);
    }
    else
    {
      error = true;
      break;
    }
  }

  list.clear();
  QApplication::restoreOverrideCursor();

  if (error)
  {
    QMessageBox::critical(this,
                          tr("BiblioteQ: Error"),
                          tr("Unable to determine the selected item's type."));
    QApplication::processEvents();
  }
}

void biblioteq::slotExecuteCustomQuery(void)
{
  QString querystr = "";

  querystr = cq.query_te->toPlainText().trimmed();

  if (querystr.isEmpty())
  {
    QMessageBox::critical(m_customquery_diag, tr("BiblioteQ: User Error"),
                          tr("Please provide a valid SQL statement."));
    QApplication::processEvents();
    return;
  }

  const auto &q(querystr.toLower());

  if (q.contains("alter ") ||
      q.contains("cluster ") ||
      q.contains("create ") ||
      q.contains("drop ") ||
      q.contains("grant ") ||
      q.contains("insert ") ||
      q.contains("lock ") ||
      q.contains("revoke ") ||
      q.contains("truncate ") ||
      q.contains("update "))
  {
    QMessageBox::critical(m_customquery_diag,
                          tr("BiblioteQ: User Error"),
                          tr("Please provide a non-destructive SQL statement."));
    QApplication::processEvents();
    return;
  }
  else if (q.contains("delete "))
  {
    if (QMessageBox::
            question(m_customquery_diag,
                     tr("BiblioteQ: Question"),
                     tr("Are you sure that you wish to execute the statement?"),
                     QMessageBox::No | QMessageBox::Yes,
                     QMessageBox::No) == QMessageBox::No)
    {
      QApplication::processEvents();
      return;
    }
    else
      QApplication::processEvents();

    QSqlQuery query(m_db);

    if (query.exec(querystr))
      slotRefresh();

    return;
  }

  (void)populateTable(CUSTOM_QUERY, "Custom", querystr);
}

void biblioteq::slotExit(void)
{
  QSettings settings;

  settings.setValue("mainwindowState", saveState());
  settings.sync();
  slotLastWindowClosed();
  quit();
}

void biblioteq::slotExportAsCSV(void)
{
  exportAsCSV(ui.table, tr("BiblioteQ: Export Table View as CSV"));
}

void biblioteq::slotInsertBook(void)
{
  QString id("");
  biblioteq_book *book = nullptr;

  m_idCt += 1;
  id = QString("insert_%1").arg(m_idCt);
  book = new biblioteq_book(this, id, QModelIndex());
  book->insert();
}

void biblioteq::slotInsertPhotograph(void)
{
  QString id("");
  biblioteq_photographcollection *photograph_collection = nullptr;

  m_idCt += 1;
  id = QString("insert_%1").arg(m_idCt);
  photograph_collection = new biblioteq_photographcollection(this, id, QModelIndex());
  photograph_collection->insert();
}

void biblioteq::slotLanguageChanged(void)
{
  auto action = qobject_cast<QAction *>(sender());

  if (action && action->isChecked())
  {
    QApplication::removeTranslator(s_appTranslator);
    QApplication::removeTranslator(s_qtTranslator);
    s_locale = action->data().toString();

    if (s_appTranslator->load(":/biblioteq_" + s_locale + ".qm"))
      QApplication::installTranslator(s_appTranslator);

    if (s_qtTranslator->load(":/qtbase_" + s_locale + ".qm"))
      QApplication::installTranslator(s_qtTranslator);
  }
}

void biblioteq::slotModify(void)
{
  if (!m_db.isOpen())
    return;

  QString oid = "";
  QString type = "";
  auto error = false;
  auto list(ui.table->selectionModel()->selectedRows());
  auto table = ui.table;
  biblioteq_book *book = nullptr;
  biblioteq_photographcollection *photograph = nullptr;
  int i = 0;

  if (list.isEmpty())
  {
    QMessageBox::critical(this, tr("BiblioteQ: User Error"),
                          tr("Please select at least one item to modify."));
    QApplication::processEvents();
    return;
  }
  else if (list.size() >= MAXIMUM_DEVICES_CONFIRMATION)
  {
    if (QMessageBox::question(this, tr("BiblioteQ: Question"),
                              tr("Are you sure that you wish to modify the ") +
                                  QString::number(list.size()) +
                                  tr(" selected items? BiblioteQ will exit if it's unable "
                                     "to acquire resources."),
                              QMessageBox::Yes | QMessageBox::No,
                              QMessageBox::No) == QMessageBox::No)
    {
      QApplication::processEvents();
      list.clear();
      return;
    }

    QApplication::processEvents();
  }

  QApplication::setOverrideCursor(Qt::WaitCursor);
  std::stable_sort(list.begin(), list.end());

  foreach (const auto &index, list)
  {
    i = index.row();
    oid = biblioteq_misc_functions::getColumnString(table, i, table->columnNumber("MYOID"));
    type = biblioteq_misc_functions::getColumnString(table, i, table->columnNumber("Type"));
    book = nullptr;
    photograph = nullptr;

    if (type.toLower() == "book")
    {
      foreach (auto w, QApplication::topLevelWidgets())
      {
        auto b = qobject_cast<biblioteq_book *>(w);

        if (b && b->getID() == oid)
        {
          book = b;
          break;
        }
      }

      if (!book)
        book = new biblioteq_book(this, oid, index);

      book->modify(EDITABLE);
    }
    else if (type.toLower() == "photograph collection")
    {
      foreach (auto w, QApplication::topLevelWidgets())
      {
        auto p = qobject_cast<biblioteq_photographcollection *>(w);

        if (p && p->getID() == oid)
        {
          photograph = p;
          break;
        }
      }

      if (!photograph)
        photograph = new biblioteq_photographcollection(this, oid, index);

      photograph->modify(EDITABLE);
    }
    else
    {
      error = true;
      break;
    }
  }

  list.clear();
  QApplication::restoreOverrideCursor();

  if (error)
  {
    QMessageBox::critical(this,
                          tr("BiblioteQ: Error"),
                          tr("Unable to determine the selected item's type."));
    QApplication::processEvents();
  }
}

void biblioteq::slotNextPage(void)
{
  if (m_db.isOpen())
  {
    if (m_lastSearchStr == "Item Search Query")
      (void)populateTable(m_searchQuery, m_previousTypeFilter, NEXT_PAGE, m_lastSearchType);
    else
      (void)populateTable(m_lastSearchType, m_previousTypeFilter, m_lastSearchStr, NEXT_PAGE);
  }
}

void biblioteq::slotPageClicked(const QString &link)
{
  if (m_db.isOpen())
  {
    if (m_lastSearchStr == "Item Search Query")
      (void)populateTable(m_searchQuery,
                          m_previousTypeFilter,
                          -link.toInt(),
                          m_lastSearchType);
    else
      (void)populateTable(m_lastSearchType,
                          m_previousTypeFilter,
                          m_lastSearchStr,
                          -link.toInt());
  }
}

void biblioteq::slotPreviousPage(void)
{
  if (m_db.isOpen())
  {
    if (m_lastSearchStr == "Item Search Query")
      (void)populateTable(m_searchQuery,
                          m_previousTypeFilter,
                          PREVIOUS_PAGE,
                          m_lastSearchType);
    else
      (void)populateTable(m_lastSearchType,
                          m_previousTypeFilter,
                          m_lastSearchStr,
                          PREVIOUS_PAGE);
  }
}

void biblioteq::slotPrintPreview(QPrinter *printer)
{
  if (printer)
    m_printPreview->print(printer);
}

void biblioteq::slotPrintView(void)
{
  QPrinter printer;
  QScopedPointer<QPrintDialog> dialog(new QPrintDialog(&printer, this));
  QString html(viewHtml());
  QTextDocument document;

  printer.setColorMode(QPrinter::GrayScale);
  printer.setResolution(600);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
  printer.setPageOrientation(QPageLayout::Landscape);
  printer.setPageSize(QPageSize(QPageSize::Letter));
#else
  printer.setOrientation(QPrinter::Landscape);
  printer.setPageSize(QPrinter::Letter);
#endif

  if (dialog->exec() == QDialog::Accepted)
  {
    QApplication::processEvents();
    document.setHtml(html);
    document.print(&printer);
  }

  QApplication::processEvents();
}

void biblioteq::slotPrintViewPreview(void)
{
  QPrinter printer;
  QScopedPointer<QPrintPreviewDialog> printDialog(new QPrintPreviewDialog(&printer, this));

  printDialog->setWindowModality(Qt::ApplicationModal);
  printer.setColorMode(QPrinter::GrayScale);
  printer.setResolution(600);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
  printer.setPageOrientation(QPageLayout::Landscape);
  printer.setPageSize(QPageSize(QPageSize::Letter));
#else
  printer.setOrientation(QPrinter::Landscape);
  printer.setPageSize(QPrinter::Letter);
#endif
  connect(printDialog.data(), SIGNAL(paintRequested(QPrinter *)), this, SLOT(slotPrintPreview(QPrinter *)));
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  m_printPreview->setHtml(viewHtml());
  printDialog->show();
  QApplication::restoreOverrideCursor();

  if (printDialog->exec() == QDialog::Accepted)
  {
    QApplication::processEvents();
    m_printPreview->print(&printer);
  }

  QApplication::processEvents();
}

void biblioteq::slotRefresh(void)
{
  if (m_db.isOpen())
  {
    QApplication::processEvents();

    QString str = "";
    QVariant data(ui.menu_Category->defaultAction() ? ui.menu_Category->defaultAction()->data().toString() : "All");

    (void)populateTable(POPULATE_ALL, data.toString(), str.trimmed());
  }
}

void biblioteq::slotResetErrorLog(void)
{
  QStringList list;

  list.append(tr("Event Time"));
  list.append(tr("Event Type"));
  list.append(tr("Summary"));
  list.append(tr("Full Description"));
  list.append(tr("File"));
  list.append(tr("Line Number"));
  er.table->setCurrentItem(nullptr);
  er.table->setColumnCount(0);
  er.table->setRowCount(0);
  er.table->setColumnCount(0);
  er.table->scrollToTop();
  er.table->horizontalScrollBar()->setValue(0);
  er.table->setColumnCount(list.size());
  er.table->setHorizontalHeaderLabels(list);
  list.clear();
  er.table->horizontalHeader()->setSortIndicator(1, Qt::AscendingOrder);

  for (int i = 0; i < er.table->columnCount() - 1; i++)
    er.table->resizeColumnToContents(i);

  if (m_error_bar_label != nullptr)
  {
    m_error_bar_label->setIcon(QIcon(":/16x16/ok.png"));
    m_error_bar_label->setToolTip(tr("Empty Error Log"));
  }
}

void biblioteq::slotResetLoginDialog(void)
{
  br.filename->clear();

  QSettings settings;
  int index = 0;

  index = br.branch_name->findText(settings.value("previous_branch_name").toString());

  if (index >= 0)
    br.branch_name->setCurrentIndex(index);
  else
    br.branch_name->setCurrentIndex(0);

  slotBranchChanged();
}

void biblioteq::slotResizeColumns(void)
{
  if (!sender())
    if (!ui.actionAutomatically_Resize_Column_Widths->isChecked())
      return;

  QApplication::setOverrideCursor(Qt::WaitCursor);

  for (int i = 0; i < ui.table->columnCount() - 1; i++)
    ui.table->resizeColumnToContents(i);

  ui.table->horizontalHeader()->setStretchLastSection(true);
  QApplication::restoreOverrideCursor();
}

void biblioteq::slotResizeColumnsAfterSort(void)
{
  QObject *parent = nullptr;
  auto object = qobject_cast<QObject *>(sender());

  if (object != nullptr && object->parent() != nullptr)
  {
    if (object->parent() == ui.table)
      if (!ui.actionAutomatically_Resize_Column_Widths->isChecked())
        return;

    QApplication::setOverrideCursor(Qt::WaitCursor);
    parent = object->parent();
    (qobject_cast<QTableWidget *>(parent))->resizeColumnsToContents();
    (qobject_cast<QTableWidget *>(parent))->horizontalHeader()->setStretchLastSection(true);
    QApplication::restoreOverrideCursor();
  }
}

void biblioteq::slotSearch(void)
{
  if (!m_db.isOpen())
    return;

  m_allSearchShown = true;
}

void biblioteq::slotSectionResized(int logicalIndex, int oldSize, int newSize)
{
  Q_UNUSED(logicalIndex);
  Q_UNUSED(newSize);
  Q_UNUSED(oldSize);
}

void biblioteq::slotSelectDatabaseFile(void)
{
  QFileDialog dialog(m_branch_diag);

  dialog.setFileMode(QFileDialog::ExistingFile);
  dialog.setDirectory(QDir::homePath());
  dialog.setNameFilter("SQLite Database (*.sqlite)");
  dialog.setOption(QFileDialog::DontUseNativeDialog);
  dialog.setWindowTitle(tr("BiblioteQ: SQLite Database Selection"));
  dialog.exec();
  QApplication::processEvents();

  if (dialog.result() == QDialog::Accepted)
    br.filename->setText(dialog.selectedFiles().value(0));
}

void biblioteq::slotSetColumns(void)
{
  createConfigToolMenu();

  for (int i = 0; i < m_configToolMenu->actions().size(); i++)
  {
    ui.table->setColumnHidden(i, !m_configToolMenu->actions().at(i)->isChecked());
    ui.table->recordColumnHidden(dbUserName(),
                                 m_configToolMenu->actions().at(i)->data().toString(),
                                 i,
                                 !m_configToolMenu->actions().at(i)->isChecked());
  }
}

void biblioteq::slotSetFonts(void)
{
  QFontDialog dialog(this);

  dialog.setCurrentFont(QApplication::font());
  dialog.setOption(QFontDialog::DontUseNativeDialog);
  dialog.setWindowTitle(tr("BiblioteQ: Select Global Font"));

  if (dialog.exec() == QDialog::Accepted)
  {
    QApplication::processEvents();
    setGlobalFonts(dialog.selectedFont());
  }

  QApplication::processEvents();
}

void biblioteq::slotShowConnectionDB(void)
{
  slotBranchChanged();
}

void biblioteq::slotShowCustomQuery(void)
{
  if (cq.tables_t->columnCount() == 0)
    slotRefreshCustomQuery();

  static auto resized = false;

  if (!resized)
    m_customquery_diag->resize(qRound(0.85 * size().width()),
                               qRound(0.85 * size().height()));

  resized = true;
  biblioteq_misc_functions::center(m_customquery_diag, this);
  m_customquery_diag->showNormal();
  m_customquery_diag->activateWindow();
  m_customquery_diag->raise();
}

void biblioteq::slotShowDbEnumerations(void)
{
  if (!db_enumerations)
    db_enumerations = new biblioteq_dbenumerations(this);

  db_enumerations->show(this,
                        ui.actionPopulate_Database_Enumerations_Browser_on_Display->isChecked());
}

void biblioteq::slotShowErrorDialog(void)
{
  er.table->horizontalHeader()->setSortIndicator(1, Qt::AscendingOrder);

  for (int i = 0; i < er.table->columnCount() - 1; i++)
    er.table->resizeColumnToContents(i);

  static auto resized = false;

  if (!resized)
    m_error_diag->resize(qRound(0.85 * size().width()),
                         qRound(0.85 * size().height()));

  resized = true;
  biblioteq_misc_functions::center(m_error_diag, this);
  m_error_diag->showNormal();
  m_error_diag->activateWindow();
  m_error_diag->raise();
}

void biblioteq::slotShowGrid(void)
{
  ui.table->setShowGrid(ui.actionShowGrid->isChecked());
}

void biblioteq::slotShowMenu(void)
{
  QPoint point;
  auto widget = widgetForAction(qobject_cast<QAction *>(sender()));

  if (widget)
    point = widget->mapToGlobal(widget->rect().bottomRight() - QPoint(5, 5));
  else
    point = QCursor::pos();

  if (sender() == ui.configTool)
  {
    createConfigToolMenu();

    auto typefilter = ui.menu_Category->defaultAction() ? ui.menu_Category->defaultAction()->data().toString() : "All";

    addConfigOptions(typefilter);
    m_configToolMenu->exec(point);
  }
  else if (sender() == ui.createTool)
  {
    QMenu menu(this);

    connect(menu.addAction(tr("Add &Book...")), SIGNAL(triggered()), this, SLOT(slotInsertBook()));
    connect(menu.addAction(tr("Add &Photograph Collection...")), SIGNAL(triggered()), this, SLOT(slotInsertPhotograph()));
    menu.exec(point);
  }
  else if (sender() == ui.printTool)
  {
    QMenu menu(this);

    connect(menu.addAction(tr("Print...")), SIGNAL(triggered()), this, SLOT(slotPrintView()));
    connect(menu.addAction(tr("Print Preview...")), SIGNAL(triggered()), this, SLOT(slotPrintViewPreview()));
    menu.exec(point);
  }
  else if (sender() == ui.searchTool)
  {
    QMenu menu(this);

    connect(menu.addAction(tr("General &Search...")), SIGNAL(triggered()), this, SLOT(slotSearch()));
    menu.addSeparator();
    connect(menu.addAction(tr("&Book Search...")), SIGNAL(triggered()), this, SLOT(slotBookSearch()));
    connect(menu.addAction(tr("&Photograph Collection Search...")), SIGNAL(triggered()), this, SLOT(slotPhotographSearch()));
    menu.exec(point);
  }
}

void biblioteq::slotSqliteFileSelected(bool state)
{
  Q_UNUSED(state);

  auto action = qobject_cast<QAction *>(sender());

  if (!action)
    return;

  br.filename->setText(action->data().toString());
  br.filename->setCursorPosition(0);

  for (int i = 0; i < br.branch_name->count(); i++)
  {
    if (m_branches.contains(br.branch_name->itemText(i)))
      if (m_branches[br.branch_name->itemText(i)].value("database_type") ==
          "sqlite")
      {
        br.branch_name->setCurrentIndex(i);
        break;
      }
  }

  slotConnectDB();
  slotRefresh();
}

void biblioteq::slotUpdateIndicesAfterSort(int column)
{
  auto order = Qt::AscendingOrder;

  QApplication::setOverrideCursor(Qt::WaitCursor);

  if (ui.table->horizontalHeader()->sortIndicatorOrder() != Qt::AscendingOrder)
    order = Qt::DescendingOrder;
  else
    order = Qt::AscendingOrder;

  ui.table->horizontalHeader()->setSortIndicator(column, order);
  ui.table->sortByColumn(column, order);
  QApplication::restoreOverrideCursor();
}

void biblioteq::slotViewDetails(void)
{
  QString oid = "";
  QString type = "";
  auto error = false;
  auto list(ui.table->selectionModel()->selectedRows());
  auto table = ui.table;
  biblioteq_book *book = nullptr;
  biblioteq_photographcollection *photograph = nullptr;
  int i = 0;

  if (list.isEmpty())
  {
    QMessageBox::critical(this, tr("BiblioteQ: User Error"),
                          tr("Please select at least one item to view."));
    QApplication::processEvents();
    return;
  }
  else if (list.size() >= MAXIMUM_DEVICES_CONFIRMATION)
  {
    if (QMessageBox::question(this, tr("BiblioteQ: Question"),
                              tr("Are you sure that you wish to view the ") +
                                  QString::number(list.size()) +
                                  tr(" selected items? BiblioteQ will exit if "
                                     "it's unable "
                                     "to acquire resources."),
                              QMessageBox::Yes | QMessageBox::No,
                              QMessageBox::No) == QMessageBox::No)
    {
      QApplication::processEvents();
      list.clear();
      return;
    }

    QApplication::processEvents();
  }

  QApplication::setOverrideCursor(Qt::WaitCursor);
  std::stable_sort(list.begin(), list.end());

  foreach (const auto &index, list)
  {
    i = index.row();
    oid = biblioteq_misc_functions::getColumnString(table, i, table->columnNumber("MYOID"));
    type = biblioteq_misc_functions::getColumnString(table, i, table->columnNumber("Type"));
    book = nullptr;
    photograph = nullptr;

    if (type.toLower() == "book")
    {
      foreach (auto w, QApplication::topLevelWidgets())
      {
        auto b = qobject_cast<biblioteq_book *>(w);

        if (b && b->getID() == oid)
        {
          book = b;
          break;
        }
      }

      if (!book)
        book = new biblioteq_book(this, oid, index);

      book->modify(VIEW_ONLY);
    }
    else if (type.toLower() == "photograph collection")
    {
      foreach (auto w, QApplication::topLevelWidgets())
      {
        auto p = qobject_cast<biblioteq_photographcollection *>(w);

        if (p && p->getID() == oid)
        {
          photograph = p;
          break;
        }
      }

      if (!photograph)
        photograph = new biblioteq_photographcollection(this, oid, index);

      photograph->modify(VIEW_ONLY);
    }
    else
    {
      error = true;
      break;
    }
  }

  list.clear();
  QApplication::restoreOverrideCursor();

  if (error)
  {
    QMessageBox::critical(this,
                          tr("BiblioteQ: Error"),
                          tr("Unable to determine the selected item's type."));
    QApplication::processEvents();
  }
}

void biblioteq::updateItemWindows(void)
{
  QApplication::setOverrideCursor(Qt::WaitCursor);

  foreach (auto w, QApplication::topLevelWidgets())
  {
    auto book = qobject_cast<biblioteq_book *>(w);
    auto photograph = qobject_cast<biblioteq_photographcollection *>(w);

    if (book)
      book->updateWindow(EDITABLE);

    if (photograph)
      photograph->updateWindow(EDITABLE);
  }

  QApplication::restoreOverrideCursor();
}

void biblioteq::updateRows(const QString &oid, const QTableWidgetItem *item, const QString &it)
{
  auto index = ui.table->indexFromItem(item);
  auto itemType(it.toLower().remove(" ").trimmed());

  if (itemType == "book")
  {
    foreach (auto w, QApplication::topLevelWidgets())
    {
      auto book = qobject_cast<biblioteq_book *>(w);

      if (book && book->getID() == oid)
      {
        book->updateRow(index);
        break;
      }
    }
  }
  else if (itemType == "photographcollection")
  {
    foreach (auto w, QApplication::topLevelWidgets())
    {
      auto photograph = qobject_cast<biblioteq_photographcollection *>(w);

      if (photograph && photograph->getID() == oid)
      {
        photograph->updateRow(index);
        break;
      }
    }
  }
}

void biblioteq::updateSceneItem(const QString &oid,
                                const QString &type,
                                const QImage &image)
{
  QGraphicsPixmapItem *item = nullptr;
  auto items(ui.graphicsView->scene()->items());

  for (int i = 0; i < items.size(); i++)
    if ((item = qgraphicsitem_cast<QGraphicsPixmapItem *>(items.at(i))))
      if (oid == item->data(0).toString() && type == item->data(1).toString())
      {
        auto l_image(image);

        if (!l_image.isNull())
          l_image = l_image.scaled(126, 187, Qt::KeepAspectRatio, Qt::SmoothTransformation);

        auto pixmap(QPixmap::fromImage(l_image));

        if (!pixmap.isNull())
          item->setPixmap(pixmap);
        else
        {
          QImage l_image(":/no_image.png");

          if (!l_image.isNull())
            l_image = l_image.scaled(126, 187, Qt::KeepAspectRatio, Qt::SmoothTransformation);

          item->setPixmap(QPixmap::fromImage(l_image));
        }

        break;
      }
}
