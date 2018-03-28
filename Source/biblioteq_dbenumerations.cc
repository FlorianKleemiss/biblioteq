/*
** -- Local Includes --
*/

#include "biblioteq.h"
#include "biblioteq_dbenumerations.h"
#include "biblioteq_misc_functions.h"

extern biblioteq *qmain;

biblioteq_dbenumerations::biblioteq_dbenumerations(QWidget *parent):
  QMainWindow(parent)
{
  m_ui.setupUi(this);
#ifdef Q_OS_MAC
#if QT_VERSION < 0x050000
  setAttribute(Qt::WA_MacMetalStyle, BIBLIOTEQ_WA_MACMETALSTYLE);
#endif
#endif
  connect(m_ui.saveButton,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotSave(void)));
  connect(m_ui.cancelButton,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotClose(void)));
  connect(m_ui.reloadButton,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotReload(void)));
  connect(m_ui.addBookBinding,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotAdd(void)));
  connect(m_ui.removeBookBinding,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotRemove(void)));
  connect(m_ui.addCdFormat,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotAdd(void)));
  connect(m_ui.removeCdFormat,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotRemove(void)));
  connect(m_ui.addDvdAspectRatio,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotAdd(void)));
  connect(m_ui.removeDvdAspectRatio,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotRemove(void)));
  connect(m_ui.addDvdRating,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotAdd(void)));
  connect(m_ui.removeDvdRating,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotRemove(void)));
  connect(m_ui.addDvdRegion,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotAdd(void)));
  connect(m_ui.removeDvdRegion,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotRemove(void)));
  connect(m_ui.addLanguage,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotAdd(void)));
  connect(m_ui.removeLanguage,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotRemove(void)));
  connect(m_ui.addLocation,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotAdd(void)));
  connect(m_ui.removeLocation,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotRemove(void)));
  connect(m_ui.addMonetaryUnit,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotAdd(void)));
  connect(m_ui.removeMonetaryUnit,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotRemove(void)));
  connect(m_ui.addVideoGamePlatform,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotAdd(void)));
  connect(m_ui.removeVideoGamePlatform,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotRemove(void)));
  connect(m_ui.addVideoGameRating,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotAdd(void)));
  connect(m_ui.removeVideoGameRating,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotRemove(void)));
  connect(m_ui.addGreyLiteratureType,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotAdd(void)));
  connect(m_ui.removeGreyLiteratureType,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotRemove(void)));
#if QT_VERSION >= 0x050000
  m_ui.locationsTable->verticalHeader()->setSectionResizeMode
    (QHeaderView::Fixed);
  m_ui.minimumDaysTable->verticalHeader()->setSectionResizeMode
    (QHeaderView::Fixed);
#else
  m_ui.locationsTable->verticalHeader()->setResizeMode(QHeaderView::Fixed);
  m_ui.minimumDaysTable->verticalHeader()->setResizeMode(QHeaderView::Fixed);
#endif
}

void biblioteq_dbenumerations::show(QMainWindow *parent, const bool populate)
{
  bool wasVisible = isVisible();

  static bool resized = false;

  if(parent && !resized)
    resize(qRound(0.95 * parent->size().width()),
	   parent->size().height());

  resized = true;
  biblioteq_misc_functions::center(this, parent);
  showNormal();
  activateWindow();
  raise();
  m_ui.emptyLabel->setMinimumHeight(m_ui.addCdFormat->height());

  if(populate)
    populateWidgets();
  else
    {
      if(!wasVisible)
	{
	  m_listData.clear();
	  m_tableData.clear();
	  saveData(m_listData, m_tableData);
	}

      m_ui.bookBindingsList->setFocus();
    }
}

void biblioteq_dbenumerations::clear(void)
{
  m_listData.clear();
  m_tableData.clear();

  foreach(QListWidget *listwidget, findChildren<QListWidget *> ())
    listwidget->clear();

  while(m_ui.locationsTable->rowCount() > 0)
    m_ui.locationsTable->removeRow(0);

  m_ui.minimumDaysTable->clearSelection();
}

