#ifndef SESSION_H
#define SESSION_H





#include <memory>

#include <QObject>





// fwd:
class ContactBook;
typedef std::shared_ptr<ContactBook> ContactBookPtr;





class Session:
	public QObject
{
	Q_OBJECT

	typedef QObject Super;

public:

	/** Creates a new instance, with no ContactBooks contained within. */
	explicit Session();

	/** Adds the specified contact book to the session. */
	void addContactBook(ContactBookPtr a_ContactBook);

	/** Returns all the contact books currently stored within. */
	const std::vector<ContactBookPtr> & getContactBooks() const { return m_ContactBooks; }

	/** Returns a writable reference to the specified read-only ContactBook.
	Returns nullptr if the ContactBook is not part of this session. */
	ContactBookPtr getContactBook(const ContactBook * a_ContactBook);

protected:
	std::vector<ContactBookPtr> m_ContactBooks;

signals:

public slots:
};

#endif // SESSION_H
