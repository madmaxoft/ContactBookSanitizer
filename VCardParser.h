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

	/** Breaks into components a VCard value that follows the regular composition rules:
	Components are delimited by semicolons, multiple values in a component are delimited by a comma. */
	static std::vector<std::vector<QByteArray>> breakValueIntoComponents(const QByteArray & a_Value);

	// TODO: Encoding parser and serializer (base64, quoted-printable etc.)
};





#endif // VCARDPARSER_H
