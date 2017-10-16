#include "ContactBookModel.h"
#include <assert.h>
#include "ContactBook.h"
#include "DisplayContact.h"





ContactBookModel::ContactBookModel(ContactBookPtr a_ContactBook):
	Super(nullptr),
	m_ContactBook(nullptr)
{
	setContactBook(a_ContactBook);
}





void ContactBookModel::setContactBook(ContactBookPtr a_ContactBook)
{
	// Clear previous contents:
	clear();
	m_DisplayContacts.clear();
	setColumnCount(2);

	// Insert the current contents:
	m_ContactBook = a_ContactBook;
	if (a_ContactBook != nullptr)
	{
		for (const auto & contact: a_ContactBook->getContacts())
		{
			auto displayContact = DisplayContact::fromContact(*contact);
			m_DisplayContacts.push_back(displayContact);
			auto item = new QStandardItem(displayContact->displayName());
			item->setData(QVariant(reinterpret_cast<qulonglong>(displayContact.get())));
			appendRow(item);
			updateContactItem(item);
		}
	}
}





void ContactBookModel::updateContactItem(QStandardItem * a_ContactItem)
{
	a_ContactItem->setRowCount(0);
	auto contact = reinterpret_cast<const DisplayContact *>(a_ContactItem->data().toULongLong());

	// Add items:
	for (const auto & item: contact->items())
	{
		for (const auto & v: item->m_Values)
		{
			auto label = new QStandardItem(item->m_Label);
			if (item->m_Icon != nullptr)
			{
				label->setIcon(*item->m_Icon);
			}
			auto val   = new QStandardItem(v);
			QList<QStandardItem *> numbers{label, val};
			a_ContactItem->appendRow(numbers);
		}
	}  // for number
}