void biblioteq_dbenumerations::slotClose(void)
{
  close();
}

void biblioteq_dbenumerations::populateWidgets(void)
{
  clear();

  QList<QPair<QString, QString> > pairList;
  QString errorstr("");
  QStringList list;
  QStringList tables;

  tables << "book_binding_types"
	 << "cd_formats"
	 << "dvd_aspect_ratios"
	 << "dvd_ratings"
	 << "dvd_regions"
	 << "grey_literature_types"
	 << "languages"
	 << "locations"
	 << "minimum_days"
	 << "monetary_units"
	 << "videogame_platforms"
	 << "videogame_ratings";

  for(int i = 0; i < tables.size(); i++)
    {
      QListWidget *listwidget = 0;
      QString str(tables.at(i));
      QTableWidget *tablewidget = 0;

      QApplication::setOverrideCursor(Qt::WaitCursor);

      if(str == "book_binding_types")
	{
	  list = biblioteq_misc_functions::getBookBindingTypes(qmain->getDB(),
							       errorstr);
	  listwidget = m_ui.bookBindingsList;
	}
      else if(str == "cd_formats")
	{
	  list = biblioteq_misc_functions::getCDFormats(qmain->getDB(),
							errorstr);
	  listwidget = m_ui.cdFormatsList;
	}
      else if(str == "dvd_aspect_ratios")
	{
	  list = biblioteq_misc_functions::getDVDAspectRatios(qmain->getDB(),
							      errorstr);
	  listwidget = m_ui.dvdAspectRatiosList;
	}
      else if(str == "dvd_ratings")
	{
	  list = biblioteq_misc_functions::getDVDRatings(qmain->getDB(),
							 errorstr);
	  listwidget = m_ui.dvdRatingsList;
	}
      else if(str == "dvd_regions")
	{
	  list = biblioteq_misc_functions::getDVDRegions(qmain->getDB(),
							 errorstr);
	  listwidget = m_ui.dvdRegionsList;
	}
      else if(str == "grey_literature_types")
	{
	  list = biblioteq_misc_functions::getGreyLiteratureTypes
	    (qmain->getDB(), errorstr);
	  listwidget = m_ui.greyLiteratureTypes;
	}
      else if(str == "languages")
	{
	  list = biblioteq_misc_functions::getLanguages(qmain->getDB(),
							errorstr);
	  listwidget = m_ui.languagesList;
	}
      else if(str == "locations")
	{
	  pairList = biblioteq_misc_functions::getLocations(qmain->getDB(),
							    errorstr);
	  tablewidget = m_ui.locationsTable;
	}
      else if(str == "minimum_days")
	{
	  list = biblioteq_misc_functions::getMinimumDays(qmain->getDB(),
							  errorstr);
	  tablewidget = m_ui.minimumDaysTable;
	}
      else if(str == "monetary_units")
	{
	  list = biblioteq_misc_functions::getMonetaryUnits(qmain->getDB(),
							    errorstr);
	  listwidget = m_ui.monetaryUnitsList;
	}
      else if(str == "videogame_platforms")
	{
	  list = biblioteq_misc_functions::getVideoGamePlatforms(qmain->getDB(),
								 errorstr);
	  listwidget = m_ui.videoGamePlatformsList;
	}
      else if(str == "videogame_ratings")
	{
	  list = biblioteq_misc_functions::getVideoGameRatings(qmain->getDB(),
							       errorstr);
	  listwidget = m_ui.videoGameRatingsList;
	}

      QApplication::restoreOverrideCursor();

      if(!errorstr.isEmpty())
	qmain->addError
	  (QString(tr("Database Error")),
	   QString(tr("Unable to retrieve the contents of ")) +
	   tables.at(i) + tr("."),
	   errorstr, __FILE__, __LINE__);
      else if(listwidget)
	while(!list.isEmpty())
	  {
	    QListWidgetItem *item = 0;

	    item = new(std::nothrow) QListWidgetItem(list.takeFirst());

	    if(item)
	      {
		item->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled |
			       Qt::ItemIsSelectable);
		listwidget->addItem(item);
	      }
	  }
      else if(tablewidget == m_ui.locationsTable)
	{
	  m_ui.locationsTable->setRowCount(pairList.size());

	  for(int j = 0; j < pairList.size(); j++)
	    {
	      QComboBox *item1 = new(std::nothrow) QComboBox();
	      QTableWidgetItem *item2 = new(std::nothrow) QTableWidgetItem
		(pairList.at(j).second);

	      if(item1 && item2)
		{
		  QStringList list;

		  list << tr("Book")
		       << tr("DVD")
		       << tr("Grey Literature")
		       << tr("Journal")
		       << tr("Magazine")
		       << tr("Music CD")
		       << tr("Photograph Collection")
		       << tr("Video Game");
		  item1->addItems(list);

		  if(pairList.at(j).first == "Book")
		    item1->setCurrentIndex(0);
		  else if(pairList.at(j).first == "CD")
		    item1->setCurrentIndex(5);
		  else if(pairList.at(j).first == "DVD")
		    item1->setCurrentIndex(1);
		  else if(pairList.at(j).first == "Grey Literature")
		    item1->setCurrentIndex(2);
		  else if(pairList.at(j).first == "Journal")
		    item1->setCurrentIndex(3);
		  else if(pairList.at(j).first == "Magazine")
		    item1->setCurrentIndex(4);
		  else if(pairList.at(j).first == "Photograph Collection")
		    item1->setCurrentIndex(6);
		  else if(pairList.at(j).first == "Video Game")
		    item1->setCurrentIndex(7);

		  list.clear();
		  item2->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled |
				  Qt::ItemIsSelectable);
		  m_ui.locationsTable->setCellWidget(j, 0, item1);
		  m_ui.locationsTable->setItem(j, 1, item2);
		}
	      else
		{
		  if(item1)
		    item1->deleteLater();

		  delete item2;
		}
	    }

	  m_ui.locationsTable->resizeColumnToContents(0);
	}
      else if(tablewidget == m_ui.minimumDaysTable)
	{
	  for(int j = 0; j < list.size(); j++)
	    if(tablewidget->item(j, 1))
	      tablewidget->item(j, 1)->setText(list.at(j));

	  m_ui.minimumDaysTable->resizeColumnToContents(0);
	}

      list.clear();
      pairList.clear();
    }

  m_listData.clear();
  m_tableData.clear();
  saveData(m_listData, m_tableData);
  m_ui.bookBindingsList->setFocus();
}

