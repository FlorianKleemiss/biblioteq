#include "biblioteq.h"
#include "biblioteq_main_table.h"

#include <QScrollBar>
#include <QSettings>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>

class NumericSortModel : public QSortFilterProxyModel
{
protected:
  bool lessThan(const QModelIndex &left, const QModelIndex &right) const override
  {
    bool ok1, ok2;
    int leftNumber = sourceModel()->data(left).toInt(&ok1);
    int rightNumber = sourceModel()->data(right).toInt(&ok2);

    if (ok1 && ok2)
      return leftNumber < rightNumber;
    else
      return QSortFilterProxyModel::lessThan(left, right);
  }
};

biblioteq_main_table::biblioteq_main_table(QWidget *parent) : QTableWidget(parent)
{
  QStandardItemModel *model = new QStandardItemModel;
  NumericSortModel *proxyModel = new NumericSortModel;
  proxyModel->setSourceModel(model);
  m_qmain = nullptr;
  setAcceptDrops(false);
  setDragEnabled(false);
  horizontalHeader()->setSectionsMovable(true);
  horizontalHeader()->setSortIndicator(1, Qt::AscendingOrder);
  horizontalHeader()->setSortIndicatorShown(true);
  horizontalHeader()->setStretchLastSection(true);
  verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
}

QHash<QString, QString> biblioteq_main_table::friendlyStates(void) const
{
  QHash<QString, QString> states;

  for (int i = 0; i < m_hiddenColumns.keys().size(); i++)
  {
    QString state("");

    for (int j = 0; j < m_hiddenColumns.value(m_hiddenColumns.keys().at(i)).size(); j++)
      state += QString::number(m_hiddenColumns.value(m_hiddenColumns.keys().at(i)).at(j)).append(",");

    if (state.endsWith(","))
      state = state.mid(0, state.length() - 1);

    states[m_hiddenColumns.keys().at(i)] = state;
  }

  return states;
}

QStringList biblioteq_main_table::columnNames(void) const
{
  return m_columnHeaderIndexes.toList();
}

bool biblioteq_main_table::isColumnHidden(const int index, const QString &type, const QString &username) const
{
  QString indexstr("");
  auto l_type(type);

  indexstr.append(username);
  indexstr.append(l_type.replace(" ", "_"));
  indexstr.append("_header_state");
  return m_hiddenColumns.value(indexstr).contains(index);
}

bool biblioteq_main_table::isColumnHidden(int index) const
{
  return QTableWidget::isColumnHidden(index);
}

int biblioteq_main_table::columnNumber(const QString &name) const
{
  auto index = m_columnHeaderIndexes.indexOf(name);

  if (index >= 0)
    return index;

  for (int i = 0; i < m_columnHeaderIndexes.size(); i++)
    if (m_columnHeaderIndexes.at(i).toLower() == name.toLower())
    {
      index = i;
      break;
    }

  return index;
}

void biblioteq_main_table::keyPressEvent(QKeyEvent *event)
{
  if (event)
    switch (event->key())
    {
    case Qt::Key_Enter:
    case Qt::Key_Return:
    {
      emit enterKeyPressed();
      break;
    }
    default:
    {
      break;
    }
    }

  QTableWidget::keyPressEvent(event);
}

void biblioteq_main_table::parseStates(const QHash<QString, QString> &states)
{
  m_hiddenColumns.clear();

  for (int i = 0; i < states.keys().size(); i++)
  {
    QList<int> intList;
    auto strList(states[states.keys().at(i)].split(",",
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
                                                   Qt::SkipEmptyParts
#else
                                                   QString::SkipEmptyParts
#endif
                                                   ));

    for (int j = 0; j < strList.size(); j++)
      intList.append(qBound(0, strList.at(j).toInt(), strList.size()));

    m_hiddenColumns[states.keys().at(i)] = intList;
  }
}

void biblioteq_main_table::recordColumnHidden(const QString &username,
                                              const QString &type,
                                              const int index,
                                              const bool hidden)
{
  QString indexstr("");
  auto l_type(type);

  indexstr.append(username);
  indexstr.append(l_type.replace(" ", "_"));
  indexstr.append("_header_state");

  if (hidden)
  {
    if (!m_hiddenColumns[indexstr].contains(index))
      m_hiddenColumns[indexstr].append(index);
  }
  else if (m_hiddenColumns.contains(indexstr))
    m_hiddenColumns[indexstr].removeAll(index);
}

void biblioteq_main_table::resetTable(const QString &username,
                                      const QString &type)
{
  setColumnCount(0);
  setRowCount(0);
  scrollToTop();
  horizontalScrollBar()->setValue(0);
  setColumns(username, type);

  if (m_qmain && m_qmain->setting("automatically_resize_column_widths").toBool())
  {
    for (int i = 0; i < columnCount() - 1; i++)
      resizeColumnToContents(i);

    horizontalHeader()->setStretchLastSection(true);
  }

  clearSelection();
  horizontalHeader()->setSortIndicator(1, Qt::AscendingOrder);
  horizontalHeader()->setSortIndicatorShown(true);
  setCurrentItem(nullptr);
  sortByColumn(0, Qt::AscendingOrder);
}

void biblioteq_main_table::setColumnNames(const QStringList &list)
{
  m_columnHeaderIndexes.clear();

  for (int i = 0; i < list.size(); i++)
    m_columnHeaderIndexes.append(list.at(i));
}

