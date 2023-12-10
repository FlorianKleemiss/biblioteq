#ifndef _BIBLIOTEQ_H_
#define _BIBLIOTEQ_H_

#define BIBLIOTEQ_GUEST_ACCOUNT "xbook_guest"
#define BIBLIOTEQ_VERSION "2022.03.30"

#include "biblioteq_book.h"
#include "biblioteq_dbenumerations.h"
#include "biblioteq_generic_thread.h"
#include "biblioteq_import.h"
#include "biblioteq_misc_functions.h"
#include "biblioteq_photographcollection.h"
#include "ui_biblioteq_branch_s.h"
#include "ui_biblioteq_customquery.h"
#include "ui_biblioteq_errordiag.h"
#include "ui_biblioteq_mainwindow.h"

#include <QMessageBox>

class biblioteq_documentationwindow;
class biblioteq_files;
class biblioteq_otheroptions;
class biblioteq_sqlite_merge_databases;

class biblioteq : public QMainWindow
{
  Q_OBJECT

public:
  enum Limits
  {
    QUANTITY = 1000 // Copies per item.
  };

  static QString s_locale;
  static QString s_unknown;
  static QTranslator *s_appTranslator;
  static QTranslator *s_qtTranslator;
  static const int CUSTOM_QUERY = 0;
  static const int EDITABLE = 0;
  static const int MAXIMUM_DEVICES_CONFIRMATION = 5;
  static const int NEW_PAGE = 0;
  static const int NEXT_PAGE = 1;
  static const int POPULATE_ALL = 1;
  static const int POPULATE_SEARCH = 2;
  static const int POPULATE_SEARCH_BASIC = 3;
  static const int PREVIOUS_PAGE = 2;
  static const int VIEW_ONLY = 1;
  biblioteq(void);
  ~biblioteq();
  QHash<QAction *, QPointer<biblioteq_documentationwindow>> m_documentation;
  QHash<QAction *, QPointer<biblioteq_documentationwindow>> m_releaseNotes;
  QPointer<QMenu> m_configToolMenu;
  QPointer<biblioteq_documentationwindow> m_contributors;
  QString m_unaccent;
  QColor availabilityColor(const QString &itemType) const;
  QHash<QString, QString> getAmazonHash(void) const;
  QHash<QString, QString> getOpenLibraryImagesHash(void) const;
  QHash<QString, QString> getOpenLibraryItemsHash(void) const;
  QHash<QString, QString> getSRUHash(const QString &name) const;
  QHash<QString, QString> getZ3950Hash(const QString &name) const;
  QSqlDatabase getDB(void) const;
  QString formattedISBN10(const QString &str) const;
  QString formattedISBN13(const QString &str) const;
  QString getPreferredSRUSite(void) const;
  QString getPreferredZ3950Site(void) const;
  QString getTypeFilterString(void) const;
  QString publicationDateFormat(const QString &itemType) const;
  QString unaccent(void) const;
  QStringList getSRUNames(void) const;
  QStringList getZ3950Names(void) const;
  QVariant setting(const QString &name) const;
  Ui_mainWindow getUI(void) const;
  bool availabilityColors(void) const;
  bool showBookReadStatus(void) const;
  bool showMainTableImages(void) const;
  int pageLimit(void) const;
  int populateTable(QSqlQuery &query,
                    const QString &typefilter,
                    const int pagingType,
                    const int searchType);
  int populateTable(const int search_type_arg,
                    const QString &typefilter,
                    const QString &searchstrArg,
                    const int pagingType = NEW_PAGE);
  static QString homePath(void);
  static void quit(const char *msg, const char *file, const int line);
  static void quit(void);
  void addError(const QString &type,
                const QString &summary,
                const QString &error = "",
                const char *file = "",
                const int line = 0);
  void bookSearch(const QString &field, const QString &value);
  void pcSearch(const QString &field, const QString &value);
  void removeBook(biblioteq_book *book);
  void removePhotographCollection(biblioteq_photographcollection *pc);
  void replaceBook(const QString &id, biblioteq_book *book);
  void replacePhotographCollection(const QString &id,
                                   biblioteq_photographcollection *photograph);
  void setGlobalFonts(const QFont &font);
  void setSummaryImages(const QImage &back, const QImage &front);
  void showMain(void);
  void updateItemWindows(void);
  void updateRows(const QString &oid, const QTableWidgetItem *item, const QString &it);
  QRect get_Screensize(void);
  void set_Screensize(const QRect &size);
  void updateSceneItem(const QString &oid,
                       const QString &type,
                       const QImage &image);

public slots:
  void slotDisplaySummary(void);
  void slotResizeColumns(void);
  void slotRefresh(void);

private:
  enum AdminSetupColumns
  {
    ADMINISTRATOR = 1,
    CIRCULATION = 2,
    ID = 0,
    LIBRARIAN = 3,
    MEMBERSHIP = 4
  };