void biblioteq_dbenumerations::slotReload(void)
{
  QHash<QWidget *, QStringList> listData;
  QHash<QWidget *, QMap<QString, QString> > tableData;

  saveData(listData, tableData);

  if(listData != m_listData ||
     tableData != m_tableData)
    if(QMessageBox::
       question(this, tr("BiblioteQ: Question"),
		tr("Your changes have not been saved. Continue?"),
		QMessageBox::Yes | QMessageBox::No,
		QMessageBox::No) == QMessageBox::No)
      return;

  populateWidgets();
}

void biblioteq_dbenumerations::slotAdd(void)
{
  QListWidget *list = 0;
  QListWidgetItem *listItem = 0;
  QToolButton *toolButton = qobject_cast<QToolButton *> (sender());

  if(toolButton == m_ui.addBookBinding)
    {
      list = m_ui.bookBindingsList;
      listItem = new(std::nothrow) QListWidgetItem(tr("Book Binding"));
    }
  else if(toolButton == m_ui.addCdFormat)
    {
      list = m_ui.cdFormatsList;
      listItem = new(std::nothrow) QListWidgetItem(tr("CD Format"));
    }
  else if(toolButton == m_ui.addDvdAspectRatio)
    {
      list = m_ui.dvdAspectRatiosList;
      listItem = new(std::nothrow) QListWidgetItem(tr("DVD Aspect Ratio"));
    }
  else if(toolButton == m_ui.addDvdRating)
    {
      list = m_ui.dvdRatingsList;
      listItem = new(std::nothrow) QListWidgetItem(tr("DVD Rating"));
    }
  else if(toolButton == m_ui.addDvdRegion)
    {
      list = m_ui.dvdRegionsList;
      listItem = new(std::nothrow) QListWidgetItem(tr("DVD Region"));
    }
  else if(toolButton == m_ui.addGreyLiteratureType)
    {
      list = m_ui.greyLiteratureTypes;
      listItem = new(std::nothrow) QListWidgetItem(tr("Document Type"));
    }
  else if(toolButton == m_ui.addLanguage)
    {
      list = m_ui.languagesList;
      listItem = new(std::nothrow) QListWidgetItem(tr("Language"));
    }
  else if(toolButton == m_ui.addLocation)
    {
      QComboBox *item1 = new(std::nothrow) QComboBox();
      QTableWidgetItem *item2 = new(std::nothrow) QTableWidgetItem();

      if(item1 && item2)
	{
	  QStringList list;

	  list << tr("Book")
	       << tr("DVD")
	       << tr("Grey Literature")
	       << tr("Journal")
	       << tr("Magazine")
	       << tr("Music CD")
	       << tr("Photograph Collection")
	       << tr("Video Game");
	  item1->addItems(list);
	  list.clear();
	  item2->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled |
			  Qt::ItemIsSelectable);
	  m_ui.locationsTable->setRowCount
	    (m_ui.locationsTable->rowCount() + 1);
	  m_ui.locationsTable->setCellWidget
	    (m_ui.locationsTable->rowCount() - 1,
	     0,
	     item1);
	  m_ui.locationsTable->setItem(m_ui.locationsTable->rowCount() - 1,
				       1,
				       item2);
	  m_ui.locationsTable->setCurrentCell
	    (m_ui.locationsTable->rowCount() - 1,
	     0);
	}
      else
	{
	  if(item1)
	    item1->deleteLater();

	  delete item2;
	}
    }
  else if(toolButton == m_ui.addMonetaryUnit)
    {
      list = m_ui.monetaryUnitsList;
      listItem = new(std::nothrow) QListWidgetItem(tr("Monetary Unit"));
    }
  else if(toolButton == m_ui.addVideoGamePlatform)
    {
      list = m_ui.videoGamePlatformsList;
      listItem = new(std::nothrow) QListWidgetItem(tr("Video Game Platform"));
    }
  else if(toolButton == m_ui.addVideoGameRating)
    {
      list = m_ui.videoGameRatingsList;
      listItem = new(std::nothrow) QListWidgetItem(tr("Video Game Rating"));
    }

  if(list && listItem)
    {
      listItem->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled |
			 Qt::ItemIsSelectable);
      list->addItem(listItem);
      list->setCurrentItem(listItem);
      list->editItem(listItem);
    }
  else
    delete listItem;
}

