/*
** Includes magazine-specific methods.
*/

/*
** Note: The ISSN is not a unique value.
*/

/*
** -- Local Includes --
*/

#include "ui_locresults.h"
#include "qtbook_magazine.h"

extern qtbook *qmain;
extern QApplication *qapp;

/*
** -- qtbook_magazine() --
*/

qtbook_magazine::qtbook_magazine(QMainWindow *parent,
				 const QStringList &categories,
				 const QStringList &languages,
				 const QStringList &monetary_units,
				 const QStringList &locations,
				 const QString &oidArg,
				 const int rowArg):
  QMainWindow()
{
  QMenu *menu = NULL;
  QPoint p(0, 0);
  QRegExp rx("[0-9][0-9][0-9][0-9]-[0-9][0-9][0-9][0-9X]");
  QValidator *validator1 = NULL;

  if((menu = new QMenu()) == NULL)
    qtbook::quit("Memory allocation failure", __FILE__, __LINE__);

  if((validator1 = new QRegExpValidator(rx, this)) == NULL)
    qtbook::quit("Memory allocation failure", __FILE__, __LINE__);

  thread = NULL;
  oid = oidArg;
  row = rowArg;
  subType = "Magazine";
  oldq = misc_functions::getColumnString
    (qmain->getUI().table, row, "Quantity").toInt();
  ma.setupUi(this);
  updateFont(qapp->font(), (QWidget *) this);
  connect(ma.okButton, SIGNAL(clicked(void)), this, SLOT(slotGo(void)));
  connect(ma.showUserButton, SIGNAL(clicked(void)), this,
	  SLOT(slotShowUsers(void)));
  connect(ma.queryButton, SIGNAL(clicked(void)), this,
	  SLOT(slotQuery(void)));
  connect(ma.cancelButton, SIGNAL(clicked(void)), this,
	  SLOT(slotCancel(void)));
  connect(ma.copiesButton, SIGNAL(clicked(void)), this,
	  SLOT(slotPopulateCopiesEditor(void)));
  connect(ma.resetButton, SIGNAL(clicked(void)), this,
	  SLOT(slotReset(void)));
  connect(ma.printButton, SIGNAL(clicked(void)), this, SLOT(slotPrint(void)));
  connect(menu->addAction("Reset &ISSN"),
	  SIGNAL(triggered(void)), this, SLOT(slotReset(void)));
  connect(menu->addAction("Reset &Volume"),
	  SIGNAL(triggered(void)), this, SLOT(slotReset(void)));
  connect(menu->addAction("Reset &Issue"),
	  SIGNAL(triggered(void)), this, SLOT(slotReset(void)));
  connect(menu->addAction("Reset &LC Control Number"),
	  SIGNAL(triggered(void)), this, SLOT(slotReset(void)));
  connect(menu->addAction("Reset &Call Number"),
	  SIGNAL(triggered(void)), this, SLOT(slotReset(void)));
  connect(menu->addAction("Reset &Dewey Number"),
	  SIGNAL(triggered(void)), this, SLOT(slotReset(void)));
  connect(menu->addAction("Reset &Title"),
	  SIGNAL(triggered(void)), this, SLOT(slotReset(void)));
  connect(menu->addAction("Reset &Publication Date"),
	  SIGNAL(triggered(void)), this, SLOT(slotReset(void)));
  connect(menu->addAction("Reset &Publisher"),
	  SIGNAL(triggered(void)), this, SLOT(slotReset(void)));
  connect(menu->addAction("Reset &Category"),
	  SIGNAL(triggered(void)), this, SLOT(slotReset(void)));
  connect(menu->addAction("Reset &Price"),
	  SIGNAL(triggered(void)), this, SLOT(slotReset(void)));
  connect(menu->addAction("Reset &Language"),
	  SIGNAL(triggered(void)), this, SLOT(slotReset(void)));
  connect(menu->addAction("Reset &Monetary Units"),
	  SIGNAL(triggered(void)), this, SLOT(slotReset(void)));
  connect(menu->addAction("Reset &Copies"),
	  SIGNAL(triggered(void)), this, SLOT(slotReset(void)));
  connect(menu->addAction("Reset &Location"),
	  SIGNAL(triggered(void)), this, SLOT(slotReset(void)));
  connect(menu->addAction("Reset &Abstract"),
	  SIGNAL(triggered(void)), this, SLOT(slotReset(void)));
  ma.id->setCursorPosition(0);
  ma.id->setValidator(validator1);
  ma.resetButton->setMenu(menu);
  ma.category->addItems(categories);
  ma.language->addItems(languages);
  ma.monetary_units->addItems(monetary_units);
  ma.location->addItems(locations);

  if(ma.category->count() == 0)
    ma.category->addItem("UNKNOWN");

  if(ma.language->count() == 0)
    ma.language->addItem("UNKNOWN");

  if(ma.monetary_units->count() == 0)
    ma.monetary_units->addItem("UNKNOWN");

  if(ma.location->count() == 0)
    ma.location->addItem("UNKNOWN");

  /*
  ** Save some palettes.
  */

  dt_orig_pal = ma.publication_date->palette();
  te_orig_pal = ma.description->viewport()->palette();

  /*
  ** Prepare the form.
  */

  resize(baseSize());
  p = parent->mapToGlobal(p);
  move(p.x() + parent->width() / 2  - width() / 2,
       p.y() + parent->height() / 2 - height() / 2);
}

/*
** -- ~qtbook_magazine() --
*/

qtbook_magazine::~qtbook_magazine()
{
}

/*
** -- slotGo() --
*/

