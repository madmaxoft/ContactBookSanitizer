#ifndef CONTACTBOOKMODEL_H
#define CONTACTBOOKMODEL_H





#include <QStandardItemModel>

#include "ContactBook.h"






// fwd:
class DisplayContact;
using DisplayContactPtr = std::shared_ptr<DisplayContact>;





/** Provides a model for displaying a contact book.
Each contact is a top level item in a tree, and has sub-items for each element in their vcard. */
class ContactBookModel:
	public QStandardItemModel
{
	Q_OBJECT
	typedef QStandardItemModel Super;


public:

	/** Creates a new instance bound to the specified contact book. */
	explicit ContactBookModel(ContactBookPtr a_ContactBook);

	/** Changes the contact book being modelled to the specified one, redrawing the data as necessary. */
	void setContactBook(ContactBookPtr a_ContactBook);


protected:

	/** The contact book being modelled. */
	ContactBookPtr m_ContactBook;

	/** Contacts from m_ContactBook, parsed for UI display. */
	std::vector<DisplayContactPtr> m_DisplayContacts;


	/** Updates the children of the specified item representing a contact, to match the current contact data. */
	void updateContactItem(QStandardItem * a_ContactItem);
};





#endif // CONTACTBOOKMODEL_H
