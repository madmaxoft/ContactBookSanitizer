#include "ContactBookModel.h"
#include "ContactBook.h"
#include "Contact.h"





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
	setColumnCount(2);

	// Insert the current contents:
	m_ContactBook = a_ContactBook;
	if (a_ContactBook != nullptr)
	{
		for (const auto & contact: a_ContactBook->getContacts())
		{
			auto item = new QStandardItem(contact->getDisplayName());
			appendRow(item);
			for (const auto & element: contact->getElements())
			{
				auto elementName = new QStandardItem(element.getTypeDesc());
				auto elementValue = new QStandardItem(element.m_Value);
				QList<QStandardItem *> element;
				element << elementName << elementValue;
				item->appendRow(element);
			}
		}
	}
}