void qtbook_magazine::slotGo(void)
{
  int i = 0;
  int newq = 0;
  int maxcopynumber = 0;
  QString str = "";
  QString errorstr = "";
  QString searchstr = "";
  QSqlQuery query(qmain->getDB());
  QTableWidgetItem *column = NULL;

  if(windowTitle().contains("Create") ||
     windowTitle().contains("Modify"))
    {
      if(windowTitle().contains("Modify") && row > -1)
	{
	  newq = ma.quantity->value();
	  qapp->setOverrideCursor(Qt::WaitCursor);
	  maxcopynumber = misc_functions::getMaxCopyNumber
	    (qmain->getDB(), oid, "magazine", errorstr);

	  if(maxcopynumber < 0)
	    {
	      qapp->restoreOverrideCursor();
	      qmain->addError
		(QString("Database Error"),
		 QString("Unable to determine the maximum copy number of "
			 "the item."),
		 errorstr, __FILE__, __LINE__);
	      QMessageBox::critical
		(this, "BiblioteQ: Database Error",
		 "Unable to determine the maximum copy number of "
		 "the item.");
	      return;
	    }

	  qapp->restoreOverrideCursor();

	  if(newq < maxcopynumber)
	    {
	      QMessageBox::critical
		(this, "BiblioteQ: User Error",
		 "It appears that you are decreasing the "
		 "number of copies while there are copies "
		 "that have been reserved.");
	      ma.quantity->setValue(oldq);
	      return;
	    }
	  else if(newq > oldq)
	    if(QMessageBox::question
	       (this, "BiblioteQ: Question",
		"Would you like to modify copy information?",
		QMessageBox::Yes | QMessageBox::No,
		QMessageBox::No) == QMessageBox::Yes)
	      slotPopulateCopiesEditor();
	}

      qapp->setOverrideCursor(Qt::WaitCursor);

      if(!qmain->getDB().transaction())
	{
	  qapp->restoreOverrideCursor();
	  qmain->addError
	    (QString("Database Error"),
	     QString("Unable to create a database transaction."),
	     qmain->getDB().lastError().text(), __FILE__, __LINE__);
	  QMessageBox::critical
	    (this, "BiblioteQ: Database Error",
	     "Unable to create a database transaction.");
	  return;
	}

      qapp->restoreOverrideCursor();
      str = ma.id->text().trimmed();
      ma.id->setText(str);

      if(ma.id->text().length() != 9)
	{
	  QMessageBox::critical(this, "BiblioteQ: User Error",
				"Please complete the ISSN field.");
	  ma.id->setFocus();
	  goto db_rollback;
	}

      str = ma.title->text().trimmed();
      ma.title->setText(str);

      if(ma.title->text().isEmpty())
	{
	  QMessageBox::critical(this, "BiblioteQ: User Error",
				"Please complete the Title field.");
	  ma.title->setFocus();
	  goto db_rollback;
	}

      str = ma.publisher->text().trimmed();
      ma.publisher->setText(str);

      if(ma.publisher->text().isEmpty())
	{
	  QMessageBox::critical(this, "BiblioteQ: User Error",
				"Please complete the Publisher field.");
	  ma.publisher->setFocus();
	  goto db_rollback;
	}

      str = ma.description->toPlainText().trimmed();
      ma.description->setPlainText(str);

      if(ma.description->toPlainText().isEmpty())
	{
	  QMessageBox::critical(this, "BiblioteQ: User Error",
				"Please complete the Abstract field.");
	  ma.description->setFocus();
	  goto db_rollback;
	}

      str = ma.lcnum->text().trimmed();
      ma.lcnum->setText(str);
      str = ma.callnum->text().trimmed();
      ma.callnum->setText(str);
      str = ma.deweynum->text().trimmed();
      ma.deweynum->setText(str);

      if(windowTitle().contains("Modify"))
	query.prepare(QString("UPDATE magazine SET "
			      "id = ?, "
			      "title = ?, "
			      "pdate = ?, "
			      "publisher = ?, "
			      "category = ?, price = ?, "
			      "description = ?, "
			      "language = ?, "
			      "monetary_units = ?, "
			      "quantity = ?, "
			      "location = ?, "
			      "mag_volume = ?, "
			      "mag_no = ?, "
			      "lccontrolnumber = ?, "
			      "callnumber = ?, "
			      "deweynumber = ? "
			      "WHERE "
			      "myoid = ?"));
      else
	query.prepare(QString("INSERT INTO magazine "
			      "(id, "
			      "title, "
			      "pdate, publisher, "
			      "category, price, description, language, "
			      "monetary_units, quantity, "
			      "location, mag_volume, mag_no, "
			      "lccontrolnumber, callnumber, deweynumber, "
			      "type) "
			      "VALUES (?, ?, "
			      "?, ?, "
			      "?, ?, ?, "
			      "?, ?, "
			      "?, ?, ?, ?, ?, ?, ?, ?)"));

      query.bindValue(0, ma.id->text());
      query.bindValue(1, ma.title->text());
      query.bindValue(2, ma.publication_date->date().toString("MM/dd/yyyy"));
      query.bindValue(3, ma.publisher->text());
      query.bindValue(4, ma.category->currentText().trimmed());
      query.bindValue(5, ma.price->text());
      query.bindValue(6, ma.description->toPlainText());
      query.bindValue(7, ma.language->currentText().trimmed());
      query.bindValue(8, ma.monetary_units->currentText().trimmed());
      query.bindValue(9, ma.quantity->text());
      query.bindValue(10, ma.location->currentText().trimmed());
      query.bindValue(11, ma.volume->text());
      query.bindValue(12, ma.issue->text());
      query.bindValue(13, ma.lcnum->text());
      query.bindValue(14, ma.callnum->text());
      query.bindValue(15, ma.deweynum->text());

      if(windowTitle().contains("Modify"))
	query.bindValue(16, oid);
      else
	query.bindValue(16, subType);

      qapp->setOverrideCursor(Qt::WaitCursor);

      if(!query.exec())
	{
	  qapp->restoreOverrideCursor();
	  qmain->addError(QString("Database Error"),
			  QString("Unable to create or update the entry."),
			  query.lastError().text(), __FILE__, __LINE__);
	  QMessageBox::critical
	    (this, "BiblioteQ: Database Error",
	     "Unable to create or update the entry. "
	     "Please verify that "
	     "the entry does not already exist and "
	     "that you "
	     "have completed all of the required fields.");
	  goto db_rollback;
	}
      else
	{
	  /*
	  ** Remove copies if the quantity has been decreased.
	  */

	  if(windowTitle().contains("Modify"))
	    {
	      query.prepare(QString("DELETE FROM magazine_copy_info WHERE "
				    "copy_number > ? AND "
				    "item_oid = "
				    "?"));
	      query.bindValue(0, ma.quantity->text());
	      query.bindValue(1, oid);

	      if(!query.exec())
		{
		  qapp->restoreOverrideCursor();
		  qmain->addError(QString("Database Error"),
				  QString("Unable to purge unnecessary copy "
					  "data."),
				  query.lastError().text(), __FILE__,
				  __LINE__);
		  QMessageBox::critical(this,
					"BiblioteQ: Database Error",
					"Unable to purge unnecessary "
					"copy data.");
		  goto db_rollback;
		}

	      if(!qmain->getDB().commit())
		{
		  qapp->restoreOverrideCursor();
		  qmain->addError
		    (QString("Database Error"),
		     QString("Unable to commit the current database "
			     "transaction."),
		     qmain->getDB().lastError().text(), __FILE__, __LINE__);
		  QMessageBox::critical(this, "BiblioteQ: Database Error",
					"Unable to commit the current "
					"database transaction.");
		  goto db_rollback;
		}
	    }
	  else
	    {
	      /*
	      ** Create initial copies.
	      */

	      misc_functions::createInitialCopies
		(ma.id->text() + "," + ma.volume->text() + "," +
		 ma.issue->text(), ma.quantity->value(),
		 qmain->getDB(), "magazine", errorstr);

	      if(!errorstr.isEmpty())
		{
		  qapp->restoreOverrideCursor();
		  qmain->addError
		    (QString("Database Error"),
		     QString("Unable to create initial copies."),
		     errorstr, __FILE__, __LINE__);
		  QMessageBox::critical(this,
					"BiblioteQ: Database Error",
					"Unable to create initial copies.");
		  goto db_rollback;
		}

	      if(!qmain->getDB().commit())
		{
		  qapp->restoreOverrideCursor();
		  qmain->addError
		    (QString("Database Error"),
		     QString("Unable to commit the current database "
			     "transaction."),
		     qmain->getDB().lastError().text(), __FILE__,
		     __LINE__);
		  QMessageBox::critical(this, "BiblioteQ: Database Error",
					"Unable to commit the current "
					"database transaction.");
		  goto db_rollback;
		}
	    }

	  foreach(QLineEdit *textfield, findChildren<QLineEdit *>())
	    textfield->setPalette(ma.id->palette());

	  ma.publication_date->setPalette(dt_orig_pal);
	  ma.description->viewport()->setPalette(te_orig_pal);
	  qapp->restoreOverrideCursor();
	  oldq = ma.quantity->value();

	  if(windowTitle().contains("Modify"))
	    {
	      str = QString("BiblioteQ: Modify %1 Entry (%2)").arg
		(subType).arg(ma.id->text());
	      setWindowTitle(str);

	      if((qmain->getUI().typefilter->currentText() == "All" ||
		  qmain->getUI().typefilter->currentText() == "Journals" ||
		  qmain->getUI().typefilter->currentText() == "Magazines") &&
		 oid == misc_functions::getColumnString(qmain->getUI().table,
							row, "OID"))
		{
		  for(i = 0; i < qmain->getUI().table->columnCount(); i++)
		    {
		      column = qmain->getUI().table->horizontalHeaderItem(i);

		      if(column == NULL)
			continue;

		      if(column->text() == "ISSN" ||
			 column->text() == "ID Number")
			qmain->getUI().table->item(row, i)->setText
			  (ma.id->text());
		      else if(column->text() == "Title")
			qmain->getUI().table->item(row, i)->setText
			  (ma.title->text());
		      else if(column->text() == "Publication Date")
			qmain->getUI().table->item(row, i)->setText
			  (ma.publication_date->date().toString("MM/dd/yyyy"));
		      else if(column->text() == "Publisher")
			qmain->getUI().table->item(row, i)->setText
			  (ma.publisher->text());
		      else if(column->text() == "Category")
			qmain->getUI().table->item(row, i)->setText
			  (ma.category->currentText().trimmed());
		      else if(column->text() == "Price")
			qmain->getUI().table->item(row, i)->setText
			  (ma.price->text());
		      else if(column->text() == "Language")
			qmain->getUI().table->item(row, i)->setText
			  (ma.language->currentText().trimmed());
		      else if(column->text() == "Monetary Units")
			qmain->getUI().table->item(row, i)->setText
			  (ma.monetary_units->currentText().trimmed());
		      else if(column->text() == "Quantity")
			qmain->getUI().table->item(row, i)->setText
			  (ma.quantity->text());
		      else if(column->text() == "Location")
			qmain->getUI().table->item(row, i)->setText
			  (ma.location->currentText().trimmed());
		      else if(column->text() == "Volume")
			qmain->getUI().table->item(row, i)->setText
			  (ma.volume->text());
		      else if(column->text() == "Issue")
			qmain->getUI().table->item(row, i)->setText
			  (ma.issue->text());
		      else if(column->text() == "LC Control Number")
			qmain->getUI().table->item(row, i)->setText
			  (ma.lcnum->text());
		      else if(column->text() == "Call Number")
			qmain->getUI().table->item(row, i)->setText
			  (ma.callnum->text());
		      else if(column->text() == "Dewey Number")
			qmain->getUI().table->item(row, i)->setText
			  (ma.deweynum->text());
		      else if(column->text() == "Availability")
			{
			  qmain->getUI().table->item(row, i)->setText
			    (misc_functions::getAvailability
			     (oid, qmain->getDB(), "magazine",
			      errorstr));

			  if(!errorstr.isEmpty())
			    qmain->addError
			      (QString("Database Error"),
			       QString("Retrieving availability."),
			       errorstr, __FILE__, __LINE__);
			}
		    }

		  foreach(QLineEdit *textfield, findChildren<QLineEdit *>())
		    textfield->setCursorPosition(0);

		  if(qmain->getUI().actionAutoResizeColumns->isChecked())
		    qmain->slotResizeColumns();
		}
	    }
	  else
	    {
	      qapp->setOverrideCursor(Qt::WaitCursor);
	      oid = misc_functions::getOID(ma.id->text() + "," +
					   ma.volume->text() + "," +
					   ma.issue->text(),
					   "magazine",
					   qmain->getDB(),
					   errorstr);
	      qapp->restoreOverrideCursor();

	      if(!errorstr.isEmpty())
		{
		  oid = "insert";
		  qmain->addError(QString("Database Error"),
				  QString("Unable to retrieve the "
					  "%1's OID.").arg
				  (subType.toLower()),
				  errorstr, __FILE__, __LINE__);
		  QMessageBox::critical
		    (this, "BiblioteQ: Database Error",
		     QString("Unable to retrieve the "
			     "%1's OID.").arg(subType.toLower()));
		}
	      else if(subType == "Journal")
		qmain->replaceJournal
		  ("insert", (qobject_cast<qtbook_journal *>)(this));
	      else if(subType == "Magazine")
		qmain->replaceMagazine("insert", this);

	      updateWindow(qtbook::EDITABLE);

	      if(qmain->getUI().actionAutoPopulateOnCreation->isChecked())
		(void) qmain->populateTable
		  (qtbook::POPULATE_ALL,
		   qmain->getUI().typefilter->findText
		   (QString("%1s").arg(subType)),
		   QString(""));

	      raise();
	    }
	}

      return;

    db_rollback:

      qapp->setOverrideCursor(Qt::WaitCursor);

      if(!qmain->getDB().rollback())
	qmain->addError(QString("Database Error"),
			QString("Rollback failure."),
			qmain->getDB().lastError().text(), __FILE__,
			__LINE__);

      qapp->restoreOverrideCursor();
    }
  else
    {
      searchstr = QString("SELECT magazine.title, "
			  "magazine.publisher, magazine.pdate, "
			  "magazine.mag_volume, "
			  "magazine.mag_no, "
			  "magazine.category, magazine.language, "
			  "magazine.id, "
			  "magazine.price, magazine.monetary_units, "
			  "magazine.quantity, "
			  "magazine.location, "
			  "magazine.lccontrolnumber, "
			  "magazine.callnumber, "
			  "magazine.deweynumber, "
			  "magazine.quantity - COUNT(magazine_borrower."
			  "item_oid) "
			  "AS availability, "
			  "magazine.type, "
			  "magazine.myoid "
			  "FROM "
			  "magazine LEFT JOIN magazine_borrower ON "
			  "magazine.myoid = magazine_borrower.item_oid "
			  "WHERE magazine.type = '%1' AND ").arg(subType);
      searchstr.append("id LIKE '%" + ma.id->text() + "%' AND ");
      searchstr.append("LOWER(lccontrolnumber) LIKE '%" +
		       myqstring::escape(ma.lcnum->text().toLower()) +
		       "%' AND ");
      searchstr.append("LOWER(callnumber) LIKE '%" +
		       myqstring::escape(ma.callnum->text().toLower()) +
		       "%' AND ");
      searchstr.append("LOWER(deweynumber) LIKE '%" +
		       myqstring::escape(ma.deweynum->text().toLower()) +
		       "%' AND ");

      if(ma.volume->value() != -1)
	searchstr.append("mag_volume = " + ma.volume->text() +
			 " AND ");

      if(ma.issue->value() != -1)
	searchstr.append("mag_no = " + ma.issue->text() +
			 " AND ");

      searchstr.append("LOWER(title) LIKE '%" +
		       myqstring::escape(ma.title->text().toLower()) +
		       "%' AND ");

      if(ma.publication_date->date().toString
	 ("MM/dd/yyyy") != "01/01/7999")
	searchstr.append("pdate = '" +
			 ma.publication_date->date().toString
			 ("MM/dd/yyyy") +
			 "' AND ");

      searchstr.append("LOWER(publisher) LIKE '%" +
		       myqstring::escape(ma.publisher->text().toLower()) +
		       "%' AND ");

      if(ma.category->currentText() != "Any")
	searchstr.append("category = '" +
			 myqstring::escape(ma.category->currentText()) +
			 "' AND ");

      if(ma.price->value() > 0)
	{
	  searchstr.append("price = ");
	  searchstr.append(ma.price->text());
	  searchstr.append(" AND ");
	}

      if(ma.language->currentText() != "Any")
	searchstr.append("language = '" +
			 myqstring::escape(ma.language->currentText()) +
			 "' AND ");

      if(ma.monetary_units->currentText() != "Any")
	searchstr.append("monetary_units = '" +
			 myqstring::escape
			 (ma.monetary_units->currentText()) +
			 "' AND ");

      searchstr.append("LOWER(description) LIKE '%" +
		       myqstring::escape
		       (ma.description->toPlainText().toLower()) + "%' ");

      if(ma.quantity->value() != 0)
	searchstr.append("AND quantity = " + ma.quantity->text() + " ");

      if(ma.location->currentText() != "Any")
	searchstr.append("AND location = '" +
			 myqstring::escape
			 (ma.location->currentText()) + "' ");

      if(subType == "Journal")
	{
	  hide();
	  qmain->removeJournal(qobject_cast<qtbook_journal *>(this));
	}
      else
	slotCancel();

      /*
      ** Search the database.
      */

      (void) qmain->populateTable
	(qtbook::POPULATE_SEARCH,
	 qmain->getUI().typefilter->findText
	 (QString("%1s").arg(subType)), searchstr);
    }
}

