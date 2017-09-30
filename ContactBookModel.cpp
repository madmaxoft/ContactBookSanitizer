#include "ContactBookModel.h"
#include <assert.h>
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
			item->setData(QVariant(reinterpret_cast<qulonglong>(contact.get())));
			appendRow(item);
			updateContactItem(item);
		}
	}
}





void ContactBookModel::updateContactItem(QStandardItem * a_ContactItem)
{
	a_ContactItem->setRowCount(0);
	auto contact = reinterpret_cast<const Contact *>(a_ContactItem->data().toULongLong());

	// Add numbers:
	for (const auto & number: contact->getNumbers())
	{
		auto label = new QStandardItem(getNumberUsageLabel(number.m_Usage, number.m_CustomUsage));
		auto val   = new QStandardItem(number.m_Number);
		QList<QStandardItem *> numbers{label, val};
		a_ContactItem->appendRow(numbers);
	}  // for number

	// Add emails:
	for (const auto & email: contact->getEmails())
	{
		a_ContactItem->appendRow(QList<QStandardItem *>
		{
			new QStandardItem(getEmailUsageLabel(email.m_Usage, email.m_CustomUsage)),
			new QStandardItem(email.m_Email)
		});
	}
}





QString ContactBookModel::getNumberUsageLabel(Contact::Number::Usage a_NumberUsage, const QString & a_CustomUsage)
{
	switch (a_NumberUsage)
	{
		case Contact::Number::cnuMobile: return QString::fromUtf8("Mobile");
		case Contact::Number::cnuHome:   return QString::fromUtf8("Home");
		case Contact::Number::cnuWork:   return QString::fromUtf8("Work");
		case Contact::Number::cnuFax:    return QString::fromUtf8("Fax");
		case Contact::Number::cnuPager:  return QString::fromUtf8("Pager");
		case Contact::Number::cnuOther:  return QString::fromUtf8("Other");
		case Contact::Number::cnuCustom: return a_CustomUsage;
	}

	#ifndef __clang__
		assert(!"Invalid Number usage");
		return QString::fromUtf8("Invalid");
	#endif
}





QString ContactBookModel::getEmailUsageLabel(Contact::Email::Usage a_EmailUsage, const QString & a_CustomUsage)
{
	switch (a_EmailUsage)
	{
		case Contact::Email::ceuHome:   return QString::fromUtf8("Home");
		case Contact::Email::ceuWork:   return QString::fromUtf8("Work");
		case Contact::Email::ceuMobile: return QString::fromUtf8("Mobile");
		case Contact::Email::ceuOther:  return QString::fromUtf8("Other");
		case Contact::Email::ceuCustom: return a_CustomUsage;
	}

	#ifndef __clang__
		assert(!"Invalid Email usage");
		return QString::fromUtf8("Invalid");
	#endif
}





