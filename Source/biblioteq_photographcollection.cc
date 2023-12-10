#include "biblioteq.h"
#include "biblioteq_bgraphicsscene.h"
#include "biblioteq_graphicsitempixmap.h"
#include "biblioteq_photographcollection.h"
#include "ui_biblioteq_photographview.h"
#include "ui_biblioteq_photographcompare.h"
#include "biblioteq_myqstring.h"

#include <QFileDialog>
#include <QScrollBar>
#include <QShortcut>
#include <QSqlField>
#include <QSqlRecord>
#include <QUuid>
#include <QtCore/qmath.h>

#include <limits>

biblioteq_photographcollection::biblioteq_photographcollection(biblioteq *parentArg,
                                                               const QString &oidArg,
                                                               const QModelIndex &index) : QMainWindow(), biblioteq_item(index)
{
  qmain = parentArg;

  QGraphicsScene *scene1 = nullptr;
  QGraphicsScene *scene2 = nullptr;
  QGraphicsScene *scene3 = nullptr;
  // QMenu *menu1 = nullptr;
  QMenu *menu2 = nullptr;

  m_photo_diag = new QDialog(this);
  m_photo_compare_diag = new QDialog(this);
  // menu1 = new QMenu(this);
  menu2 = new QMenu(this);
  scene1 = new QGraphicsScene(this);
  scene2 = new QGraphicsScene(this);
  scene3 = new QGraphicsScene(this);
  pc.setupUi(this);
  setQMain(this);
  pc.thumbnail_item->enableDoubleClickResize(true);
  m_scene = new biblioteq_bgraphicsscene(pc.graphicsView);
  connect(m_scene, SIGNAL(selectionChanged()), this, SLOT(slotSceneSelectionChanged()));
  m_oid = oidArg;
  m_isQueryEnabled = false;
  m_parentWid = parentArg;
  photo.setupUi(m_photo_diag);
  // photo.quantity->setMaximum(static_cast<int> (biblioteq::Limits::QUANTITY));
  photo.thumbnail_item->enableDoubleClickResize(true);
  p_compare.setupUi(m_photo_compare_diag);

  pc.graphicsView->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(pc.graphicsView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotViewContextMenu(QPoint)));
  pc.graphicsView->setScene(m_scene);
  pc.graphicsView->setDragMode(QGraphicsView::RubberBandDrag);
  pc.graphicsView->setRubberBandSelectionMode(Qt::IntersectsItemShape);

  pc.thumbnail_item->setReadOnly(true);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
  new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_A), this, SLOT(slotSelectAll(void)));
  new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_S), this, SLOT(slotGo(void)));
#else
  new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_A), this, SLOT(slotSelectAll(void)));
  new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_S), this, SLOT(slotGo(void)));
#endif
  updateFont(QApplication::font(), qobject_cast<QWidget *>(this));
  m_photo_diag->setWindowModality(Qt::WindowModal);
  m_photo_compare_diag->setWindowModality(Qt::WindowModal);
  updateFont(QApplication::font(), qobject_cast<QWidget *>(m_photo_diag));
  connect(pc.select_image_collection, SIGNAL(clicked()), this, SLOT(slotSelectImage()));
  connect(photo.select_image_item, SIGNAL(clicked()), this, SLOT(slotSelectImage()));
  connect(pc.okButton, SIGNAL(clicked()), this, SLOT(slotGo()));
  connect(pc.cancelButton, SIGNAL(clicked()), this, SLOT(slotCancel()));
  connect(pc.importItems, SIGNAL(clicked()), this, SLOT(slotImportItems()));
  connect(pc.printButton, SIGNAL(clicked()), this, SLOT(slotPrint()));
  connect(pc.addItemButton, SIGNAL(clicked()), this, SLOT(slotAddItem()));
  connect(photo.cancelButton, SIGNAL(clicked()), this, SLOT(slotClosePhoto()));
  connect(menu2->addAction(tr("&All...")), SIGNAL(triggered()), this, SLOT(slotExportPhotographs()));
  connect(menu2->addAction(tr("&Current Page...")), SIGNAL(triggered()), this, SLOT(slotExportPhotographs()));
  connect(menu2->addAction(tr("&Selected...")), SIGNAL(triggered()), this, SLOT(slotExportPhotographs()));
  connect(pc.page, SIGNAL(currentIndexChanged(int)), this, SLOT(slotPageChanged(int)));
  connect(pc.sort_box, SIGNAL(currentIndexChanged(int)), this, SLOT(slotSortByChanged()));
  connect(pc.graphicsView->scene(), SIGNAL(itemDoubleClicked()), this, SLOT(slotModifyItem()));
  pc.exportPhotographsToolButton->setMenu(menu2);
  connect(pc.exportPhotographsToolButton, SIGNAL(clicked()), pc.exportPhotographsToolButton, SLOT(showMenu()));
  connect(pc.linked_picture_button, SIGNAL(clicked()), this, SLOT(slotShowCompare()));

  if (menu2->actions().size() >= 3)
    menu2->actions().at(2)->setEnabled(false);

  QString errorstr("");

  QApplication::setOverrideCursor(Qt::WaitCursor);
  /*pc.location->addItems
    (biblioteq_misc_functions::getLocations(qmain->getDB(),
                        "Photograph Collection",
                        errorstr));*/
  QApplication::restoreOverrideCursor();

  if (!errorstr.isEmpty())
    qmain->addError(QString(tr("Database Error")), QString(tr("Unable to retrieve the photograph collection locations.")), errorstr, __FILE__, __LINE__);

  pc.thumbnail_collection->setScene(scene1);
  pc.thumbnail_item->setScene(scene2);
  photo.thumbnail_item->setScene(scene3);

  if (m_parentWid)
    biblioteq_misc_functions::center(this, m_parentWid);

#ifdef Q_OS_MACOS
  foreach (auto tool_button, findChildren<QToolButton *>())
#if (QT_VERSION < QT_VERSION_CHECK(5, 10, 0))
    tool_button->setStyleSheet("QToolButton {border: none; padding-right: 10px}"
                               "QToolButton::menu-button {border: none;}");
#else
    tool_button->setStyleSheet("QToolButton {border: none; padding-right: 15px}"
                               "QToolButton::menu-button {border: none; width: 15px;}");
#endif
#endif

  pc.splitter->setStretchFactor(0, 0);
  pc.splitter->setStretchFactor(1, 2);
  pc.splitter->setStretchFactor(2, 1);
  pc.sort_box->insertItem(0, tr("ID"));
  pc.sort_box->insertItem(1, tr("Title"));
  pc.sort_box->insertItem(2, tr("Page Number"));
  pc.sort_box->insertItem(3, tr("Delivery Number"));
  biblioteq_misc_functions::center(this, m_parentWid);
  biblioteq_misc_functions::hideAdminFields(this);
  biblioteq_misc_functions::highlightWidget(photo.id_item, QColor(255, 248, 220));
  biblioteq_misc_functions::highlightWidget(photo.title_item, QColor(255, 248, 220));
  biblioteq_misc_functions::highlightWidget(photo.executing_artist_item->viewport(), QColor(255, 248, 220));
}

biblioteq_photographcollection::~biblioteq_photographcollection()
{
}

bool biblioteq_photographcollection::verifyItemFields(void)
{
  QString str("");

  str = photo.id_item->text().trimmed();
  photo.id_item->setText(str);

  if (photo.id_item->text().isEmpty())
  {
    QMessageBox::critical(m_photo_diag, tr("BiblioteQ: User Error"),
                          tr("Please complete the item's "
                             "ID field."));
    QApplication::processEvents();
    photo.id_item->setFocus();
    return false;
  }

  str = photo.title_item->text().trimmed();
  photo.title_item->setText(str);

  if (photo.title_item->text().isEmpty())
  {
    QMessageBox::critical(m_photo_diag, tr("BiblioteQ: User Error"),
                          tr("Please complete the item's "
                             "Title field."));
    QApplication::processEvents();
    photo.title_item->setFocus();
    return false;
  }

  str = photo.executing_artist_item->toPlainText().trimmed();
  photo.executing_artist_item->setPlainText(str);

  if (photo.executing_artist_item->toPlainText().isEmpty())
  {
    QMessageBox::critical(m_photo_diag, tr("BiblioteQ: User Error"),
                          tr("Please complete the item's "
                             "Executing Artist(s) field."));
    QApplication::processEvents();
    photo.executing_artist_item->setFocus();
    return false;
  }

  str = photo.technique_item->text().trimmed();
  photo.technique_item->setText(str);

  str = photo.title_original_picture_item->toPlainText().trimmed();
  photo.title_original_picture_item->setPlainText(str);

  str = photo.title_collection_item->toPlainText().trimmed();
  photo.title_collection_item->setPlainText(str);

  str = photo.place_of_storage_item->text().trimmed();
  photo.place_of_storage_item->setText(str);
  return true;
}

int biblioteq_photographcollection::photographsPerPage(void)
{
  auto integer = qmain->setting("photographs_per_page").toInt();

  if (!(integer == -1 || (integer >= 25 && integer <= 100)))
    integer = 25;

  return integer;
}

void biblioteq_photographcollection::changeEvent(QEvent *event)
{
  if (event)
    switch (event->type())
    {
    case QEvent::LanguageChange:
    {
      pc.retranslateUi(this);
      photo.retranslateUi(m_photo_diag);
      break;
    }
    default:
      break;
    }

  QMainWindow::changeEvent(event);
}

void biblioteq_photographcollection::closeEvent(QCloseEvent *event)
{
  if (m_engWindowTitle.contains("Create") ||
      m_engWindowTitle.contains("Modify"))
    if (hasDataChanged(this))
    {
      if (QMessageBox::
              question(this, tr("BiblioteQ: Question"),
                       tr("Your changes have not been saved. Continue closing?"),
                       QMessageBox::Yes | QMessageBox::No,
                       QMessageBox::No) == QMessageBox::No)
      {
        QApplication::processEvents();

        if (event)
          event->ignore();

        return;
      }

      QApplication::processEvents();
    }

  qmain->removePhotographCollection(this);
}

void biblioteq_photographcollection::duplicate(const QString &p_oid,
                                               const int state)
{
  modify(state, "Create"); // Initial population.
  pc.addItemButton->setEnabled(false);
  pc.importItems->setEnabled(false);
  m_oid = p_oid;
  setWindowTitle(tr("BiblioteQ: Duplicate Photograph Collection Entry"));
}

void biblioteq_photographcollection::insert(void)
{
  pc.okButton->setText(tr("&Save"));
  pc.addItemButton->setEnabled(false);
  pc.importItems->setEnabled(false);
  pc.creation_date_item->setText("01/01/2000");
  biblioteq_misc_functions::highlightWidget(pc.id_collection, QColor(255, 248, 220));
  biblioteq_misc_functions::highlightWidget(pc.title_collection, QColor(255, 248, 220));
  setWindowTitle(tr("BiblioteQ: Create Photograph Collection Entry"));
  m_engWindowTitle = "Create";
  pc.id_collection->setFocus();
  pc.id_collection->setText(QUuid::createUuid().toString().remove("{").remove("}"));
  pc.page->blockSignals(true);
  pc.page->clear();
  pc.page->addItem("1");
  pc.page->blockSignals(false);
  pc.page->setCurrentIndex(0);
  storeData();
#ifdef Q_OS_ANDROID
  showMaximized();
#else
  biblioteq_misc_functions::center(this, m_parentWid);
  showNormal();
#endif
  activateWindow();
  raise();
}

void biblioteq_photographcollection::loadPhotographFromItem(QGraphicsScene *scene, QGraphicsPixmapItem *item, const int percent)
{
  if (!item || !scene || !scene->views().value(0))
    return;

  QSqlQuery query(qmain->getDB());

  QApplication::setOverrideCursor(Qt::WaitCursor);
  query.setForwardOnly(true);
  query.prepare("SELECT image FROM "
                "photograph WHERE "
                "collection_oid = ? AND "
                "myoid = ?");
  query.bindValue(0, m_oid);
  query.bindValue(1, item->data(0).toLongLong());

  if (query.exec())
  {
    if (query.next())
    {
      QImage image;
      auto bytes(QByteArray::fromBase64(query.value(0).toByteArray()));
      auto format(biblioteq_misc_functions::imageFormatGuess(bytes));

      image.loadFromData(bytes);

      if (image.isNull())
      {
        bytes = query.value(0).toByteArray();
        image.loadFromData(bytes);
      }

      if (image.isNull())
        image = QImage(":/no_image.png");

      QSize size;

      if (percent == 0)
        size = scene->views().value(0)->size();
      else
      {
        size = image.size();
        size.setHeight((percent * size.height()) / 100);
        size.setWidth((percent * size.width()) / 100);
      }

      if (!image.isNull())
        image = image.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);

      pc.graphicsView->scene()->clearSelection();

      QGraphicsPixmapItem *pixmapItem = nullptr;

      if (!scene->items().isEmpty())
      {
        pixmapItem = qgraphicsitem_cast<QGraphicsPixmapItem *>(scene->items().at(0));

        if (pixmapItem)
          pixmapItem->setPixmap(QPixmap::fromImage(image));
      }
      else
        pixmapItem = scene->addPixmap(QPixmap::fromImage(image));

      if (pixmapItem)
        pixmapItem->setData(1, bytes);

      item->setSelected(true);

      if (!scene->items().isEmpty())
      {
        scene->items().at(0)->setData(0, item->data(0)); // myoid
        scene->items().at(0)->setData(2, item->data(2)); // Navigation.
      }

      scene->setSceneRect(scene->itemsBoundingRect());

      auto view = qobject_cast<biblioteq_photograph_view *>(scene->views().value(0));

      if (view)
      {
        connect(view, SIGNAL(save(QImage, QString, qint64)), this, SLOT(slotSaveRotatedImage(QImage, QString, qint64)), Qt::UniqueConnection);
        view->horizontalScrollBar()->setValue(0);
        view->setBestFit(percent == 0);
        view->setImage(image, format, item->data(0).toLongLong());
        view->verticalScrollBar()->setValue(0);
      }
    }
  }

  QApplication::restoreOverrideCursor();
}

