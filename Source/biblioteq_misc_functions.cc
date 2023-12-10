#include "biblioteq.h"
#include "biblioteq_misc_functions.h"

#include <QDate>
#include <QDir>
#include <QProgressDialog>

QImage biblioteq_misc_functions::getImage(const QString &oid,
                                          const QString &which,
                                          const QString &typeArg,
                                          const QSqlDatabase &db)
{
  QSqlQuery query(db);
  auto image = QImage();
  auto type(typeArg.toLower());

  if (type == "photograph collection")
    type = type.replace(" ", "_");
  else
    type = type.remove(" ");

  if (type == "book" ||
      type == "photograph_collection")
  {
    if (which == "back_cover" ||
        which == "front_cover" ||
        which == "image_scaled")
    {
      query.prepare(QString("SELECT %1 FROM %2 WHERE myoid = ?").arg(which).arg(type));
      query.bindValue(0, oid);
    }
    else
      return image;
  }
  else
    return image;

  if (query.exec())
    if (query.next())
    {
      image.loadFromData(QByteArray::fromBase64(query.value(0).toByteArray()));

      if (image.isNull())
        image.loadFromData(query.value(0).toByteArray());
    }

  return image;
}

QList<int> biblioteq_misc_functions::selectedRows(QTableWidget *table)
{
  QList<int> rows;

  if (!table)
    return rows;

  auto indexes(table->selectionModel()->selectedRows(0));

  for (int i = 0; i < indexes.size(); i++)
    rows << indexes.at(i).row();

  std::sort(rows.begin(), rows.end());
  return rows;
}

QString biblioteq_misc_functions::getAbstractInfo(const QString &oid,
                                                  const QString &typeArg,
                                                  const QSqlDatabase &db)
{
  QSqlQuery query(db);
  QString querystr = "";
  QString str = "";
  auto type(typeArg.toLower());

  if (type == "book")
  {
    type = type.remove(" ");
    querystr = QString("SELECT description FROM %1 WHERE myoid = ?").arg(type);
  }
  else if (type == "photograph collection")
  {
    type = type.replace(" ", "_");
    querystr = QString("SELECT about FROM %1 WHERE myoid = ?").arg(type);
  }
  else
    return str;

  query.prepare(querystr);
  query.bindValue(0, oid);

  if (query.exec())
    if (query.next())
      str = query.value(0).toString().trimmed();

  return str;
}

QString biblioteq_misc_functions::getColumnString(const QTableWidget *table,
                                                  const int row,
                                                  const QString &columnName)
{
  if (columnName.isEmpty() || row < 0 || !table)
    return QString("");

  QString str = "";
  QTableWidgetItem *column = nullptr;
  int i = 0;

  if (row >= 0 && row < table->rowCount())
    for (i = 0; i < table->columnCount(); i++)
    {
      column = table->horizontalHeaderItem(i);

      if (column == nullptr || table->item(row, i) == nullptr)
        continue;

      if (column->text().toLower() == columnName.toLower())
      {
        str = table->item(row, i)->text();
        break;
      }
    }

  return str;
}

QString biblioteq_misc_functions::getColumnString(const QTableWidget *table,
                                                  const int row,
                                                  const int column)
{
  if (column < 0 || row < 0 || !table)
    return QString("");

  QTableWidgetItem *item = nullptr;

  if ((item = table->item(row, column)))
    return item->text();
  else
    return QString("");
}

QString biblioteq_misc_functions::getOID(const QString &idArg,
                                         const QString &itemTypeArg,
                                         const QSqlDatabase &db,
                                         QString &errorstr)
{
  QSqlQuery query(db);
  QString id = "";
  QString itemType = "";
  QString oid = "";
  QString querystr = "";

  id = idArg;
  itemType = itemTypeArg.toLower();

  if (itemType == "photograph collection")
    itemType = itemType.replace(" ", "_");
  else
    itemType = itemType.remove(" ");

  if (itemType == "book")
    querystr = QString("SELECT myoid FROM %1 WHERE id = ? AND "
                       "id IS NOT NULL")
                   .arg(itemType);
  else if (itemType == "photograph_collection")
    querystr = QString("SELECT myoid FROM %1 WHERE id = ?").arg(itemType);
  else
    return oid;

  query.prepare(querystr);
  query.bindValue(0, id);

  if (query.exec())
    if (query.next())
      oid = query.value(0).toString().trimmed();

  if (query.lastError().isValid())
    errorstr = query.lastError().text();

  return oid;
}