/*
** -- search() --
*/

void qtbook_magazine::search(void)
{
  ma.id->clear();
  ma.lcnum->clear();
  ma.callnum->clear();
  ma.deweynum->clear();
  ma.title->clear();
  ma.publisher->clear();
  ma.description->clear();
  ma.copiesButton->setEnabled(false);
  ma.showUserButton->setEnabled(false);
  ma.queryButton->setEnabled(false);
  ma.okButton->setText("&Search");
  ma.publication_date->setDate(QDate::fromString("01/01/7999",
						 "MM/dd/yyyy"));
  ma.id->setCursorPosition(0);
  ma.price->setMinimum(0.00);
  ma.price->setValue(0.00);
  ma.quantity->setMinimum(0);
  ma.quantity->setValue(0);
  ma.volume->setMinimum(-1);
  ma.volume->setValue(-1);
  ma.issue->setMinimum(-1);
  ma.issue->setValue(-1);

  /*
  ** Add "any".
  */

  if(ma.language->findText("Any") == -1)
    ma.language->insertItem(0, "Any");

  if(ma.category->findText("Any") == -1)
    ma.category->insertItem(0, "Any");

  if(ma.monetary_units->findText("Any") == -1)
    ma.monetary_units->insertItem(0, "Any");

  if(ma.location->findText("Any") == -1)
    ma.location->insertItem(0, "Any");

  ma.location->setCurrentIndex(0);
  ma.category->setCurrentIndex(0);
  ma.language->setCurrentIndex(0);
  ma.monetary_units->setCurrentIndex(0);
  setWindowTitle(QString("BiblioteQ: Database %1 Search").arg(subType));
  ma.id->setFocus();
  show();
}