  enum ErrorDialogColumns
  {
    EVENT_TIME = 0,
    EVENT_TYPE = 1,
    FILE = 4,
    FULL_DESCRIPTION = 3,
    LINE_NUMBER = 5,
    SUMMARY = 2
  };

  enum GenericSearchTypes
  {
    CATEGORY_GENERIC_SEARCH_TYPE = 1,
    ID_GENERIC_SEARCH_TYPE = 2,
    KEYWORD_GENERIC_SEARCH_TYPE = 3,
    TITLE_GENERIC_SEARCH_TYPE = 4
  };

  enum HistoryColumns
  {
    BARCODE = 5,
    DUE_DATE = 8,
    FIRST_NAME = 1,
    ID_NUMBER = 4,
    LAST_NAME = 2,
    LENDER = 10,
    MEMBER_ID = 0,
    MYOID = 11,
    RESERVATION_DATE = 7,
    RETURNED_DATE = 9,
    TITLE = 3,
    TYPE = 6
  };

  enum RequestActionItems
  {
    CANCEL_REQUESTED = 0,
    INACTIVE,
    REQUEST_SELECTED,
    RETURN_RESERVED
  };

  QActionGroup *m_menuCategoryActionGroup;
  QDialog *m_branch_diag;
  QDialog *m_pass_diag;
  QHash<QString, QString> m_amazonImages;
  QHash<QString, QString> m_openLibraryImages;
  QHash<QString, QString> m_openLibraryItems;
  QHash<QString, QString> m_selectedBranch;
  QLabel *m_connected_bar_label;
  QLabel *m_status_bar_label;
  QMainWindow *m_customquery_diag;
  QMainWindow *m_error_diag;
  QRect m_screensize;
  QMap<QString, QHash<QString, QString>> m_branches;
  QMap<QString, QHash<QString, QString>> m_sruMaps;
  QMultiMap<QString, QHash<QString, QString>> m_z3950Maps;
  QPointer<QMenu> m_menu;
  QPointer<biblioteq_dbenumerations> db_enumerations;
  QPointer<biblioteq_sqlite_merge_databases> m_sqliteMergeDatabases;
  QSqlDatabase m_db;
  QSqlQuery m_searchQuery;
  QString m_engUserinfoTitle;
  QString m_lastCategory;
  QString m_lastSearchStr;
  QString m_previousTypeFilter;
  QString m_roles;
  QTextBrowser *m_printPreview;
  QToolButton *m_error_bar_label;
  QVector<QString> m_abColumnHeaderIndexes;
  QVector<QString> m_bbColumnHeaderIndexes;
  QVector<QString> m_historyColumnHeaderIndexes;
  Ui_branchSelect br;
  Ui_customquery cq;
  Ui_errordialog er;
  Ui_mainWindow ui;
  biblioteq_files *m_files;
  biblioteq_import *m_import;
  biblioteq_otheroptions *m_otheroptions;
  bool m_allSearchShown;
  int m_lastSearchType;
  qint64 m_pages;
  qint64 m_queryOffset;
  quint64 m_idCt;
  QString dbUserName(void) const;
  QString reservationHistoryHtml(void) const;
  QString viewHtml(void) const;
  QWidget *widgetForAction(QAction *action) const;
  void adminSetup(void);
  bool emptyContainers(void);
  bool isCurrentItemAPhotograph(void) const;
  void addConfigOptions(const QString &typefilter);
  void changeEvent(QEvent *event);
  void cleanup(void);
  void closeEvent(QCloseEvent *event);
  void createConfigToolMenu(void);
  void createSqliteMenuActions(void);
  void deleteItem(const QString &oid, const QString &itemType);
  void exportAsCSV(biblioteq_main_table *table, const QString &title);
  void initialUpdate(void);
  void prepareContextMenus(void);
  void prepareFilter(void);
  void preparePhotographsPerPageMenu(void);
  void prepareUpgradeNotification(void);
  void readConfig(void);
  void readGlobalSetup(void);

private slots:
  void slotAbout(void);
  void slotAllGo(void);
  void slotAutoPopOnFilter(QAction *action);
  void slotBookSearch(void);
  void slotBranchChanged(void);
  void slotChangeView(bool checked);
  void slotClearSqliteMenu(bool state);
  void slotCloseCustomQueryDialog(void);
  void slotConnectDB(void);
  void slotContextMenu(const QPoint &point);
  void slotContributors(void);
  void slotCopyError(void);
  void slotDelete(void);
  void slotDisconnect(void);
  void slotDisplayNewSqliteDialog(void);
  void slotDuplicate(void);
  void slotExecuteCustomQuery(void);
  void slotExit(void);
  void slotExportAsCSV(void);
  void slotExportAsPNG(void);
  void slotGraphicsSceneEnterKeyPressed(void);
  void slotInsertBook(void);
  void slotInsertPhotograph(void);
  void slotItemChanged(QTableWidgetItem *item);
  void slotLanguageChanged(void);
  void slotLastWindowClosed(void);
  void slotMainTableEnterKeyPressed(void);
  void slotMainWindowCanvasBackgroundColorChanged(const QColor &color);
  void slotMergeSQLiteDatabases(void);
  void slotModify(void);
  void slotNextPage(void);
  void slotOpenOnlineDocumentation(void);
  void slotOpenPDFFiles(void);
  void slotOtherOptionsSaved(void);
  void slotPageClicked(const QString &link);
  void slotPhotographSearch(void);
  void slotPhotographsPerPageChanged(void);
  void slotPreviewCanvasBackgroundColor(const QColor &color);
  void slotPreviousPage(void);
  void slotPrintIconsView(void);
  void slotPrintPreview(QPrinter *printer);
  void slotPrintView(void);
  void slotPrintViewPreview(void);
  void slotRefreshCustomQuery(void);
  void slotReloadBiblioteqConf(void);
  void slotResetAllSearch(void);
  void slotResetErrorLog(void);
  void slotResetLoginDialog(void);
  void slotResizeColumnsAfterSort(void);
  void slotSaveConfig(void);
  void slotSaveGeneralSearchCaseSensitivity(bool state);
  void slotSceneSelectionChanged(void);
  void slotSearch(void);
  void slotSearchBasic(void);
  void slotSectionResized(int logicalIndex, int oldSize, int newSize);
  void slotSelectDatabaseFile(void);
  void slotSetColumns(void);
  void slotSetFonts(void);
  void slotShowConnectionDB(void);
  void slotShowCustomQuery(void);
  void slotShowDbEnumerations(void);
  void slotShowDocumentation(void);
  void slotShowErrorDialog(void);
  void slotShowFiles(void);
  void slotShowGrid(void);
  void slotShowImport(void);
  void slotShowMenu(void);
  void slotShowOtherOptions(void);
  void slotShowReleaseNotes(void);
  void slotSqliteFileSelected(bool state);
  void slotUpdateIndicesAfterSort(int column);
  void slotUpgradeSqliteScheme(void);
  void slotVacuum(void);
  void slotViewDetails(void);
  void slotViewFullOrNormalScreen(void);

signals:
  void fontChanged(const QFont &font);
};

#endif