QString biblioteq_misc_functions::imageFormatGuess(const QByteArray &bytes)
{
  QString format("");

  if (bytes.size() >= 4 &&
      tolower(bytes[1]) == 'p' &&
      tolower(bytes[2]) == 'n' &&
      tolower(bytes[3]) == 'g')
    format = "PNG";
  else if (bytes.size() >= 10 &&
           tolower(bytes[6]) == 'j' && tolower(bytes[7]) == 'f' &&
           tolower(bytes[8]) == 'i' && tolower(bytes[9]) == 'f')
    format = "JPG";
  else if (bytes.size() >= 2 &&
           tolower(bytes[0]) == 'b' &&
           tolower(bytes[1]) == 'm')
    format = "BMP";
  else // Guess!
    format = "JPG";

  return format;
}

QString biblioteq_misc_functions::isbn10to13(const QString &text)
{
  if (QString(text).remove('-').length() != 10)
    return text;

  QList<int> array;
  QString numberstr("");
  QString str("978" + QString(text).remove('-').trimmed().left(9));
  int check = 0;
  int total = 0;

  array << 1 << 3 << 1 << 3 << 1 << 3 << 1 << 3 << 1 << 3 << 1 << 3;

  for (int i = 0; i < array.size(); i++)
    if (i < str.length())
      total += array.at(i) * str.at(i).digitValue();
    else
      break;

  check = 10 - (total % 10);

  if (check == 10)
    check = 0;

  numberstr.setNum(check);
  return str.append(numberstr);
}

QString biblioteq_misc_functions::isbn13to10(const QString &text)
{
  if (QString(text).remove('-').length() != 13)
    return text;

  QString z("");
  auto str(QString(text).remove('-').trimmed().mid(3, 9));
  int total = 0;

  for (int i = 0; i < 9; i++)
    if (i < str.length())
      total += str[i].digitValue() * (10 - i);
    else
      break;

  z = QString::number((11 - (total % 11)) % 11);

  if (z == "10")
    z = "X";

  return str + z;
}

QStringList biblioteq_misc_functions::getBookBindingTypes(const QSqlDatabase &db, QString &errorstr)
{
  QSqlQuery query(db);
  QString querystr("");
  QStringList types;

  errorstr = "";
  querystr = "SELECT binding_type FROM book_binding_types "
             "WHERE LENGTH(TRIM(binding_type)) > 0 "
             "ORDER BY binding_type";

  if (query.exec(querystr))
    while (query.next())
      types.append(query.value(0).toString().trimmed());

  if (query.lastError().isValid())
    errorstr = query.lastError().text();

  return types;
}

bool biblioteq_misc_functions::isGnome(void)
{
  auto session(qgetenv("DESKTOP_SESSION").toLower().trimmed());

  if (session == "gnome" || session == "ubuntu")
    return true;
  else
    return false;
}

int biblioteq_misc_functions::sqliteQuerySize(const QString &querystr,
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
                                              const QMap<QString, QVariant> &boundValues,
#else
                                              const QVariantList &boundValues,
#endif
                                              const QSqlDatabase &db,
                                              const char *file,
                                              const int line,
                                              biblioteq *qmain)
{
  int count = 0;

  if (querystr.trimmed().isEmpty())
    return count;

  QSqlQuery query(db);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
  auto list(boundValues.values());
#else
  auto list(boundValues);
#endif

  query.prepare(querystr);

  for (int i = 0; i < list.size(); i++)
    query.bindValue(i, list.at(i));

  if (query.exec())
    while (query.next())
      count += 1;

  if (query.lastError().isValid())
    if (qmain)
      qmain->addError(QString(QObject::tr("Database Error")),
                      QString(QObject::tr("Unable to determine the query size.")),
                      query.lastError().text(), file, line);

  return count;
}

int biblioteq_misc_functions::sqliteQuerySize(const QString &querystr,
                                              const QSqlDatabase &db,
                                              const char *file,
                                              const int line,
                                              biblioteq *qmain)
{
  int count = 0;

  if (querystr.trimmed().isEmpty())
    return count;

  QSqlQuery query(db);

  if (query.exec(querystr))
    while (query.next())
      count += 1;

  if (query.lastError().isValid())
    if (qmain)
      qmain->addError(QString(QObject::tr("Database Error")),
                      QString(QObject::tr("Unable to determine the query size.")),
                      query.lastError().text(), file, line);

  return count;
}

qint64 biblioteq_misc_functions::getSqliteUniqueId(const QSqlDatabase &db,
                                                   QString &errorstr)
{
  qint64 value = -1;

  QSqlQuery query(db);

  errorstr = "";

  if (query.exec("INSERT INTO sequence VALUES (NULL)"))
  {
    auto variant(query.lastInsertId());

    if (variant.isValid())
    {
      value = variant.toLongLong();

      /*
      ** Store only one entry in the table.
      */

      query.exec(QString("DELETE FROM sequence WHERE value < %1").arg(value));
    }
    else
      errorstr = "Invalid variant.";
  }
  else if (query.lastError().isValid())
    errorstr = query.lastError().text();
  else
    errorstr = "Query failure.";

  return value;
}

