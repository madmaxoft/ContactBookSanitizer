#include "VCardParser.h"
#include <assert.h>
#include <QIODevice>
#include <QByteArray>
#include <QDebug>
#include "Exceptions.h"
#include "Contact.h"





/** Provides the actual parsing implementation. */
class VCardParserImpl
{
public:
	/** Creates a new parser instance and binds it to the specified data source and destination contact book. */
	VCardParserImpl(QIODevice & a_Source, ContactBookPtr a_Dest):
		m_State(psIdle),
		m_Source(a_Source),
		m_Dest(a_Dest),
		m_CurrentLineNum(0)
	{
	}



	/** Parses the source data from m_Source into the bound destination contact book m_Dest.
	Throws an EException descendant on error. Note that m_Dest may still be filled with some contacts
	that parsed successfully. */
	void parse()
	{
		// Parse line-by-line, unwrap the lines here:
		QByteArray acc;  // Accumulator for the current line
		while (!m_Source.atEnd())
		{
			QByteArray cur = m_Source.readLine();
			if (
				!cur.isEmpty() &&
				(
					(cur.at(0) == 0x20) ||  // Continuation by a SP
					(cur.at(0) == 0x09) ||  // Continuation by a HT
					!cur.contains(':')      // Some bad serializers don't fold the lines properly, continuations are made without the leading space (LG G4)
				)
			)
			{
				// This was a folded line, append it to the accumulator:
				cur.remove(0, 1);
				acc.append(cur);
			}
			else
			{
				// This is a (start of a) new line, process the accumulator and store the new line in it:
				if (!acc.isEmpty())
				{
					processLine(acc);
				}
				std::swap(acc, cur);
			}
		}
		// Process the last line:
		if (!acc.isEmpty())
		{
			processLine(acc);
		}
	}


protected:

	/** The state of the outer state-machine, checking the sentences' sequencing (begin, version, <data>, end). */
	enum State
	{
		psIdle,        //< The parser has no current contact, it expects a new "BEGIN:VCARD" sentence
		psBeginVCard,  //< The parser has just read the "BEGIN:VCARD" line, expects a "VERSION" sentence
		psContact,     //< The parser is reading individual contact property sentence
	} m_State;

	/** The source data provider. */
	QIODevice & m_Source;

	/** The contact book into which the data is to be written. */
	ContactBookPtr m_Dest;

	/** The current contact being parsed.
	Only valid in the psContact state. */
	ContactPtr m_CurrentContact;

	/** The number of the line currently being processed (for error reporting). */
	int m_CurrentLineNum;





	/** Parses the given single (unfolded) line. */
	void processLine(const QByteArray & a_Line)
	{
		m_CurrentLineNum += 1;
		if (a_Line.isEmpty() || (a_Line == "\n"))
		{
			return;
		}
		try
		{
			auto sentence = breakUpSentence(a_Line);
			switch (m_State)
			{
				case psIdle:       processSentenceIdle(sentence);       break;
				case psBeginVCard: processSentenceBeginVCard(sentence); break;
				case psContact:    processSentenceContact(sentence);    break;
			}
		}
		catch (const EParseError & exc)
		{
			qWarning() << QString::fromUtf8("Cannot parse VCARD, line %1: %2. The line contents: \"%3\"")
				.arg(m_CurrentLineNum)
				.arg(QString::fromStdString(exc.m_Message))
				.arg(QString::fromUtf8(a_Line));
			throw;
		}
	}





