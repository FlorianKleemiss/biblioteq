#include "biblioteq.h"
#include "biblioteq_copy_editor.h"
#include "biblioteq_files.h"
#include "biblioteq_graphicsitempixmap.h"
#include "biblioteq_callnum_table_item.h"
#include "biblioteq_otheroptions.h"
#include "biblioteq_numeric_table_item.h"

#include <QActionGroup>
#include <QDesktopServices>
#include <QFileDialog>
#include <QScrollBar>
#include <QSettings>
#include <QSqlDriver>
#include <QSqlField>
#include <QSqlRecord>
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
#include <QTextCodec>
#endif
#include <QTextStream>
#include <QtMath>

QColor biblioteq::availabilityColor(const QString &itemType) const
{
	return m_otheroptions->availabilityColor(itemType);
}

QHash<QString, QString> biblioteq::getOpenLibraryImagesHash(void) const
{
	return m_openLibraryImages;
}

QHash<QString, QString> biblioteq::getOpenLibraryItemsHash(void) const
{
	return m_openLibraryItems;
}

QString biblioteq::dbUserName(void) const
{
	return "SQLITE";
}

QString biblioteq::publicationDateFormat(const QString &itemType) const
{
	return m_otheroptions->publicationDateFormat(itemType);
}

QString biblioteq::viewHtml(void) const
{
	QApplication::setOverrideCursor(Qt::WaitCursor);

	QString html = "<html>";

	html += "<table border=1>";
	html += "<tr>";

	for (int i = 0; i < ui.table->columnCount(); i++)
		if (!ui.table->isColumnHidden(i))
			html += "<th>" + ui.table->horizontalHeaderItem(i)->text() +
					"</th>";

	html += "</tr>";

	for (int i = 0; i < ui.table->rowCount(); i++)
	{
		html += "<tr>";

		for (int j = 0; j < ui.table->columnCount(); j++)
			if (!ui.table->isColumnHidden(j))
				html += "<td>" + ui.table->item(i, j)->text() + "</td>";

		html += "</tr>";
	}

	html += "</table>";
	html += "</html>";
	QApplication::restoreOverrideCursor();
	return html;
}

QStringList biblioteq::getSRUNames(void) const
{
	return m_sruMaps.keys();
}

QStringList biblioteq::getZ3950Names(void) const
{
	return m_z3950Maps.keys();
}

QWidget *biblioteq::widgetForAction(QAction *action) const
{
	if (!action)
		return nullptr;

	if (action == ui.configTool)
		return ui.toolBar_5->widgetForAction(action);
	else if (action == ui.createTool)
		return ui.toolBar_2->widgetForAction(action);
	else if (action == ui.printTool)
		return ui.toolBar_2->widgetForAction(action);
	else if (action == ui.searchTool)
		return ui.toolBar_4->widgetForAction(action);

	return nullptr;
}

bool biblioteq::availabilityColors(void) const
{
	QSettings settings;

	return settings.value("otheroptions/availability_colors", false).toBool();
}

bool biblioteq::emptyContainers(void)
{
	foreach (auto w, QApplication::topLevelWidgets())
	{
		auto book = qobject_cast<biblioteq_book *>(w);
		auto photograph = qobject_cast<biblioteq_photographcollection *>(w);

		if (book)
		{
			if (book->isVisible() && !book->close())
				return false;
			else
				book->deleteLater();
		}

		if (photograph)
		{
			if (photograph->isVisible() && !photograph->close())
				return false;
			else
				photograph->deleteLater();
		}
	}

	return true;
}