void biblioteq_photographcollection::loadTwoPhotographFromItem(QGraphicsScene *scene1, QGraphicsScene *scene2, QGraphicsPixmapItem *item1, QGraphicsPixmapItem *item2, const int percent)
{
  if (!item1 || !item2 || !scene1 || !scene1->views().value(0))
    return;

  QSqlQuery query(qmain->getDB());

  QApplication::setOverrideCursor(Qt::WaitCursor);
  query.setForwardOnly(true);
  query.prepare("SELECT image, image_original FROM "
                "photograph WHERE "
                "collection_oid = ? AND "
                "myoid = ?");
  query.bindValue(0, m_oid);
  query.bindValue(1, item1->data(0).toLongLong());

  if (query.exec())
  {
    if (query.next())
    {
      QImage image1, image2;
      auto bytes1(QByteArray::fromBase64(query.value(0).toByteArray()));
      auto bytes2(QByteArray::fromBase64(query.value(0).toByteArray()));
      auto format1(biblioteq_misc_functions::imageFormatGuess(bytes1));
      auto format2(biblioteq_misc_functions::imageFormatGuess(bytes2));

      image1.loadFromData(bytes1);
      image2.loadFromData(bytes2);

      if (image1.isNull())
      {
        bytes1 = query.value(0).toByteArray();
        image1.loadFromData(bytes1);
      }
      if (image2.isNull())
      {
        bytes2 = query.value(1).toByteArray();
        image2.loadFromData(bytes2);
      }

      if (image1.isNull())
        image1 = QImage(":/no_image.png");
      if (image2.isNull())
        image2 = QImage(":/no_image.png");

      QSize size1, size2;

      if (percent == 0)
      {
        size1 = scene1->views().value(0)->size();
        size2 = scene2->views().value(0)->size();
      }
      else
      {
        size1 = image1.size();
        size1.setHeight((percent * size1.height()) / 100);
        size1.setWidth((percent * size1.width()) / 100);
        size2 = image2.size();
        size2.setHeight((percent * size2.height()) / 100);
        size2.setWidth((percent * size2.width()) / 100);
      }

      if (!image1.isNull())
        image1 = image1.scaled(size1, Qt::KeepAspectRatio, Qt::SmoothTransformation);
      if (!image2.isNull())
        image2 = image2.scaled(size2, Qt::KeepAspectRatio, Qt::SmoothTransformation);

      QGraphicsPixmapItem *pixmapItem1 = nullptr;

      if (!scene1->items().isEmpty())
      {
        pixmapItem1 = qgraphicsitem_cast<QGraphicsPixmapItem *>(scene1->items().at(0));

        if (pixmapItem1)
          pixmapItem1->setPixmap(QPixmap::fromImage(image1));
      }
      else
        pixmapItem1 = scene1->addPixmap(QPixmap::fromImage(image1));

      if (pixmapItem1)
        pixmapItem1->setData(1, bytes1);

      QGraphicsPixmapItem *pixmapItem2 = nullptr;

      if (!scene2->items().isEmpty())
      {
        pixmapItem2 = qgraphicsitem_cast<QGraphicsPixmapItem *>(scene2->items().at(0));

        if (pixmapItem2)
          pixmapItem2->setPixmap(QPixmap::fromImage(image2));
      }
      else
        pixmapItem2 = scene2->addPixmap(QPixmap::fromImage(image2));

      if (pixmapItem2)
        pixmapItem2->setData(1, bytes2);

      item1->setSelected(true);
      item2->setSelected(true);

      if (!scene1->items().isEmpty())
      {
        scene1->items().at(0)->setData(0, item1->data(0)); // myoid
        scene1->items().at(0)->setData(2, item1->data(2)); // Navigation.
      }
      if (!scene2->items().isEmpty())
      {
        scene2->items().at(0)->setData(0, item2->data(0)); // myoid
        scene2->items().at(0)->setData(2, item2->data(2)); // Navigation.
      }

      scene1->setSceneRect(scene1->itemsBoundingRect());
      scene2->setSceneRect(scene2->itemsBoundingRect());

      auto view1 = qobject_cast<biblioteq_photograph_view *>(scene1->views().value(0));
      auto view2 = qobject_cast<biblioteq_photograph_view *>(scene2->views().value(0));

      if (view1)
      {
        view1->horizontalScrollBar()->setValue(0);
        view1->setBestFit(percent == 0);
        view1->setImage(image1, format1, item1->data(0).toLongLong());
        view1->verticalScrollBar()->setValue(0);
      }
      if (view2)
      {
        view2->horizontalScrollBar()->setValue(0);
        view2->setBestFit(percent == 0);
        view2->setImage(image2, format2, item2->data(0).toLongLong());
        view2->verticalScrollBar()->setValue(0);
      }
    }
  }

  QApplication::restoreOverrideCursor();
}

void biblioteq_photographcollection::loadPhotographFromItemInNewWindow(QGraphicsPixmapItem *item)
{
  if (item)
  {
    QMainWindow *mainWindow = nullptr;
    Ui_photographView ui;

    mainWindow = new QMainWindow(this);
    mainWindow->setAttribute(Qt::WA_DeleteOnClose, true);
    ui.setupUi(mainWindow);
    connect(ui.closeButton, SIGNAL(clicked()), mainWindow, SLOT(close()));
    connect(ui.exportItem, SIGNAL(clicked()), this, SLOT(slotExportItem()));
    connect(ui.next, SIGNAL(clicked()), this, SLOT(slotViewNextPhotograph()));
    connect(ui.previous, SIGNAL(clicked()), this, SLOT(slotViewPreviousPhotograph()));
    connect(ui.rotate_left, SIGNAL(clicked()), ui.view, SLOT(slotRotateLeft()));
    connect(ui.rotate_right, SIGNAL(clicked()), ui.view, SLOT(slotRotateRight()));
    connect(ui.save, SIGNAL(clicked()), ui.view, SLOT(slotSave()));
    connect(ui.view_size, SIGNAL(currentIndexChanged(int)), this, SLOT(slotImageViewSizeChanged(int)));
    ui.save->setVisible(m_engWindowTitle.contains("Modify"));

    auto scene = new QGraphicsScene(mainWindow);

    mainWindow->show();
    biblioteq_misc_functions::center(mainWindow, this);
    mainWindow->hide();
    scene->setProperty("view_size", ui.view->viewport()->size());
    ui.view->setScene(scene);
    loadPhotographFromItem(scene, item, ui.view_size->currentText().remove("%").toInt());
    mainWindow->show();
  }
}

void biblioteq_photographcollection::modify(const int state,
                                            const QString &behavior)
{
  QSqlQuery query(qmain->getDB());
  QString fieldname("");
  QString str("");
  QVariant var;

  if (state == biblioteq::EDITABLE)
  {
    disconnect(m_scene, SIGNAL(deleteKeyPressed()), this, SLOT(slotDeleteItem()));
    connect(m_scene, SIGNAL(deleteKeyPressed()), this, SLOT(slotDeleteItem()));

    if (behavior.isEmpty())
    {
      setWindowTitle(tr("BiblioteQ: Modify Photograph Collection Entry"));
      m_engWindowTitle = "Modify";
    }
    else
      m_engWindowTitle = behavior;

    setReadOnlyFields(this, false);
    pc.okButton->setVisible(true);
    pc.addItemButton->setEnabled(true);
    pc.importItems->setEnabled(true);
    pc.select_image_collection->setVisible(true);
    biblioteq_misc_functions::highlightWidget(pc.id_collection, QColor(255, 248, 220));
    biblioteq_misc_functions::highlightWidget(pc.title_collection, QColor(255, 248, 220));
  }
  else
  {
    if (behavior.isEmpty())
    {
      setWindowTitle(tr("BiblioteQ: View Photograph Collection Details"));
      m_engWindowTitle = "View";
    }
    else
      m_engWindowTitle = behavior;

    setReadOnlyFields(this, true);
    pc.okButton->setVisible(false);
    pc.page->setEnabled(true);
    pc.addItemButton->setVisible(false);
    pc.importItems->setVisible(false);
    pc.select_image_collection->setVisible(false);
  }

  query.setForwardOnly(true);
  query.prepare("SELECT id, "
                "title, "
                "about, "
                "creation_date, "
                "circulation_height, "
                "total_number, "
                "by_artist, "
                "publisher, "
                "keywords, "
                "notes, "
                "first_release_date, "
                "catalogue, "
                "image "
                "FROM "
                "photograph_collection "
                "WHERE myoid = ?");
  query.bindValue(0, m_oid);
  pc.okButton->setText(tr("&Save"));
  QApplication::setOverrideCursor(Qt::WaitCursor);

  if (!query.exec() || !query.next())
  {
    QApplication::restoreOverrideCursor();
    qmain->addError(QString(tr("Database Error")), QString(tr("Unable to retrieve the selected photograph collection's data.")), query.lastError().text(), __FILE__, __LINE__);
    QMessageBox::critical(this, tr("BiblioteQ: Database Error"), tr("Unable to retrieve the selected photograph collection's data."));
    QApplication::processEvents();
    close();
    return;
  }
  else
  {
    QApplication::restoreOverrideCursor();

    auto record(query.record());

    for (int i = 0; i < record.count(); i++)
    {
      var = record.field(i).value();
      fieldname = record.fieldName(i);

      if (fieldname == "id")
      {
        pc.id_collection->setText(var.toString().trimmed());

        if (behavior.isEmpty())
        {
          if (state == biblioteq::EDITABLE)
          {
            str = QString(tr("BiblioteQ: Modify Photograph Collection "
                             "Entry (")) +
                  var.toString().trimmed() + tr(")");
            m_engWindowTitle = "Modify";
          }
          else
          {
            str = QString(tr("BiblioteQ: View Photograph "
                             "Collection Details (")) +
                  var.toString().trimmed() + tr(")");
            m_engWindowTitle = "View";
          }

          setWindowTitle(str);
        }
      }
      else if (fieldname == "title")
      {
        pc.title_collection->setText(var.toString().trimmed());
      }
      else if (fieldname == "creation_date")
      {
        pc.creation_date->setText(var.toString().trimmed());
      }
      else if (fieldname == "circulation_height")
      {
        pc.circulation_height->setText(var.toString().trimmed());
      }
      else if (fieldname == "keywords")
      {
        pc.keywords->setText(var.toString().trimmed());
      }
      else if (fieldname == "notes")
      {
        pc.notes->setText(var.toString().trimmed());
      }
      else if (fieldname == "by_artist")
      {
        pc.by_artist->setText(var.toString().trimmed());
      }
      else if (fieldname == "publisher")
      {
        pc.publisher->setText(var.toString().trimmed());
      }
      else if (fieldname == "total_number")
      {
        pc.total_number->setText(var.toString().trimmed());
      }
      else if (fieldname == "first_release_date")
      {
        pc.first_release->setText(var.toString().trimmed());
      }
      else if (fieldname == "catalogue")
      {
        pc.catalogue->setText(var.toString().trimmed());
      }
      else if (fieldname == "image")
      {
        if (!record.field(i).isNull())
        {
          pc.thumbnail_collection->loadFromData(QByteArray::fromBase64(var.toByteArray()));

          if (pc.thumbnail_collection->m_image.isNull())
            pc.thumbnail_collection->loadFromData(var.toByteArray());
        }
      }
      else if (fieldname == "image")
      {
        if (!record.field(i).isNull())
        {
          pc.thumbnail_collection->loadFromData(QByteArray::fromBase64(var.toByteArray()));

          if (pc.thumbnail_collection->m_image.isNull())
            pc.thumbnail_collection->loadFromData(var.toByteArray());
        }
      }
    }

    int pages = 1;

    if (!m_engWindowTitle.contains("Create"))
    {
      QApplication::setOverrideCursor(Qt::WaitCursor);
      query.prepare("SELECT COUNT(*) "
                    "FROM photograph "
                    "WHERE collection_oid = ?");
      query.bindValue(0, m_oid);

      if (query.exec())
        if (query.next())
        {
          int i = photographsPerPage();

          int nr_photos = query.value(0).toInt();

          if (i == -1) // Unlimited.
          {
            pages = 1;
            setSceneRect(nr_photos);
          }
          else
            pages = qCeil((double)nr_photos / qMax(1, i));
        }

      QApplication::restoreOverrideCursor();
      pages = qMax(1, pages);
    }

    int old_page = pc.page->currentText().toInt();

    pc.page->blockSignals(true);
    pc.page->clear();

    for (int i = 1; i <= pages; i++)
      pc.page->addItem(QString::number(i));

    pc.page->blockSignals(false);

    foreach (auto textfield, findChildren<QLineEdit *>())
      textfield->setCursorPosition(0);

    storeData();
#ifdef Q_OS_ANDROID
    showMaximized();
#else
    biblioteq_misc_functions::center(this, m_parentWid);
    showNormal();
#endif
    activateWindow();
    raise();
    repaint();
    QApplication::processEvents();

    if (!m_engWindowTitle.contains("Create"))
    {
      showPhotographs(old_page - 1);
      pc.page->setCurrentIndex(fmax(old_page - 1, 0));
    }
  }

  pc.id_collection->setFocus();
}

