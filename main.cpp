#include "MainWindow.h"
#include <QApplication>
#include <QFile>
#include <QtXml/QDomDocument>

#include "Session.h"
#include "ContactBook.h"
#include "Contact.h"





/** Creates a new session and fills it with example data. */
std::unique_ptr<Session> makeExampleSession()
{
	std::unique_ptr<Session> session(new Session);
	ContactBookPtr book(new ContactBook(QString::fromUtf8("Example contact book")));
	session->addContactBook(book);
	ContactPtr contact(new Contact(QString::fromUtf8("Example contact")));
	contact->addNumber({Contact::Number::cnuWork, QString::fromUtf8("112")});
	contact->addEmail({Contact::Email::ceuWork, QString::fromUtf8("always.busy@emergency.com")});
	book->addContact(contact);
	return session;
}





/** Loads the session from the specified filename.
Returns the loaded session, or nullptr if the loading failed. */
std::unique_ptr<Session> loadSessionFromFile(const QString & a_FileName)
{
	QFile file(a_FileName);
	if (!file.open(QFile::ReadOnly | QFile::Text))
	{
		return nullptr;
	}
	QDomDocument doc;
	if (!doc.setContent(&file, false))
	{
		return nullptr;
	}

	// TODO: parse the document

	return makeExampleSession();
}





/** Loads the session, either from the current folder ("portable app") or from the user's home.
If neither contain any reasonable data, creates an empty data file. */
std::unique_ptr<Session> loadSession()
{
	// First try loading from the current folder ("portable app"):
	auto session = loadSessionFromFile("ContactBookSanitizer.cbsSession");
	if (session != nullptr)
	{
		return session;
	}

	// No current folder data, load from user's home:
	// QDir::
	session = loadSessionFromFile(/* TODO */ "ContactBookSanitizer.cbsSession");
	if (session != nullptr)
	{
		return session;
	}

	// No data could be read, create a new session file
	// First try writing it to the current folder; if it fails, we're installed, if it succeeds, we're portable
	session = makeExampleSession();
	// TODO
	return session;
}





int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	MainWindow w(loadSession());
	w.show();

	return a.exec();
}