void biblioteq_misc_functions::center(QWidget *child, QMainWindow *parent)
{
  if (!child || !parent)
    return;

  QPoint p(0, 0);
  int X = 0;
  int Y = 0;

  p = parent->pos();

  if (parent->width() >= child->width())
    X = p.x() + (parent->width() - child->width()) / 2;
  else
    X = p.x() - (child->width() - parent->width()) / 2;

  if (parent->height() >= child->height())
    Y = p.y() + (parent->height() - child->height()) / 2;
  else
    Y = p.y() - (child->height() - parent->height()) / 2;

  child->move(X, Y);
}

void biblioteq_misc_functions::createInitialCopies(const QString &idArg,
                                                   const int numCopies,
                                                   const QSqlDatabase &db,
                                                   const QString &itemTypeArg,
                                                   QString &errorstr)
{
  QSqlQuery query(db);
  QString id = "";
  QString itemType = "";
  QString itemoid = "";
  auto copies = qBound(0, numCopies, static_cast<int>(biblioteq::Limits::QUANTITY));
  int i = 0;

  /*
  ** Retrieve the item's OID. Use the OID to create initial copies.
  */

  errorstr = "";
  id = idArg;
  itemType = itemTypeArg.toLower().remove(" ");
  itemoid = getOID(id, itemType, db, errorstr);

  if (!errorstr.isEmpty())
    return;

  if (itemType == "book")
  {
    if (itemoid.isEmpty())
      /*
      ** If the id from getOID() is empty, createInitialCopies() was called
      ** with an oid.
      */

      itemoid = id;
  }

  if (!itemoid.isEmpty())
    for (i = 0; i < copies; i++)
    {

      if (itemType == "book")
        query.prepare(QString("INSERT INTO %1_copy_info "
                              "(item_oid, copy_number, "
                              "copyid, myoid) "
                              "VALUES (?, "
                              "?, ?, ?)")
                          .arg(itemType));

      query.bindValue(0, itemoid);
      query.bindValue(1, i + 1);
      query.bindValue(2, id + "-" + QString::number(i + 1));

      if (db.driverName() == "QSQLITE")
      {
        auto value = getSqliteUniqueId(db, errorstr);

        if (errorstr.isEmpty())
          query.bindValue(3, value);
        else
          break;
      }

      if (!query.exec())
      {
        errorstr = query.lastError().text();
        break;
      }
    }
}

void biblioteq_misc_functions::exportPhotographs(const QSqlDatabase &db,
                                                 const QString &collectionOid,
                                                 const QString &destinationPath,
                                                 const QList<QGraphicsItem *> &items,
                                                 QWidget *parent)
{
  QProgressDialog progress(parent);

  progress.setLabelText(QObject::tr("Exporting image(s)..."));
  progress.setMaximum(items.size());
  progress.setMinimum(0);
  progress.setModal(true);
  progress.setWindowTitle(QObject::tr("BiblioteQ: Progress Dialog"));
  progress.show();
  progress.repaint();
  QApplication::processEvents();

  QSqlQuery query(db);

  query.prepare("SELECT image FROM photograph WHERE "
                "collection_oid = ? AND image IS NOT NULL AND myoid = ?");
  query.bindValue(0, collectionOid);

  for (int i = 0; i < items.size(); i++)
  {
    auto item = items.at(i);

    if (!item)
      continue;

    if (i + 1 <= progress.maximum())
      progress.setValue(i + 1);

    query.bindValue(1, item->data(0).toString());
    progress.repaint();
    QApplication::processEvents();

    if (progress.wasCanceled())
      break;

    if (query.exec() && query.next())
    {
      QImage image;
      auto bytes(QByteArray::fromBase64(query.value(0).toByteArray()));
      auto id = QDateTime::currentMSecsSinceEpoch();
      auto format(imageFormatGuess(bytes));

      image.loadFromData(bytes, format.toLatin1().constData());

      if (!image.isNull())
        image.save(destinationPath + QDir::separator() +
                       QString("%1_%2.%3").arg(id).arg(i + 1).arg(format).toLower(),
                   format.toLatin1().constData(), 100);
    }
  }
}