/*
** -- updateWindow() --
*/

void qtbook_magazine::updateWindow(const int state)
{
  QString str = "";

  if(state == qtbook::EDITABLE)
    {
      ma.showUserButton->setEnabled(true);
      ma.copiesButton->setEnabled(true);
      ma.queryButton->setVisible(true);
      ma.okButton->setVisible(true);
      ma.resetButton->setVisible(true);
      str = QString("BiblioteQ: Modify %1 Entry "
		    "(%2)").arg(subType).arg(ma.id->text());
    }
  else
    {
      ma.showUserButton->setEnabled(true);
      ma.copiesButton->setEnabled(false);
      ma.queryButton->setVisible(false);
      ma.okButton->setVisible(false);
      ma.resetButton->setVisible(false);
      str = QString("BiblioteQ: View %1 Details "
		    "(%2)").arg(subType).arg(ma.id->text());
    }

  setWindowTitle(str);
}

/*
** -- modify() --
*/

void qtbook_magazine::modify(const int state)
{
  int i = 0;
  QString str = "";
  QString fieldname = "";
  QString searchstr = "";
  QVariant var;
  QSqlQuery query(qmain->getDB());

  if(state == qtbook::EDITABLE)
    {
      setWindowTitle(QString("BiblioteQ: Modify %1 Entry").arg(subType));
      ma.showUserButton->setEnabled(true);
      ma.copiesButton->setEnabled(true);
      ma.queryButton->setVisible(true);
      ma.okButton->setVisible(true);
      ma.resetButton->setVisible(true);
    }
  else
    {
      setWindowTitle(QString("BiblioteQ: View %1 Details").arg(subType));
      ma.showUserButton->setEnabled(true);
      ma.copiesButton->setEnabled(false);
      ma.queryButton->setVisible(false);
      ma.okButton->setVisible(false);
      ma.resetButton->setVisible(false);
    }

  ma.quantity->setMinimum(1);
  ma.queryButton->setEnabled(true);
  ma.price->setMinimum(0.01);
  ma.okButton->setText("&Save");
  ma.volume->setMinimum(0);
  ma.issue->setMinimum(0);
  str = oid;
  searchstr = "SELECT title, "
    "publisher, pdate, mag_volume, "
    "category, language, id, "
    "price, monetary_units, quantity, "
    "mag_no, "
    "location, lccontrolnumber, callnumber, deweynumber, description "
    "FROM "
    "magazine "
    "WHERE myoid = ";
  searchstr.append(str);
  qapp->setOverrideCursor(Qt::WaitCursor);

  if(!query.exec(searchstr) || !query.next())
    {
      qapp->restoreOverrideCursor();
      qmain->addError
	(QString("Database Error"),
	 QString("Unable to retrieve the selected %1's data.").arg
	 (subType.toLower()),
	 query.lastError().text(), __FILE__, __LINE__);
      QMessageBox::critical
	(this, "BiblioteQ: Database Error",
	 QString("Unable to retrieve the selected %1's "
		 "data.").arg(subType.toLower()));
      return;
    }
  else
    {
      qapp->restoreOverrideCursor();

      for(i = 0; i < query.record().count(); i++)
	{
	  var = query.record().field(i).value();
	  fieldname = query.record().fieldName(i);

	  if(fieldname == "title")
	    ma.title->setText(var.toString());
	  else if(fieldname == "publisher")
	    ma.publisher->setText(var.toString());
	  else if(fieldname == "pdate")
	    ma.publication_date->setDate
	      (QDate::fromString(var.toString(), "MM/dd/yyyy"));
	  else if(fieldname == "price")
	    ma.price->setValue(var.toDouble());
	  else if(fieldname == "category")
	    {
	      if(ma.category->findText(var.toString()) > -1)
		ma.category->setCurrentIndex
		  (ma.category->findText(var.toString()));
	      else
		ma.category->setCurrentIndex
		  (ma.category->findText("UNKNOWN"));
	    }
	  else if(fieldname == "language")
	    {
	      if(ma.language->findText(var.toString()) > -1)
		ma.language->setCurrentIndex
		  (ma.language->findText(var.toString()));
	      else
		ma.language->setCurrentIndex
		  (ma.language->findText("UNKNOWN"));
	    }
	  else if(fieldname == "quantity")
	    ma.quantity->setValue(var.toInt());
	  else if(fieldname == "monetary_units")
	    {
	      if(ma.monetary_units->findText(var.toString()) > -1)
		ma.monetary_units->setCurrentIndex
		  (ma.monetary_units->findText(var.toString()));
	      else
		ma.monetary_units->setCurrentIndex
		  (ma.monetary_units->findText("UNKNOWN"));
	    }
	  else if(fieldname == "mag_volume")
	    ma.volume->setValue(var.toInt());
	  else if(fieldname == "mag_no")
	    ma.issue->setValue(var.toInt());
	  else if(fieldname == "location")
	    {
	      if(ma.location->findText(var.toString()) > -1)
		ma.location->setCurrentIndex
		  (ma.location->findText(var.toString()));
	      else
		ma.location->setCurrentIndex
		  (ma.location->findText("UNKNOWN"));
	    }
	  else if(fieldname == "id")
	    {
	      if(state == qtbook::EDITABLE)
		str = QString("BiblioteQ: Modify %1 Entry (%2)").arg
		  (subType).arg(var.toString());
	      else
		str = QString("BiblioteQ: View %1 Details (%2)").arg
		  (subType).arg(var.toString());

	      setWindowTitle(str);
	      ma.id->setText(var.toString());
	    }
	  else if(fieldname == "description")
	    ma.description->setPlainText(var.toString());
	  else if(fieldname == "lccontrolnumber")
	    ma.lcnum->setText(var.toString());
	  else if(fieldname == "callnumber")
	    ma.callnum->setText(var.toString());
	  else if(fieldname == "deweynumber")
	    ma.deweynum->setText(var.toString());
	}

      foreach(QLineEdit *textfield, findChildren<QLineEdit *>())
	textfield->setCursorPosition(0);
    }

  ma.id->setFocus();
  showNormal();
  raise();
}

