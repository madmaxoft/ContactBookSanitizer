#include "SessionModel.h"
#include "Session.h"
#include "ContactBook.h"





SessionModel::SessionModel(Session * a_Session):
	m_Session(nullptr)
{
	setSession(a_Session);
}





void SessionModel::setSession(Session * a_Session)
{
	// Clear the previous session's data:
	clear();

	// Add the new session's data:
	m_Session = a_Session;
	if (a_Session != nullptr)
	{
		for (const auto & cbook: a_Session->getContactBooks())
		{
			auto item = new QStandardItem(cbook->getDisplayName());
			item->setData(QVariant(reinterpret_cast<qulonglong>(cbook.get())));
			appendRow(item);
		}
	}
}

const ContactBook * SessionModel::getContactBook(const QModelIndex & a_Index) const
{
	auto item = itemFromIndex(a_Index);
	if (item == nullptr)
	{
		return nullptr;
	}
	return reinterpret_cast<const ContactBook *>(item->data().toULongLong());
}