void biblioteq_photographcollection::search(const QString &field,
                                            const QString &value)
{
  Q_UNUSED(field);
  Q_UNUSED(value);
  pc.addItemButton->setVisible(false);
  pc.importItems->setVisible(false);
  pc.thumbnail_collection->setVisible(false);
  pc.select_image_collection->setVisible(false);
  pc.collectionGroup->setVisible(false);
  pc.itemGroup->setVisible(false);
  pc.exportPhotographsToolButton->setVisible(false);
  setWindowTitle(tr("BiblioteQ: Database Photograph Collection Search"));
  m_engWindowTitle = "Search";
  pc.okButton->setText(tr("&Search"));
  pc.id_collection->setFocus();
#ifdef Q_OS_ANDROID
  showMaximized();
#else
  biblioteq_misc_functions::center(this, m_parentWid);
  showNormal();
#endif
  activateWindow();
  raise();
}

void biblioteq_photographcollection::setSceneRect(const int size)
{
  if (size > 0 && (size * 250 + 15 <= std::numeric_limits<int>::max()))
    pc.graphicsView->setSceneRect(0, 0, 5 * 150, size * 250 + 15);
  else
    pc.graphicsView->setSceneRect(0, 0, 5 * 150, std::numeric_limits<int>::max());
}

void biblioteq_photographcollection::showPhotographs(const int &page)
{
  QProgressDialog progress(this);

  progress.setLabelText(tr("Loading image(s)..."));
  progress.setMaximum(0);
  progress.setMinimum(0);
  progress.setModal(true);
  progress.setWindowTitle(tr("BiblioteQ: Progress Dialog"));
  progress.show();
  progress.repaint();
  QApplication::processEvents();

  QSqlQuery query(qmain->getDB());

  query.setForwardOnly(true);
  QString orderBy("");
  QString selection(pc.sort_box->currentText());

  if (selection == "ID")
  {
    orderBy = "id";
  }
  else if (selection == "Page Number")
  {
    orderBy = "CAST(page_number AS INTEGER)";
  }
  else if (selection == "Delivery Number")
  {
    orderBy = "delivery_number";
  }
  else if (selection == "Title")
  {
    orderBy = "title";
  }
  else
  {
    orderBy = "myoid"; // Default case if none of the above match.
  }

  if (photographsPerPage() == -1) // Unlimited.
  {
    query.prepare("SELECT image_scaled, myoid, " + orderBy +
                  " FROM "
                  "photograph WHERE "
                  "collection_oid = ? "
                  "ORDER BY CAST(" +
                  orderBy + " AS INTEGER) ASC");
    query.bindValue(0, m_oid);
  }
  else
  {
    auto temp = photographsPerPage();
    query.prepare("SELECT image_scaled, myoid, " + orderBy +
                  " FROM "
                  "photograph WHERE "
                  "collection_oid = ? "
                  "ORDER BY CAST(" +
                  orderBy +
                  " AS INTEGER) ASC "
                  "LIMIT ? "
                  "OFFSET ?");
    query.bindValue(0, m_oid);
    query.bindValue(1, temp);
    query.bindValue(2, page * temp);
  }

  if (query.exec())
  {
    pc.graphicsView->scene()->clear();
    pc.graphicsView->resetTransform();
    pc.graphicsView->verticalScrollBar()->setValue(0);
    pc.graphicsView->horizontalScrollBar()->setValue(0);

    int columnIdx = 0;
    int i = -1;
    int rowIdx = 0;

    while (query.next())
    {
      i += 1;
      progress.repaint();
      QApplication::processEvents();

      if (progress.wasCanceled())
        break;

      QImage image;
      biblioteq_graphicsitempixmap *pixmapItem = nullptr;

      image.loadFromData(QByteArray::fromBase64(query.value(0).toByteArray()));

      if (image.isNull())
        image.loadFromData(query.value(0).toByteArray());

      if (image.isNull())
        image = QImage(":/no_image.png");

      /*
      ** The size of no_image.png is 126x187.
      */

      if (!image.isNull())
        image = image.scaled(126, 187, Qt::KeepAspectRatio, Qt::SmoothTransformation);

      pixmapItem = new biblioteq_graphicsitempixmap(QPixmap::fromImage(image), nullptr);

      if (rowIdx == 0)
        pixmapItem->setPos(140 * columnIdx + 15, 15);
      else
        pixmapItem->setPos(140 * columnIdx + 15, 200 * rowIdx);

      pixmapItem->setData(0, query.value(1)); // myoid
      pixmapItem->setData(2, i);              // Next / previous navigation.
      pixmapItem->setFlag(QGraphicsItem::ItemIsSelectable, true);
      pc.graphicsView->scene()->addItem(pixmapItem);
      columnIdx += 1;

      if (columnIdx >= 5)
      {
        rowIdx += 1;
        columnIdx = 0;
      }
    }
  }

  progress.close();
}

void biblioteq_photographcollection::slotAddItem(void)
{
  photo.saveButton->disconnect(SIGNAL(clicked()));
  connect(photo.saveButton, SIGNAL(clicked()), this,
          SLOT(slotInsertItem()));
  m_photo_diag->resize(m_photo_diag->width(),
                       qRound(0.95 * size().height()));
  biblioteq_misc_functions::center(m_photo_diag, this);
  photo.thumbnail_item->clear();
  photo.id_item->setText(QString::number(QDateTime::currentMSecsSinceEpoch()));
  photo.title_item->clear();
  photo.title_old_item->clear();
  photo.title_description_item->clear();
  photo.executing_artist_item->clear();
  photo.creation_date_item->clear();
  photo.creation_date_original_item->clear();
  photo.technique_item->clear();
  photo.title_original_picture_item->clear();
  photo.title_collection_item->setPlainText(pc.title_collection->toPlainText());
  photo.based_on_artist_item->clear();
  photo.delivery_number_item->clear();
  photo.page_number_item->clear();
  photo.notes_item->clear();
  photo.signed_item->clear();
  photo.format_item->clear();
  photo.technique_item->clear();
  photo.place_of_storage_item->clear();
  photo.inventor_new_item->clear();
  photo.inventor_old_item->clear();
  photo.printer_item->clear();
  photo.inventory_number_item->clear();
  photo.keywords_item->clear();
  photo.scrollArea->ensureVisible(0, 0);
  photo.id_item->setFocus();
  m_photo_diag->show();
}

void biblioteq_photographcollection::slotCancel(void)
{
  close();
}

void biblioteq_photographcollection::slotClosePhoto(void)
{
#ifdef Q_OS_ANDROID
  m_photo_diag->hide();
#else
  m_photo_diag->close();
#endif
}

void biblioteq_photographcollection::slotDeleteItem(void)
{
  auto items(pc.graphicsView->scene()->selectedItems());

  if (items.isEmpty())
    return;
  else
  {
    if (QMessageBox::question(this, tr("BiblioteQ: Question"),
                              tr("Are you sure that you wish to permanently "
                                 "delete the selected %1 item(s)?")
                                  .arg(items.size()),
                              QMessageBox::Yes | QMessageBox::No,
                              QMessageBox::No) == QMessageBox::No)
    {
      QApplication::processEvents();
      return;
    }

    QApplication::processEvents();
  }

  QProgressDialog progress(this);

  progress.setCancelButton(nullptr);
  progress.setModal(true);
  progress.setWindowTitle(tr("BiblioteQ: Progress Dialog"));
  progress.setLabelText(tr("Deleting the selected item(s)..."));
  progress.setMaximum(items.size());
  progress.setMinimum(0);
  progress.show();
  progress.repaint();
  QApplication::processEvents();

  for (int i = 0; i < items.size(); i++)
  {
    if (i + 1 <= progress.maximum())
      progress.setValue(i + 1);

    progress.repaint();
    QApplication::processEvents();

    QGraphicsPixmapItem *item = nullptr;

    if ((item = qgraphicsitem_cast<QGraphicsPixmapItem *>(items.at(i))))
    {
      QSqlQuery query(qmain->getDB());
      auto itemOid(item->data(0).toString());

      query.prepare("DELETE FROM photograph WHERE "
                    "collection_oid = ? AND myoid = ?");
      query.bindValue(0, m_oid);
      query.bindValue(1, itemOid);

      if (query.exec())
      {
        pc.graphicsView->scene()->removeItem(item);
        delete item;
      }
    }
  }

  QSqlQuery query(qmain->getDB());
  int pages = 1;

  query.prepare("SELECT COUNT(*) "
                "FROM photograph "
                "WHERE collection_oid = ?");
  query.bindValue(0, m_oid);

  if (query.exec())
    if (query.next())
    {
      updateTablePhotographCount(query.value(0).toInt());

      auto i = photographsPerPage();

      if (i == -1) // Unlimited.
      {
        pages = 1;
        setSceneRect(query.value(0).toInt());
      }
      else
        pages = qCeil(query.value(0).toDouble() / qMax(1, i));
    }

  pages = qMax(1, pages);
  auto old_page = pc.page->currentText().toInt();
  pc.page->blockSignals(true);
  pc.page->clear();

  for (int i = 1; i <= pages; i++)
    pc.page->addItem(QString::number(i));

  if (old_page < pages)
  {
    old_page -= 1;
  }
  pc.page->blockSignals(false);
  progress.close();
  repaint();
  QApplication::processEvents();
  showPhotographs(old_page - 1);
  pc.page->setCurrentIndex(fmax(old_page - 1, 0));
}

void biblioteq_photographcollection::slotExportItem(void)
{
  auto pushButton = qobject_cast<QPushButton *>(sender());

  if (!pushButton)
    return;

  auto parent = pushButton->parentWidget();

  do
  {
    if (!parent)
      break;

    if (qobject_cast<QMainWindow *>(parent))
      break;

    parent = parent->parentWidget();
  } while (true);

  if (!parent)
    return;

  QByteArray bytes;
  auto scene = parent->findChild<QGraphicsScene *>();

  if (scene)
  {
    auto item = qgraphicsitem_cast<QGraphicsPixmapItem *>(scene->items().value(0));

    if (item)
      bytes = item->data(1).toByteArray();
  }

  if (bytes.isEmpty())
    return;

  QFileDialog dialog(this);

  dialog.setAcceptMode(QFileDialog::AcceptSave);
  dialog.setDirectory(QDir::homePath());
  dialog.setFileMode(QFileDialog::AnyFile);
  dialog.setOption(QFileDialog::DontUseNativeDialog);
  dialog.setWindowTitle(tr("BiblioteQ: Photograph Collection Photograph Export"));
  dialog.selectFile(QString("biblioteq-image-export.%1").arg(biblioteq_misc_functions::imageFormatGuess(bytes)));

  if (dialog.exec() == QDialog::Accepted)
  {
    QApplication::processEvents();

    QFile file(dialog.selectedFiles().value(0));

    if (file.open(QIODevice::WriteOnly))
    {
      file.write(bytes);
      file.flush();
      file.close();
    }
  }

  QApplication::processEvents();
}

