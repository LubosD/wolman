#ifndef ___EXCEPTION_H
#define ___EXCEPTION_H

#include <QString>

class Exception
{
public:
	Exception(QString str = QString())
	: m_error(str)
	{
	}
	QString what() const
	{
		return m_error;
	}
private:
	QString m_error;
};

#endif
