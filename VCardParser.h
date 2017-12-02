#ifndef VCARDPARSER_H
#define VCARDPARSER_H





#include "ContactBook.h"




// fwd:
class QIODevice;





class VCardParser
{
public:

	/** Parses the vCard data from a_Source into the destination contact book a_Dest.
	Throws an EException descendant on error. Note that in such a case a_Dest may contain contacts / data
	that parsed successfully before the error was encountered.
	Reads the entire a_Source until there's no more data to read. */
	static void parse(QIODevice & a_Source, ContactBookPtr a_Dest);

	/** Parses the vCard data from a_Source into the destination contact a_Dest.
	Throws an EException descendant on error. Note that in such a case a_Dest may contain data that parsed
	successfully before the error was encountered.
	Finishes parsing when it encounters the "END:VCARD" line, leaving the rest of a_Source available for
	further reading.
	a_LineNumberOffset is the linenumber of the last line read from a_Source.
	Returns the linenumber of the last line read from a_Source for parsing the contact. */
	static int parse(QIODevice & a_Source, ContactPtr a_Dest, int a_LineNumberOffset = 0);

	/** Breaks into parts a VCard value that follows the regular composition rules:
	Components are delimited by semicolons, parts within components are delimited by commas.
	The backslashes are unescaped properly; any escaping errors are ignored (with a qWarning).
	The assumed format is "<component1part1>,<component1part2>,...;<component2part1>,<component2part2>,..." */
	static std::vector<std::vector<QByteArray>> breakValueIntoParts(const QByteArray & a_Value);

	/** Breaks into components a VCard value that follows the regular composition rules:
	Components are delimited by semicolons, using a backslash as an escape char.
	The backslashes are unescaped properly; any escaping errors are ignored (with a qWarning).
	The assumed format is "<component1>;<component2>;..." */
	static std::vector<QByteArray> breakValueIntoComponents(const QByteArray & a_Value);

	/** Breaks into parts a VCard component that follows the regular composition rules:
	Components are delimited by commas, using a backslash as an escape char.
	The backslashes are unescaped properly; any escaping errors are ignored (with a qWarning).
	The assumed format is "<part1>,<part2>,..." */
	static std::vector<QByteArray> breakComponentIntoParts(const QByteArray & a_Component);

	/** Removes the backslash escaping for the specified value.
	\; -> ;
	\: -> :
	\, -> ,
	\\ -> \
	\n -> <LF>
	\N -> <LF>
	\xAB -> <0xAB> */
	static QByteArray unescapeBackslashes(const QByteArray & a_Part);

	// TODO: Encoding parser and serializer (base64, quoted-printable etc.)
};





#endif // VCARDPARSER_H