int biblioteq::populateTable(QSqlQuery &query,
							 const QString &typefilter,
							 const int pagingType,
							 const int searchType)
{
	if (pagingType == NEW_PAGE)
	{
		if (m_searchQuery.isActive())
			m_searchQuery.clear();

#if (QT_VERSION >= QT_VERSION_CHECK(6, 2, 0))
		m_searchQuery.swap(query);
#else
		m_searchQuery = query;
#endif
	}

	if (m_configToolMenu)
		m_configToolMenu->close();

	m_lastSearchStr = "Item Search Query";

	if (searchType == CUSTOM_QUERY)
	{
		if (m_configToolMenu)
			m_configToolMenu->deleteLater();

		ui.configTool->setEnabled(false);
		ui.configTool->setToolTip(tr("Disabled for custom queries."));
	}
	else
	{
		ui.configTool->setEnabled(true);
		ui.configTool->setToolTip("");
	}

	ui.itemsCountLabel->setText(tr("0 Results"));

	QScopedPointer<QProgressDialog> progress;

	if (m_otheroptions->showMainTableProgressDialogs())
	{
		auto closeButton = new QPushButton(tr("Interrupt"));

		closeButton->setShortcut(QKeySequence(Qt::Key_F8));
		progress.reset(new QProgressDialog(this));
		progress->hide();
		progress->setCancelButton(closeButton);
	}

	QTableWidgetItem *item = nullptr;
	QString itemType("");
	QString str("");
	auto columns = m_otheroptions->iconsViewColumnCount();
	auto limit = pageLimit();
	auto offset = m_queryOffset;
	int i = -1;

	if (limit != -1)
	{
		if (pagingType != NEW_PAGE)
		{
			if (pagingType == PREVIOUS_PAGE)
			{
				offset -= limit;

				if (offset < 0)
					offset = 0;
			}
			else if (pagingType == NEXT_PAGE)
				offset += limit;
			else
			{
				/*
				** A specific page was selected from ui.pagesLabel.
				*/

				offset = 0;

				for (int ii = 1; ii < qAbs(pagingType); ii++)
					offset += limit;
			}
		}
		else
			offset = 0;

		ui.graphicsView->setSceneRect(0.0,
									  0.0,
									  150.0 * static_cast<qreal>(columns),
									  limit / (static_cast<qreal>(columns)) * 200.0 + 200.0);
	}

	QApplication::setOverrideCursor(Qt::WaitCursor);

	if (ui.table->rowCount() == 0)
		ui.itemsCountLabel->setText(tr("0 Results"));
	else
		ui.itemsCountLabel->setText(QString(tr("%1 Result(s)")).arg(ui.table->rowCount()));

	auto ok = true;

	if (pagingType == NEW_PAGE)
		ok = m_searchQuery.exec();
	else if (pagingType == NEXT_PAGE || pagingType == PREVIOUS_PAGE)
		ok = m_searchQuery.seek(static_cast<int>(offset));
	else if (pagingType < 0)
		ok = m_searchQuery.seek(limit * qAbs(pagingType + 1));

	if (m_searchQuery.lastError().isValid() || !ok)
	{
		if (progress)
			progress->close();

		QApplication::processEvents();
		QApplication::restoreOverrideCursor();

		if (!m_previousTypeFilter.isEmpty())
			for (int ii = 0; ii < ui.menu_Category->actions().size();
				 ii++)
				if (m_previousTypeFilter ==
					ui.menu_Category->actions().at(ii)->data().toString())
				{
					ui.categoryLabel->setText(ui.menu_Category->actions().at(ii)->text());
					ui.menu_Category->actions().at(ii)->setChecked(true);
					ui.menu_Category->setDefaultAction(ui.menu_Category->actions().at(ii));
					break;
				}

		if (m_searchQuery.lastError().isValid())
			addError(QString(tr("Database Error")),
					 QString(tr("Unable to retrieve the data required for "
								"populating the main views.")),
					 m_searchQuery.lastError().text(), __FILE__, __LINE__);

		QMessageBox::critical(this, tr("BiblioteQ: Database Error"),
							  tr("Unable to retrieve the data required for "
								 "populating the main views."));
		QApplication::processEvents();
		return 1;
	}

	auto found = false;

	for (int ii = 0; ii < ui.menu_Category->actions().size(); ii++)
		if (typefilter == ui.menu_Category->actions().at(ii)->data().toString())
		{
			found = true;
			m_previousTypeFilter = typefilter;
			ui.categoryLabel->setText(ui.menu_Category->actions().at(ii)->text());
			ui.menu_Category->actions().at(ii)->setChecked(true);
			ui.menu_Category->setDefaultAction(ui.menu_Category->actions().at(ii));
			break;
		}

	if (typefilter.isEmpty())
	{
		ui.categoryLabel->setText(tr("All"));

		if (!ui.menu_Category->actions().isEmpty())
			ui.menu_Category->actions().at(0)->setChecked(true);

		ui.menu_Category->setDefaultAction(ui.menu_Category->actions().value(0));
	}
	else if (!found)
	{
		ui.categoryLabel->setText(tr("All"));

		if (!ui.menu_Category->actions().isEmpty())
			ui.menu_Category->actions().at(0)->setChecked(true);

		ui.menu_Category->setDefaultAction(ui.menu_Category->actions().value(0));
	}

	disconnect(ui.table, SIGNAL(itemChanged(QTableWidgetItem *)), this, SLOT(slotItemChanged(QTableWidgetItem *)));
	ui.table->resetTable(dbUserName(), typefilter);

	qint64 currentPage = 0;

	if (limit <= 0)
		currentPage = 1;
	else
		currentPage = offset / limit + 1;

	if (pagingType == NEW_PAGE)
		m_pages = 0;

	if (pagingType >= 0 &&
		pagingType != PREVIOUS_PAGE &&
		currentPage > m_pages)
		m_pages += 1;

	if (limit == -1)
		m_pages = 1;

	if (m_pages == 1)
		ui.pagesLabel->setText(tr("1"));
	else if (m_pages >= 2 && m_pages <= 10)
	{
		QString str("");

		for (qint64 ii = 1; ii <= m_pages; ii++)
			if (ii == currentPage)
				str += QString(tr(" %1 ")).arg(currentPage);
			else
				str += QString(" <a href=\"%1\">" + tr("%1") + "</a> ").arg(ii);

		str = str.trimmed();
		ui.pagesLabel->setText(str);
	}
	else
	{
		QString str("");
		qint64 start = 2;

		if (currentPage == 1)
			str += tr(" 1 ... ");
		else
			str += " <a href=\"1\">" + tr("1") + "</a>" + tr(" ... ");

		if (currentPage != 1)
			while (!(start <= currentPage && currentPage <= start + 6))
				start += 7;

		for (qint64 ii = start; ii <= start + 6; ii++)
			if (ii == currentPage && ii <= m_pages - 1)
				str += QString(tr(" %1 ")).arg(ii);
			else if (ii <= m_pages - 1)
				str += QString(" <a href=\"%1\">" + tr("%1") + "</a> ").arg(ii);

		if (currentPage == m_pages)
			str += QString(tr(" ... %1 ")).arg(currentPage);
		else
			str += QString(" ... <a href=\"%1\">" + tr("%1") + "</a> ").arg(m_pages);

		str = str.trimmed();
		ui.pagesLabel->setText(str);
	}

	m_lastSearchType = searchType;
	ui.table->scrollToTop();
	ui.table->horizontalScrollBar()->setValue(0);
	ui.table->clearSelection();
	ui.table->setCurrentItem(nullptr);
	slotDisplaySummary();
	ui.graphicsView->scene()->clear();
	ui.graphicsView->resetTransform();
	ui.graphicsView->verticalScrollBar()->setValue(0);
	ui.graphicsView->horizontalScrollBar()->setValue(0);
	ui.table->setSortingEnabled(false);

	if (progress)
	{
		QApplication::restoreOverrideCursor();
		progress->setLabelText(tr("Populating the views..."));

		if (limit == -1)
			progress->setMaximum(0);
		else
			progress->setMaximum(limit);

		progress->setMinimum(0);
		progress->setModal(true);
		progress->setWindowTitle(tr("BiblioteQ: Progress Dialog"));
		raise();
		progress->show();
		progress->update();
		progress->repaint();
		QApplication::processEvents();
	}

	int iconTableColumnIdx = 0;
	int iconTableRowIdx = 0;

	/*
	** Adjust the dimensions of the graphics scene if pagination is disabled.
	*/

	if (limit == -1)
	{
		m_searchQuery.seek(0);

		int size = 0;

		while (m_searchQuery.next())
			size += 1;

		if (size > 0)
			ui.graphicsView->setSceneRect(0.0,
										  0.0,
										  150.0 * static_cast<qreal>(columns),
										  (size / static_cast<qreal>(columns)) * 200.0 + 200.0);

		if (progress && size >= 0)
			progress->setMaximum(size);

		m_searchQuery.seek(static_cast<int>(offset));
	}

	if (limit != -1 &&
		m_db.driver()->hasFeature(QSqlDriver::QuerySize) &&
		progress)
		progress->setMaximum(qMin(limit, m_searchQuery.size()));

	QFontMetrics fontMetrics(ui.table->font());
	QSettings settings;
	QString dateFormat("");
	auto availabilityColors = this->availabilityColors();
	auto columnNames(ui.table->columnNames());
	auto showBookReadStatus = m_db.driverName() == "QSQLITE" &&
							  m_otheroptions->showBookReadStatus() &&
							  typefilter == "Books";
	auto showMainTableImages = m_otheroptions->showMainTableImages();
	auto showToolTips = settings.value("show_maintable_tooltips", false).toBool();

	if (typefilter == "Books" ||
		typefilter == "Photograph Collections")
		dateFormat = publicationDateFormat(QString(typefilter).remove(' ').toLower());

	i = -1;

	while (i++, true)
	{
		if (i == limit)
			break;

		if (progress && progress->wasCanceled())
			break;

		if (m_searchQuery.at() == QSql::BeforeFirstRow)
			if (!m_searchQuery.next())
				break;

		biblioteq_graphicsitempixmap *pixmapItem = nullptr;
		biblioteq_numeric_table_item *availabilityItem = nullptr;
		quint64 myoid = 0;

		if (m_searchQuery.isValid())
		{
			QString tooltip("");
			QTableWidgetItem *first = nullptr;
			auto record(m_searchQuery.record());

			if (showToolTips)
			{
				tooltip = "<html>";

				for (int j = 0; j < record.count(); j++)
				{
					auto fieldName(record.fieldName(j));

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
					if (QMetaType::Type(record.field(j).metaType().id()) ==
							QMetaType::QByteArray ||
#else
					if (record.field(j).type() == QVariant::ByteArray ||
#endif
						fieldName.contains("cover") ||
						fieldName.contains("image"))
						continue;

					QString columnName("");

					if (showBookReadStatus)
						columnName = columnNames.value(j + 1);
					else
						columnName = columnNames.value(j);

					if (columnName.isEmpty())
						columnName = "N/A";

					tooltip.append("<b>");
					tooltip.append(columnName);
					tooltip.append(":</b> ");

#if QT_VERSION > 0x050501
					if (record.field(j).tableName() == "book" &&
						(fieldName == "id" || fieldName == "isbn13"))
#else
					if (fieldName == "id" || fieldName == "isbn13")
#endif
					{
						auto str(m_searchQuery.value(j).toString().trimmed());

						if (fieldName == "id")
							str = m_otheroptions->isbn10DisplayFormat(str);
						else
							str = m_otheroptions->isbn13DisplayFormat(str);

						tooltip.append(str);
					}
					else
						tooltip.append(m_searchQuery.value(j).toString().simplified().replace("<br>", " ").simplified().trimmed());

					tooltip.append("<br>");
				}

				if (tooltip.endsWith("<br>"))
					tooltip = tooltip.mid(0, tooltip.length() - 4);

				tooltip.append("</html>");
			}

			for (int j = 0; j < record.count(); j++)
			{
				item = nullptr;

				auto fieldName(record.fieldName(j));

				if (!fieldName.endsWith("front_cover") &&
					!fieldName.endsWith("image_scaled"))
				{
					if (fieldName.contains("date"))
					{
						auto date(QDate::fromString(m_searchQuery.value(j).toString().trimmed(),
													"MM/dd/yyyy"));

						if (dateFormat.isEmpty())
							str = date.toString(Qt::ISODate);
						else
							str = date.toString(dateFormat);

						if (str.isEmpty())
							str = m_searchQuery.value(j).toString().trimmed();
					}
					else
						str = m_searchQuery.value(j).toString().trimmed();
				}

#if QT_VERSION > 0x050501
				if (record.field(j).tableName() == "book" &&
					(fieldName == "id" || fieldName == "isbn13"))
#else
				if (fieldName == "id" || fieldName == "isbn13")
#endif
				{
					if (fieldName == "id")
						str = m_otheroptions->isbn10DisplayFormat(str);
					else
						str = m_otheroptions->isbn13DisplayFormat(str);

					item = new QTableWidgetItem(str);
				}
				else if (fieldName.endsWith("availability") ||
						 fieldName.endsWith("file_count") ||
						 fieldName.endsWith("issue") ||
						 fieldName.endsWith("issueno") ||
						 fieldName.endsWith("issuevolume") ||
						 fieldName.endsWith("photograph_count") ||
						 fieldName.endsWith("quantity") ||
						 fieldName.endsWith("total_reserved") ||
						 fieldName.endsWith("volume"))
				{
					item = new biblioteq_numeric_table_item(m_searchQuery.value(j).toInt());

					if (availabilityColors &&
						fieldName.endsWith("availability"))
						availabilityItem = dynamic_cast<biblioteq_numeric_table_item *>(item);
				}
				else if (fieldName.endsWith("callnumber"))
				{
					str = m_searchQuery.value(j).toString().trimmed();
					item = new biblioteq_callnum_table_item(str);
				}
				else if (fieldName.endsWith("front_cover") ||
						 fieldName.endsWith("image_scaled"))
				{
					QImage image;

					if (showMainTableImages)
					{
						if (!m_searchQuery.isNull(j))
						{
							image.loadFromData(QByteArray::fromBase64(m_searchQuery.value(j).toByteArray()));

							if (image.isNull())
								image.loadFromData(m_searchQuery.value(j).toByteArray());
						}
					}

					if (image.isNull())
						image = QImage(":/no_image.png");

					/*
					** The size of no_image.png is 126x187.
					*/

					if (!image.isNull())
						image = image.scaled(126, 187, Qt::KeepAspectRatio,
											 Qt::SmoothTransformation);

					pixmapItem = new biblioteq_graphicsitempixmap(QPixmap::fromImage(image), nullptr);

					if (iconTableRowIdx == 0)
						pixmapItem->setPos(140.0 * iconTableColumnIdx, 15.0);
					else
						pixmapItem->setPos(140.0 * iconTableColumnIdx,
										   200.0 * iconTableRowIdx + 15.0);

					pixmapItem->setFlag(QGraphicsItem::ItemIsSelectable, true);
					ui.graphicsView->scene()->addItem(pixmapItem);
					iconTableColumnIdx += 1;

					if (columns <= iconTableColumnIdx)
					{
						iconTableRowIdx += 1;
						iconTableColumnIdx = 0;
					}
				}
				else
					item = new QTableWidgetItem();

				if (item != nullptr)
				{
					item->setText(str.simplified().replace("<br>", " ").simplified().trimmed());
					item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

					if (j == 0)
					{
						first = item;
						ui.table->setRowCount(ui.table->rowCount() + 1);
					}

					if (!tooltip.isEmpty())
						item->setToolTip(tooltip);

					if (showBookReadStatus)
						ui.table->setItem(i, j + 1, item);
					else
						ui.table->setItem(i, j, item);

					if (fieldName.endsWith("type"))
					{
						itemType = str;
						itemType = itemType.toLower().remove(" ");
					}

					if (fieldName.endsWith("myoid"))
					{
						myoid = query.value(j).toULongLong();
						updateRows(str, item, itemType);
					}
				}
			}

			if (availabilityItem && availabilityItem->value() > 0.0)
				availabilityItem->setBackground(availabilityColor(itemType));

			if (first && showMainTableImages)
			{
				if (pixmapItem)
					first->setIcon(pixmapItem->pixmap());
				else
					first->setIcon(QIcon(":/no_image.png"));

				ui.table->setRowHeight(i, qMax(fontMetrics.height() + 10,
											   ui.table->iconSize().height()));
			}

			if (showBookReadStatus)
			{
				/*
				** Was the book read?
				*/

				auto item = new QTableWidgetItem();

				if (itemType == "book")
				{
					item->setCheckState(biblioteq_misc_functions::isBookRead(m_db, myoid) ? Qt::Checked : Qt::Unchecked);
					item->setData(Qt::UserRole, myoid);
					item->setFlags(Qt::ItemIsEnabled |
								   Qt::ItemIsSelectable |
								   Qt::ItemIsUserCheckable);
				}
				else
					item->setFlags(Qt::ItemIsSelectable);

				if (!tooltip.isEmpty())
					item->setToolTip(tooltip);

				ui.table->setItem(i, 0, item);
			}
		}

		if (m_searchQuery.isValid())
			if (pixmapItem)
			{
				pixmapItem->setData(0, myoid);
				pixmapItem->setData(1, itemType);
			}

		if (progress)
		{
			if (i + 1 <= progress->maximum())
				progress->setValue(i + 1);

			progress->repaint();
			QApplication::processEvents();
		}

		if (m_searchQuery.at() != QSql::BeforeFirstRow)
			if (!m_searchQuery.next())
				break;
	}

	ui.itemsCountLabel->setText(QString(tr("%1 Result(s)")).arg(ui.table->rowCount()));

	if (limit != -1 &&
		!m_db.driver()->hasFeature(QSqlDriver::QuerySize) &&
		progress)
		progress->setValue(limit);

	auto wasCanceled = false;

	if (progress)
	{
		progress->wasCanceled(); // QProgressDialog::close()!
		progress->close();
	}

	ui.table->setSortingEnabled(true);

	if (searchType == POPULATE_SEARCH_BASIC)
		if (ui.table->rowCount() == 0)
		{
			ui.case_insensitive->setEnabled(true);
			ui.search->setEnabled(true);
			ui.searchType->setEnabled(true);
		}

	addConfigOptions(typefilter);

	if (ui.actionAutomatically_Resize_Column_Widths->isChecked())
		slotResizeColumns();

	m_queryOffset = offset;
	ui.previousPageButton->setEnabled(m_queryOffset > 0);

	if (ui.table->rowCount() == 0)
		ui.itemsCountLabel->setText(tr("0 Results"));
	else
		ui.itemsCountLabel->setText(QString(tr("%1 Result(s)")).arg(ui.table->rowCount()));

	if (limit == -1)
		ui.nextPageButton->setEnabled(false);
	else if (ui.table->rowCount() < limit)
	{
		if (wasCanceled)
			/*
			** Allow viewing of the next potential page if the user
			** canceled the query.
			*/

			ui.nextPageButton->setEnabled(true);
		else
			ui.nextPageButton->setEnabled(false);
	}
	else
		ui.nextPageButton->setEnabled(true);

#ifdef Q_OS_MACOS
	ui.table->hide();
	ui.table->show();
#endif
	connect(ui.table, SIGNAL(itemChanged(QTableWidgetItem *)), this, SLOT(slotItemChanged(QTableWidgetItem *)));
	QApplication::restoreOverrideCursor();
	return 0;
}

void biblioteq::bookSearch(const QString &field, const QString &value)
{
	auto book = new biblioteq_book(this, "", QModelIndex());

	book->search(field, value);
	book->deleteLater();
}

void biblioteq::createConfigToolMenu(void)
{
	if (!m_configToolMenu)
	{
		m_configToolMenu = new QMenu(this);
#ifndef Q_OS_ANDROID
		m_configToolMenu->setTearOffEnabled(true);
		m_configToolMenu->setWindowIcon(QIcon(":/book.png"));
		m_configToolMenu->setWindowTitle(tr("BiblioteQ"));
#endif
	}
}

void biblioteq::deleteItem(const QString &oid, const QString &itemType)
{
	if (itemType == "book")
	{
		foreach (auto w, QApplication::topLevelWidgets())
		{
			auto book = qobject_cast<biblioteq_book *>(w);

			if (book && book->getID() == oid)
			{
				removeBook(book);
				break;
			}
		}
	}
	else if (itemType == "photograph_collection")
	{
		foreach (auto w, QApplication::topLevelWidgets())
		{
			auto photograph = qobject_cast<biblioteq_photographcollection *>(w);

			if (photograph && photograph->getID() == oid)
			{
				removePhotographCollection(photograph);
				break;
			}
		}
	}
}

void biblioteq::exportAsCSV(biblioteq_main_table *table, const QString &title)
{
	if (!table)
		return;

	QFileDialog dialog(this);

	dialog.setAcceptMode(QFileDialog::AcceptSave);
	dialog.setDefaultSuffix("csv");
	dialog.setDirectory(QDir::homePath());
	dialog.setFileMode(QFileDialog::AnyFile);
	dialog.setNameFilter(tr("CSV (*.csv)"));
	dialog.setOption(QFileDialog::DontUseNativeDialog);
	dialog.setWindowTitle(title);
	dialog.exec();
	QApplication::processEvents();

	if (dialog.result() == QDialog::Accepted)
	{
		QApplication::setOverrideCursor(Qt::WaitCursor);

		QFile file(dialog.selectedFiles().value(0));

		if (file.open(QIODevice::Text |
					  QIODevice::Truncate |
					  QIODevice::WriteOnly))
		{
			QString str("");
			QTextStream stream(&file);

			for (int i = 0; i < table->columnCount(); i++)
				if (!table->isColumnHidden(i))
				{
					if (!table->horizontalHeaderItem(i))
						continue;

					if (table->columnNames().value(i) == "Read Status")
						/*
						** We cannot export the Read Status column because
						** it is not supported by PostgreSQL.
						*/

						continue;

					if (table->horizontalHeaderItem(i)->text().contains(","))
						str += QString("\"%1\",").arg(table->horizontalHeaderItem(i)->text());
					else
						str += QString("%1,").arg(table->horizontalHeaderItem(i)->text());
				}

			if (str.endsWith(","))
				str = str.mid(0, str.length() - 1);

			if (!str.isEmpty())
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
				stream << str << Qt::endl;
#else
				stream << str << endl;
#endif

			for (int i = 0; i < table->rowCount(); i++)
			{
				str = "";

				for (int j = 0; j < table->columnCount(); j++)
					if (!table->isColumnHidden(j))
					{
						/*
						** A lot of awful things will occur if
						** !table->item(i, j) is true!
						*/

						if (!table->item(i, j))
						{
							str += ",";
							continue;
						}

						if (table->columnNames().value(j) == "Read Status")
							/*
							** We cannot export the Read Status column because
							** it is not supported by PostgreSQL.
							*/

							continue;

						auto cleaned(table->item(i, j)->text());

						cleaned.replace("\n", " ");
						cleaned.replace("\r\n", " ");
						cleaned = cleaned.trimmed();

						if (cleaned.contains(","))
							str += QString("\"%1\",").arg(cleaned);
						else
							str += QString("%1,").arg(cleaned);
					}

				if (str.endsWith(","))
					str = str.mid(0, str.length() - 1);

				if (!str.isEmpty())
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
					stream << str << Qt::endl;
#else
					stream << str << endl;
#endif
			}

			file.close();
		}

		QApplication::restoreOverrideCursor();
	}
}

void biblioteq::pcSearch(const QString &field, const QString &value)
{
	auto photograph = new biblioteq_photographcollection(this, "", QModelIndex());

	photograph->search(field, value);
	photograph->deleteLater();
}

void biblioteq::readConfig(void)
{
	QFont font;
	QSettings settings;

	ui.actionAutoPopulateOnCreation->setChecked(settings.value("automatically_populate_on_create", false).toBool());
	ui.actionAutomatically_Resize_Column_Widths->setChecked(settings.value("automatically_resize_column_widths", false).toBool());

	ui.actionPopulateOnStart->setChecked(settings.value("populate_table_on_connect", false).toBool());
	ui.actionResetErrorLogOnDisconnect->setChecked(settings.value("reset_error_log_on_disconnect", false).toBool());
	ui.actionShowGrid->setChecked(settings.value("show_table_grid", false).toBool());

	if (settings.contains("main_window_geometry"))
	{
		ui.actionPreserveGeometry->setChecked(true);

		if (biblioteq_misc_functions::isGnome())
			setGeometry(settings.value("main_window_geometry").toRect());
		else
			restoreGeometry(settings.value("main_window_geometry").toByteArray());
	}
	else
		ui.actionPreserveGeometry->setChecked(false);

#ifndef Q_OS_MACOS
	font = QApplication::font();

	if (settings.contains("global_font"))
		if (!font.fromString(settings.value("global_font", "").toString()))
			font = QApplication::font();

	QApplication::setFont(font);
#endif
	ui.actionAutomaticallySaveSettingsOnExit->setChecked(settings.value("save_settings_on_exit", true).toBool());
	ui.actionPopulate_Database_Enumerations_Browser_on_Display->setChecked(settings.value("automatically_populate_enum_list_on_display",
																						  false)
																			   .toBool());

	QHash<QString, QString> states;

	for (int i = 0; i < settings.allKeys().size(); i++)
		if (settings.allKeys().at(i).contains("_header_state"))
			states[settings.allKeys().at(i)] =
				settings.value(settings.allKeys().at(i)).toString();

	ui.table->parseStates(states);
	states.clear();

	auto found = false;

	for (int i = 0; i < ui.menuPreferredSRUSite->actions().size(); i++)
		if (QString(settings.value("preferred_sru_site").toString()).remove("&").trimmed() ==
			QString(ui.menuPreferredSRUSite->actions().at(i)->text()).remove("&"))
		{
			found = true;
			ui.menuPreferredSRUSite->actions().at(i)->setChecked(true);
			break;
		}

	if (!found && !ui.menuPreferredSRUSite->actions().isEmpty())
		ui.menuPreferredSRUSite->actions().at(0)->setChecked(true);

	found = false;

	for (int i = 0; i < ui.menuPreferredZ3950Server->actions().size(); i++)
		if (QString(settings.value("preferred_z3950_site").toString()).remove("&").trimmed() ==
			QString(ui.menuPreferredZ3950Server->actions().at(i)->text()).remove("&"))
		{
			found = true;
			ui.menuPreferredZ3950Server->actions().at(i)->setChecked(true);
			break;
		}

	if (!found && !ui.menuPreferredZ3950Server->actions().isEmpty())
		ui.menuPreferredZ3950Server->actions().at(0)->setChecked(true);

	auto index = br.branch_name->findText(settings.value("previous_branch_name", "").toString());

	if (index >= 0)
		br.branch_name->setCurrentIndex(index);
	else
		br.branch_name->setCurrentIndex(0);

	auto viewModeIndex = settings.value("view_mode_index", 1).toInt();

	if (viewModeIndex < 0 || viewModeIndex > 1)
		viewModeIndex = 1;

	auto ag = findChild<QActionGroup *>("ViewModeMenu");

	if (ag && ag->actions().size() > viewModeIndex)
		ag->actions().at(viewModeIndex)->setChecked(true);

	ui.stackedWidget->setCurrentIndex(viewModeIndex);

	if (ui.stackedWidget->currentIndex() == 0)
		ui.table->setSelectionMode(QAbstractItemView::MultiSelection);
	else
		ui.table->setSelectionMode(QAbstractItemView::ExtendedSelection);

	QColor color(settings.value("mainwindow_canvas_background_color").toString().trimmed());

	if (!color.isValid())
		color = Qt::white;

	ui.graphicsView->scene()->setBackgroundBrush(color);
	slotResizeColumns();
	createSqliteMenuActions();
}

void biblioteq::readGlobalSetup(void)
{
#if defined(Q_OS_ANDROID)
	QSettings settings("assets:/biblioteq.conf", QSettings::IniFormat);
#elif defined(Q_OS_MACOS)
	QSettings settings(QCoreApplication::applicationDirPath() + "/../../../biblioteq.conf",
					   QSettings::IniFormat);
#elif defined(Q_OS_OS2)
	QSettings settings(qgetenv("unixroot") + "/usr/local/biblioteq.conf", QSettings::IniFormat);
#elif defined(Q_OS_WIN)
	QSettings settings(QCoreApplication::applicationDirPath() +
						   QDir::separator() +
						   "biblioteq.conf",
					   QSettings::IniFormat);
#else
	QSettings settings(BIBLIOTEQ_CONFIGFILE, QSettings::IniFormat);
#endif

	m_amazonImages.clear();
	m_branches.clear();
	m_openLibraryImages.clear();
	m_openLibraryItems.clear();
	m_sruMaps.clear();
	m_z3950Maps.clear();
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
	settings.setIniCodec(QTextCodec::codecForName("UTF-8"));
#endif

	for (int i = 0; i < settings.childGroups().size(); i++)
	{
		settings.beginGroup(settings.childGroups().at(i));

		if (settings.group() == "Amazon Back Cover Images")
		{
			m_amazonImages["back_cover_host"] = settings.value("host", "").toString().trimmed();
			m_amazonImages["back_cover_path"] = settings.value("path", "").toString().trimmed().remove('"');
			m_amazonImages["back_proxy_host"] = settings.value("proxy_host", "").toString().trimmed();
			m_amazonImages["back_proxy_password"] = settings.value("proxy_password", "").toString().trimmed();
			m_amazonImages["back_proxy_port"] = settings.value("proxy_port", "").toString().trimmed();
			m_amazonImages["back_proxy_type"] = settings.value("proxy_type", "").toString().trimmed();
			m_amazonImages["back_proxy_username"] = settings.value("proxy_username", "").toString().trimmed();
		}
		else if (settings.group() == "Amazon Front Cover Images")
		{
			m_amazonImages["front_cover_host"] = settings.value("host", "").toString().trimmed();
			m_amazonImages["front_cover_path"] = settings.value("path", "").toString().trimmed().remove('"');
			m_amazonImages["front_proxy_host"] = settings.value("proxy_host", "").toString().trimmed();
			m_amazonImages["front_proxy_password"] = settings.value("proxy_password", "").toString().trimmed();
			m_amazonImages["front_proxy_port"] = settings.value("proxy_port", "").toString().trimmed();
			m_amazonImages["front_proxy_type"] = settings.value("proxy_type", "").toString().trimmed();
			m_amazonImages["front_proxy_username"] = settings.value("proxy_username", "").toString().trimmed();
		}
		else if (settings.group().startsWith("Branch"))
		{
			if (!settings.value("database_name", "").toString().trimmed().isEmpty())
			{
				QHash<QString, QString> hash;

				hash["branch_name"] = settings.value("database_name", "").toString().trimmed();
				hash["connection_options"] = settings.value("connection_options", "").toString().trimmed();
				hash["database_type"] = settings.value("database_type", "").toString().trimmed();
				hash["hostname"] = settings.value("hostname", "").toString().trimmed();
				hash["port"] = settings.value("port", "").toString().trimmed();
				hash["ssl_enabled"] = settings.value("ssl_enabled", "").toString().trimmed();
				m_branches[settings.value("database_name", "").toString().trimmed()] = hash;
			}
		}
		else if (settings.group().startsWith("Open Library"))
		{
			if (settings.group() == "Open Library")
				m_openLibraryItems["url_isbn"] =
					settings.value("url_isbn").toString().trimmed();
			else
			{
				m_openLibraryImages["back_url"] =
					settings.value("back_url", "").toString().trimmed();
				m_openLibraryImages["front_url"] =
					settings.value("front_url", "").toString().trimmed();
				m_openLibraryImages["proxy_host"] = settings.value("proxy_host", "").toString().trimmed();
				m_openLibraryImages["proxy_password"] = settings.value("proxy_password", "").toString().trimmed();
				m_openLibraryImages["proxy_port"] = settings.value("proxy_port", "").toString().trimmed();
				m_openLibraryImages["proxy_type"] = settings.value("proxy_type", "").toString().trimmed();
				m_openLibraryImages["proxy_username"] = settings.value("proxy_username", "").toString().trimmed();
			}
		}
		else if (settings.group().startsWith("SRU"))
		{
			if (!settings.value("name", "").toString().trimmed().isEmpty())
			{
				QHash<QString, QString> hash;

				hash["Name"] = settings.value("name", "").toString().trimmed();
				hash["proxy_host"] = settings.value("proxy_host", "").toString().trimmed();
				hash["proxy_password"] = settings.value("proxy_password", "").toString().trimmed();
				hash["proxy_port"] = settings.value("proxy_port", "").toString().trimmed();
				hash["proxy_type"] = settings.value("proxy_type", "").toString().trimmed();
				hash["proxy_username"] = settings.value("proxy_username", "").toString().trimmed();
				hash["url_isbn"] = settings.value("url_isbn", "").toString().trimmed().remove('"');
				hash["url_issn"] = settings.value("url_issn", "").toString().trimmed().remove('"');
				m_sruMaps[settings.value("name", "").toString().trimmed()] = hash;
			}
		}
		else if (settings.group().startsWith("Z39.50"))
		{
			if (!settings.value("name", "").toString().trimmed().isEmpty())
			{
				QHash<QString, QString> hash;

				hash["Name"] = settings.value("name", "Z39.50 Site").toString().trimmed();
				hash["Address"] = settings.value("hostname", "").toString().trimmed();
				hash["Database"] = settings.value("database_name", "").toString().trimmed();
				hash["Format"] = settings.value("format", "marc8,utf-8").toString().trimmed().remove('"');
				hash["Password"] = settings.value("password", "").toString().trimmed();
				hash["Port"] = settings.value("port", "").toString().trimmed();
				hash["RecordSyntax"] = settings.value("record_syntax", "MARC21").toString().trimmed();
				hash["Userid"] = settings.value("username", "").toString().trimmed();
				hash["proxy_host"] = settings.value("proxy_host", "").toString().trimmed();
				hash["proxy_port"] = settings.value("proxy_port", "").toString().trimmed();
				hash["timeout"] = settings.value("timeout", "30").toString().trimmed();

				for (int j = 0; j < settings.allKeys().size(); j++)
					if (settings.allKeys().at(j).length() > 4 &&
						settings.allKeys().at(j).startsWith("yaz_"))
						hash[settings.allKeys().at(j)] = settings.value(settings.allKeys().at(j)).toString().trimmed();

				m_z3950Maps.insert(settings.value("name", "Z39.50 Site").toString().trimmed(),
								   hash);
			}
		}

		settings.endGroup();
	}

	br.branch_name->clear();

	auto list(m_branches.keys());

	for (int i = 0; i < list.size(); i++)
		br.branch_name->addItem(list.at(i));

	if (br.branch_name->count() == 0)
	{
		QHash<QString, QString> hash;

		hash["branch_name"] = "local_db";
		hash["hostname"] = "127.0.0.1";
		hash["database_type"] = "sqlite";
		hash["port"] = "-1";
		hash["ssl_enabled"] = "false";

		if (!m_branches.contains(hash.value("branch_name")))
			m_branches[hash.value("branch_name")] = hash;

		br.branch_name->addItem(hash.value("branch_name"));
	}

	if (m_sruMaps.isEmpty())
	{
		QHash<QString, QString> hash;

		hash["Name"] = "Library of Congress";
		hash["url_isbn"] = "https://www.loc.gov/z39voy?operation=searchRetrieve&"
						   "version=1.1&query=bath.isbn=%1 or bath.isbn=%2&"
						   "recordSchema=marcxml&startRecord=1&maximumRecords=1";
		hash["url_issn"] = "https://www.loc.gov/z39voy?operation="
						   "searchRetrieve&version=1.1&query=bath.issn=%1&"
						   "recordSchema=marcxml&startRecord=1&maximumRecords=100";
		m_sruMaps["Library of Congress"] = hash;
	}

	if (m_z3950Maps.isEmpty())
	{
		QHash<QString, QString> hash;

		hash["Name"] = "Library of Congress";
		hash["Address"] = "lx2.loc.gov";
		hash["Port"] = "210";
		hash["Database"] = "LCDB";
		hash["Format"] = "marc8,utf-8";
		hash["RecordSyntax"] = "MARC21";
		hash["timeout"] = "30";
		m_z3950Maps.insert("Library of Congress", hash);
	}
}

void biblioteq::slotAllGo(void)
{
	if (!m_db.isOpen())
		return;

	QApplication::setOverrideCursor(Qt::WaitCursor);

	QList<QVariant> values;
	QSqlQuery query(m_db);
	QString bookFrontCover("'' AS front_cover ");
	QString photographCollectionFrontCover("'' AS image_scaled ");
	QString searchstr("");
	QString str("");
	QString type("");
	QStringList types;

	if (m_otheroptions->showMainTableImages())
	{
		bookFrontCover = "book.front_cover ";
		photographCollectionFrontCover = "photograph_collection.image_scaled ";
	}

	types.append("Book");
	types.append("Photograph Collection");

	for (int i = 0; i < types.size(); i++)
	{
		type = types.at(i);

		if (type == "Photograph Collection")
		{
			str = "SELECT DISTINCT photograph_collection.title, "
				  "photograph_collection.id, "
				  "'', "
				  "'', "
				  "'', "
				  "'', "
				  "0.00, "
				  "'', "
				  "1 AS quantity, "
				  "'' AS location,  "
				  "0 AS availability, "
				  "0 AS total_reserved, "
				  "photograph_collection.type, "
				  "photograph_collection.myoid, " +
				  photographCollectionFrontCover +
				  "FROM photograph_collection "
				  "WHERE ";
		}
		else
		{
			str = QString("SELECT DISTINCT %1.title, "
						  "%1.id, "
						  "%1.publisher, %1.pdate, "
						  "%1.category, "
						  "%1.quantity, "
						  "%1.location, "
						  "%1.quantity AS availability, "
						  "%1.quantity AS total_reserved, "
						  "%1.type, "
						  "%1.myoid, ")
					  .arg(type.toLower().remove(" "));

			if (type == "Book")
				str.append(bookFrontCover);

			str += QString("FROM "
						   "%1 WHERE ")
					   .arg(type.toLower().remove(" "));
		}

		QString ESCAPE("");

		str.append(")");
		str += " UNION ALL ";

		searchstr += str;
	}

	if (m_db.driverName() == "QSQLITE")
		query.exec("PRAGMA case_sensitive_like = TRUE");

	query.prepare(searchstr);

	for (int i = 0; i < values.size(); i++)
		query.addBindValue(values.at(i));

	QApplication::restoreOverrideCursor();
	(void)populateTable(query, "All", NEW_PAGE, POPULATE_SEARCH);
}

void biblioteq::slotConnectDB(void)
{
	slotDisconnect();

	auto tmphash(m_branches[br.branch_name->currentText()]);

	if (tmphash.value("database_type") == "sqlite")
	{
		QFileInfo fileInfo(br.filename->text());

		if (!fileInfo.exists() || !fileInfo.isReadable() || !fileInfo.isWritable())
		{
			QWidget *parent = this;

			if (m_branch_diag->isVisible())
				parent = m_branch_diag;

			QMessageBox::critical(parent, tr("BiblioteQ: User Error"),
								  tr("The selected SQLite file is not accessible. Please "
									 "verify that the file exists, is readable, and is writable."));
			QApplication::processEvents();
			return;
		}
	}

	QString drivers = "";
	QString errorstr = "";
	QString plugins = "";
	QString str = "";
	auto error = false;

	/*
	** Configure some database attributes.
	*/

	if (tmphash.value("database_type") == "sqlite")
		str = "QSQLITE";

	foreach (const auto &driver, QSqlDatabase::drivers())
		drivers += driver + ", ";

	if (drivers.endsWith(", "))
		drivers = drivers.mid(0, drivers.length() - 2);

	if (drivers.isEmpty())
		drivers = "N/A";

	foreach (const auto &path, QApplication::libraryPaths())
		if (path.toLower().contains("plugin"))
		{
			plugins = path;
			break;
		}

	if (plugins.isEmpty())
		plugins = "N/A";

	if (!QSqlDatabase::isDriverAvailable(str))
	{
		tmphash.clear();

		QFileInfo fileInfo("qt.conf");
		QString str("");

		if (fileInfo.isReadable() && fileInfo.size() > 0)
			str = tr("\nThe file qt.conf is present in BiblioteQ's "
					 "current working directory. Perhaps a conflict "
					 "exists.");

		QMessageBox::critical(m_branch_diag, tr("BiblioteQ: Database Error"),
							  tr("The selected branch's database type does not "
								 "have a driver associated with it.") +
								  "\n" +
								  tr("The following drivers are available: ") +
								  drivers + tr(".") + "\n" +
								  tr("In addition, Qt expects plugins to exist "
									 "in: ") +
								  plugins + tr(".") + str + "\n" +
								  tr("Please contact your administrator."));
		QApplication::processEvents();
		return;
	}

	m_db = QSqlDatabase::addDatabase(str, "Default");

	if (tmphash.value("database_type") == "sqlite")
		m_db.setDatabaseName(br.filename->text());
	else
	{
		m_db.setHostName(tmphash.value("hostname"));
		m_db.setDatabaseName(br.branch_name->currentText());
		m_db.setPort(static_cast<int>(tmphash.value("port").toUShort()));
	}

	if (tmphash.value("database_type") != "sqlite")
	{
		auto str(tmphash.value("connection_options"));

		if (tmphash.value("ssl_enabled") == "true")
			str.append(";requiressl=1");

		m_db.setConnectOptions(str);
	}

	QApplication::setOverrideCursor(Qt::WaitCursor);

	if (tmphash.value("database_type") == "sqlite")
		(void)m_db.open();

	QApplication::restoreOverrideCursor();

	if (!m_db.isOpen())
	{
		error = true;
		addError(QString(tr("Database Error")),
				 QString(tr("Unable to open a database connection "
							"with the provided information.")),
				 m_db.lastError().text(),
				 __FILE__, __LINE__);
		QMessageBox::critical(m_branch_diag, tr("BiblioteQ: Database Error"),
							  tr("Unable to open a database "
								 "connection with the provided information. Please "
								 "review the Error Log."));
		QApplication::processEvents();
	}
	else
	{
		if (!m_db.driver()->hasFeature(QSqlDriver::Transactions))
		{
			error = true;
			addError(QString(tr("Database Error")),
					 QString(tr("The current database driver that you're using "
								"does not support transactions. "
								"Please upgrade your database and/or driver.")),
					 m_db.lastError().text(),
					 __FILE__, __LINE__);
			QMessageBox::critical(m_branch_diag, tr("BiblioteQ: Database Error"),
								  tr("The current database driver that you're using "
									 "does not support transactions. "
									 "Please upgrade your database and/or driver."));
			QApplication::processEvents();
		}
	}

	if (!error)
		m_roles = "administrator";

	tmphash.clear();

	if (error)
	{
		m_db = QSqlDatabase();
		QSqlDatabase::removeDatabase("Default");
		return;
	}
	else
#ifdef Q_OS_ANDROID
		m_branch_diag->hide();
#else
		m_branch_diag->close();
#endif

	/*
	** We've connected successfully. Let's initialize other containers and
	** widgets.
	*/

	QSettings settings;

	settings.setValue("previous_branch_name", br.branch_name->currentText());
	m_selectedBranch = m_branches[br.branch_name->currentText()];

	if (m_connected_bar_label != nullptr)
	{
		m_connected_bar_label->setPixmap(QPixmap(":/16x16/connected.png"));
		m_connected_bar_label->setToolTip(tr("Connected"));
	}

	ui.actionDatabaseSearch->setEnabled(true);
	ui.actionDisconnect->setEnabled(true);
	ui.actionRefreshTable->setEnabled(true);
	ui.actionViewDetails->setEnabled(true);
	ui.configTool->setEnabled(true);
	ui.customQueryTool->setEnabled(true);
	ui.detailsTool->setEnabled(true);
	ui.disconnectTool->setEnabled(true);
	ui.filesTool->setEnabled(true);
	ui.printTool->setEnabled(true);
	ui.refreshTool->setEnabled(true);
	ui.searchTool->setEnabled(true);

	if (m_db.driverName() == "QSQLITE")
	{
		ui.actionImportCSV->setEnabled(true);
		ui.action_Merge_SQLite_Databases->setEnabled(true);
		ui.action_Upgrade_SQLite_Schema->setEnabled(true);
		ui.action_VacuumDatabase->setEnabled(true);
		ui.menuEntriesPerPage->setEnabled(true);

		if (!ui.menuEntriesPerPage->actions().isEmpty())
			ui.menuEntriesPerPage->actions().at(ui.menuEntriesPerPage->actions().size() - 1)->setEnabled(true);

		/*
		** Set the window's title.
		*/

		if (!m_roles.isEmpty())
			setWindowTitle(tr("BiblioteQ: %1 (%2)").arg(QFileInfo(br.filename->text()).fileName()).arg(m_roles));
		else
			setWindowTitle(tr("BiblioteQ: %1 (%2)").arg(QFileInfo(br.filename->text()).fileName()).arg("missing roles"));
	}

	prepareFilter();

	if (m_db.driverName() == "QSQLITE")
	{
		if (m_db.driverName() == "QSQLITE")
		{
			/*
			** Add the database's information to the pulldown menu.
			*/

			auto actions = ui.menu_Recent_SQLite_Files->actions();
			auto exists = false;

			for (int i = 0; i < actions.size(); i++)
				if (actions[i]->data().toString() == br.filename->text())
				{
					exists = true;
					break;
				}

			actions.clear();

			if (!exists)
			{
				QSettings settings;
				auto allKeys(settings.allKeys());
				int index = 1;

				for (int i = 0; i < allKeys.size(); i++)
					if (allKeys[i].startsWith("sqlite_db_"))
						index += 1;

				allKeys.clear();
				settings.setValue(QString("sqlite_db_%1").arg(index),
								  br.filename->text());
				createSqliteMenuActions();
			}
		}

		adminSetup();
	}

	auto found = false;

	if (m_db.driverName() == "QSQLITE")
	{
		for (int i = 0; i < ui.menuEntriesPerPage->actions().size(); i++)
			if (ui.menuEntriesPerPage->actions().at(i)->data().toInt() ==
				settings.value("sqlite_entries_per_page").toInt())
			{
				found = true;
				ui.menuEntriesPerPage->actions().at(i)->setChecked(true);
				break;
			}
	}

	if (!found && !ui.menuEntriesPerPage->actions().isEmpty())
		ui.menuEntriesPerPage->actions().at(0)->setChecked(true);

	found = false;

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

	if (ui.actionPopulateOnStart->isChecked())
		slotRefresh();

	prepareContextMenus();
	prepareUpgradeNotification();
}

void biblioteq::slotDisconnect(void)
{
	if (db_enumerations &&
		db_enumerations->isVisible() &&
		!db_enumerations->close())
		return;

	QApplication::setOverrideCursor(Qt::WaitCursor);

	if (!emptyContainers())
	{
		QApplication::restoreOverrideCursor();
		return;
	}
	else
		QApplication::restoreOverrideCursor();

	m_allSearchShown = false;

	if (m_files)
		m_files->reset();

	m_roles = "";
	m_pages = 0;
	m_queryOffset = 0;

	if (m_searchQuery.isActive())
		m_searchQuery.clear();

#ifdef Q_OS_ANDROID
	m_customquery_diag->hide();
	m_import->hide();
#else
	m_customquery_diag->close();
	m_import->close();
#endif
	m_unaccent.clear();
	cq.tables_t->clear();
	cq.tables_t->setColumnCount(0);
	cq.tables_t->scrollToTop();
	cq.tables_t->horizontalScrollBar()->setValue(0);
	cq.tables_t->clearSelection();
	cq.tables_t->setCurrentItem(nullptr);
	cq.query_te->clear();

	if (db_enumerations)
		db_enumerations->clear();

	ui.actionAutoPopulateOnCreation->setEnabled(false);
	ui.actionDatabaseSearch->setEnabled(false);
	ui.actionDeleteEntry->setEnabled(false);
	ui.actionDisconnect->setEnabled(false);
	ui.actionDuplicateEntry->setEnabled(false);
	ui.actionImportCSV->setEnabled(false);
	ui.actionModifyEntry->setEnabled(false);
	ui.actionRefreshTable->setEnabled(false);
	ui.actionViewDetails->setEnabled(false);
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
	ui.pagesLabel->setText(tr("1"));
	ui.previousPageButton->setEnabled(false);
	ui.printTool->setEnabled(false);
	ui.refreshTool->setEnabled(false);
	ui.searchTool->setEnabled(false);

	if (!ui.menuEntriesPerPage->actions().isEmpty())
		ui.menuEntriesPerPage->actions().at(ui.menuEntriesPerPage->actions().size() - 1)->setEnabled(true);

	ui.actionPopulate_Administrator_Browser_Table_on_Display->setEnabled(false);
	ui.actionPopulate_Database_Enumerations_Browser_on_Display->setEnabled(false);
	ui.actionDatabase_Enumerations->setEnabled(false);
	ui.action_Database_Enumerations->setEnabled(false);
	ui.graphicsView->scene()->clear();
	ui.table->disconnect(SIGNAL(itemDoubleClicked(QTableWidgetItem *)));
	ui.graphicsView->scene()->disconnect(SIGNAL(itemDoubleClicked(void)));
	slotResetAllSearch();

	if (m_db.isOpen())
	{
		QSettings settings;

		if (m_db.driverName() == "QSQLITE")
		{
			for (int i = 0; i < ui.menuEntriesPerPage->actions().size(); i++)
				if (ui.menuEntriesPerPage->actions().at(i)->isChecked())
				{
					settings.setValue("sqlite_entries_per_page",
									  ui.menuEntriesPerPage->actions().at(i)->data().toInt());
					break;
				}
		}
	}

	if (!ui.menuEntriesPerPage->actions().isEmpty())
		ui.menuEntriesPerPage->actions().at(0)->setChecked(true);

	if (m_connected_bar_label != nullptr)
	{
		m_connected_bar_label->setPixmap(QPixmap(":/16x16/disconnected.png"));
		m_connected_bar_label->setToolTip(tr("Disconnected"));
	}

	if (m_status_bar_label != nullptr)
	{
		m_status_bar_label->setPixmap(QPixmap(":/16x16/lock.png"));
		m_status_bar_label->setToolTip(tr("Standard User Mode"));
	}

	if (ui.actionResetErrorLogOnDisconnect->isChecked())
		slotResetErrorLog();

	ui.graphicsView->scene()->clear();
	ui.graphicsView->resetTransform();
	ui.graphicsView->verticalScrollBar()->setValue(0);
	ui.graphicsView->horizontalScrollBar()->setValue(0);
	ui.nextPageButton->setEnabled(false);
	ui.pagesLabel->setText(tr("1"));
	ui.previousPageButton->setEnabled(false);
	ui.table->resetTable(dbUserName(), m_previousTypeFilter);
	ui.itemsCountLabel->setText(tr("0 Results"));
	prepareFilter();

	for (int i = 0; i < ui.menu_Category->actions().size(); i++)
		if (m_previousTypeFilter ==
			ui.menu_Category->actions().at(i)->data().toString())
		{
			ui.categoryLabel->setText(ui.menu_Category->actions().at(i)->text());
			ui.menu_Category->actions().at(i)->setChecked(true);
			ui.menu_Category->setDefaultAction(ui.menu_Category->actions().at(i));
			break;
		}

	addConfigOptions(m_previousTypeFilter);
	slotDisplaySummary();
	QApplication::setOverrideCursor(Qt::WaitCursor);

	if (m_db.isOpen())
		m_db.close();

	QApplication::restoreOverrideCursor();
	m_db = QSqlDatabase();

	if (QSqlDatabase::contains("Default"))
		QSqlDatabase::removeDatabase("Default");

	setWindowTitle(tr("BiblioteQ"));
}

void biblioteq::slotDisplaySummary(void)
{
	QImage backImage;
	QImage frontImage;
	QString oid = "";
	QString summary = "";
	QString tmpstr = "";
	QString type = "";
	int i = 0;

	/*
	** Display a preview.
	*/

	if (ui.itemSummary->width() > 0 && ui.table->currentRow() > -1)
	{
		i = ui.table->currentRow();
		oid = biblioteq_misc_functions::getColumnString(ui.table, i, ui.table->columnNumber("MYOID"));

		if (ui.stackedWidget->currentIndex() == 1)
		{
			/*
			** This method is also called by slotSceneSelectionChanged().
			*/

			QPainterPath painterPath;
			auto items(ui.graphicsView->scene()->items());
			auto tableItems(ui.table->selectedItems());

			for (int ii = 0; ii < tableItems.size(); ii++)
			{
				auto oid = biblioteq_misc_functions::getColumnString(ui.table,
																	 tableItems.at(ii)->row(),
																	 ui.table->columnNumber("MYOID"));
				auto type = biblioteq_misc_functions::getColumnString(ui.table,
																	  tableItems.at(ii)->row(),
																	  ui.table->columnNumber("Type"))
								.remove(' ')
								.toLower();

				for (int jj = 0; jj < items.size(); jj++)
					if (oid == items.at(jj)->data(0).toString() &&
						type == items.at(jj)->data(1).toString())
					{
						QRectF rect;

						rect.setTopLeft(items.at(jj)->scenePos());
						rect.setWidth(126);
						rect.setHeight(187);
						painterPath.addRect(rect);
					}
					else
						items.at(jj)->setSelected(false);
			}

			items.clear();
			ui.graphicsView->scene()->setSelectionArea(painterPath);
		}

		type = biblioteq_misc_functions::getColumnString(ui.table, i, ui.table->columnNumber("Type"));
		summary = "<html>";

		if (type == "Book")
		{
			summary += "<b>" +
					   biblioteq_misc_functions::getColumnString(ui.table, i, ui.table->columnNumber("Title")) +
					   "</b>";
			summary += "<br>";
			tmpstr = biblioteq_misc_functions::getColumnString(ui.table, i, ui.table->columnNumber("ISBN-10"));

			if (tmpstr.isEmpty())
				tmpstr = biblioteq_misc_functions::getColumnString(ui.table, i, ui.table->columnNumber("ID Number"));

			if (tmpstr.isEmpty())
				tmpstr = "<br>";

			summary += tmpstr;
			summary += "<br>";
			summary += biblioteq_misc_functions::getColumnString(ui.table, i, ui.table->columnNumber("Publication Date"));
			summary += "<br>";
			summary += biblioteq_misc_functions::getColumnString(ui.table, i, ui.table->columnNumber("Publisher"));
			summary += "<br>";
			summary += biblioteq_misc_functions::getColumnString(ui.table, i, ui.table->columnNumber("Place of Publication"));
			summary += "<br>";
		}
		else if (type == "Photograph Collection")
		{
			summary += "<b>" +
					   biblioteq_misc_functions::getColumnString(ui.table, i, ui.table->columnNumber("Title")) +
					   "</b>";
			summary += "<br>ID: ";
			tmpstr = biblioteq_misc_functions::getColumnString(ui.table, i, ui.table->columnNumber("ID"));

			if (tmpstr.isEmpty())
				tmpstr = biblioteq_misc_functions::getColumnString(ui.table, i, ui.table->columnNumber("ID Number"));

			if (tmpstr.isEmpty())
				tmpstr = "<br>";

			summary += tmpstr;

			summary += "<br>" + tr("Creation Date: ");
			tmpstr = biblioteq_misc_functions::getColumnString(ui.table, i, ui.table->columnNumber("Creation Date"));
			if (tmpstr.isEmpty())
				tmpstr = tr("Unknown");
			tmpstr += "<br>";
			summary += tmpstr;
			tmpstr = biblioteq_misc_functions::getColumnString(ui.table, i, ui.table->columnNumber("Element Count"));

			if (!tmpstr.isEmpty())
				summary += "<br>" + QString(tr("%1 Photograph(s)")).arg(tmpstr);

			summary += "<br>";
		}

		summary += biblioteq_misc_functions::getAbstractInfo(oid, type, m_db);
		summary += "<br>";

		if (type != "Photograph Collection")
		{
			tmpstr = biblioteq_misc_functions::getColumnString(ui.table, i, ui.table->columnNumber("Availability"));

			if (!tmpstr.isEmpty())
			{
				if (tmpstr.toInt() > 0)
					summary += tr("Available") + "<br>";
				else
					summary += tr("Unavailable") + "<br>";
			}
		}

		summary += biblioteq_misc_functions::getColumnString(ui.table, i, ui.table->columnNumber("Location"));

		while (summary.contains("<br><br>"))
			summary.replace("<br><br>", "<br>");

		if (type == "Book")
			if (biblioteq_misc_functions::isBookRead(m_db, oid.toULongLong()))
			{
				summary += "<br>";
				summary += tr("<b>Read</b>");
			}

		if (type == "Photograph Collection")
		{
			QString about = "<html>" + biblioteq_misc_functions::getColumnString(ui.table, i, ui.table->columnNumber("About")) + "</html>";
			ui.about->setText(about);
			ui.about->setVisible(true);
		}
		summary += "</html>";
		ui.summary->setText(summary);
		ui.summary->setVisible(true);
		QApplication::setOverrideCursor(Qt::WaitCursor);

		if (type != "Photograph Collection")
			frontImage = biblioteq_misc_functions::getImage(oid, "front_cover", type,
															m_db);
		else
			frontImage = biblioteq_misc_functions::getImage(oid, "image_scaled", type,
															m_db);

		if (type != "Photograph Collection")
			backImage = biblioteq_misc_functions::getImage(oid, "back_cover", type, m_db);

		QApplication::restoreOverrideCursor();

		/*
		** The size of no_image.png is 126x187.
		*/

		if (frontImage.isNull())
			frontImage = QImage(":/no_image.png");

		if (!frontImage.isNull())
			frontImage = frontImage.scaled(126, 187, Qt::KeepAspectRatio, Qt::SmoothTransformation);

		if (type != "Photograph Collection")
		{
			if (backImage.isNull())
				backImage = QImage(":/no_image.png");

			if (!backImage.isNull())
				backImage = backImage.scaled(126, 187, Qt::KeepAspectRatio, Qt::SmoothTransformation);
		}

		if (!frontImage.isNull())
		{
			ui.frontImage->setVisible(true);
			ui.frontImage->setPixmap(QPixmap::fromImage(frontImage));
		}
		else
			ui.frontImage->clear();

		if (type != "Photograph Collection")
		{
			if (!backImage.isNull())
			{
				ui.backImage->setVisible(true);
				ui.backImage->setPixmap(QPixmap::fromImage(backImage));
			}
			else
				ui.backImage->clear();
		}
		else
			ui.backImage->clear();
	}
	else
	{
		/*
		** Clear the scene.
		*/

		ui.backImage->clear();
		ui.backImage->setVisible(false);
		ui.frontImage->clear();
		ui.frontImage->setVisible(false);
		ui.summary->clear();
		ui.summary->setVisible(false);
		ui.about->clear();
		ui.about->setVisible(false);
	}
}

void biblioteq::slotGraphicsSceneEnterKeyPressed(void)
{
	slotMainTableEnterKeyPressed();
}

void biblioteq::slotItemChanged(QTableWidgetItem *item)
{
	if (!item || !(Qt::ItemIsUserCheckable & item->flags()))
		return;

	slotDisplaySummary();
}

void biblioteq::slotLastWindowClosed(void)
{
	if (ui.actionAutomaticallySaveSettingsOnExit->isChecked())
		slotSaveConfig();

	cleanup();
}

void biblioteq::slotMainTableEnterKeyPressed(void)
{
	if (m_roles.contains("administrator") || m_roles.contains("librarian"))
		slotModify();
	else
		slotViewDetails();
}

void biblioteq::slotMainWindowCanvasBackgroundColorChanged(const QColor &color)
{
	QSettings settings;

	if (color.isValid())
	{
		settings.setValue("mainwindow_canvas_background_color", color.name());
		ui.graphicsView->scene()->setBackgroundBrush(color);
	}
	else
	{
		QColor color(settings.value("mainwindow_canvas_background_color").toString().trimmed());

		if (!color.isValid())
			color = Qt::white;

		ui.graphicsView->scene()->setBackgroundBrush(color);
	}
}

void biblioteq::slotOpenOnlineDocumentation(void)
{
	QDesktopServices::openUrl(QUrl("https://github.com/textbrowser/biblioteq/"
								   "blob/master/Documentation/BiblioteQ.pdf"));
}

void biblioteq::slotOpenPDFFiles(void)
{
#ifdef BIBLIOTEQ_LINKED_WITH_POPPLER
	QFileDialog dialog(this);

	dialog.setDirectory(QDir::homePath());
	dialog.setFileMode(QFileDialog::ExistingFiles);
	dialog.setNameFilter("PDF (*.pdf)");
	dialog.setOption(QFileDialog::DontUseNativeDialog);
	dialog.setWindowTitle(tr("BiblioteQ: Open PDF File(s)"));

	if (dialog.exec() == QDialog::Accepted)
	{
		QApplication::processEvents();

		if (dialog.selectedFiles().size() >= MAXIMUM_DEVICES_CONFIRMATION)
			if (QMessageBox::
					question(this,
							 tr("BiblioteQ: Question"),
							 tr("Are you sure that you wish to open %1 PDF files?").arg(dialog.selectedFiles().size()),
							 QMessageBox::Yes | QMessageBox::No,
							 QMessageBox::No) == QMessageBox::No)
			{
				QApplication::processEvents();
				return;
			}

		QApplication::setOverrideCursor(Qt::WaitCursor);

		for (int i = 0; i < dialog.selectedFiles().size(); i++)
		{
			auto reader = new biblioteq_pdfreader(this);

			reader->load(dialog.selectedFiles().at(i));
			biblioteq_misc_functions::center(reader, this);
			reader->show();
		}

		QApplication::restoreOverrideCursor();
	}

	QApplication::processEvents();
#endif
}

void biblioteq::slotOtherOptionsSaved(void)
{
	QApplication::setOverrideCursor(Qt::WaitCursor);

	foreach (auto widget, QApplication::topLevelWidgets())
		if (qobject_cast<biblioteq_book *>(widget))
			qobject_cast<biblioteq_book *>(widget)->setPublicationDateFormat(m_otheroptions->publicationDateFormat("books"));

	if (m_otheroptions->showMainTableImages())
		ui.table->setIconSize(QSize(64, 94));
	else
		ui.table->setIconSize(QSize(0, 0));

	QFontMetrics fontMetrics(ui.table->font());

	for (int i = 0; i < ui.table->rowCount(); i++)
		ui.table->setRowHeight(i, qMax(fontMetrics.height() + 10, ui.table->iconSize().height()));

	QApplication::restoreOverrideCursor();
}

void biblioteq::slotPreviewCanvasBackgroundColor(const QColor &color)
{
	ui.graphicsView->scene()->setBackgroundBrush(color);
}

void biblioteq::slotRefreshCustomQuery(void)
{
	if (!m_db.isOpen())
		return;

	QApplication::setOverrideCursor(Qt::WaitCursor);

	QSqlField field;
	QSqlRecord rec;
	QStringList list;
	QTreeWidgetItem *item1 = nullptr;
	QTreeWidgetItem *item2 = nullptr;
	int i = 0;
	int j = 0;

	cq.tables_t->clear();

	if (m_db.driverName() == "QSQLITE")
		list << "book"
			 << "book_binding_types"
			 << "book_copy_info"
			 << "book_files"
			 << "photograph"
			 << "photograph_collection";

	list.sort();
	cq.tables_t->setSortingEnabled(false);
	cq.tables_t->setColumnCount(3);
	cq.tables_t->setHeaderLabels(QStringList()
								 << tr("Table Name")
								 << tr("Column")
								 << tr("Column Type")
								 << tr("NULL"));

	for (i = 0; i < list.size(); i++)
	{
		item1 = new QTreeWidgetItem(cq.tables_t);
		item1->setText(0, list[i]);
		rec = m_db.record(list[i]);

		for (j = 0; j < rec.count(); j++)
		{
			item2 = new QTreeWidgetItem(item1);
			field = rec.field(rec.fieldName(j));
			item2->setText(1, rec.fieldName(j));
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
			item2->setText(2, QMetaType(rec.field(j).metaType().id()).name());
#else
			item2->setText(2, QVariant::typeToName(field.type()));
#endif

			if (field.requiredStatus() == QSqlField::Required)
				item2->setText(3, tr("No"));
			else
				item2->setText(3, "");
		}
	}

	for (i = 0; i < cq.tables_t->columnCount() - 1; i++)
		cq.tables_t->resizeColumnToContents(i);

	cq.tables_t->setSortingEnabled(true);
	cq.tables_t->sortByColumn(0, Qt::AscendingOrder);
	QApplication::restoreOverrideCursor();
}

void biblioteq::slotSaveConfig(void)
{
	QSettings settings;

	settings.setValue("automatically_populate_admin_list_on_display",
					  ui.actionPopulate_Administrator_Browser_Table_on_Display->isChecked());
	settings.setValue("automatically_populate_enum_list_on_display",
					  ui.actionPopulate_Database_Enumerations_Browser_on_Display->isChecked());
	settings.setValue("automatically_populate_on_create",
					  ui.actionAutoPopulateOnCreation->isChecked());
	settings.setValue("automatically_resize_column_widths",
					  ui.actionAutomatically_Resize_Column_Widths->isChecked());
	settings.setValue("global_font", font().toString());
	settings.setValue("last_category", getTypeFilterString());
	settings.setValue("locale", s_locale);
	settings.setValue("main_splitter_state", ui.splitter->saveState());

	settings.setValue("populate_table_on_connect",
					  ui.actionPopulateOnStart->isChecked());
	settings.setValue("reset_error_log_on_disconnect",
					  ui.actionResetErrorLogOnDisconnect->isChecked());
	settings.setValue("save_settings_on_exit",
					  ui.actionAutomaticallySaveSettingsOnExit->isChecked());
	settings.setValue("show_table_grid", ui.actionShowGrid->isChecked());

	if (ui.actionPreserveGeometry->isChecked())
	{
		if (!isFullScreen())
		{
			if (biblioteq_misc_functions::isGnome())
				settings.setValue("main_window_geometry", geometry());
			else
				settings.setValue("main_window_geometry", saveGeometry());
		}
	}
	else
		settings.remove("main_window_geometry");

	if (m_db.isOpen())
	{
		if (m_db.driverName() == "QSQLITE")
		{
			for (int i = 0; i < ui.menuEntriesPerPage->actions().size(); i++)
				if (ui.menuEntriesPerPage->actions().at(i)->isChecked())
				{
					settings.setValue("sqlite_entries_per_page",
									  ui.menuEntriesPerPage->actions().at(i)->data().toInt());
					break;
				}
		}
	}

	for (int i = 0; i < ui.menuPreferredSRUSite->actions().size(); i++)
		if (ui.menuPreferredSRUSite->actions().at(i)->isChecked())
		{
			settings.setValue("preferred_sru_site",
							  ui.menuPreferredSRUSite->actions().at(i)->text().trimmed());
			break;
		}

	for (int i = 0; i < ui.menuPreferredZ3950Server->actions().size(); i++)
		if (ui.menuPreferredZ3950Server->actions().at(i)->isChecked())
		{
			settings.setValue("preferred_z3950_site",
							  ui.menuPreferredZ3950Server->actions().at(i)->text().trimmed());
			break;
		}

	for (int i = 0; i < ui.table->friendlyStates().keys().size(); i++)
		settings.setValue(ui.table->friendlyStates().keys().at(i),
						  ui.table->friendlyStates().value(ui.table->friendlyStates().keys().at(i)));

	settings.sync();
}

void biblioteq::slotSceneSelectionChanged(void)
{
	if (ui.stackedWidget->currentIndex() != 0)
		return;

	ui.table->clearSelection();
	ui.table->setCurrentCell(-1, -1);
	slotDisplaySummary();

	auto items(ui.graphicsView->scene()->selectedItems());

	if (!items.isEmpty())
	{
		QGraphicsItem *item = nullptr;
		QStringList oids;
		QStringList types;

		for (int i = 0; i < items.size(); i++)
			if ((item = items.at(i)))
			{
				oids.append(item->data(0).toString());
				types.append(item->data(1).toString());
			}

		auto column1 = ui.table->columnNumber("MYOID");
		auto column2 = ui.table->columnNumber("Type");

		for (int i = 0; i < ui.table->rowCount(); i++)
			if (ui.table->item(i, column1) &&
				oids.contains(ui.table->item(i, column1)->text()) &&
				ui.table->item(i, column2) &&
				types.contains(ui.table->item(i, column2)->text().remove(' ').toLower()))
				ui.table->selectRow(i);

		oids.clear();
		types.clear();
	}
}

void biblioteq::slotShowImport(void)
{
	m_import->show(this);
}

void biblioteq::slotShowOtherOptions(void)
{
	biblioteq_misc_functions::center(m_otheroptions, this);
	m_otheroptions->showNormal();
	m_otheroptions->activateWindow();
	m_otheroptions->raise();
}

void biblioteq::slotVacuum(void)
{
	if (QMessageBox::question(this,
							  tr("BiblioteQ: Question"),
							  tr("Vacuuming a database may require a "
								 "significant amount of time to complete. "
								 "Continue?"),
							  QMessageBox::Yes | QMessageBox::No,
							  QMessageBox::No) == QMessageBox::No)
	{
		QApplication::processEvents();
		return;
	}

	QApplication::processEvents();

	QProgressDialog progress(this);

	progress.setCancelButton(nullptr);
	progress.setMaximum(0);
	progress.setMinimum(0);
	progress.setModal(true);
	progress.setWindowTitle(tr("BiblioteQ: Vacuuming Database"));
	progress.show();
	progress.repaint();
	QApplication::processEvents();

	if (statusBar())
	{
		statusBar()->showMessage(tr("Vacuuming the database. Please be patient."));
		statusBar()->repaint();
		statusBar()->update();
	}

	QSqlQuery query(m_db);

	if (m_db.driverName() == "QSQLITE")
	{
		query.exec("DELETE FROM book_copy_info WHERE item_oid NOT IN "
				   "(SELECT myoid FROM book)");
		progress.setValue(0);
		query.exec("DELETE FROM book_files WHERE item_oid NOT IN "
				   "(SELECT myoid FROM book)");
		progress.setValue(0);
		query.exec("DELETE FROM photograph WHERE collection_oid NOT IN "
				   "(SELECT myoid FROM photograph_collection)");
		progress.setValue(0);
	}

	query.exec("VACUUM");
	progress.setValue(0);

	if (statusBar())
		statusBar()->clearMessage();

	progress.close();
	QApplication::restoreOverrideCursor();
}