	/** Breaks the specified single (unfolded) line into the contact sentence representation. */
	Contact::Sentence breakUpSentence(const QByteArray & a_Line)
	{
		Contact::Sentence res;

		// The state of the inner state-machine, parsing a single sentence.
		// Each sentence has a basic structure of "[group.]key[;param1;param2]=value"
		enum
		{
			ssBegin,             //< The parser is reading the beginning of the sentence (group or key)
			ssKey,               //< The parser is reading the key (the group was present)
			ssParamName,         //< The parser is reading the param name
			ssParamValue,        //< The parser is reading the param value
			ssParamValueDQuote,  //< The parser is reading the param value and is inside a dquote
			ssParamValueEnd,     //< The parser has just finished the DQuote of a param value
		} sentenceState = ssBegin;

		auto len = a_Line.length();
		assert(len > 0);
		if (a_Line.at(len - 1) == '\n')
		{
			assert(len > 1);
			len -= 1;
		}
		int last = 0;
		QByteArray currentParamName, currentParamValue;
		for (auto i = 0; i < len; ++i)
		{
			auto ch = a_Line.at(i);
			switch (sentenceState)
			{
				case ssBegin:
				{
					if (ch == '.')
					{
						res.m_Group = a_Line.mid(0, i - 1);
						last = i + 1;
						sentenceState = ssKey;
						continue;
					}
					// fallthrough
				}  // case ssBegin

				case ssKey:
				{
					if (ch == ';')
					{
						if (last == i)
						{
							throw EParseError(__FILE__, __LINE__, "An empty key is not allowed");
						}
						res.m_Key = a_Line.mid(last, i - last).toLower();
						last = i + 1;
						sentenceState = ssParamName;
						continue;
					}
					if (ch == ':')
					{
						if (last == i)
						{
							throw EParseError(__FILE__, __LINE__, "An empty key is not allowed");
						}
						res.m_Key = a_Line.mid(last, i - last).toLower();
						res.m_Value = a_Line.mid(i + 1, len - i - 1);
						return res;
					}
					if (ch == '.')
					{
						throw EParseError(__FILE__, __LINE__, "A group has already been parsed, cannot add another one.");
					}
					break;
				}  // case ssKey

				case ssParamName:
				{
					if (ch == '=')
					{
						if (i == last)
						{
							throw EParseError(__FILE__, __LINE__, "A parameter with no name is not allowed");
						}
						currentParamName = a_Line.mid(last, i - last).toLower();
						last = i + 1;
						currentParamValue.clear();
						sentenceState = ssParamValue;
						continue;
					}
					if (ch == ';')
					{
						// Value-less parameter with another parameter following ("TEL;CELL;OTHER:...")
						res.m_Params.emplace_back(a_Line.mid(last, i - last), QByteArray());
						last = i + 1;
						currentParamValue.clear();
						currentParamName.clear();
						continue;
					}
					if (ch == ':')
					{
						// Value-less parameter ending the params ("TEL;CELL:...")
						res.m_Params.emplace_back(a_Line.mid(last, i - last), QByteArray());
						last = i + 1;
						last = i + 1;
						res.m_Value = a_Line.mid(i + 1, len - i - 1);
						return res;
					}
					break;
				}  // case ssParamName

				case ssParamValue:
				{
					if (ch == '"')
					{
						if (i > last)
						{
							throw EParseError(__FILE__, __LINE__, "Param value double-quoting is wrong");
						}
						last = i + 1;
						sentenceState = ssParamValueDQuote;
						continue;
					}
					if (ch == ',')
					{
						res.m_Params.emplace_back(currentParamName, a_Line.mid(last, i - last));
						last = i + 1;
						continue;
					}
					if (ch == ':')
					{
						res.m_Params.emplace_back(currentParamName, a_Line.mid(last, i - last));
						last = i + 1;
						res.m_Value = a_Line.mid(i + 1, len - i - 1);
						return res;
					}
					if (ch == ';')
					{
						res.m_Params.emplace_back(currentParamName, a_Line.mid(last, i - last));
						last = i + 1;
						sentenceState = ssParamName;
						currentParamName.clear();
						continue;
					}
					break;
				}  // case ssParamValue

				case ssParamValueDQuote:
				{
					if (ch == '"')
					{
						res.m_Params.emplace_back(currentParamName, std::move(currentParamValue));
						last = i + 1;
						sentenceState = ssParamValueEnd;
						continue;
					}
					if (ch == '\\')
					{
						// Skip the escape
						continue;
					}
					else
					{
						currentParamValue.append(ch);
					}
					break;
				}  // case ssParamValueDQuote

				case ssParamValueEnd:
				{
					if (ch == ':')
					{
						last = i + 1;
						res.m_Value = a_Line.mid(i + 1, len - i - 1);
						return res;
					}
					if (ch == ';')
					{
						last = i + 1;
						sentenceState = ssParamName;
						continue;
					}
					throw EParseError(__FILE__, __LINE__, "An invalid character following a param value double-quote");
				}  // case ssParamValueEnd
			}
		}  // for i - a_Line[]

		throw EParseError(__FILE__, __LINE__, "Incomplete sentence");
	}





	/** Processes the given sentence in the psIdle parser state. */
	void processSentenceIdle(const Contact::Sentence & a_Sentence)
	{
		// The only valid line in this context is the "BEGIN:VCARD" line.
		// Any other line will cause an error throw
		if (
			!a_Sentence.m_Group.isEmpty()
		)
		{
			throw EParseError(__FILE__, __LINE__, "Expected a BEGIN:VCARD sentence, got a different sentence");
		}
		m_State = psBeginVCard;
	}





