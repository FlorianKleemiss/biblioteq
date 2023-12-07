#include "biblioteq.h"
#include "biblioteq_dbenumerations.h"
#include "biblioteq_misc_functions.h"

biblioteq_dbenumerations::biblioteq_dbenumerations(biblioteq *parent) : QMainWindow(parent)
{
	m_ui.setupUi(this);
	qmain = parent;
	connect(m_ui.addBookBinding, SIGNAL(clicked()), this, SLOT(slotAdd()));
	connect(m_ui.cancelButton, SIGNAL(clicked()), this, SLOT(slotClose()));
	connect(m_ui.reloadButton, SIGNAL(clicked()), this, SLOT(slotReload()));
	connect(m_ui.removeBookBinding, SIGNAL(clicked()), this, SLOT(slotRemove()));
	connect(m_ui.saveButton, SIGNAL(clicked()), this, SLOT(slotSave()));

	if (qmain)
		connect(qmain, SIGNAL(fontChanged(QFont)), this, SLOT(setGlobalFonts(QFont)));

	setAttribute(Qt::WA_DeleteOnClose, true);
}

void biblioteq_dbenumerations::changeEvent(QEvent *event)
{
	if (event)
		switch (event->type())
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

void biblioteq_dbenumerations::clear(void)
{
	m_listData.clear();
	m_tableData.clear();

	foreach (auto listwidget, findChildren<QListWidget *>())
		listwidget->clear();
}

void biblioteq_dbenumerations::closeEvent(QCloseEvent *event)
{
	QHash<QWidget *, QStringList> listData;
	QHash<QWidget *, QMap<QString, QString>> tableData;

	saveData(listData, tableData);

	if (listData != m_listData || tableData != m_tableData)
	{
		if (QMessageBox::
				question(this, tr("BiblioteQ: Question"),
						 tr("Your changes have not been saved. Continue?"),
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

	QMainWindow::closeEvent(event);
}

void biblioteq_dbenumerations::populateWidgets(void)
{
	clear();

	QList<QPair<QString, QString>> pairList;
	QString errorstr("");
	QStringList list;
	QStringList tables;

	tables << "book_binding_types";

	for (int i = 0; i < tables.size(); i++)
	{
		QListWidget *listwidget = nullptr;
		const auto &str(tables.at(i));

		QApplication::setOverrideCursor(Qt::WaitCursor);

		if (str == "book_binding_types")
		{
			list = biblioteq_misc_functions::getBookBindingTypes(qmain->getDB(),
																 errorstr);
			listwidget = m_ui.bookBindingsList;
		}

		QApplication::restoreOverrideCursor();

		if (!errorstr.isEmpty())
			qmain->addError(QString(tr("Database Error")),
							QString(tr("Unable to retrieve the contents of ")) +
								tables.at(i) + tr("."),
							errorstr, __FILE__, __LINE__);
		else if (listwidget)
			for (int i = 0; i < list.size(); i++)
			{
				QListWidgetItem *item = nullptr;

				item = new QListWidgetItem(list.at(i));
				item->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
				listwidget->addItem(item);
			}

		list.clear();
		pairList.clear();
	}

	m_listData.clear();
	m_tableData.clear();
	saveData(m_listData, m_tableData);
	m_ui.bookBindingsList->setFocus();
}

void biblioteq_dbenumerations::saveData(QHash<QWidget *, QStringList> &listData,
										QHash<QWidget *, QMap<QString, QString>> &tableData)
{
	QApplication::setOverrideCursor(Qt::WaitCursor);

	foreach (auto widget, findChildren<QListWidget *>())
	{
		QStringList list;

		for (int i = 0; i < widget->count(); i++)
		{
			auto item = widget->item(i);

			if (item)
				list << item->text();
		}

		listData[widget] = list;
	}

	foreach (auto table, findChildren<QTableWidget *>())
	{
		QMap<QString, QString> map;

		for (int i = 0; i < table->rowCount(); i++)
		{
			QString text("");
			auto item = table->item(i, 1);

			if (!item)
				continue;
			else
			{
				auto widget = table->cellWidget(i, 0);

				if (widget)
				{
					auto comboBox = widget->findChild<QComboBox *>();

					if (comboBox)
						text = comboBox->currentText();
					else
						continue;
				}
				else if (table->item(i, 0))
					text = table->item(i, 0)->text();
				else
					continue;
			}

			map[text] = item->text();
		}

		tableData[table] = map;
	}

	QApplication::restoreOverrideCursor();
}

void biblioteq_dbenumerations::setGlobalFonts(const QFont &font)
{
	setFont(font);

	foreach (auto widget, findChildren<QWidget *>())
	{
		widget->setFont(font);
		widget->update();
	}
	update();
}

void biblioteq_dbenumerations::show(QMainWindow *parent, const bool populate)
{
	auto wasVisible = isVisible();

#ifdef Q_OS_ANDROID
	Q_UNUSED(parent);
	showMaximized();
#else
	static auto resized = false;

	if (parent && !resized)
		resize(qRound(0.85 * parent->size().width()),
			   qRound(0.85 * parent->size().height()));

	resized = true;
	biblioteq_misc_functions::center(this, parent);
	showNormal();
#endif
	activateWindow();
	raise();
	// m_ui.emptyLabel->setMinimumHeight(m_ui.addCdFormat->height());

	if (populate)
		populateWidgets();
	else
	{
		if (!wasVisible)
		{
			m_listData.clear();
			m_tableData.clear();
			saveData(m_listData, m_tableData);
		}

		m_ui.bookBindingsList->setFocus();
	}
}

void biblioteq_dbenumerations::slotAdd(void)
{
	QListWidget *list = nullptr;
	QListWidgetItem *listItem = nullptr;
	auto toolButton = qobject_cast<QToolButton *>(sender());

	if (toolButton == m_ui.addBookBinding)
	{
		list = m_ui.bookBindingsList;
		listItem = new QListWidgetItem(tr("Book Binding"));
	}

	if (list && listItem)
	{
		listItem->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		list->addItem(listItem);
		list->setCurrentItem(listItem);
		list->editItem(listItem);
	}
	else
		delete listItem;
}

void biblioteq_dbenumerations::slotClose(void)
{
	close();
}

void biblioteq_dbenumerations::slotReload(void)
{
	QHash<QWidget *, QStringList> listData;
	QHash<QWidget *, QMap<QString, QString>> tableData;

	saveData(listData, tableData);

	if (listData != m_listData ||
		tableData != m_tableData)
	{
		if (QMessageBox::
				question(this, tr("BiblioteQ: Question"),
						 tr("Your changes have not been saved. Continue?"),
						 QMessageBox::Yes | QMessageBox::No,
						 QMessageBox::No) == QMessageBox::No)
		{
			QApplication::processEvents();
			return;
		}

		QApplication::processEvents();
	}

	populateWidgets();
}

void biblioteq_dbenumerations::slotRemove(void)
{
	QListWidget *list = nullptr;
	auto toolButton = qobject_cast<QToolButton *>(sender());

	if (toolButton == m_ui.removeBookBinding)
		list = m_ui.bookBindingsList;

	if (list)
		if (list->item(list->currentRow()))
			delete list->takeItem(list->currentRow());
}

void biblioteq_dbenumerations::slotSave(void)
{
	QListWidget *listwidget = nullptr;
	QSqlQuery query(qmain->getDB());
	QString querystr("");
	QStringList tables;
	auto error = false;

	QApplication::setOverrideCursor(Qt::WaitCursor);
	tables << "book_binding_types";

	for (int i = 0; i < tables.size(); i++)
	{
		listwidget = nullptr;
		querystr = QString("DELETE FROM %1").arg(tables.at(i));

		if (!qmain->getDB().transaction())
		{
			error = true;
			qmain->addError(QString(tr("Database Error")),
							QString(tr("Unable to create a database transaction.")),
							qmain->getDB().lastError().text(), __FILE__, __LINE__);
			continue;
		}

		if (!query.exec(querystr))
		{
			qmain->addError(QString(tr("Database Error")),
							QString(tr("An error occurred while attempting to "
									   "remove entries from the %1 table.")
										.arg(tables.at(i))),
							query.lastError().text(), __FILE__, __LINE__);
			goto db_rollback;
		}

		if (i == 0)
			listwidget = m_ui.bookBindingsList;

		if (listwidget)
		{
			for (int j = 0; j < listwidget->count(); j++)
				if (listwidget->item(j))
				{
					query.prepare(QString("INSERT INTO %1 VALUES (?)").arg(tables.at(i)));
					query.bindValue(0,
									listwidget->item(j)->text().trimmed());

					if (!query.exec())
					{
						qmain->addError(QString(tr("Database Error")),
										QString(tr("Unable to create an entry in ")) +
											tables.at(i) + tr("for ") +
											listwidget->item(j)->text().trimmed() +
											QString(tr(".")),
										query.lastError().text(), __FILE__, __LINE__);
						goto db_rollback;
					}
				}
		}

		if (!qmain->getDB().commit())
			qmain->addError(QString(tr("Database Error")),
							QString(tr("Unable to commit the current database "
									   "transaction.")),
							qmain->getDB().lastError().text(), __FILE__,
							__LINE__);
		else
			continue;

	db_rollback:

		error = true;

		if (!qmain->getDB().rollback())
			qmain->addError(QString(tr("Database Error")),
							QString(tr("Rollback failure.")),
							qmain->getDB().lastError().text(),
							__FILE__, __LINE__);
	}

	QApplication::restoreOverrideCursor();

	if (error)
	{
		QMessageBox::critical(this, tr("BiblioteQ: Database Error"),
							  tr("An error occurred while attempting to save "
								 "the database enumerations."));
		QApplication::processEvents();
	}
	else
		populateWidgets();
}