void biblioteq_photographcollection::slotExportPhotographs(void)
{
  if (pc.graphicsView->scene()->items().isEmpty())
    return;
  else
  {
    if (!qobject_cast<QAction *>(sender()))
      if (pc.graphicsView->scene()->selectedItems().isEmpty())
        return;
  }

  QFileDialog dialog(this);

  dialog.setFileMode(QFileDialog::Directory);
  dialog.setDirectory(QDir::homePath());
  dialog.setOption(QFileDialog::DontUseNativeDialog);
  dialog.setWindowTitle(tr("BiblioteQ: Photograph Collection Photographs "
                           "Export"));
  dialog.exec();
  QApplication::processEvents();

  if (dialog.result() == QDialog::Accepted &&
      dialog.selectedFiles().size() > 0)
  {
    repaint();
    QApplication::processEvents();

    auto action = qobject_cast<QAction *>(sender());

    if (!action ||
        action == pc.exportPhotographsToolButton->menu()->actions().value(0))
      /*
      ** Export all photographs.
      */

      biblioteq_misc_functions::exportPhotographs(qmain->getDB(),
                                                  m_oid,
                                                  -1,
                                                  photographsPerPage(),
                                                  dialog.selectedFiles().value(0),
                                                  this);
    else if (action ==
             pc.exportPhotographsToolButton->menu()->actions().value(1))
      /*
      ** Export the current page.
      */

      biblioteq_misc_functions::exportPhotographs(qmain->getDB(),
                                                  m_oid,
                                                  pc.page->currentText().toInt(),
                                                  photographsPerPage(),
                                                  dialog.selectedFiles().value(0),
                                                  this);
    else
      /*
      ** Export the selected photograp(s).
      */

      biblioteq_misc_functions::exportPhotographs(qmain->getDB(),
                                                  m_oid,
                                                  dialog.selectedFiles().value(0),
                                                  pc.graphicsView->scene()->selectedItems(),
                                                  this);
  }
}

void biblioteq_photographcollection::slotGo(void)
{
  QString errorstr("");
  QString str("");

  if (m_engWindowTitle.contains("Create") ||
      m_engWindowTitle.contains("Modify"))
  {
    str = pc.id_collection->text().trimmed();
    pc.id_collection->setText(str);

    if (pc.id_collection->text().isEmpty())
    {
      QMessageBox::critical(this, tr("BiblioteQ: User Error"),
                            tr("Please complete the collection's "
                               "ID field."));
      QApplication::processEvents();
      pc.id_collection->setFocus();
      return;
    }

    str = pc.title_collection->toPlainText().trimmed();
    pc.title_collection->setText(str);

    if (pc.title_collection->toPlainText().isEmpty())
    {
      QMessageBox::critical(this, tr("BiblioteQ: User Error"),
                            tr("Please complete the collection's "
                               "Title field."));
      QApplication::processEvents();
      pc.title_collection->setFocus();
      return;
    }

    pc.notes->setPlainText(pc.notes->toPlainText().trimmed());
    pc.creation_date->setText(pc.creation_date->text().trimmed());
    pc.circulation_height->setText(pc.circulation_height->text().trimmed());
    pc.total_number->setText(pc.total_number->text().trimmed());
    pc.by_artist->setText(pc.by_artist->text().trimmed());
    pc.publisher->setPlainText(pc.publisher->toPlainText().trimmed());
    pc.keywords->setPlainText(pc.keywords->toPlainText().trimmed());
    pc.catalogue->setPlainText(pc.catalogue->toPlainText().trimmed());
    pc.first_release->setText(pc.first_release->text().trimmed());

    if (!qmain->getDB().transaction())
    {
      QApplication::restoreOverrideCursor();
      qmain->addError(QString(tr("Database Error")),
                      QString(tr("Unable to create a database transaction.")),
                      qmain->getDB().lastError().text(), __FILE__, __LINE__);
      QMessageBox::critical(this, tr("BiblioteQ: Database Error"),
                            tr("Unable to create a database transaction."));
      QApplication::processEvents();
      return;
    }
    else
      QApplication::restoreOverrideCursor();

    QSqlQuery query(qmain->getDB());

    if (m_engWindowTitle.contains("Modify"))
      query.prepare("UPDATE photograph_collection SET "
                    "id = ?, "
                    "title = ?, "
                    "creation_date = ?, "
                    "circulation_height = ?, "
                    "total_number = ?, "
                    "by_artist = ?, "
                    "publisher = ?, "
                    "keywords = ?, "
                    "notes = ?, "
                    "first_release_date = ?, "
                    "catalogue = ?, "
                    "image = ?, "
                    "image_scaled = ? "
                    "WHERE "
                    "myoid = ?");
    else
      query.prepare("INSERT INTO photograph_collection "
                    "(id, title, creation_date, circulation_height, total_number, by_artist, "
                    "publisher, keywords, notes, first_release_date, catalogue, image,"
                    "image_scaled, myoid) "
                    "VALUES (?, ?, ?, ?, ?, ?, "
                    "?, ?, ?, ?, ?, ?, "
                    "?, ?)");

    query.bindValue(0, pc.id_collection->text().trimmed());
    query.bindValue(1, pc.title_collection->toPlainText().trimmed());
    query.bindValue(2, pc.creation_date->text().trimmed());
    query.bindValue(3, pc.circulation_height->text().trimmed());
    query.bindValue(4, pc.total_number->text().trimmed());
    query.bindValue(5, pc.by_artist->text().trimmed());
    query.bindValue(6, pc.publisher->toPlainText().trimmed());
    query.bindValue(7, pc.keywords->toPlainText().trimmed());
    query.bindValue(8, pc.notes->toPlainText().trimmed());
    query.bindValue(9, pc.first_release->text().trimmed());
    query.bindValue(10, pc.catalogue->toPlainText().trimmed());

    if (!pc.thumbnail_collection->m_image.isNull())
    {
      QBuffer buffer;
      QByteArray bytes;
      QImage image;

      buffer.setBuffer(&bytes);

      if (buffer.open(QIODevice::WriteOnly))
      {
        pc.thumbnail_collection->m_image.save(&buffer, pc.thumbnail_collection->m_imageFormat.toLatin1(), 100);
        query.bindValue(11, bytes.toBase64());
      }
      else
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
        query.bindValue(11, QVariant(QMetaType(QMetaType::QByteArray)));
#else
        query.bindValue(11, QVariant(QVariant::ByteArray));
#endif

      buffer.close();
      bytes.clear();
      image = pc.thumbnail_collection->m_image;

      if (!image.isNull())
        image = image.scaled(126, 187, Qt::KeepAspectRatio, Qt::SmoothTransformation);

      if (buffer.open(QIODevice::WriteOnly))
      {
        image.save(&buffer, pc.thumbnail_collection->m_imageFormat.toLatin1(),
                   100);
        query.bindValue(12, bytes.toBase64());
      }
      else
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
        query.bindValue(12, QVariant(QMetaType(QMetaType::QByteArray)));
#else
        query.bindValue(12, QVariant(QVariant::ByteArray));
#endif
    }
    else
    {
      pc.thumbnail_collection->m_imageFormat = "";
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
      query.bindValue(11, QVariant(QMetaType(QMetaType::QByteArray)));
      query.bindValue(12, QVariant(QMetaType(QMetaType::QByteArray)));
#else
      query.bindValue(11, QVariant(QVariant::ByteArray));
      query.bindValue(12, QVariant(QVariant::ByteArray));
#endif
    }

    if (m_engWindowTitle.contains("Modify"))
      query.bindValue(13, m_oid);
    else
    {
      auto value = biblioteq_misc_functions::getSqliteUniqueId(qmain->getDB(), errorstr);

      if (errorstr.isEmpty())
        query.bindValue(13, value);
      else
        qmain->addError(QString(tr("Database Error")), QString(tr("Unable to generate a unique integer.")), errorstr);
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);

    if (!query.exec())
    {
      QApplication::restoreOverrideCursor();
      qmain->addError(QString(tr("Database Error")),
                      QString(tr("Unable to create or update the entry.")),
                      query.lastError().text(), __FILE__, __LINE__);
      goto db_rollback;
    }
    else
    {
      if (!qmain->getDB().commit())
      {
        QApplication::restoreOverrideCursor();
        qmain->addError(QString(tr("Database Error")),
                        QString(tr("Unable to commit the current database "
                                   "transaction.")),
                        qmain->getDB().lastError().text(), __FILE__,
                        __LINE__);
        goto db_rollback;
      }

      QApplication::restoreOverrideCursor();

      if (m_engWindowTitle.contains("Modify"))
      {
        str = QString(tr("BiblioteQ: Modify Photograph Collection "
                         "Entry (")) +
              pc.id_collection->text() + tr(")");
        setWindowTitle(str);
        m_engWindowTitle = "Modify";

        if (m_index->isValid() &&
            (qmain->getTypeFilterString() == "All" ||
             qmain->getTypeFilterString() == "Photograph Collections"))
        {
          qmain->getUI().table->setSortingEnabled(false);
          if (qmain->getTypeFilterString() == "Photograph Collections")
            qmain->getUI().table->horizontalHeader()->setSortIndicator(1, Qt::AscendingOrder);

          auto names(qmain->getUI().table->columnNames());

          for (int i = 0; i < names.size(); i++)
          {
            if (i == 0 && qmain->showMainTableImages())
            {
              auto pixmap(QPixmap::
                              fromImage(pc.thumbnail_collection->m_image));

              if (!pixmap.isNull())
                qmain->getUI().table->item(m_index->row(), i)->setIcon(pixmap);
              else
                qmain->getUI().table->item(m_index->row(), i)->setIcon(QIcon(":/no_image.png"));
            }

            if (names.at(i) == "Call Number")
              qmain->getUI().table->item(m_index->row(), i)->setText(pc.executing_artist_item->toPlainText());
            else if (names.at(i) == "ID" || names.at(i) == "ID Number")
              qmain->getUI().table->item(m_index->row(), i)->setText(pc.id_collection->text());
            else if (names.at(i) == "Title")
              qmain->getUI().table->item(m_index->row(), i)->setText(pc.title_collection->toPlainText());
          }

          qmain->getUI().table->setSortingEnabled(true);
          if (qmain->getTypeFilterString() == "Photograph Collections")
            qmain->getUI().table->horizontalHeader()->setSortIndicator(1, Qt::AscendingOrder);
          qmain->getUI().table->updateToolTips(m_index->row());

          foreach (auto textfield, findChildren<QLineEdit *>())
            textfield->setCursorPosition(0);

          qmain->slotResizeColumns();
        }

        qmain->slotDisplaySummary();
        qmain->updateSceneItem(m_oid, "Photograph Collection",
                               pc.thumbnail_collection->m_image);
      }
      else
      {
        QApplication::setOverrideCursor(Qt::WaitCursor);
        m_oid = biblioteq_misc_functions::getOID(pc.id_collection->text(),
                                                 "Photograph Collection",
                                                 qmain->getDB(),
                                                 errorstr);
        QApplication::restoreOverrideCursor();

        if (!errorstr.isEmpty())
        {
          qmain->addError(QString(tr("Database Error")),
                          QString(tr("Unable to retrieve the "
                                     "photograph collection's OID.")),
                          errorstr, __FILE__, __LINE__);
          QMessageBox::critical(this, tr("BiblioteQ: Database Error"),
                                tr("Unable to retrieve the "
                                   "photograph collection's OID."));
          QApplication::processEvents();
        }
        else
          qmain->replacePhotographCollection(m_oid, this);

        updateWindow(biblioteq::EDITABLE);

        if (qmain->getUI().actionAutoPopulateOnCreation->isChecked())
          (void)qmain->populateTable(biblioteq::POPULATE_ALL, "Photograph Collections",
                                     QString(""));

        raise();
      }

      storeData();
    }

    qmain->slotRefresh();
    this->raise();
    this->activateWindow();

    return;

  db_rollback:

    QApplication::setOverrideCursor(Qt::WaitCursor);

    if (m_engWindowTitle.contains("Create"))
      m_oid.clear();

    if (!qmain->getDB().rollback())
      qmain->addError(QString(tr("Database Error")), QString(tr("Rollback failure.")),
                      qmain->getDB().lastError().text(), __FILE__, __LINE__);

    QApplication::restoreOverrideCursor();
    QMessageBox::critical(this, tr("BiblioteQ: Database Error"),
                          tr("Unable to create or update the entry. "
                             "Please verify that "
                             "the entry does not already exist."));
    QApplication::processEvents();
  }
  else if (m_engWindowTitle.contains("Search"))
  {
    QSqlQuery query(qmain->getDB());
    QString searchstr("");
    QString frontCover("'' AS image_scaled ");

    if (qmain->showMainTableImages())
      frontCover = "photograph_collection.image_scaled ";

    searchstr = "SELECT DISTINCT photograph_collection.title, "
                "photograph_collection.id, "
                "COUNT(photograph.myoid) AS photograph_count, "
                "photograph_collection.type, "
                "photograph_collection.myoid, " +
                frontCover +
                "FROM photograph_collection LEFT JOIN "
                "photograph "
                "ON photograph_collection.myoid = photograph.collection_oid "
                "WHERE ";

    QString ESCAPE("");
    auto UNACCENT(qmain->unaccent());

    searchstr.append(UNACCENT + "(LOWER(photograph_collection.id)) LIKE " + UNACCENT +
                     "(LOWER(" + ESCAPE + "'%' || ? || '%')) AND ");

    searchstr.append(UNACCENT + "(LOWER(photograph_collection.title)) LIKE " +
                     UNACCENT + "(LOWER(" + ESCAPE + "'%' || ? || '%')) AND ");

    searchstr.append("GROUP BY photograph_collection.title, "
                     "photograph_collection.id, "
                     "photograph_collection.type, "
                     "photograph_collection.myoid, "
                     "photograph_collection.image_scaled");
    query.prepare(searchstr);
    query.addBindValue(pc.id_collection->text().trimmed());
    query.addBindValue(biblioteq_myqstring::escape(pc.title_collection->toPlainText().trimmed()));
    (void)qmain->populateTable(query,
                               "Photograph Collections",
                               biblioteq::NEW_PAGE,
                               biblioteq::POPULATE_SEARCH);
  }
}

