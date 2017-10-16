#ifndef VCARDPARSER_H
#define VCARDPARSER_H





#include "ContactBook.h"




// fwd:
class QIODevice;





class VCardParser
{
public:

	/** Parses the vCard data from a_Source into the destination contact book a_Dest.
	Throws an EException descendant on error. Note that in such a case a_Dest may contain contacts that
	parsed successfully before the error was encountered. */
	static void parse(QIODevice & a_Source, ContactBookPtr a_Dest);

	/** Breaks into components a VCard value that follows the regular composition rules:
	Components are delimited by semicolons, multiple values in a component are delimited by a comma. */
	static std::vector<std::vector<QByteArray>> breakValueIntoComponents(const QByteArray & a_Value);

	// TODO: Encoding parser and serializer (base64, quoted-printable etc.)
};





#endif // VCARDPARSER_H