void biblioteq_main_table::setColumns(const QString &username,
                                      const QString &t)
{
  QStringList list;
  auto type(t.trimmed());

  m_columnHeaderIndexes.clear();

  if (type.isEmpty())
    type = "All";

  if (type == "All")
  {

    list.append(tr("Title"));
    list.append(tr("ID Number"));
    list.append(tr("Publisher"));
    list.append(tr("Publication Date"));
    list.append(tr("Categories"));
    list.append(tr("Quantity"));
    list.append(tr("Type"));
    list.append("MYOID");
    m_columnHeaderIndexes.append("Title");
    m_columnHeaderIndexes.append("ID Number");
    m_columnHeaderIndexes.append("Publisher");
    m_columnHeaderIndexes.append("Publication Date");
    m_columnHeaderIndexes.append("Categories");
    m_columnHeaderIndexes.append("Quantity");
    m_columnHeaderIndexes.append("Type");
    m_columnHeaderIndexes.append("MYOID");
  }
  else if (type == "Books")
  {
    list.append(tr("Title"));
    list.append(tr("ISBN-10"));
    list.append(tr("Authors"));
    list.append(tr("Publisher"));
    list.append(tr("Publication Date"));
    list.append(tr("Place of Publication"));
    list.append(tr("Edition"));
    list.append(tr("Categories"));
    list.append(tr("Quantity"));
    list.append(tr("Book Binding Type"));
    list.append(tr("ISBN-13"));
    list.append(tr("LC Control Number"));
    list.append(tr("Call Number"));
    list.append(tr("Dewey Class Number"));
    list.append(tr("Availability"));
    list.append(tr("Total Reserved"));
    list.append(tr("Originality"));
    list.append(tr("Condition"));
    list.append(tr("Type"));
    list.append("MYOID");
    m_columnHeaderIndexes.append("Title");
    m_columnHeaderIndexes.append("ISBN-10");
    m_columnHeaderIndexes.append("Authors");
    m_columnHeaderIndexes.append("Publisher");
    m_columnHeaderIndexes.append("Publication Date");
    m_columnHeaderIndexes.append("Place of Publication");
    m_columnHeaderIndexes.append("Edition");
    m_columnHeaderIndexes.append("Categories");
    m_columnHeaderIndexes.append("Quantity");
    m_columnHeaderIndexes.append("Book Binding Type");
    m_columnHeaderIndexes.append("ISBN-13");
    m_columnHeaderIndexes.append("LC Control Number");
    m_columnHeaderIndexes.append("Call Number");
    m_columnHeaderIndexes.append("Dewey Class Number");
    m_columnHeaderIndexes.append("Availability");
    m_columnHeaderIndexes.append("Total Reserved");
    m_columnHeaderIndexes.append("Originality");
    m_columnHeaderIndexes.append("Condition");
    m_columnHeaderIndexes.append("Type");
    m_columnHeaderIndexes.append("MYOID");
  }
  else if (type == "Photograph Collections")
  {
    list.append(tr("Title"));
    list.append(tr("ID"));
    list.append(tr("Creation Date"));
    list.append(tr("Total Size"));
    list.append(tr("Element Count"));
    list.append(tr("Element Count Strixner"));
    list.append(tr("About"));
    list.append(tr("Type"));
    list.append("MYOID");
    m_columnHeaderIndexes.append("Title");
    m_columnHeaderIndexes.append("ID");
    m_columnHeaderIndexes.append("Creation date");
    m_columnHeaderIndexes.append("Total Size");
    m_columnHeaderIndexes.append("Element Count");
    m_columnHeaderIndexes.append("Element Count Strixner");
    m_columnHeaderIndexes.append("About");
    m_columnHeaderIndexes.append("Type");
    m_columnHeaderIndexes.append("MYOID");
  }

  if (m_qmain &&
      m_qmain->getDB().driverName() == "QSQLITE" &&
      m_qmain->showBookReadStatus() &&
      type == "Books")
  {
    list.prepend(tr("Read Status"));
    m_columnHeaderIndexes.prepend("Read Status");
  }

  setColumnCount(list.size());
  setHorizontalHeaderLabels(list);

  if (type != "All" &&
      type != "Custom")
  {
    /*
    ** Hide the Type and MYOID columns.
    */

    setColumnHidden(list.size() - 1, true);
    setColumnHidden(list.size() - 2, true);
  }
  else if (type != "Custom")
  {
    /*
    ** Hide the MYOID and REQUESTOID columns.
    */

    setColumnHidden(list.size() - 1, true);
  }

  list.clear();

  QString indexstr("");
  auto l_type(type);

  indexstr.append(username);
  indexstr.append(l_type.replace(" ", "_"));
  indexstr.append("_header_state");

  for (int i = 0; i < m_hiddenColumns[indexstr].size(); i++)
    setColumnHidden(m_hiddenColumns[indexstr][i], true);
}

void biblioteq_main_table::setQMain(biblioteq *biblioteq)
{
  m_qmain = biblioteq;
}

void biblioteq_main_table::updateToolTips(const int row)
{
  if (row < 0)
    return;

  QSettings settings;
  auto showToolTips = settings.value("show_maintable_tooltips", false).toBool();

  if (!showToolTips)
    return;

  QString tooltip("<html>");

  for (int i = 0; i < columnCount(); i++)
  {
    auto columnName(columnNames().value(i));
    auto item = this->item(row, i);

    if (columnName.isEmpty())
      columnName = "N/A";

    tooltip.append("<b>");
    tooltip.append(columnName);
    tooltip.append(":</b> ");

    if (item)
      tooltip.append(item->text().trimmed());
    else
      tooltip.append("");

    tooltip.append("<br>");
  }

  if (tooltip.endsWith("<br>"))
    tooltip = tooltip.mid(0, tooltip.length() - 4);

  tooltip.append("</html>");

  for (int i = 0; i < columnCount(); i++)
  {
    auto item = this->item(row, i);

    if (item)
      item->setToolTip(tooltip);
  }
}