void biblioteq_misc_functions::exportPhotographs(const QSqlDatabase &db,
                                                 const QString &collectionOid,
                                                 const int pageOffset,
                                                 const int photographsPerPage,
                                                 const QString &destinationPath,
                                                 QWidget *parent)
{
  QProgressDialog progress(parent);

  progress.setLabelText(QObject::tr("Exporting image(s)..."));
  progress.setMinimum(0);
  progress.setModal(true);
  progress.setWindowTitle(QObject::tr("BiblioteQ: Progress Dialog"));
  progress.show();
  progress.repaint();
  QApplication::processEvents();

  QSqlQuery query(db);

  if (pageOffset <= 0)
  {
    query.prepare("SELECT image FROM photograph WHERE "
                  "collection_oid = ? AND image IS NOT NULL");
    query.bindValue(0, collectionOid);
  }
  else
  {
    query.prepare(QString("SELECT image FROM photograph WHERE "
                          "collection_oid = ? AND image IS NOT NULL "
                          "LIMIT %1 "
                          "OFFSET %2")
                      .arg(photographsPerPage)
                      .arg(photographsPerPage * (pageOffset - 1)));
    query.bindValue(0, collectionOid);
  }

  if (query.exec())
  {
    if (db.driverName() == "QPSQL")
      progress.setMaximum(query.size());
    else
      progress.setMaximum(0);

    int i = 0;
    int j = -1;
    auto id = QDateTime::currentMSecsSinceEpoch();

    while (query.next())
    {
      if (db.driverName() == "QPSQL")
      {
        j += 1;

        if (j + 1 <= progress.maximum())
          progress.setValue(j + 1);
      }

      progress.repaint();
      QApplication::processEvents();

      if (progress.wasCanceled())
        break;

      QImage image;
      auto bytes(QByteArray::fromBase64(query.value(0).toByteArray()));
      auto format(imageFormatGuess(bytes));

      image.loadFromData(bytes, format.toLatin1().constData());

      if (!image.isNull())
      {
        i += 1;
        image.save(destinationPath + QDir::separator() +
                       QString("%1_%2.%3").arg(id).arg(i).arg(format).toLower(),
                   format.toLatin1().constData(), 100);
      }
    }
  }
}

void biblioteq_misc_functions::hideAdminFields(QMainWindow *window)
{
  if (!window)
    return;

  QString str = "";

  foreach (auto widget, window->findChildren<QWidget *>())
  {
    str = widget->objectName().toLower();
  }

  foreach (auto widget, window->findChildren<QLabel *>())
  {
    str = widget->text().toLower();
  }

  foreach (auto button, window->findChildren<QToolButton *>())
    if (button->menu())
    {
      foreach (auto action, button->menu()->findChildren<QAction *>())
      {
        str = action->text().toLower();
      }
    }
}

void biblioteq_misc_functions::highlightWidget(QWidget *widget,
                                               const QColor &color)
{
  if (!widget)
    return;

  QPalette pal;

  pal = widget->palette();
  pal.setColor(widget->backgroundRole(), color);
  widget->setPalette(pal);
}

void biblioteq_misc_functions::saveQuantity(const QSqlDatabase &db,
                                            const QString &oid,
                                            const int quantity,
                                            const QString &itemTypeArg,
                                            QString &errorstr)
{
  QSqlQuery query(db);
  QString itemType = "";
  QString querystr = "";

  errorstr = "";
  itemType = itemTypeArg.toLower().remove(" ");

  if (itemType == "book")
    querystr = QString("UPDATE %1 SET quantity = ? WHERE "
                       "myoid = ?")
                   .arg(itemType);
  else
    return;

  query.prepare(querystr);
  query.bindValue(0, quantity);
  query.bindValue(1, oid);
  (void)query.exec();

  if (query.lastError().isValid())
    errorstr = query.lastError().text();
}

void biblioteq_misc_functions::updateColumn(QTableWidget *table,
                                            const int row,
                                            int column,
                                            const QString &value)
{
  if (column < 0 || row < 0 || !table || !table->item(row, column))
    return;

  auto sortingEnabled = false;

  if (table->isSortingEnabled())
    sortingEnabled = true;

  if (sortingEnabled)
    table->setSortingEnabled(false);

  table->item(row, column)->setText(value);

  if (sortingEnabled)
    table->setSortingEnabled(true);

  if (qobject_cast<biblioteq_main_table *>(table))
    qobject_cast<biblioteq_main_table *>(table)->updateToolTips(row);
}

void biblioteq_misc_functions::updateColumnColor(QTableWidget *table,
                                                 const int row,
                                                 int column,
                                                 const QColor &color)
{
  if (column < 0 || row < 0 || !table || !table->item(row, column))
    return;

  table->item(row, column)->setBackground(color);
}
