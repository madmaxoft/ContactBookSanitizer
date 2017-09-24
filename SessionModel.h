#ifndef SESSIONMODEL_H
#define SESSIONMODEL_H





#include <QStandardItemModel>





// fwd:
class Session;
class ContactBook;





/** A model that displays a Session in a tree-like manner, down to its individual address-books. */
class SessionModel:
	public QStandardItemModel
{
	typedef QStandardItemModel Super;

public:

	/** Creates a new instance that displays the specified session. */
	explicit SessionModel(Session * a_Session);

	/** Resets the model to display the specified session. */
	void setSession(Session * a_Session);

	/** Returns the contact book that is represented by the specified model index.
	Returns nullptr if the model index doesn't represent a ContactBook. */
	const ContactBook * getContactBook(const QModelIndex & a_Index) const;


protected:

	/** The session to be modelled by this instance. */
	Session * m_Session;


};





#endif // SESSIONMODEL_H