void biblioteq_photographcollection::slotImageViewSizeChanged(const int &text)
{
  auto comboBox = qobject_cast<QComboBox *>(sender());

  if (!comboBox)
    return;

  auto parent = comboBox->parentWidget();

  do
  {
    if (!parent)
      break;

    if (qobject_cast<QMainWindow *>(parent))
      break;

    parent = parent->parentWidget();
  } while (true);

  if (!parent)
    return;

  auto scene = parent->findChild<QGraphicsScene *>();

  if (scene)
  {
    auto item = qgraphicsitem_cast<QGraphicsPixmapItem *>(scene->items().value(0));

    if (item)
    {
      QImage image;

      if (image.loadFromData(item->data(1).toByteArray()))
      {
        QSize size;
        auto percent = comboBox->itemText(text).remove("%").toInt();

        if (percent == 0)
        {
          if (scene->views().value(0))
          {
            scene->setProperty("view_size", scene->views().value(0)->size());
            size = scene->views().value(0)->size();
          }
          else
            size = scene->property("view_size").toSize();
        }
        else
        {
          size = image.size();
          size.setHeight((percent * size.height()) / 100);
          size.setWidth((percent * size.width()) / 100);
        }

        if (!image.isNull())
          image = image.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);

        item->setPixmap(QPixmap::fromImage(image));
        scene->setSceneRect(scene->itemsBoundingRect());

        auto view = qobject_cast<biblioteq_photograph_view *>(scene->views().value(0));

        if (view)
          view->setBestFit(percent == 0);
      }
    }
  }
}

