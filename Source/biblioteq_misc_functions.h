#ifndef _BIBLIOTEQ_MISC_FUNCTIONS_H_
#define _BIBLIOTEQ_MISC_FUNCTIONS_H_

#include <QGraphicsItem>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QTableWidget>

class QMainWindow;
class biblioteq;

class biblioteq_misc_functions
{
public:
	static const int CREATE_USER = 100;
	static const int DELETE_USER = 200;
	static const int UPDATE_USER = 300;
	static QImage getImage(const QString &,
						   const QString &,
						   const QString &,
						   const QSqlDatabase &);
	static QList<QPair<QString, QString>> getLocations(const QSqlDatabase &,
													   QString &);
	static QList<int> selectedRows(QTableWidget *);
	static QString getAbstractInfo(const QString &,
								   const QString &,
								   const QSqlDatabase &);
	static QString getColumnString(const QTableWidget *,
								   const int,
								   const QString &);
	static QString getColumnString(const QTableWidget *, const int, const int);
	static QString getOID(const QString &,
						  const QString &,
						  const QSqlDatabase &,
						  QString &);
	static QString getRoles(const QSqlDatabase &, const QString &, QString &);
	static QString imageFormatGuess(const QByteArray &bytes);
	static QString isbn10to13(const QString &text);
	static QString isbn13to10(const QString &text);
	static QStringList getBookBindingTypes(const QSqlDatabase &, QString &);

	static bool isBookRead(const QSqlDatabase &db, const quint64 myoid)
	{
		QSqlQuery query(db);

		query.setForwardOnly(true);
		query.prepare("SELECT book_read FROM book WHERE myoid = ?");
		query.addBindValue(myoid);

		if (query.exec() && query.next())
			return query.value(0).toBool();

		return false;
	}

	static bool isGnome(void);
    //static int getColumnNumber(const QTableWidget *, const QString &);
	static int sqliteQuerySize(const QString &,
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
							   const QMap<QString, QVariant> &,
#else
							   const QVariantList &,
#endif
							   const QSqlDatabase &,
							   const char *,
							   const int,
							   biblioteq *);
	static int sqliteQuerySize(const QString &,
							   const QSqlDatabase &,
							   const char *,
							   const int,
							   biblioteq *);
	static qint64 getSqliteUniqueId(const QSqlDatabase &, QString &);
	static void center(QWidget *, QMainWindow *);
	static void createInitialCopies(QString const &,
									const int,
									const QSqlDatabase &,
									const QString &,
									QString &);
	static void exportPhotographs(const QSqlDatabase &,
								  const QString &,
								  const QString &,
								  const QList<QGraphicsItem *> &,
								  QWidget *);
	static void exportPhotographs(const QSqlDatabase &,
								  const QString &,
								  const int,
								  const int,
								  const QString &,
								  QWidget *);
	static void hideAdminFields(QMainWindow *);
	static void highlightWidget(QWidget *, const QColor &);
	static void saveQuantity(const QSqlDatabase &,
							 const QString &,
							 const int,
							 const QString &,
							 QString &);
	static void updateColumn(QTableWidget *,
							 const int,
							 const int,
							 const QString &);
	static void updateColumnColor(QTableWidget *,
								  const int,
								  const int,
								  const QColor &);

private:
	biblioteq_misc_functions(void);
	~biblioteq_misc_functions();
};

#endif
