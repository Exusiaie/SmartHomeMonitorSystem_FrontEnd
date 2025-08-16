#include "CryptString.h"
CryptString::CryptString(const QString & data)
    : _data(data)
{}
QString CryptString::generateMD5() const
{
    QByteArray arr = QCryptographicHash::hash(_data.toUtf8(),
                                              QCryptographicHash::Md5);
    return arr.toHex();
}
