#include "VCardParser.h"
#include <assert.h>
#include <QIODevice>
#include <QByteArray>
#include <QDebug>
#include "Exceptions.h"
#include "Contact.h"





/** Converts a (displayable) hex character into its numeric value.
Returns 0 if the input is not a hex character. */
static int charToHex(char a_HexChar)
{
	switch (a_HexChar)
	{
		case '0': return 0x00;
		case '1': return 0x01;
		case '2': return 0x02;
		case '3': return 0x03;
		case '4': return 0x04;
		case '5': return 0x05;
		case '6': return 0x06;
		case '7': return 0x07;
		case '8': return 0x08;
		case '9': return 0x09;
		case 'a': return 0x0a;
		case 'b': return 0x0b;
		case 'c': return 0x0c;
		case 'd': return 0x0d;
		case 'e': return 0x0e;
		case 'f': return 0x0f;
		case 'A': return 0x0a;
		case 'B': return 0x0b;
		case 'C': return 0x0c;
		case 'D': return 0x0d;
		case 'E': return 0x0e;
		case 'F': return 0x0f;
	}
	qWarning() << __FUNCTION__ << ": Bad hex character: " << a_HexChar;
	return 0;
}





/** Provides the actual parsing implementation. */
class VCardParserImpl
{
public:
	/** Creates a new parser instance and binds it to the specified data source and destination contact.
	a_CurrentLineNum is the line number of the first line in a_Source, used for reporting errors. */
	VCardParserImpl(QIODevice & a_Source, ContactPtr a_Dest, int a_CurrentLineNum):
		m_State(psIdle),
		m_Source(a_Source),
		m_Dest(a_Dest),
		m_CurrentLineNum(a_CurrentLineNum)
	{
	}



	/** Parses the source data from m_Source into the bound destination contact book m_Dest.
	Throws an EException descendant on error. Note that m_Dest may still be filled with some contacts
	that parsed successfully.
	Returns the line number of the line that was last parsed from the source. */
	int parse()
	{
		// Parse line-by-line, unwrap the lines here:
		QByteArray acc;  // Accumulator for the current line
		while (!m_Source.atEnd())
		{
			QByteArray cur = m_Source.readLine();
			// Remove the trailing CR/LF:
			if (cur.endsWith('\n'))
			{
				cur.remove(cur.size() - 1, 1);
			}
			if (cur.endsWith('\r'))
			{
				cur.remove(cur.size() - 1, 1);
			}
			if (QString::fromUtf8(cur).toLower() == "end:vcard")
			{
				// This is the last line to be parsed, we can't afford to buffer it in the un-folder
				if (!acc.isEmpty())
				{
					processLine(acc);
				}
				return m_CurrentLineNum;
			}
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
					if (processLine(acc))
					{
						return m_CurrentLineNum;
					}
				}
				std::swap(acc, cur);
			}
		}
		// Process the last line:
		if (!acc.isEmpty())
		{
			if (!processLine(acc))
			{
				throw EParseError(__FILE__, __LINE__, "Contact data is incomplete, missing the END:VCARD sentence.");
			}
		}
		return m_CurrentLineNum;
	}


