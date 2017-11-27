#ifndef CONTACT_H
#define CONTACT_H





#include <vector>
#include <memory>

#include <QByteArray>





class Contact
{
public:
	/** Wrapper for VCard sentence parameters.
	Each parameter has a name and an array of values.
	The m_Name is lowercased before being stored. */
	struct SentenceParam
	{
		QByteArray m_Name;
		std::vector<QByteArray> m_Values;

		/** Creates a new instance that has no value attached to it.
		This is used for v2.1 value-less parameters, such as in "TEL;WORK:..." */
		SentenceParam(const QByteArray & a_Name):
			m_Name(a_Name)
		{
		}

		/** Creates a new instance with a single value attached to it.
		This is used for regular v4.0 parameters, such as in "TEL;TYPE=WORK..." */
		SentenceParam(const QByteArray & a_Name, QByteArray && a_Value):
			m_Name(a_Name)
		{
			m_Values.push_back(std::move(a_Value));
		}
	};

	using SentenceParams = std::vector<SentenceParam>;

	/** Encapsulates an entire VCard sentence.
	Each sentence has a basic structure of "[group.]key[;param1;param2]=value"
	The m_Group and m_Key are lowercased before being stored in the contact. */
	struct Sentence
	{
		QByteArray m_Group;
		QByteArray m_Key;
		SentenceParams m_Params;
		QByteArray m_Value;
	};



	// Force destructors in all descendants to be virtual:
	virtual ~Contact() {}

	/** Adds a new VCard sentence to the contact. */
	void addSentence(const Sentence & a_Sentence);

	/** Returns all the VCard sentences currently present in the contact. */
	const std::vector<Sentence> & sentences() const { return m_Sentences; }

protected:

	/** The VCard sentences associated with this contact. */
	std::vector<Sentence> m_Sentences;
};

using ContactPtr = std::shared_ptr<Contact> ;





#endif // CONTACT_H