void biblioteq_dbenumerations::slotRemove(void)
{
  QListWidget *list = 0;
  QToolButton *toolButton = qobject_cast<QToolButton *>
    (sender());

  if(toolButton == m_ui.removeBookBinding)
    list = m_ui.bookBindingsList;
  else if(toolButton == m_ui.removeCdFormat)
    list = m_ui.cdFormatsList;
  else if(toolButton == m_ui.removeDvdAspectRatio)
    list = m_ui.dvdAspectRatiosList;
  else if(toolButton == m_ui.removeDvdRating)
    list = m_ui.dvdRatingsList;
  else if(toolButton == m_ui.removeDvdRegion)
    list = m_ui.dvdRegionsList;
  else if(toolButton == m_ui.removeGreyLiteratureType)
    list = m_ui.greyLiteratureTypes;
  else if(toolButton == m_ui.removeLanguage)
    list = m_ui.languagesList;
  else if(toolButton == m_ui.removeLocation)
    m_ui.locationsTable->removeRow(m_ui.locationsTable->currentRow());
  else if(toolButton == m_ui.removeMonetaryUnit)
    list = m_ui.monetaryUnitsList;
  else if(toolButton == m_ui.removeVideoGamePlatform)
    list = m_ui.videoGamePlatformsList;
  else if(toolButton == m_ui.removeVideoGameRating)
    list = m_ui.videoGameRatingsList;

  if(list)
    if(list->item(list->currentRow()))
      delete list->takeItem(list->currentRow());
}