protected:

	/** The state of the outer state-machine, checking the sentences' sequencing (begin, version, <data>, end). */
	enum State
	{
		psIdle,        //< The parser has no current contact, it expects a new "BEGIN:VCARD" sentence
		psBeginVCard,  //< The parser has just read the "BEGIN:VCARD" line, expects a "VERSION" sentence
		psContact,     //< The parser is reading individual contact property sentence
		psFinished,    //< The parser has finished the contact, no more data is expected
	} m_State;

	/** The source data provider. */
	QIODevice & m_Source;

	/** The current contact being parsed.
	Only valid in the psContact state. */
	ContactPtr m_Dest;

	/** The number of the line currently being processed (for error reporting). */
	int m_CurrentLineNum;





	/** Parses the given single (unfolded) line.
	Returns true if this line is a terminator for the contact (no more lines should be parsed for this
	contact - the "END:VCARD" line). */
	bool processLine(const QByteArray & a_Line)
	{
		m_CurrentLineNum += 1;
		if (a_Line.isEmpty() || (a_Line == "\n"))
		{
			return false;
		}
		try
		{
			auto sentence = breakUpSentence(a_Line);
			switch (m_State)
			{
				case psIdle:       processSentenceIdle(sentence);       break;
				case psBeginVCard: processSentenceBeginVCard(sentence); break;
				case psContact:
				{
					if (processSentenceContact(sentence))
					{
						return true;
					}
					break;
				}
				case psFinished:
				{
					qWarning() << __FUNCTION__ << ": The parser has already finished parsing the contacts.";
					assert(!"Parsing already finished, should not be here");
					return true;
				}
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
		return false;
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
		Contact::SentenceParam * currentParam = nullptr;  // The parameter currently being parsed (already added to the contact)
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
						res.m_Params.emplace_back(currentParamName);
						currentParam = &res.m_Params.back();
						last = i + 1;
						currentParamValue.clear();
						sentenceState = ssParamValue;
						continue;
					}
					if (ch == ';')
					{
						// Value-less parameter with another parameter following ("TEL;CELL;OTHER:...")
						res.m_Params.emplace_back(a_Line.mid(last, i - last));
						last = i + 1;
						currentParamValue.clear();
						currentParamName.clear();
						continue;
					}
					if (ch == ':')
					{
						// Value-less parameter ending the params ("TEL;CELL:...")
						res.m_Params.emplace_back(a_Line.mid(last, i - last));
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
						currentParamValue.clear();
						continue;
					}
					if (ch == ',')
					{
						assert(currentParam != nullptr);
						currentParam->m_Values.push_back(a_Line.mid(last, i - last));
						last = i + 1;
						continue;
					}
					if (ch == ':')
					{
						assert(currentParam != nullptr);
						currentParam->m_Values.push_back(a_Line.mid(last, i - last));
						last = i + 1;
						res.m_Value = a_Line.mid(i + 1, len - i - 1);
						return res;
					}
					if (ch == ';')
					{
						assert(currentParam != nullptr);
						currentParam->m_Values.push_back(a_Line.mid(last, i - last));
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
						currentParam->m_Values.push_back(currentParamValue);
						currentParamValue.clear();
						last = i + 1;
						sentenceState = ssParamValueEnd;
						continue;
					}
					if (ch == '\\')
					{
						i += 1;
						if (i >= len)
						{
							throw EParseError(__FILE__, __LINE__, "Invalid parameter value escape at the end of sentence");
						}
						auto nextCh = a_Line.at(i);
						switch (nextCh)
						{
							case ',': currentParamValue.append(','); break;
							case ';': currentParamValue.append(';'); break;
							case 'n': currentParamValue.append('\n'); break;
							case '\\': currentParamValue.append('\\'); break;
							default:
							{
								throw EParseError(__FILE__, __LINE__, "Invalid parameter value escape char");
							}
						}
						continue;
					}
					if (ch == ',')
					{
						currentParam->m_Values.push_back(currentParamValue);
						last = i + 1;
						currentParamValue.clear();
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

		m_State = psContact;
	}





	/** Parses the given single (unfolded) line in the psContact parser state.
	May modify the sentence / trash it.
	Returns true if this sentence is a terminator for the contact (no more sentences should be parsed for
	this contact - the "END:VCARD" sentence). */
	bool processSentenceContact(Contact::Sentence & a_Sentence)
	{
		// If the sentence is "END:VCARD", terminate:
		if (
			a_Sentence.m_Group.isEmpty() &&
			(a_Sentence.m_Key == "end") &&
			a_Sentence.m_Params.empty() &&
			(a_Sentence.m_Value.toLower() == "vcard")
		)
		{
			m_State = psFinished;
			return true;
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
		m_Dest->addSentence(a_Sentence);
		return false;
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
	int lineNum = 0;
	while (!a_Source.atEnd())
	{
		auto contact = a_Dest->createNewContact();
		lineNum = parse(a_Source, contact, lineNum);
	}
}





int VCardParser::parse(QIODevice & a_Source, ContactPtr a_Dest, int a_LineNumberOffset)
{
	VCardParserImpl impl(a_Source, a_Dest, a_LineNumberOffset);
	return impl.parse();
}





std::vector<std::vector<QByteArray>> VCardParser::breakValueIntoParts(const QByteArray & a_Value)
{
	std::vector<std::vector<QByteArray>> res;
	auto components = breakValueIntoComponents(a_Value);
	res.reserve(components.size());
	for (const auto component: components)
	{
		res.push_back(breakComponentIntoParts(component));
	}
	return res;
}





std::vector<QByteArray> VCardParser::breakValueIntoComponents(const QByteArray & a_Value)
{
	std::vector<QByteArray> res;
	QByteArray curComponent;
	auto len = a_Value.length();
	for (int i = 0; i < len; ++i)
	{
		auto ch = a_Value.at(i);
		switch (ch)
		{
			case ';':
			{
				// Start a new component:
				res.push_back(std::move(curComponent));
				curComponent = QByteArray();
				break;
			}

			case '\\':
			{
				// Unescape:
				// vCard uses the following escapes: \:, \;  , \, , \\ , \n , \N , \xAB , \XAB
				i = i + 1;
				if (i < len)
				{
					switch (a_Value.at(i))
					{
						case ':':  curComponent.append(':');  break;
						case ';':  curComponent.append(';');  break;
						case ',':  curComponent.append(',');  break;
						case '\\': curComponent.append('\\'); break;
						case 'n':
						case 'N':
						{
							curComponent.append('\n');
							break;
						}
						case 'x':
						case 'X':
						{
							if (i + 2 < len)
							{
								curComponent.append(static_cast<char>(
									charToHex(a_Value.at(i + 1)) * 16 +
									charToHex(a_Value.at(i + 2))
								));
								i = i + 2;
							}
							else
							{
								// Bad escape, ignore?
								qWarning() << __FUNCTION__ << ": Bad hex escape detected at the end of value: " << a_Value;
							}
							break;
						}
					}
					curComponent.append(a_Value.at(i));
				}
				break;
			}  // case '\\'

			default:
			{
				curComponent.append(ch);
				break;
			}
		}  // switch (ch)
	}  // for i - a_Value[]
	if (!curComponent.isEmpty())
	{
		res.push_back(curComponent);
	}

	return res;
}





std::vector<QByteArray> VCardParser::breakComponentIntoParts(const QByteArray & a_Component)
{
	// Identical to breakValueIntoComponents, but splits on a comma, rather than semicolon
	std::vector<QByteArray> res;
	QByteArray curPart;
	auto len = a_Component.length();
	for (int i = 0; i < len; ++i)
	{
		auto ch = a_Component.at(i);
		switch (ch)
		{
			case ',':
			{
				// Start a new part:
				res.push_back(std::move(curPart));
				curPart = QByteArray();
				break;
			}

			case '\\':
			{
				// Unescape:
				// vCard uses the following escapes: \:, \;  , \, , \\ , \n , \N , \xAB , \XAB
				i = i + 1;
				if (i < len)
				{
					switch (a_Component.at(i))
					{
						case ':':  curPart.append(':');  break;
						case ';':  curPart.append(';');  break;
						case ',':  curPart.append(',');  break;
						case '\\': curPart.append('\\'); break;
						case 'n':
						case 'N':
						{
							curPart.append('\n');
							break;
						}
						case 'x':
						case 'X':
						{
							if (i + 2 < len)
							{
								curPart.append(static_cast<char>(
									charToHex(a_Component.at(i + 1)) * 16 +
									charToHex(a_Component.at(i + 2))
								));
								i = i + 2;
							}
							else
							{
								// Bad escape, ignore?
								qWarning() << __FUNCTION__ << ": Bad hex escape detected at the end of component: " << a_Component;
							}
							break;
						}
					}
					curPart.append(a_Component.at(i));
				}
				break;
			}  // case '\\'

			default:
			{
				curPart.append(ch);
				break;
			}
		}  // switch (ch)
	}  // for i - a_Value[]
	if (!curPart.isEmpty())
	{
		res.push_back(curPart);
	}

	return res;
}





QByteArray VCardParser::unescapeBackslashes(const QByteArray & a_Part)
{
	QByteArray res;
	auto len = a_Part.length();
	for (int i = 0; i < len; ++i)
	{
		auto ch = a_Part.at(i);
		if (ch != '\\')
		{
			res.push_back(ch);
			continue;
		}
		i = i + 1;
		if (i >= len)
		{
			qWarning() << __FUNCTION__ << ": Invalid escape at the end of value: " << a_Part;
			return res;
		}
		// vCard uses the following escapes: \:, \;  , \, , \\ , \n , \N , \xAB , \XAB
		switch (a_Part.at(i))
		{
			case ':':  res.append(':');  break;
			case ';':  res.append(';');  break;
			case ',':  res.append(',');  break;
			case '\\': res.append('\\'); break;
			case 'n':
			case 'N':
			{
				res.append('\n');
				break;
			}
			case 'x':
			case 'X':
			{
				if (i + 2 < len)
				{
					res.append(static_cast<char>(
						charToHex(a_Part.at(i + 1)) * 16 +
						charToHex(a_Part.at(i + 2))
					));
					i = i + 2;
				}
				else
				{
					// Bad escape, ignore
					qWarning() << __FUNCTION__ << ": Bad hex escape detected at the end of value: " << a_Part;
				}
				break;
			}
		}
	}  // for i - a_Value[]
	return res;
}