/*
** -- insert() --
*/

void qtbook_magazine::insert(void)
{
  ma.id->clear();
  ma.lcnum->clear();
  ma.callnum->clear();
  ma.deweynum->clear();
  ma.title->clear();
  ma.publisher->clear();
  ma.description->clear();
  ma.copiesButton->setEnabled(false);
  ma.queryButton->setEnabled(true);
  ma.okButton->setText("&Save");
  ma.publication_date->setDate(QDate::fromString("01/01/2000",
						 "MM/dd/yyyy"));
  ma.id->setCursorPosition(0);
  ma.price->setMinimum(0.01);
  ma.price->setValue(0.01);
  ma.quantity->setMinimum(1);
  ma.quantity->setValue(1);
  ma.volume->setMinimum(0);
  ma.volume->setValue(0);
  ma.issue->setMinimum(0);
  ma.issue->setValue(0);
  ma.showUserButton->setEnabled(false);
  ma.location->setCurrentIndex(0);
  ma.category->setCurrentIndex(0);
  ma.language->setCurrentIndex(0);
  ma.monetary_units->setCurrentIndex(0);
  setWindowTitle(QString("BiblioteQ: Create %1 Entry").arg(subType));
  ma.id->setFocus();
  show();
}

/*
** -- slotReset() --
*/