void biblioteq_dbenumerations::slotSave(void)
{
  QListWidget *listwidget = 0;
  QSqlQuery query(qmain->getDB());
  QString querystr("");
  QStringList tables;
  QTableWidget *tablewidget = 0;
  bool error = false;

  QApplication::setOverrideCursor(Qt::WaitCursor);
  tables << "book_binding_types"
	 << "cd_formats"
	 << "dvd_aspect_ratios"
	 << "dvd_ratings"
	 << "dvd_regions"
	 << "languages"
	 << "locations"
	 << "minimum_days"
	 << "monetary_units"
	 << "videogame_platforms"
	 << "videogame_ratings";

  for(int i = 0; i < tables.size(); i++)
    {
      listwidget = 0;
      tablewidget = 0;
      querystr = QString("DELETE FROM %1").arg(tables.at(i));

      if(!qmain->getDB().transaction())
	{
	  error = true;
	  qmain->addError
	    (QString(tr("Database Error")),
	     QString(tr("Unable to create a database transaction.")),
	     qmain->getDB().lastError().text(), __FILE__, __LINE__);
	  continue;
	}

      if(!query.exec(querystr))
	{
	  qmain->addError
	    (QString(tr("Database Error")),
	     QString(tr("An error occurred while attempting to "
			"remove entries from the %1 table.").
		     arg(tables.at(i))),
	     query.lastError().text(), __FILE__, __LINE__);
	  goto db_rollback;
	}

      if(i == 0)
	listwidget = m_ui.bookBindingsList;
      else if(i == 1)
	listwidget = m_ui.cdFormatsList;
      else if(i == 2)
	listwidget = m_ui.dvdAspectRatiosList;
      else if(i == 3)
	listwidget = m_ui.dvdRatingsList;
      else if(i == 4)
	listwidget = m_ui.dvdRegionsList;
      else if(i == 5)
	listwidget = m_ui.languagesList;
      else if(i == 6)
	tablewidget = m_ui.locationsTable;
      else if(i == 7)
	tablewidget = m_ui.minimumDaysTable;
      else if(i == 8)
	listwidget = m_ui.monetaryUnitsList;
      else if(i == 9)
	listwidget = m_ui.videoGamePlatformsList;
      else if(i == 10)
	listwidget = m_ui.videoGameRatingsList;

      if(listwidget)
	{
	  for(int j = 0; j < listwidget->count(); j++)
	    if(listwidget->item(j))
	      {
		query.prepare(QString("INSERT INTO %1 VALUES (?)").
			      arg(tables.at(i)));
		query.bindValue(0,
				listwidget->item(j)->text().trimmed());

		if(!query.exec())
		  {
		    qmain->addError
		      (QString(tr("Database Error")),
		       QString(tr("Unable to create an entry in ")) +
		       tables.at(i) + tr("for ") +
		       listwidget->item(j)->text().trimmed() +
		       QString(tr(".")),
		       query.lastError().text(), __FILE__, __LINE__);
		    goto db_rollback;
		  }
	      }
	}
      else if(tablewidget == m_ui.locationsTable)
	{
	  for(int j = 0; j < tablewidget->rowCount(); j++)
	    if(tablewidget->cellWidget(j, 0) &&
	       tablewidget->item(j, 1))
	      {
		int index = qobject_cast<QComboBox *>
		  (tablewidget->cellWidget(j, 0))->currentIndex();

		if(index == 0)
		  {
		    query.prepare("INSERT INTO locations "
				  "(location, type) VALUES "
				  "(?, 'Book')");
		    query.bindValue(0,
				    tablewidget->item(j, 1)->text().trimmed());
		  }
		else if(index == 1)
		  {
		    query.prepare("INSERT INTO locations "
				  "(location, type) VALUES "
				  "(?, 'DVD')");
		    query.bindValue(0,
				    tablewidget->item(j, 1)->text().trimmed());
		  }
		else if(index == 2)
		  {
		    query.prepare("INSERT INTO locations "
				  "(location, type) VALUES "
				  "(?, 'Grey Literature')");
		    query.bindValue
		      (0, tablewidget->item(j, 1)->text().trimmed());
		  }
		else if(index == 3)
		  {
		    query.prepare("INSERT INTO locations "
				  "(location, type) VALUES "
				  "(?, 'Journal')");
		    query.bindValue(0,
				    tablewidget->item(j, 1)->text().trimmed());
		  }
		else if(index == 4)
		  {
		    query.prepare("INSERT INTO locations "
				  "(location, type) VALUES "
				  "(?, 'Magazine')");
		    query.bindValue(0,
				    tablewidget->item(j, 1)->text().trimmed());
		  }
		else if(index == 5)
		  {
		    query.prepare("INSERT INTO locations "
				  "(location, type) VALUES "
				  "(?, 'CD')");
		    query.bindValue(0,
				    tablewidget->item(j, 1)->text().trimmed());
		  }
		else if(index == 6)
		  {
		    query.prepare("INSERT INTO locations "
				  "(location, type) VALUES "
				  "(?, 'Photograph Collection')");
		    query.bindValue(0,
				    tablewidget->item(j, 1)->text().trimmed());
		  }
		else if(index == 7)
		  {
		    query.prepare("INSERT INTO locations "
				  "(location, type) VALUES "
				  "(?, 'Video Game')");
		    query.bindValue(0,
				    tablewidget->item(j, 1)->text().trimmed());
		  }

		if(!query.exec())
		  {
		    qmain->addError
		      (QString(tr("Database Error")),
		       QString(tr("Unable to create the location (")) +
		       qobject_cast<QComboBox *>
		       (tablewidget->cellWidget(j, 0))->currentText() +
		       tr(", ") +
		       tablewidget->item(j, 1)->text().trimmed() +
		       QString(tr(").")),
		       query.lastError().text(), __FILE__, __LINE__);
		    goto db_rollback;
		  }
	      }
	}
      else if(tablewidget == m_ui.minimumDaysTable)
	{
	  for(int j = 0; j < tablewidget->rowCount(); j++)
	    if(tablewidget->item(j, 1))
	      {
		if(j == 0)
		  {
		    query.prepare("INSERT INTO minimum_days "
				  "(days, type) VALUES "
				  "(?, 'Book')");
		    query.bindValue(0,
				    tablewidget->item(j, 1)->text().trimmed());
		  }
		else if(j == 1)
		  {
		    query.prepare("INSERT INTO minimum_days "
				  "(days, type) VALUES "
				  "(?, 'DVD')");
		    query.bindValue(0,
				    tablewidget->item(j, 1)->text().trimmed());
		  }
		else if(j == 2)
		  {
		    query.prepare("INSERT INTO minimum_days "
				  "(days, type) VALUES "
				  "(?, 'Journal')");
		    query.bindValue(0,
				    tablewidget->item(j, 1)->text().trimmed());
		  }
		else if(j == 3)
		  {
		    query.prepare("INSERT INTO minimum_days "
				  "(days, type) VALUES "
				  "(?, 'Magazine')");
		    query.bindValue(0,
				    tablewidget->item(j, 1)->text().trimmed());
		  }
		else if(j == 4)
		  {
		    query.prepare("INSERT INTO minimum_days "
				  "(days, type) VALUES "
				  "(?, 'CD')");
		    query.bindValue(0,
				    tablewidget->item(j, 1)->text().trimmed());
		  }
		else if(j == 5)
		  {
		    query.prepare("INSERT INTO minimum_days "
				  "(days, type) VALUES "
				  "(?, 'Video Game')");
		    query.bindValue(0,
				    tablewidget->item(j, 1)->text().trimmed());
		  }

		if(!query.exec())
		  {
		    qmain->addError
		      (QString(tr("Database Error")),
		       QString(tr("Unable to create the minimum day (")) +
		       tablewidget->item(j, 0)->text() +
		       tr(", ") +
		       tablewidget->item(j, 1)->text().trimmed() +
		       QString(tr(").")),
		       query.lastError().text(), __FILE__, __LINE__);
		    goto db_rollback;
		  }
	      }
	}

      if(!qmain->getDB().commit())
	qmain->addError
	  (QString(tr("Database Error")),
	   QString(tr("Unable to commit the current database "
		      "transaction.")),
	   qmain->getDB().lastError().text(), __FILE__,
	   __LINE__);
      else
	continue;

    db_rollback:

      error = true;

      if(!qmain->getDB().rollback())
	qmain->addError(QString(tr("Database Error")),
			QString(tr("Rollback failure.")),
			qmain->getDB().lastError().text(),
			__FILE__, __LINE__);
    }

  QApplication::restoreOverrideCursor();

  if(error)
    QMessageBox::critical(this, tr("BiblioteQ: Database Error"),
			  tr("An error occurred while attempting to save "
			     "the database enumerations."));
  else
    populateWidgets();
}

