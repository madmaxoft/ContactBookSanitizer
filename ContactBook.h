#ifndef CONTACTBOOK_H
#define CONTACTBOOK_H




#include <memory>
#include <QObject>

#include "Contact.h"





/** Container of multiple Contact instances, logically coming from a single source. */
class ContactBook:
	public QObject
{
	Q_OBJECT

	typedef QObject Super;

public:

	/** Creates a new instance, with the specified display name. */
	explicit ContactBook(const QString & a_DisplayName);

	const QString & displayName() const { return m_DisplayName; }
	void setDisplayName(const QString & a_NewDisplayName);

	/** Adds the specified contact to the container. */
	void addContact(ContactPtr a_Contact);

	/** Returns a read-only reference to all the contained. */
	const std::vector<ContactPtr> & getContacts() const { return m_Contacts; }


protected:

	/** The (source) name to be displayed with the contact book. */
	QString m_DisplayName;

	/** All the contained contacts. */
	std::vector<ContactPtr> m_Contacts;


signals:

	void displayNameChanged(const QString & a_NewDisplayName);

public slots:
};

typedef std::shared_ptr<ContactBook> ContactBookPtr;





#endif // CONTACTBOOK_H