void qtbook_magazine::slotReset(void)
{
  QAction *action = qobject_cast<QAction *>(sender());
  QString name = "";

  if(action != NULL)
    {
      name = action->text();

      if(name.contains("ISSN"))
	{
	  ma.id->clear();
	  ma.id->setCursorPosition(0);
	  ma.id->setFocus();
	}
      else if(name.contains("Title"))
	{
	  ma.title->clear();
	  ma.title->setPalette(ma.id->palette());
	  ma.title->setFocus();
	}
      else if(name.contains("Volume"))
	{
	  ma.volume->setValue(ma.volume->minimum());
	  ma.volume->setFocus();
	}
      else if(name.contains("Issue"))
	{
	  ma.issue->setValue(ma.issue->minimum());
	  ma.issue->setFocus();
	}
      else if(name.contains("Publication Date"))
	{
	  if(windowTitle().contains("Search"))
	    ma.publication_date->setDate
	      (QDate::fromString("01/01/7999", "MM/dd/yyyy"));
	  else
	    ma.publication_date->setDate
	      (QDate::fromString("01/01/2000", "MM/dd/yyyy"));

	  ma.publication_date->setPalette(dt_orig_pal);
	  ma.publication_date->setFocus();
	}
      else if(name.contains("Publisher"))
	{
	  ma.publisher->clear();
	  ma.publisher->setPalette(ma.id->palette());
	  ma.publisher->setFocus();
	}
      else if(name.contains("Category"))
	{
	  ma.category->setCurrentIndex(0);
	  ma.category->setFocus();
	}
      else if(name.contains("Price"))
	{
	  ma.price->setValue(ma.price->minimum());
	  ma.price->setFocus();
	}
      else if(name.contains("Language"))
	{
	  ma.language->setCurrentIndex(0);
	  ma.language->setFocus();
	}
      else if(name.contains("Monetary Units"))
	{
	  ma.monetary_units->setCurrentIndex(0);
	  ma.monetary_units->setFocus();
	}
      else if(name.contains("Abstract"))
	{
	  ma.description->clear();
	  ma.description->viewport()->setPalette(te_orig_pal);
	  ma.description->setFocus();
	}
      else if(name.contains("Copies"))
	{
	  ma.quantity->setValue(ma.quantity->minimum());
	  ma.quantity->setFocus();
	}
      else if(name.contains("Location"))
	{
	  ma.location->setCurrentIndex(0);
	  ma.location->setFocus();
	}
      else if(name.contains("LC Control Number"))
	{
	  ma.lcnum->clear();
	  ma.lcnum->setPalette(ma.id->palette());
	  ma.lcnum->setFocus();
	}
      else if(name.contains("Call Number"))
	{
	  ma.callnum->clear();
	  ma.callnum->setPalette(ma.id->palette());
	  ma.callnum->setFocus();
	}
      else if(name.contains("Dewey Number"))
	{
	  ma.deweynum->clear();
	  ma.deweynum->setPalette(ma.id->palette());
	  ma.deweynum->setFocus();
	}
    }
  else
    {
      /*
      ** Reset all.
      */

      ma.id->clear();
      ma.id->setCursorPosition(0);
      ma.title->clear();
      ma.volume->setValue(ma.volume->minimum());
      ma.issue->setValue(ma.issue->minimum());
      ma.price->setValue(ma.price->minimum());

      if(windowTitle().contains("Search"))
	ma.publication_date->setDate
	  (QDate::fromString("01/01/7999", "MM/dd/yyyy"));
      else
	ma.publication_date->setDate
	  (QDate::fromString("01/01/2000", "MM/dd/yyyy"));

      ma.publisher->clear();
      ma.category->setCurrentIndex(0);
      ma.language->setCurrentIndex(0);
      ma.monetary_units->setCurrentIndex(0);
      ma.description->clear();
      ma.quantity->setValue(ma.quantity->minimum());
      ma.location->setCurrentIndex(0);
      ma.lcnum->clear();
      ma.callnum->clear();
      ma.deweynum->clear();

      foreach(QLineEdit *textfield, findChildren<QLineEdit *>())
	textfield->setPalette(ma.id->palette());

      ma.publication_date->setPalette(dt_orig_pal);
      ma.description->viewport()->setPalette(te_orig_pal);
      ma.id->setFocus();
    }
}

