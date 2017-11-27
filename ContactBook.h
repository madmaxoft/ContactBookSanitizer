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

	// Force a virtual destructor in descendants:
	virtual ~ContactBook() {}

	const QString & displayName() const { return m_DisplayName; }
	void setDisplayName(const QString & a_NewDisplayName);

	/** Adds a new contact to the container.
	Note that descendants can override this to return a pointer to a Contact descendant.
	The default implementation creates an empty Contact instance. */
	virtual ContactPtr createNewContact();

	/** Returns a read-only reference to all the contained contacts. */
	const std::vector<ContactPtr> & contacts() const { return m_Contacts; }


protected:

	/** The (source) name to be displayed with the contact book. */
	QString m_DisplayName;

	/** All the contained contacts. */
	std::vector<ContactPtr> m_Contacts;

	/** Adds the specified contact into m_Contacts.
	TODO: Emits the appropriate signals. */
	void addContact(ContactPtr a_Contact);


signals:

	void displayNameChanged(const QString & a_NewDisplayName);

public slots:
};

typedef std::shared_ptr<ContactBook> ContactBookPtr;





#endif // CONTACTBOOK_H
