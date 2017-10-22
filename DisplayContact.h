#ifndef DISPLAYCONTACT_H
#define DISPLAYCONTACT_H





#include <memory>
#include <vector>

#include <QString>
#include <QIcon>

#include "Contact.h"





/** Encapsulation of the contact data used for displaying the contact.
The VCard data in the Contact instance are parsed into displayable items and elements by this class.
Each contact has a DisplayName, an optional picture, and some Item instances that describe all its data.
An item is an element that has a single label and one or many values associated to it.
Typically, it represents a single editable entry in the phonebook, such as "home number" (single-line)
or "work address" (multi-line). */
class DisplayContact:
	public QObject
{
	Q_OBJECT
	using Super = QObject;

public:

	/** Representation of a single logical item in the contact. */
	struct Item
	{
		const QIcon * m_Icon;  // May be nullptr
		QString m_Label;
		std::vector<QString> m_Values;

		Item(const QIcon * a_Icon, const QString & a_Label, const std::vector<QString> & a_Values):
			m_Icon(a_Icon),
			m_Label(a_Label),
			m_Values(a_Values)
		{
		}
	};

	using ItemPtr = std::unique_ptr<Item>;


	/** Creates a new DisplayContact instance based on the specified Contact.
	Parses the VCard items in a_Contact into Item instances in the returned value. */
	static std::shared_ptr<DisplayContact> fromContact(const Contact & a_Contact);

	/** Returns the total (sum) number of Values across all the contact's Items. */
	int totalNumValues() const;

	/** Returns the display name for the contact. */
	const QString & displayName() const { return m_DisplayName; }

	const std::vector<ItemPtr> & items() const { return m_Items; }


protected:

	/** The contact from which this class was created. */
	const Contact & m_Contact;

	/** The main name to be displayed for the contact. */
	QString m_DisplayName;

	/** The picture to be shown for the contact (check isNull). */
	QPixmap m_Picture;

	/** All the items parsed from the contact. */
	std::vector<ItemPtr> m_Items;


	/** Creates a new instance and binds it to the specified contact.
	No data is parsed within this constructor! */
	DisplayContact(const Contact & a_Contact);

	/** Adds a new item for the specified "N" VCard sentence. */
	void addNameItem(const Contact::Sentence & a_NameSentence);

	/** Adds a new item for the specified "TEL" VCard sentence. */
	void addTelItem(const Contact::Sentence & a_TelSentence);

	/** Adds a new item for the specified "EMAIL" VCard sentence. */
	void addEmailItem(const Contact::Sentence & a_EmailSentence);

	/** Adds a new item with the specified contents. */
	void addItem(const QIcon * a_Icon, const QString & a_Label, const std::vector<QString> & a_Values);

	/** Composes a name out of its (VCard) components. */
	QString composeName(
		const std::vector<QByteArray> & a_FirstNames,
		const std::vector<QByteArray> & a_MiddleNames,
		const std::vector<QByteArray> & a_LastNames,
		const std::vector<QByteArray> & a_Prefixes,
		const std::vector<QByteArray> & a_Suffixes
	);

	/** Returns true if the sentence params either contain "type=<type>" (v4) or "<type>" (v2.1).
	Used to determine home vs work vs mobile vs ... type of sentence.
	a_LcType is the lowercase of the type to check. */
	static bool isType(const Contact::SentenceParams & a_SentenceParams, const QByteArray & a_LcType);

	// Getters for the static shared icons
	static QIcon * icoTel();
	static QIcon * icoEmail();
};

using DisplayContactPtr = std::shared_ptr<DisplayContact>;





#endif // DISPLAYCONTACT_H