void biblioteq_photographcollection::slotImportItems(void)
{
  QFileDialog dialog(this);
  QStringList list;

  list << "*"
       << "*.bmp"
       << "*.jpg"
       << "*.jpeg"
       << "*.png";

  dialog.setDirectory(QDir::homePath());
  dialog.setFileMode(QFileDialog::ExistingFiles);
  dialog.setNameFilters(list);
  dialog.setOption(QFileDialog::DontUseNativeDialog);
  dialog.setWindowTitle(tr("BiblioteQ: Photograph Collection Import"));
  dialog.exec();
  QApplication::processEvents();

  if (dialog.result() != QDialog::Accepted)
    return;

  repaint();
  QApplication::processEvents();

  auto files(dialog.selectedFiles());

  if (files.isEmpty())
    return;

  QProgressDialog progress(this);

  progress.setLabelText(tr("Importing image(s)..."));
  progress.setMaximum(files.size());
  progress.setMinimum(0);
  progress.setModal(true);
  progress.setWindowTitle(tr("BiblioteQ: Progress Dialog"));
  progress.show();
  progress.repaint();
  QApplication::processEvents();

  int imported = 0;
  int pages = 0;

  for (int i = 0; i < files.size(); i++)
  {
    if (i + 1 <= progress.maximum())
      progress.setValue(i + 1);

    progress.repaint();
    QApplication::processEvents();

    if (progress.wasCanceled())
      break;

    QByteArray bytes1;
    QFile file(files.at(i));

    if (!file.open(QIODevice::ReadOnly))
      continue;
    else
      bytes1 = file.readAll();

    if (static_cast<qint64>(bytes1.length()) != file.size())
      continue;

    QImage image;

    if (!image.loadFromData(bytes1))
      continue;

    QSqlQuery query(qmain->getDB());

    query.prepare("INSERT INTO photograph "
                  "(id, collection_oid, title, creators, pdate, "
                  "quantity, medium, reproduction_number, "
                  "copyright, other_number, notes, subjects, "
                  "format, image, image_scaled, myoid) "
                  "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, "
                  "?, ?, ?, ?, ?, ?, ?, ?)");

    QString id("");

    id = QString::number(QDateTime::currentMSecsSinceEpoch() +
                         static_cast<qint64>(imported));
    query.bindValue(0, id);
    query.bindValue(1, m_oid);
    query.bindValue(2, "N/A");
    query.bindValue(3, "N/A");
    query.bindValue(4, "01/01/2000");
    query.bindValue(5, 1);
    query.bindValue(6, "N/A");
    query.bindValue(7, "N/A");
    query.bindValue(8, "N/A");
    query.bindValue(9, "N/A");
    query.bindValue(10, "N/A");
    query.bindValue(11, "N/A");
    query.bindValue(12, "N/A");
    query.bindValue(13, bytes1.toBase64());

    QBuffer buffer;
    QByteArray bytes2;
    auto format(biblioteq_misc_functions::imageFormatGuess(bytes1));

    buffer.setBuffer(&bytes2);
    buffer.open(QIODevice::WriteOnly);

    if (!image.isNull())
      image = image.scaled(126, 187, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    if (image.isNull() || !image.save(&buffer,
                                      format.toLatin1().constData(),
                                      100))
      bytes2 = bytes1;

    query.bindValue(14, bytes2.toBase64());

    QString errorstr("");
    auto value = biblioteq_misc_functions::getSqliteUniqueId(qmain->getDB(), errorstr);

    if (errorstr.isEmpty())
      query.bindValue(15, value);
    else
      qmain->addError(QString(tr("Database Error")), QString(tr("Unable to generate a unique integer.")), errorstr);

    if (!query.exec())
      qmain->addError(QString(tr("Database Error")), QString(tr("Unable to import photograph.")), query.lastError().text(), __FILE__, __LINE__);
    else
      imported += 1;
  }

  progress.close();
  repaint();
  QApplication::processEvents();
  QApplication::setOverrideCursor(Qt::WaitCursor);

  QSqlQuery query(qmain->getDB());

  query.prepare("SELECT COUNT(*) "
                "FROM photograph "
                "WHERE collection_oid = ?");
  query.bindValue(0, m_oid);

  if (query.exec())
    if (query.next())
    {
      updateTablePhotographCount(query.value(0).toInt());

      auto i = photographsPerPage();

      if (i == -1) // Unlimited.
      {
        pages = 1;
        setSceneRect(query.value(0).toInt());
      }
      else
        pages = qCeil(query.value(0).toDouble() / qMax(1, i));
    }

  QApplication::restoreOverrideCursor();
  pages = qMax(1, pages);
  int old_page = pc.page->currentText().toInt();
  pc.page->blockSignals(true);
  pc.page->clear();

  for (int i = 1; i <= pages; i++)
    pc.page->addItem(QString::number(i));

  pc.page->blockSignals(false);
  showPhotographs(old_page - 1);
  pc.page->setCurrentIndex(fmax(old_page - 1, 0));
  QMessageBox::information(this,
                           tr("BiblioteQ: Information"),
                           tr("Imported a total of %1 image(s) from the "
                              "directory %2.")
                               .arg(imported)
                               .arg(dialog.directory().absolutePath()));
  QApplication::processEvents();
}

void biblioteq_photographcollection::slotInsertItem(void)
{
  int temp;
  if (!verifyItemFields())
    return;

  QApplication::setOverrideCursor(Qt::WaitCursor);

  if (!qmain->getDB().transaction())
  {
    QApplication::restoreOverrideCursor();
    qmain->addError(QString(tr("Database Error")),
                    QString(tr("Unable to create a database transaction.")),
                    qmain->getDB().lastError().text(), __FILE__, __LINE__);
    QMessageBox::critical(m_photo_diag, tr("BiblioteQ: Database Error"),
                          tr("Unable to create a database transaction."));
    QApplication::processEvents();
    return;
  }
  else
    QApplication::restoreOverrideCursor();

  QSqlQuery query(qmain->getDB());
  QString errorstr("");
  int pages = 1;

  query.prepare("INSERT INTO photograph "
                "(id, collection_oid, title, executing_artist, creation_date, "
                "creation_date_original, technique, title_original_picture, "
                "based_on_artist, title_old, title_description, delivery_number, notes, signed, "
                "format, printer, catalogue, inventor_new, inventor_old, inventory_number, "
                "keywords, material, page_number, place_of_storage, image, image_scaled, "
                "creators, copyright, reproduction_number, myoid) "
                "VALUES (?, ?, ?, ?, ?, "
                "?, ?, ?, "
                "?, ?, ?, ?, ?, ?, "
                "?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");

  QString blanky(' ');
  query.bindValue(0, photo.id_item->text());
  query.bindValue(1, m_oid);
  query.bindValue(2, photo.title_item->text());
  query.bindValue(3, photo.executing_artist_item->toPlainText());
  query.bindValue(4, photo.creation_date_item->text().trimmed());
  query.bindValue(5, photo.creation_date_original_item->text().trimmed());
  query.bindValue(6, photo.technique_item->text().trimmed());
  query.bindValue(7, photo.title_original_picture_item->toPlainText());
  query.bindValue(8, photo.based_on_artist_item->text().trimmed());
  query.bindValue(9, photo.title_old_item->text().trimmed());
  query.bindValue(10, photo.title_description_item->text().trimmed());
  query.bindValue(11, photo.delivery_number_item->text().trimmed());
  query.bindValue(12, photo.notes_item->toPlainText().trimmed());
  query.bindValue(13, photo.signed_item->text().trimmed());
  query.bindValue(14, photo.format_item->toPlainText().trimmed());
  query.bindValue(15, photo.printer_item->text().trimmed());
  query.bindValue(16, photo.catalogue_item->toPlainText().trimmed());
  query.bindValue(17, photo.inventor_new_item->text().trimmed());
  query.bindValue(18, photo.inventor_old_item->text().trimmed());
  query.bindValue(19, photo.inventory_number_item->toPlainText().trimmed());
  query.bindValue(20, photo.keywords_item->toPlainText().trimmed());
  query.bindValue(21, photo.material_item->text().trimmed());
  query.bindValue(22, photo.page_number_item->text().trimmed());
  query.bindValue(23, photo.place_of_storage_item->text().trimmed());

  query.bindValue(26, blanky.trimmed());
  query.bindValue(27, blanky.trimmed());
  query.bindValue(28, blanky.trimmed());

  if (!photo.thumbnail_item->m_image.isNull())
  {
    QBuffer buffer;
    QByteArray bytes;
    QImage image;

    buffer.setBuffer(&bytes);

    if (buffer.open(QIODevice::WriteOnly))
    {
      photo.thumbnail_item->m_image.save(&buffer, photo.thumbnail_item->m_imageFormat.toLatin1(), 100);
      query.bindValue(24, bytes.toBase64());
    }
    else
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
      query.bindValue(24, QVariant(QMetaType(QMetaType::QByteArray)));
#else
      query.bindValue(24, QVariant(QVariant::ByteArray));
#endif

    buffer.close();
    bytes.clear();
    image = photo.thumbnail_item->m_image;

    if (!image.isNull())
      image = image.scaled(126, 187, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    if (buffer.open(QIODevice::WriteOnly))
    {
      image.save(&buffer, photo.thumbnail_item->m_imageFormat.toLatin1(), 100);
      query.bindValue(25, bytes.toBase64());
    }
    else
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
      query.bindValue(25, QVariant(QMetaType(QMetaType::QByteArray)));
#else
      query.bindValue(25, QVariant(QVariant::ByteArray));
#endif
  }
  else
  {
    photo.thumbnail_item->m_imageFormat = "";
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    query.bindValue(24, QVariant(QMetaType(QMetaType::QByteArray)));
    query.bindValue(25, QVariant(QMetaType(QMetaType::QByteArray)));
#else
    query.bindValue(24, QVariant(QVariant::ByteArray));
    query.bindValue(25, QVariant(QVariant::ByteArray));
#endif
  }

  auto value = biblioteq_misc_functions::getSqliteUniqueId(qmain->getDB(), errorstr);

  if (errorstr.isEmpty())
  {
    query.bindValue(29, value);
  }

  else
  {
    qmain->addError(QString(tr("Database Error")), QString(tr("Unable to generate a unique integer.")), errorstr);
  }

  QApplication::setOverrideCursor(Qt::WaitCursor);

  if (!query.exec())
  {
    QApplication::restoreOverrideCursor();
    qmain->addError(QString(tr("Database Error")), QString(tr("Unable to create or update the entry.")), query.lastError().text(), __FILE__, __LINE__);
    QMessageBox msgBox;
    auto temp = query.lastError();
    msgBox.setText(temp.text());
    msgBox.exec();
    goto db_rollback;
  }
  else
  {
    if (!qmain->getDB().commit())
    {
      QApplication::restoreOverrideCursor();
      qmain->addError(QString(tr("Database Error")), QString(tr("Unable to commit the current database transaction.")), qmain->getDB().lastError().text(), __FILE__, __LINE__);
      goto db_rollback;
    }

    QApplication::restoreOverrideCursor();
  }

  QApplication::setOverrideCursor(Qt::WaitCursor);
  query.prepare("SELECT COUNT(*) "
                "FROM photograph "
                "WHERE collection_oid = ?");
  query.bindValue(0, m_oid);

  if (query.exec())
  {
    if (query.next())
    {
      updateTablePhotographCount(query.value(0).toInt());

      auto i = photographsPerPage();

      if (i == -1) // Unlimited.
      {
        pages = 1;
        setSceneRect(query.value(0).toInt());
      }
      else
        pages = qCeil(query.value(0).toDouble() / qMax(1, i));
    }
  }
  else
  {
    QMessageBox msgBox;
    auto temp = query.lastError();
    msgBox.setText(temp.text());
    msgBox.exec();
  }

  QApplication::restoreOverrideCursor();
  pages = qMax(1, pages);
  temp = pc.page->currentText().toInt();
  pc.page->blockSignals(true);
  pc.page->clear();

  for (int i = 1; i <= pages; i++)
    pc.page->addItem(QString::number(i));

  pc.page->blockSignals(false);
  showPhotographs(temp - 1);
  pc.page->setCurrentIndex(fmax(temp - 1, 0));
  photo.saveButton->disconnect(SIGNAL(clicked(void)));
  connect(photo.saveButton, SIGNAL(clicked()), this, SLOT(slotModifyItem()));
  return;

db_rollback:

  QApplication::setOverrideCursor(Qt::WaitCursor);

  if (!qmain->getDB().rollback())
    qmain->addError(QString(tr("Database Error")), QString(tr("Rollback failure.")),
                    qmain->getDB().lastError().text(), __FILE__, __LINE__);

  QApplication::restoreOverrideCursor();
  QMessageBox::critical(m_photo_diag, tr("BiblioteQ: Database Error"),
                        tr("Unable to create the item. "
                           "Please verify that "
                           "the item does not already exist."));
  QApplication::processEvents();
}

void biblioteq_photographcollection::slotModifyItem(void)
{
  photo.saveButton->disconnect(SIGNAL(clicked()));
  connect(photo.saveButton, SIGNAL(clicked()), this, SLOT(slotUpdateItem()));
  m_photo_diag->resize(m_photo_diag->width(),
                       qRound(0.95 * size().height()));
  biblioteq_misc_functions::center(m_photo_diag, this);
  photo.id_item->setFocus();
  photo.scrollArea->ensureVisible(0, 0);
  m_photo_diag->show();
}

void biblioteq_photographcollection::slotPageChanged(const int &nr)
{
  pc.page->repaint();
  QApplication::processEvents();
  showPhotographs(nr);
}

void biblioteq_photographcollection::slotSortByChanged(void)
{
  pc.sort_box->repaint();
  QApplication::processEvents();
  auto temp = pc.page->currentText().toInt();
  showPhotographs(temp - 1);
}

void biblioteq_photographcollection::slotPrint(void)
{

  m_html = "<html>";                           // start building the html string
  QImage image(pc.thumbnail_item->getImage()); // Get a copy of the image
  int resolution = 600;
  QPageSize pageSize(QPageSize::Letter);
  auto sizeInPixels = pageSize.sizePixels(resolution).toSizeF();
  QSize size = sizeInPixels.toSize();
  size.setWidth(size.width() / 15);                                          // Set the new width to be smaller than the original width
  image = image.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation); // Scale the image to the new size

  QByteArray byteArray;
  QBuffer buffer(&byteArray);
  if (!image.save(&buffer, "PNG", 100))
  {
    qDebug() << "Error saving image: " << buffer.errorString();
  }
  QString imageBase64 = QString::fromLatin1(byteArray.toBase64().data());

  m_html += "<img src='data:image/png;base64," + imageBase64 + "' style='width: 50%; float: right; margin-left: 10px;' alt='Image Description' /><br>"; // Add the image to the HTML
  // Information about the collection
  m_html += "<b>" + tr("Collection Information:") + "</b><br>";
  m_html += "<b>" + tr("ID:") + "</b> " + pc.id_collection->text().trimmed() + "<br>";
  m_html += "<b>" + tr("Title:") + "</b> " + pc.title_collection->toPlainText().trimmed() + "<br>";
  m_html += "<b>" + tr("Creation Date:") + "</b> " + pc.creation_date->text().trimmed() + "<br>";
  m_html += "<b>" + tr("First Release Date:") + "</b> " + pc.first_release->text().trimmed() + "<br>";
  m_html += "<b>" + tr("Total Number of Pictures:") + "</b> " + pc.total_number->text().trimmed() + "<br>";
  m_html += "<b>" + tr("By Artist:") + "</b> " + pc.by_artist->text().trimmed() + "<br>";
  m_html += "<b>" + tr("Circulation height:") + "</b> " + pc.circulation_height->text().trimmed() + "<br>";
  m_html += "<b>" + tr("Publisher:") + "</b> " + pc.publisher->toPlainText().trimmed() + "<br>";
  m_html += "<b>" + tr("Keywords:") + "</b> " + pc.keywords->toPlainText().trimmed() + "<br>";
  m_html += "<b>" + tr("Notes:") + "</b> " + pc.notes->toPlainText().trimmed() + "<br>";
  m_html += "<b>" + tr("Literature/Catalogue:") + "</b> " + pc.catalogue->toPlainText().trimmed() + "<br>";

  // Information about the item
  m_html += "<br><br><b>" + tr("Item Specific Information:") + "</b><br>";
  m_html += "<b>" + tr("ID:") + "</b> " + pc.id_item->toPlainText().trimmed() + "<br>";
  m_html += "<b>" + tr("Title (new):") + "</b> " + pc.title_new_item->toPlainText().trimmed() + "<br>";
  m_html += "<b>" + tr("Title (old):") + "</b> " + pc.title_old_item->toPlainText().trimmed() + "<br>";
  m_html += "<b>" + tr("Title Description:") + "</b> " + pc.title_description_item->toPlainText().trimmed() + "<br>";
  m_html += "<b>" + tr("Inventor (old):") + "</b> " + pc.inventor_old_item->toPlainText().trimmed() + "<br>";
  m_html += "<b>" + tr("Inventor (new):") + "</b> " + pc.inventor_new_item->toPlainText().trimmed() + "<br>";
  m_html += "<b>" + tr("Based On:") + "</b> " + pc.based_on_artist_item->toPlainText().trimmed() + "<br>";
  m_html += "<b>" + tr("Executing Artist:") + "</b> " + pc.executing_artist_item->toPlainText().trimmed() + "<br>";
  m_html += "<b>" + tr("Printer:") + "</b> " + pc.printer_item->toPlainText().trimmed() + "<br>";
  m_html += "<b>" + tr("Creation Date:") + "</b> " + pc.creation_date_item->toPlainText() + "<br>";
  m_html += "<b>" + tr("Creation Date (Original):") + "</b> " + pc.creation_date_original_item->toPlainText().trimmed() + "<br>";
  m_html += "<b>" + tr("Title (Original):") + "</b> " + pc.title_original_picture_item->toPlainText().trimmed() + "<br>";
  m_html += "<b>" + tr("Inventory Number/Place of Storage:") + "</b> " + pc.inventory_number_item->toPlainText().trimmed() + "<br>";
  m_html += "<b>" + tr("Delivery Number:") + "</b> " + pc.delivery_number_item->toPlainText().trimmed() + "<br>";
  m_html += "<b>" + tr("Page Number:") + "</b> " + pc.page_number_item->toPlainText().trimmed() + "<br>";
  m_html += "<b>" + tr("Technique:") + "</b> " + pc.technique_item->toPlainText().trimmed() + "<br>";
  m_html += "<b>" + tr("Material:") + "</b> " + pc.material_item->toPlainText().trimmed() + "<br>";
  m_html += "<b>" + tr("Format:") + "</b> " + pc.format_item->toPlainText().trimmed() + "<br>";
  m_html += "<b>" + tr("Signed:") + "</b> " + pc.signed_item->toPlainText().trimmed() + "<br>";
  m_html += "<b>" + tr("Keywords:") + "</b> " + pc.keywords_item->toPlainText().trimmed() + "<br>";
  m_html += "<b>" + tr("Literature/Catalogue:") + "</b> " + pc.catalogue_item->toPlainText().trimmed() + "<br>";
  m_html += "<b>" + tr("Notes:") + "</b> " + pc.notes_item->toPlainText().trimmed() + "<br>";
  m_html += "<b>" + tr("Place of Storage:") + "</b> " + pc.place_of_storage_item->toPlainText().trimmed() + "<br>";
  m_html += "</html>";
  print(this);
}

void biblioteq_photographcollection::slotSaveRotatedImage(const QImage &image, const QString &format, const qint64 oid)
{
  if (image.isNull() || oid < 0)
    return;

  QApplication::setOverrideCursor(Qt::WaitCursor);

  QBuffer buffer;
  QByteArray bytes1;

  buffer.setBuffer(&bytes1);

  if (buffer.open(QIODevice::WriteOnly) &&
      image.save(&buffer, format.toUpper().toLatin1().constData(), 100))
  {
    QSqlQuery query(qmain->getDB());

    query.prepare("UPDATE photograph SET "
                  "image = ?, "
                  "image_scaled = ? "
                  "WHERE myoid = ?");
    query.addBindValue(bytes1);

    QBuffer buffer;
    QByteArray bytes2;
    auto i(image);

    buffer.setBuffer(&bytes2);
    buffer.open(QIODevice::WriteOnly);
    i = i.scaled(126, 187, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    if (i.isNull() || !i.save(&buffer,
                              format.toUpper().toLatin1().constData(),
                              100))
      bytes2 = bytes1;

    query.addBindValue(bytes2);
    query.addBindValue(oid);

    if (!query.exec())
      qmain->addError(QString(tr("Database Error")),
                      QString(tr("Unable to update photograph.")),
                      query.lastError().text(), __FILE__, __LINE__);
    else
    {
      auto list(pc.graphicsView->scene()->items(Qt::AscendingOrder));

      for (int i = 0; i < list.size(); i++)
        if (list.at(i)->data(0).toLongLong() == oid)
        {
          auto item = qgraphicsitem_cast<QGraphicsPixmapItem *>(list.at(i));

          if (item)
            item->setPixmap(QPixmap::
                                fromImage(image.scaled(126,
                                                       187,
                                                       Qt::KeepAspectRatio,
                                                       Qt::SmoothTransformation)));

          break;
        }
    }
  }

  QApplication::restoreOverrideCursor();
}

void biblioteq_photographcollection::slotSceneSelectionChanged(void)
{
  auto items(pc.graphicsView->scene()->selectedItems());

  if (items.isEmpty())
  {
    m_itemOid.clear();

    if (pc.exportPhotographsToolButton->menu() &&
        pc.exportPhotographsToolButton->menu()->actions().size() >= 3)
      pc.exportPhotographsToolButton->menu()->actions()[2]->setEnabled(false);

    pc.thumbnail_item->clear();
    pc.id_item->clear();
    pc.title_old_item->clear();
    pc.title_description_item->clear();
    pc.creation_date_item->clear();
    pc.technique_item->clear();
    pc.notes_item->clear();
    pc.title_new_item->clear();
    pc.keywords_item->clear();
    pc.based_on_artist_item->clear();
    pc.executing_artist_item->clear();
    pc.catalogue_item->clear();
    pc.creation_date_original_item->clear();
    pc.delivery_number_item->clear();
    pc.format_item->clear();
    pc.inventor_new_item->clear();
    pc.inventor_old_item->clear();
    pc.page_number_item->clear();
    pc.material_item->clear();
    pc.title_original_picture_item->clear();

    return;
  }

  QApplication::setOverrideCursor(Qt::WaitCursor);

  if (pc.exportPhotographsToolButton->menu() && pc.exportPhotographsToolButton->menu()->actions().size() >= 3)
    pc.exportPhotographsToolButton->menu()->actions()[2]->setEnabled(true);

  QGraphicsPixmapItem *item = nullptr;

  if ((item = qgraphicsitem_cast<QGraphicsPixmapItem *>(items.at(0))))
  {
    m_itemOid = item->data(0).toString();

    QSqlQuery query(qmain->getDB());

    query.setForwardOnly(true);
    query.prepare("SELECT id, "
                  "title, "
                  "executing_artist, "
                  "creation_date, "
                  "creation_date_original, "
                  "technique, "
                  "title_original_picture, "
                  "based_on_artist, "
                  "title_old, "
                  "delivery_number, "
                  "notes, "
                  "signed, "
                  "format, "
                  "printer, "
                  "catalogue, "
                  "inventor_new, "
                  "inventor_old, "
                  "inventory_number, "
                  "keywords, "
                  "material, "
                  "page_number, "
                  "place_of_storage, "
                  "image "
                  "FROM photograph "
                  "WHERE collection_oid = ? AND "
                  "myoid = ?");
    query.bindValue(0, m_oid);
    query.bindValue(1, m_itemOid);

    if (query.exec())
    {
      if (query.next())
      {
        auto record(query.record());
        photo.title_collection_item->setText(pc.title_collection->toPlainText().trimmed());

        for (int i = 0; i < record.count(); i++)
        {
          auto fieldname(record.fieldName(i));
          auto var(record.field(i).value());

          if (fieldname == "id")
          {
            pc.id_item->setText(var.toString().trimmed());
            photo.id_item->setText(var.toString().trimmed());
          }
          else if (fieldname == "title")
          {
            pc.title_new_item->setText(var.toString().trimmed());
            photo.title_item->setText(var.toString().trimmed());
          }
          else if (fieldname == "executing_artist")
          {
            pc.executing_artist_item->setText(var.toString().trimmed());
            photo.executing_artist_item->setPlainText(var.toString().trimmed());
          }
          else if (fieldname == "creation_date")
          {
            pc.creation_date_item->setText(var.toString().trimmed());
            photo.creation_date_item->setText(var.toString().trimmed());
          }
          else if (fieldname == "creation_date_original")
          {
            pc.creation_date_original_item->setText(var.toString().trimmed());
            photo.creation_date_original_item->setText(var.toString().trimmed());
          }
          else if (fieldname == "technique")
          {
            pc.technique_item->setText(var.toString().trimmed());
            photo.technique_item->setText(var.toString().trimmed());
          }
          else if (fieldname == "notes")
          {
            pc.notes_item->setText(var.toString().trimmed());
            photo.notes_item->setPlainText(var.toString().trimmed());
          }
          else if (fieldname == "format")
          {
            pc.format_item->setText(var.toString().trimmed());
            photo.format_item->setPlainText(var.toString().trimmed());
          }
          else if (fieldname == "title_original_picture")
          {
            pc.title_original_picture_item->setText(var.toString().trimmed());
            photo.title_original_picture_item->setPlainText(var.toString().trimmed());
          }
          else if (fieldname == "based_on_artist")
          {
            pc.based_on_artist_item->setText(var.toString().trimmed());
            photo.based_on_artist_item->setText(var.toString().trimmed());
          }
          else if (fieldname == "title_old")
          {
            pc.title_old_item->setText(var.toString().trimmed());
            photo.title_old_item->setText(var.toString().trimmed());
          }
          else if (fieldname == "title_description")
          {
            pc.title_description_item->setText(var.toString().trimmed());
            photo.title_description_item->setText(var.toString().trimmed());
          }
          else if (fieldname == "delivery_number")
          {
            pc.delivery_number_item->setText(var.toString().trimmed());
            photo.delivery_number_item->setText(var.toString().trimmed());
          }
          else if (fieldname == "signed")
          {
            pc.signed_item->setText(var.toString().trimmed());
            photo.signed_item->setText(var.toString().trimmed());
          }
          else if (fieldname == "printer")
          {
            pc.printer_item->setText(var.toString().trimmed());
            photo.printer_item->setText(var.toString().trimmed());
          }
          else if (fieldname == "catalogue")
          {
            pc.catalogue_item->setText(var.toString().trimmed());
            photo.catalogue_item->setPlainText(var.toString().trimmed());
          }
          else if (fieldname == "inventor_new")
          {
            pc.inventor_new_item->setText(var.toString().trimmed());
            photo.inventor_new_item->setText(var.toString().trimmed());
          }
          else if (fieldname == "inventor_old")
          {
            pc.inventor_old_item->setText(var.toString().trimmed());
            photo.inventor_old_item->setText(var.toString().trimmed());
          }
          else if (fieldname == "inventory_number")
          {
            pc.inventory_number_item->setText(var.toString().trimmed());
            photo.inventory_number_item->setPlainText(var.toString().trimmed());
          }
          else if (fieldname == "keywords")
          {
            pc.keywords_item->setText(var.toString().trimmed());
            photo.keywords_item->setPlainText(var.toString().trimmed());
          }
          else if (fieldname == "material")
          {
            pc.material_item->setText(var.toString().trimmed());
            photo.material_item->setText(var.toString().trimmed());
          }
          else if (fieldname == "page_number")
          {
            pc.page_number_item->setText(var.toString().trimmed());
            photo.page_number_item->setText(var.toString().trimmed());
          }
          else if (fieldname == "place_of_storage")
          {
            pc.place_of_storage_item->setText(var.toString().trimmed());
            photo.place_of_storage_item->setText(var.toString().trimmed());
          }
          else if (fieldname == "image")
          {
            if (!record.field(i).isNull())
            {
              pc.thumbnail_item->loadFromData(QByteArray::fromBase64(var.toByteArray()));

              if (pc.thumbnail_item->m_image.isNull())
                pc.thumbnail_item->loadFromData(var.toByteArray());

              photo.thumbnail_item->loadFromData(QByteArray::fromBase64(var.toByteArray()));

              if (photo.thumbnail_item->m_image.isNull())
                photo.thumbnail_item->loadFromData(var.toByteArray());
            }
            else
            {
              pc.thumbnail_item->clear();
              photo.thumbnail_item->clear();
            }
          }
        }
      }
    }
    else
    {
      QMessageBox msgBox;
      auto temp = query.lastError();
      msgBox.setText(temp.text());
      msgBox.exec();
    }
  }

  QApplication::restoreOverrideCursor();
}

void biblioteq_photographcollection::slotSelectAll(void)
{
  QApplication::setOverrideCursor(Qt::WaitCursor);

  QPainterPath painterPath;

  painterPath.addRect(pc.graphicsView->sceneRect());
  pc.graphicsView->scene()->setSelectionArea(painterPath, QTransform());
  QApplication::restoreOverrideCursor();
}

void biblioteq_photographcollection::loadcompareFromItemInNewWindow(QGraphicsPixmapItem *item1, QGraphicsPixmapItem *item2)
{
  if (item1 && item2)
  {
    QDialog *mainWindow = nullptr;
    Ui_photographCompare ui;

    mainWindow = new QDialog(this);
    mainWindow->setAttribute(Qt::WA_DeleteOnClose, true);
    ui.setupUi(mainWindow);
    connect(ui.closeButton, SIGNAL(clicked()), mainWindow, SLOT(close()));
    connect(ui.view_size, SIGNAL(currentIndexChanged(int)), this, SLOT(slotImageViewSizeChanged(int)));

    auto scene = new QGraphicsScene(mainWindow);
    auto scene2 = new QGraphicsScene(mainWindow);

    biblioteq_misc_functions::center(mainWindow, this);
    mainWindow->hide();
    scene->setProperty("view_size", ui.view->viewport()->size());
    scene2->setProperty("view_size", ui.view_2->viewport()->size());
    ui.view->setScene(scene);
    ui.view_2->setScene(scene2);
    loadTwoPhotographFromItem(scene, scene2, item1, item2, ui.view_size->currentText().remove("%").toInt());
    mainWindow->show();
  }
}

void biblioteq_photographcollection::slotSelectImage(void)
{
  QFileDialog dialog(this);
  auto button = qobject_cast<QPushButton *>(sender());

  dialog.setFileMode(QFileDialog::ExistingFile);
  dialog.setDirectory(QDir::homePath());
  dialog.setOption(QFileDialog::DontUseNativeDialog);

  if (button == pc.select_image_collection)
    dialog.setWindowTitle(tr("BiblioteQ: Photograph Collection "
                             "Image Selection"));
  else
    dialog.setWindowTitle(tr("BiblioteQ: Photograph Collection Item "
                             "Image Selection"));

  dialog.exec();
  QApplication::processEvents();

  if (dialog.result() == QDialog::Accepted)
  {
    if (button == pc.select_image_collection)
    {
      pc.thumbnail_collection->clear();
      pc.thumbnail_collection->m_image = QImage(dialog.selectedFiles().value(0));

      if (dialog.selectedFiles().value(0).lastIndexOf(".") > -1)
        pc.thumbnail_collection->m_imageFormat =
            dialog.selectedFiles().value(0).mid(dialog.selectedFiles().value(0).lastIndexOf(".") + 1).toUpper();

      pc.thumbnail_collection->scene()->addPixmap(QPixmap::fromImage(pc.thumbnail_collection->m_image));

      if (!pc.thumbnail_collection->scene()->items().isEmpty())
        pc.thumbnail_collection->scene()->items().at(0)->setFlags(QGraphicsItem::ItemIsSelectable);

      pc.thumbnail_collection->scene()->setSceneRect(pc.thumbnail_collection->scene()->itemsBoundingRect());
    }
    else
    {
      photo.thumbnail_item->clear();
      photo.thumbnail_item->m_image = QImage(dialog.selectedFiles().value(0));

      if (dialog.selectedFiles().value(0).lastIndexOf(".") > -1)
        photo.thumbnail_item->m_imageFormat = dialog.selectedFiles().value(0).mid(dialog.selectedFiles().value(0).lastIndexOf(".") + 1).toUpper();

      photo.thumbnail_item->scene()->addPixmap(QPixmap::fromImage(photo.thumbnail_item->m_image));

      if (!photo.thumbnail_item->scene()->items().isEmpty())
        photo.thumbnail_item->scene()->items().at(0)->setFlags(QGraphicsItem::ItemIsSelectable);

      photo.thumbnail_item->scene()->setSceneRect(photo.thumbnail_item->scene()->itemsBoundingRect());
    }
  }
}

void biblioteq_photographcollection::slotUpdateItem(void)
{
  if (!verifyItemFields())
    return;

  QApplication::setOverrideCursor(Qt::WaitCursor);

  if (!qmain->getDB().transaction())
  {
    QApplication::restoreOverrideCursor();
    qmain->addError(QString(tr("Database Error")),
                    QString(tr("Unable to create a database transaction.")),
                    qmain->getDB().lastError().text(), __FILE__, __LINE__);
    QMessageBox::critical(m_photo_diag, tr("BiblioteQ: Database Error"),
                          tr("Unable to create a database transaction."));
    QApplication::processEvents();
    return;
  }
  else
    QApplication::restoreOverrideCursor();

  QSqlQuery query(qmain->getDB());

  query.prepare("UPDATE photograph SET "
                "id = ?, title = ?, "
                "executing_artist = ?, creation_date = ?, creation_date_original = ?,"
                "technique = ?, title_original_picture = ?, "
                "based_on_artist = ?, title_old = ?, title_description = ?, delivery_number = ?, "
                "notes = ?, signed = ?, format = ?, printer = ?, "
                "catalogue = ?, inventor_new = ?, inventor_old = ?, "
                "inventory_number = ?, keywords = ?, material = ?,  page_number = ?, "
                "place_of_storage = ?, image = ?, image_scaled = ? "
                "WHERE collection_oid = ? AND myoid = ?");
  query.bindValue(0, photo.id_item->text());
  query.bindValue(1, photo.title_item->text());
  query.bindValue(2, photo.executing_artist_item->toPlainText().trimmed());
  query.bindValue(3, photo.creation_date_item->text().trimmed());
  query.bindValue(4, photo.creation_date_original_item->text().trimmed());
  query.bindValue(5, photo.technique_item->text().trimmed());
  query.bindValue(6, photo.title_original_picture_item->toPlainText().trimmed());
  query.bindValue(7, photo.based_on_artist_item->text().trimmed());
  query.bindValue(8, photo.title_old_item->text().trimmed());
  query.bindValue(9, photo.title_description_item->text().trimmed());
  query.bindValue(10, photo.delivery_number_item->text().trimmed());
  query.bindValue(11, photo.notes_item->toPlainText().trimmed());
  query.bindValue(12, photo.signed_item->text().trimmed());
  query.bindValue(13, photo.format_item->toPlainText().trimmed());
  query.bindValue(14, photo.printer_item->text().trimmed());
  query.bindValue(15, photo.catalogue_item->toPlainText().trimmed());
  query.bindValue(16, photo.inventor_new_item->text().trimmed());
  query.bindValue(17, photo.inventor_old_item->text().trimmed());
  query.bindValue(18, photo.inventory_number_item->toPlainText().trimmed());
  query.bindValue(19, photo.keywords_item->toPlainText().trimmed());
  query.bindValue(20, photo.material_item->text().trimmed());
  query.bindValue(21, photo.page_number_item->text().trimmed());
  query.bindValue(22, photo.place_of_storage_item->text().trimmed());

  if (!photo.thumbnail_item->m_image.isNull())
  {
    QBuffer buffer;
    QByteArray bytes;
    QImage image;

    buffer.setBuffer(&bytes);

    if (buffer.open(QIODevice::WriteOnly))
    {
      photo.thumbnail_item->m_image.save(&buffer, photo.thumbnail_item->m_imageFormat.toLatin1(), 100);
      query.bindValue(23, bytes.toBase64());
    }
    else
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
      query.bindValue(23, QVariant(QMetaType(QMetaType::QByteArray)));
#else
      query.bindValue(23, QVariant(QVariant::ByteArray));
#endif

    buffer.close();
    bytes.clear();
    image = photo.thumbnail_item->m_image;

    if (!image.isNull())
      image = image.scaled(126, 187, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    if (buffer.open(QIODevice::WriteOnly))
    {
      image.save(&buffer, photo.thumbnail_item->m_imageFormat.toLatin1(), 100);
      query.bindValue(24, bytes.toBase64());
    }
    else
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
      query.bindValue(24, QVariant(QMetaType(QMetaType::QByteArray)));
#else
      query.bindValue(24, QVariant(QVariant::ByteArray));
#endif
  }
  else
  {
    photo.thumbnail_item->m_imageFormat = "";
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    query.bindValue(23, QVariant(QMetaType(QMetaType::QByteArray)));
    query.bindValue(24, QVariant(QMetaType(QMetaType::QByteArray)));
#else
    query.bindValue(23, QVariant(QVariant::ByteArray));
    query.bindValue(24, QVariant(QVariant::ByteArray));
#endif
  }

  query.bindValue(25, m_oid);
  query.bindValue(26, m_itemOid);
  QApplication::setOverrideCursor(Qt::WaitCursor);

  if (!query.exec())
  {
    QApplication::restoreOverrideCursor();
    qmain->addError(QString(tr("Database Error")), QString(tr("Unable to create or update the entry.")), query.lastError().text(), __FILE__, __LINE__);
    QMessageBox msgBox;
    auto temp = query.lastError();
    msgBox.setText(temp.text());
    msgBox.exec();
    goto db_rollback;
  }
  else
  {
    if (!qmain->getDB().commit())
    {
      QApplication::restoreOverrideCursor();
      qmain->addError(QString(tr("Database Error")), QString(tr("Unable to commit the current database transaction.")), qmain->getDB().lastError().text(), __FILE__, __LINE__);
      goto db_rollback;
    }

    QApplication::restoreOverrideCursor();
    pc.id_item->setText(photo.id_item->text().trimmed());
    pc.title_new_item->setText(photo.title_item->text().trimmed());
    pc.executing_artist_item->setText(photo.executing_artist_item->toPlainText());
    pc.creation_date_item->setText(photo.creation_date_item->text().trimmed());
    pc.technique_item->setText(photo.technique_item->text().trimmed());
    pc.notes_item->setText(photo.notes_item->toPlainText().trimmed());
    pc.signed_item->setText(photo.signed_item->text().trimmed());
    pc.format_item->setText(photo.format_item->toPlainText().trimmed());
    pc.thumbnail_item->setImage(photo.thumbnail_item->m_image);
    pc.creation_date_item->setText(photo.creation_date_original_item->text().trimmed());
    pc.title_original_picture_item->setText(photo.title_original_picture_item->toPlainText().trimmed());
    pc.based_on_artist_item->setText(photo.based_on_artist_item->text().trimmed());
    pc.title_old_item->setText(photo.title_old_item->text().trimmed());
    pc.title_description_item->setText(photo.title_description_item->text().trimmed());
    pc.delivery_number_item->setText(photo.delivery_number_item->text().trimmed());
    pc.printer_item->setText(photo.printer_item->text().trimmed());
    pc.catalogue_item->setText(photo.catalogue_item->toPlainText().trimmed());
    pc.inventor_new_item->setText(photo.inventor_new_item->text().trimmed());
    pc.inventor_old_item->setText(photo.inventor_old_item->text().trimmed());
    pc.inventory_number_item->setText(photo.inventory_number_item->toPlainText().trimmed());
    pc.keywords_item->setText(photo.keywords_item->toPlainText().trimmed());
    pc.material_item->setText(photo.material_item->text().trimmed());
    pc.page_number_item->setText(photo.page_number_item->text().trimmed());
    pc.place_of_storage_item->setText(photo.place_of_storage_item->text().trimmed());
  }

  return;

db_rollback:

  QApplication::setOverrideCursor(Qt::WaitCursor);

  if (!qmain->getDB().rollback())
    qmain->addError(QString(tr("Database Error")), QString(tr("Rollback failure.")),
                    qmain->getDB().lastError().text(), __FILE__, __LINE__);

  QApplication::restoreOverrideCursor();
  QMessageBox::critical(m_photo_diag, tr("BiblioteQ: Database Error"),
                        tr("Unable to update the item. "
                           "Please verify that "
                           "the item does not already exist."));
  QApplication::processEvents();
}

void biblioteq_photographcollection::slotViewContextMenu(const QPoint &pos)
{
  auto item = qgraphicsitem_cast<QGraphicsPixmapItem *>(pc.graphicsView->itemAt(pos));

  if (item)
  {
    item->setSelected(true);

    QAction *action = nullptr;
    QMenu menu(this);

    action = menu.addAction(tr("&Delete Photograph"),
                            this,
                            SLOT(slotDeleteItem(void)));
    action->setData(pos);

    if (m_engWindowTitle != "Modify")
      action->setEnabled(false);

    action = menu.addAction(tr("&Modify Photograph..."),
                            this,
                            SLOT(slotModifyItem(void)));
    action->setData(pos);

    if (m_engWindowTitle != "Modify")
      action->setEnabled(false);

    menu.addSeparator();
    action = menu.addAction(tr("&View Photograph..."),
                            this,
                            SLOT(slotViewPhotograph(void)));
    action->setData(pos);
    action = menu.addAction(tr("&Show Comparison..."),
                            this,
                            SLOT(slotViewCompare(void)));
    action->setData(pos);
    menu.exec(QCursor::pos());
  }
}

void biblioteq_photographcollection::slotViewNextPhotograph(void)
{
  auto toolButton = qobject_cast<QToolButton *>(sender());

  if (!toolButton)
    return;

  auto parent = toolButton->parentWidget();

  do
  {
    if (!parent)
      break;

    if (qobject_cast<QMainWindow *>(parent))
      break;

    parent = parent->parentWidget();
  } while (true);

  if (!parent)
    return;

  auto comboBox = parent->findChild<QComboBox *>();
  auto scene = parent->findChild<QGraphicsScene *>();
  auto percent = comboBox ? comboBox->currentText().remove("%").toInt() : 100;

  if (scene)
  {
    auto item = qgraphicsitem_cast<QGraphicsPixmapItem *>(scene->items().value(0));

    if (item)
    {
      QApplication::setOverrideCursor(Qt::WaitCursor);
      auto list(pc.graphicsView->scene()->items(Qt::AscendingOrder));
      int idx = -1;

      for (int i = 0; i < list.size(); i++)
        if (item->data(0) == list.at(i)->data(0))
        {
          idx = i;
          break;
        }

      idx += 1;

      if (idx >= list.size())
        idx = 0;

      QApplication::restoreOverrideCursor();
      loadPhotographFromItem(scene, qgraphicsitem_cast<QGraphicsPixmapItem *>(list.value(idx)), percent);
    }
  }
}

void biblioteq_photographcollection::slotViewPhotograph(void)
{
  auto action = qobject_cast<QAction *>(sender());
  QPoint pos;

  if (action)
    pos = action->data().toPoint();

  if (pos.isNull())
    pos = pc.graphicsView->mapFromGlobal(QCursor::pos());

  auto i = qgraphicsitem_cast<biblioteq_graphicsitempixmap *>(pc.graphicsView->itemAt(pos));

  loadPhotographFromItemInNewWindow(i);
}

void biblioteq_photographcollection::slotShowCompare(void)
{
  QList tl = pc.graphicsView->scene()->selectedItems();
  auto temp = tl[0];
  auto i1 = qgraphicsitem_cast<biblioteq_graphicsitempixmap *>(temp);
  tl = pc.thumbnail_collection->items();
  temp = tl[0];
  auto i2 = qgraphicsitem_cast<biblioteq_graphicsitempixmap *>(temp);

  loadcompareFromItemInNewWindow(i1, i2);
}

void biblioteq_photographcollection::slotViewCompare(void)
{
  auto action = qobject_cast<QAction *>(sender());
  QPoint pos;

  if (action)
    pos = action->data().toPoint();

  if (pos.isNull())
    pos = pc.graphicsView->mapFromGlobal(QCursor::pos());

  auto i = qgraphicsitem_cast<biblioteq_graphicsitempixmap *>(pc.graphicsView->itemAt(pos));

  QList tl = pc.thumbnail_collection->items();
  auto temp = tl[0];
  auto i2 = qgraphicsitem_cast<biblioteq_graphicsitempixmap *>(temp);

  loadcompareFromItemInNewWindow(i, i2);
}

void biblioteq_photographcollection::slotViewPreviousPhotograph(void)
{
  auto toolButton = qobject_cast<QToolButton *>(sender());

  if (!toolButton)
    return;

  auto parent = toolButton->parentWidget();

  do
  {
    if (!parent)
      break;

    if (qobject_cast<QMainWindow *>(parent))
      break;

    parent = parent->parentWidget();
  } while (true);

  if (!parent)
    return;

  auto comboBox = parent->findChild<QComboBox *>();
  auto percent = comboBox ? comboBox->currentText().remove("%").toInt() : 100;
  auto scene = parent->findChild<QGraphicsScene *>();

  if (scene)
  {
    auto item = qgraphicsitem_cast<QGraphicsPixmapItem *>(scene->items().value(0));

    if (item)
    {
      QApplication::setOverrideCursor(Qt::WaitCursor);

      auto list(pc.graphicsView->scene()->items(Qt::AscendingOrder));
      int idx = -1;

      for (int i = 0; i < list.size(); i++)
        if (item->data(0) == list.at(i)->data(0))
        {
          idx = i;
          break;
        }

      idx -= 1;

      if (idx < 0)
        idx = list.size() - 1;

      QApplication::restoreOverrideCursor();
      loadPhotographFromItem(scene, qgraphicsitem_cast<QGraphicsPixmapItem *>(list.value(idx)), percent);
    }
  }
}

void biblioteq_photographcollection::storeData(void)
{
  QList<QWidget *> list;
  QString classname("");
  QString objectname("");

  m_widgetValues.clear();
  list << pc.thumbnail_collection
       << pc.id_collection
       << pc.title_collection
       << pc.creation_date
       << pc.total_number
       << pc.notes
       << pc.by_artist
       << pc.publisher
       << pc.keywords
       << pc.circulation_height
       << pc.catalogue
       << pc.first_release;

  foreach (auto widget, list)
  {
    classname = widget->metaObject()->className();
    objectname = widget->objectName();

    if (classname == "QLineEdit")
      m_widgetValues[objectname] = (qobject_cast<QLineEdit *>(widget))->text().trimmed();
    else if (classname == "QComboBox")
      m_widgetValues[objectname] = (qobject_cast<QComboBox *>(widget))->currentText().trimmed();
    else if (classname == "QTextEdit")
      m_widgetValues[objectname] = (qobject_cast<QTextEdit *>(widget))->toPlainText().trimmed();
    else if (classname == "biblioteq_image_drop_site")
      m_imageValues[objectname] = (qobject_cast<biblioteq_image_drop_site *>(widget))->m_image;
  }
}

void biblioteq_photographcollection::updateTablePhotographCount(const int count)
{
  if (m_index->isValid() &&
      (qmain->getTypeFilterString() == "All" ||
       qmain->getTypeFilterString() == "Photograph Collections"))
  {
    qmain->getUI().table->setSortingEnabled(false);
    if (qmain->getTypeFilterString() == "Photograph Collections")
      qmain->getUI().table->horizontalHeader()->setSortIndicator(1, Qt::AscendingOrder);

    auto names(qmain->getUI().table->columnNames());

    for (int i = 0; i < names.size(); i++)
      if (names.at(i) == "Photograph Count")
      {
        qmain->getUI().table->item(m_index->row(), i)->setText(QString::number(count));
        qmain->slotDisplaySummary();
        break;
      }

    qmain->getUI().table->setSortingEnabled(true);
    qmain->getUI().table->updateToolTips(m_index->row());
    if (qmain->getTypeFilterString() == "Photograph Collections")
      qmain->getUI().table->horizontalHeader()->setSortIndicator(1, Qt::AscendingOrder);
  }
}

void biblioteq_photographcollection::updateWindow(const int state)
{
  QString str("");

  if (state == biblioteq::EDITABLE)
  {
    pc.okButton->setVisible(true);
    pc.addItemButton->setEnabled(true);
    pc.importItems->setEnabled(true);
    str = QString(tr("BiblioteQ: Modify Photograph Collection Entry (")) + pc.id_collection->text() + tr(")");
    m_engWindowTitle = "Modify";
    disconnect(m_scene, SIGNAL(deleteKeyPressed()), this, SLOT(slotDeleteItem()));
    connect(m_scene, SIGNAL(deleteKeyPressed()), this, SLOT(slotDeleteItem()));
  }
  else
  {
    pc.okButton->setVisible(false);
    pc.addItemButton->setEnabled(false);
    pc.importItems->setEnabled(false);
    str = QString(tr("BiblioteQ: View Photograph Collection Details (")) +
          pc.id_collection->text() + tr(")");
    m_engWindowTitle = "View";
  }

  setReadOnlyFields(this, state != biblioteq::EDITABLE);
  setWindowTitle(str);
  pc.page->setEnabled(true);
}
