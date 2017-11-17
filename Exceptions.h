// Exceptions.h

// Declares the exception types used by the project





#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H





class EException:
	public std::runtime_error
{
	typedef std::runtime_error Super;

public:

	/** The source file name where the exception was thrown. */
	std::string m_SrcFileName;

	/** The source file line number where the exception was thrown. */
	int m_SrcLine;

	explicit EException(const char * a_SrcFileName, int a_SrcLine):
		Super("EException"),
		m_SrcFileName(a_SrcFileName),
		m_SrcLine(a_SrcLine)
	{
	}
};





class EParseError:
	public EException
{
	using Super = EException;

public:

	/** The parser's message attached to this error. */
	std::string m_Message;


	explicit EParseError(const char * a_SrcFileName, int a_SrcLine, const char * a_Message):
		Super(a_SrcFileName, a_SrcLine),
		m_Message(a_Message)
	{
	}
};





class EFileError:
	public EException
{
	using Super = EException;

public:
	/** The file on which the operation failed. */
	QString m_OperationFileName;

	/** The description of the failure. */
	QString m_Message;


	/** Creates a new exception to throw.
	a_FileName is the file on which the operation failed.
	a_Message is the description of the failure. */
	explicit EFileError(
		const char * a_SrcFileName, int a_SrcLine,
		const QString & a_OperationFileName,
		const QString & a_Message
	):
		Super(a_SrcFileName, a_SrcLine),
		m_OperationFileName(a_OperationFileName),
		m_Message(a_Message)
	{
	}
};





class EDavResponseException:
	public EException
{
	using Super = EException;


public:

	/** The description of the failure. */
	QString m_Message;


	explicit EDavResponseException(
		const char * a_SrcFileName, int a_SrcLine,
		const QString & a_Message
	):
		Super(a_SrcFileName, a_SrcLine),
		m_Message(a_Message)
	{
	}
};





#endif // EXCEPTIONS_H