void biblioteq_dbenumerations::changeEvent(QEvent *event)
{
  if(event)
    switch(event->type())
      {
      case QEvent::LanguageChange:
	{
	  m_ui.retranslateUi(this);
	  break;
	}
      default:
	break;
      }

  QMainWindow::changeEvent(event);
}

void biblioteq_dbenumerations::closeEvent(QCloseEvent *event)
{
  QHash<QWidget *, QStringList> listData;
  QHash<QWidget *, QMap<QString, QString> > tableData;

  saveData(listData, tableData);

  if(listData != m_listData ||
     tableData != m_tableData)
    if(QMessageBox::
       question(this, tr("BiblioteQ: Question"),
		tr("Your changes have not been saved. Continue?"),
		QMessageBox::Yes | QMessageBox::No,
		QMessageBox::No) == QMessageBox::No)
      {
	if(event)
	  event->ignore();

	return;
      }

  QMainWindow::closeEvent(event);
}

void biblioteq_dbenumerations::saveData
(QHash<QWidget *, QStringList> &listData,
 QHash<QWidget *, QMap<QString, QString> > &tableData)
{
  QApplication::setOverrideCursor(Qt::WaitCursor);

  foreach(QListWidget *widget, findChildren<QListWidget *> ())
    {
      QStringList list;

      for(int i = 0; i < widget->count(); i++)
	{
	  QListWidgetItem *item = widget->item(i);

	  if(item)
	    list << item->text();
	}

      listData[widget] = list;
    }

  foreach(QTableWidget *widget, findChildren<QTableWidget *> ())
    {
      QMap<QString, QString> map;

      for(int i = 0; i < widget->rowCount(); i++)
	{
	  QString text("");
	  QTableWidgetItem *item = widget->item(i, 1);

	  if(!item)
	    continue;
	  else
	    {
	      if(qobject_cast<QComboBox *> (widget->cellWidget(i, 0)))
		text = qobject_cast<QComboBox *> (widget->cellWidget(i, 0))->
		  currentText();
	      else if(widget->item(i, 0))
		text = widget->item(i, 0)->text();
	      else
		continue;
	    }

	  map[text] = item->text();
	}

      tableData[widget] = map;
    }

  QApplication::restoreOverrideCursor();
}