/*
** -- closeEvent() --
*/

void qtbook_magazine::closeEvent(QCloseEvent *e)
{
  (void) e;
  qmain->removeMagazine(this);
}

/*
** -- slotCancel() --
*/

void qtbook_magazine::slotCancel(void)
{
  hide();
  qmain->removeMagazine(this);
}

/*
** -- slotPopulateCopiesEditor() --
*/

void qtbook_magazine::slotPopulateCopiesEditor(void)
{
  copy_editor *copyeditor = NULL;

  if((copyeditor = new copy_editor
      (qobject_cast<QWidget *>(this),
       (qtbook_item *) this,
       false,
       ma.quantity->value(), oid,
       ma.id->text(),
       ma.quantity, font(), "magazine",
       ma.title->text().trimmed(), subType)) != NULL)
    copyeditor->populateCopiesEditor();
}

/*
** -- slotShowUsers() --
*/

void qtbook_magazine::slotShowUsers(void)
{
  int state = 0;
  borrowers_editor *borrowerseditor = NULL;

  if(!ma.okButton->isHidden())
    state = qtbook::EDITABLE;
  else
    state = qtbook::VIEW_ONLY;

  if((borrowerseditor = new borrowers_editor
      (qobject_cast<QWidget *>(this), (qtbook_item *) this,
       ma.quantity->value(), oid, ma.id->text(), font(), "magazine",
       state)) != NULL)
    borrowerseditor->showUsers();
}

/*
** -- slotQuery() --
*/

void qtbook_magazine::slotQuery(void)
{
  int i = 0;
  QString etype = "";
  QString errorstr = "";
  QString searchstr = "";
  locresults *dialog = NULL;
  QStringList list;

  if(thread != NULL)
    {
      QMessageBox::critical
	(this, "BiblioteQ: User Error", 
	 "A query is already in progress. Please allow it to complete.");
      return;
    }

  if(ma.id->text().trimmed().length() != 9)
    {
      QMessageBox::critical
	(this, "BiblioteQ: User Error", 
	 "In order to query the Library of Congress, the ISSN "
	 "must be provided.");
      return;
    }

  if((thread = new generic_thread()) != NULL)
    {
      searchstr = QString("@attr 1=8 %1").arg(ma.id->text());
      thread->setType(generic_thread::QUERY_LIBRARY_OF_CONGRESS);
      thread->setLOCSearchString(searchstr);
      thread->start();
      qapp->setOverrideCursor(Qt::WaitCursor);

      while(thread->isRunning())
	{
	  qapp->processEvents();
	  thread->wait(100);
	}

      qapp->restoreOverrideCursor();

      if((errorstr = thread->getErrorStr()).isEmpty())
	{
	  if(thread->getLOCResults().size() == 1)
	    {
	      if(QMessageBox::question
		 (this, "BiblioteQ: Question",
		  "Replace existing values with those retrieved "
		  "from the Library of Congress?",
		  QMessageBox::Yes | QMessageBox::No,
		  QMessageBox::No) == QMessageBox::Yes)
		{
		  list = QString(thread->getLOCResults()[0]).split("\n");
		  populateDisplayAfterLOC(list);
		  list.clear();
		}
	    }
	  else if(thread->getLOCResults().size() > 1)
	    {
	      for(i = 0; i < thread->getLOCResults().size(); i++)
		list.append(thread->getLOCResults()[i]);

	      /*
	      ** Display a selection dialog.
	      */

	      if((dialog = new locresults((QWidget *) this, list,
					  this, font())) == NULL)
		{
		  qmain->addError
		    (QString("Memory Error"),
		     QString("Unable to create a \"dialog\" object "
			     "due to insufficient resources."),
		     QString(""),
		     __FILE__, __LINE__);
		  QMessageBox::critical
		    (this, "BiblioteQ: Memory Error",
		     "Unable to create a \"dialog\" object "
		     "due to insufficient resources.");
		}
	    }
	  else
	    QMessageBox::critical
	      (this, "BiblioteQ: Z39.50 Query Error",
	       "A Library of Congress entry may not yet exist for " +
	       ma.id->text() + ".");
	}
      else
	etype = thread->getEType();

      delete thread;
      thread = NULL;
    }
  else
    {
      etype = "Memory Error";
      errorstr = "Unable to create a thread due to insufficient resources.";
    }

  if(!errorstr.isEmpty())
    {
      qmain->addError(QString("Z39.50 Query Error"), etype, errorstr,
		      __FILE__, __LINE__);
      QMessageBox::critical
	(this, "BiblioteQ: Z39.50 Query Error",
	 "The Library of Congress entry could not be retrieved.");
    }
}