	/** Parses the given single sentence in the psBeginVCard parser state. */
	void processSentenceBeginVCard(const Contact::Sentence & a_Sentence)
	{
		// The only valid sentence in this context is the "VERSION:X" line.
		if (
			!a_Sentence.m_Group.isEmpty() ||
			(a_Sentence.m_Key != "version") ||
			!a_Sentence.m_Params.empty()
		)
		{
			throw EParseError(__FILE__, __LINE__, "Expected a VERSION sentence, got a different sentence");
		}
		if (a_Sentence.m_Value.toFloat() == 0)
		{
			throw EParseError(__FILE__, __LINE__, "The VERSION sentence has an invalid value.");
		}

		m_CurrentContact.reset(new Contact);
		m_State = psContact;
	}





	/** Parses the given single (unfolded) line in the psContact parser state.
	May modify the sentence / trash it. */
	void processSentenceContact(Contact::Sentence & a_Sentence)
	{
		// If the sentence is "END:VCARD", terminate the current contact:
		if (
			a_Sentence.m_Group.isEmpty() &&
			(a_Sentence.m_Key == "end") &&
			a_Sentence.m_Params.empty() &&
			(a_Sentence.m_Value.toLower() == "vcard")
		)
		{
			if (m_CurrentContact != nullptr)
			{
				m_Dest->addContact(m_CurrentContact);
			}
			m_CurrentContact.reset();
			m_State = psIdle;
			return;
		}

		// Decode any encoding on the sentence's value:
		std::vector<QByteArray> enc;
		for (const auto & p: a_Sentence.m_Params)
		{
			if (p.m_Name == "encoding")
			{
				enc = p.m_Values;
			}
		}
		decodeValueEncoding(a_Sentence, enc);

		// Add the sentence to the current contact:
		m_CurrentContact->addSentence(a_Sentence);
	}





	/** Decodes the sentence's value according to the specified encoding.
	a_EncodingSpec is the encoding to be decoded, represented as a VCard property values. */
	void decodeValueEncoding(Contact::Sentence & a_Sentence, const std::vector<QByteArray> & a_EncodingSpec)
	{
		for (const auto & enc: a_EncodingSpec)
		{
			auto lcEnc = enc.toLower();
			if ((lcEnc == "b") || (lcEnc == "base64"))
			{
				a_Sentence.m_Value = QByteArray::fromBase64(a_Sentence.m_Value);
				return;
			}
			if (lcEnc == "quoted-printable")
			{
				a_Sentence.m_Value = decodeQuotedPrintable(a_Sentence.m_Value);
				return;
			}
		}
	}





	/** Returns the data after performing a quoted-printable decoding on it. */
	static QByteArray decodeQuotedPrintable(const QByteArray & a_Src)
	{
		QByteArray res;
		auto len = a_Src.length();
		res.reserve(len / 3);
		for (int i = 0; i < len; ++i)
		{
			auto ch = a_Src.at(i);
			if (ch == '=')
			{
				if (i + 2 == len)
				{
					// There's only one char left in the input, cannot decode -> copy to output
					res.append(a_Src.mid(i));
					return res;
				}
				auto hex = a_Src.mid(i + 1, 2);
				bool isOK;
				auto decodedCh = hex.toInt(&isOK, 16);
				if (isOK)
				{
					res.append(decodedCh);
				}
				else
				{
					res.append(a_Src.mid(i, 3));
				}
				i += 2;
			}
			else
			{
				res.append(ch);
			}
		}
		return res;
	}
};





void VCardParser::parse(QIODevice & a_Source, ContactBookPtr a_Dest)
{
	VCardParserImpl impl(a_Source, a_Dest);
	impl.parse();
}





std::vector<std::vector<QByteArray>> VCardParser::breakValueIntoComponents(const QByteArray & a_Value)
{
	std::vector<std::vector<QByteArray>> res;
	res.push_back({});
	auto * curComponent = &res.back();
	curComponent->push_back({});
	auto * curValue = &curComponent->back();
	auto len = a_Value.length();
	for (int i = 0; i < len; ++i)
	{
		auto ch = a_Value.at(i);
		switch (ch)
		{
			case ';':
			{
				// Start a new component:
				res.push_back({});
				curComponent = &res.back();
				curComponent->push_back({});
				curValue = &curComponent->back();
				break;
			}  // case ';'

			case ',':
			{
				// Start a new value:
				curComponent->push_back({});
				curValue = &curComponent->back();
				break;
			}

			case '\\':
			{
				// Skip the escape char and push the next char directly into the current value:
				i = 1 + 1;
				curValue->append(a_Value.at(i));
				break;
			}

			default:
			{
				curValue->append(ch);
				break;
			}
		}  // switch (ch)
	}  // for i - a_Value[]

	return res;
}