/*
** -- slotPrint() --
*/

void qtbook_magazine::slotPrint(void)
{
  html = "";
  html += "<b>ISSN:</b> " + ma.id->text() + "<br>";
  html += "<b>Volume:</b> " + ma.volume->text() + "<br>";
  html += "<b>Issue (Number):</b> " + ma.issue->text() + "<br>";
  html += "<b>LC Control Number:</b> " + ma.lcnum->text() + "<br>";
  html += "<b>Call Number:</b> " + ma.callnum->text() + "<br>";
  html += "<b>Dewey Class Number:</b> " + ma.deweynum->text() + "<br>";

  /*
  ** General information.
  */

  html += "<b>Title:</b> " + ma.title->text() + "<br>";
  html += "<b>Publication Date:</b> " + ma.publication_date->date().
    toString("MM/dd/yyyy") + "<br>";
  html += "<b>Publisher:</b> " + ma.publisher->text() + "<br>";
  html += "<b>Category:</b> " + ma.category->currentText() + "<br>";
  html += "<b>Price:</b> " + ma.price->text() + "<br>";
  html += "<b>Language:</b> " + ma.language->currentText() + "<br>";
  html += "<b>Monetary Units:</b> " + ma.monetary_units->currentText() +
    "<br>";
  html += "<b>Copies:</b> " + ma.quantity->text() + "<br>";
  html += "<b>Location:</b> " + ma.location->currentText() + "<br>";
  html += "<b>Abstract:</b> " + ma.description->toPlainText();
  print(this);
}

/*
** -- populateDisplayAfterLOC() --
*/

void qtbook_magazine::populateDisplayAfterLOC(const QStringList &list)
{
  int i = 0;
  int j = 0;
  QString str = "";
  QStringList tmplist;

  for(i = 0; i < list.size(); i++)
    {
      str = list[i];

      if(str.startsWith("010"))
	{
	  str = str.mid(str.indexOf("$a") + 2).trimmed();
	  ma.lcnum->setText(str);
	  misc_functions::highlightWidget
	    (ma.lcnum, QColor(162, 205, 90));
	}
      else if(str.startsWith("050"))
	{
	  str = str.mid(str.indexOf("$a") + 2).trimmed();
	  str = str.remove(" $b").trimmed();
	  ma.callnum->setText(str);
	  misc_functions::highlightWidget
	    (ma.callnum, QColor(162, 205, 90));
	}
      else if(str.startsWith("082"))
	{
	  str = str.mid(str.indexOf("$a") + 2).trimmed();
	  str = str.remove(" $2").trimmed();
	  ma.deweynum->setText(str);
	  misc_functions::highlightWidget
	    (ma.deweynum, QColor(162, 205, 90));
	}
      else if(str.startsWith("245"))
	{
	  str = str.mid(str.indexOf("$a") + 2,
			str.indexOf("/") - str.indexOf("$a") - 2).trimmed();

	  /*
	  ** Let's perform some additional massaging.
	  */

	  if(str.contains(" : "))
	    str.replace(" : ", ": ");
	  else if(str.contains(" ; "))
	    str.replace(" ; ", "; ");

	  tmplist = str.split(" ");
	  str = "";

	  for(j = 0; j < tmplist.size(); j++)
	    if(j == 0 || j == tmplist.size() - 1)
	      str += tmplist[j].mid(0, 1).toUpper() +
		tmplist[j].mid(1) + " ";
	    else if(tmplist[j] == "a" || tmplist[j] == "an" ||
		    tmplist[j] == "and" || tmplist[j] == "but" ||
		    tmplist[j] == "of" || tmplist[j] == "or" ||
		    tmplist[j] == "the")
	      {
		if(tmplist[j - 1].contains(":"))
		  str += tmplist[j].mid(0, 1).toUpper() +
		    tmplist[j].mid(1) + " ";
		else
		  str += tmplist[j] + " ";
	      }
	    else
	      str += tmplist[j].mid(0, 1).toUpper() + tmplist[j].mid(1) + " ";

	  tmplist.clear();
	  ma.title->setText(str);
	  misc_functions::highlightWidget
	    (ma.title, QColor(162, 205, 90));
	}
      else if(str.startsWith("260"))
	{
	  ma.publication_date->setDate
	    (QDate::fromString("01/01/" +
			       str.mid(str.trimmed().length() - 5, 4),
			       "MM/dd/yyyy"));
	  ma.publication_date->setStyleSheet
	    ("background-color: rgb(162, 205, 90)");
	  str = str.mid(str.indexOf("$b") + 2).trimmed();
	  str = str.mid(0, str.indexOf(",")).trimmed();
	  ma.publisher->setText(str);
	  misc_functions::highlightWidget
	    (ma.publisher, QColor(162, 205, 90));
	}
      else if(str.startsWith("300"))
	{
	  str = str.mid(str.indexOf("$a") + 2).trimmed();
	  str = str.remove(" $b").trimmed();
	  str = str.remove(" $c").trimmed();
	  ma.description->setPlainText(str);
	  misc_functions::highlightWidget
	    (ma.description->viewport(), QColor(162, 205, 90));
	}
    }

  foreach(QLineEdit *textfield, findChildren<QLineEdit *>())
    textfield->setCursorPosition(0);

  QMessageBox::information(this, "BiblioteQ: Information",
			   "The highlighted fields have been "
			   "modified with values obtained from "
			   "the Library of Congress. "
			   "Please update the remaining "
			   "fields accordingly.");
}

/*
** -- isBusy() --
*/

bool qtbook_magazine::isBusy(void)
{
  if(thread != NULL)
    return true;
  else
    return false;
}
