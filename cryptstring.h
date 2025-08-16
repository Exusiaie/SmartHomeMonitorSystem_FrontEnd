#ifndef CRYPTSTRING_H
#define CRYPTSTRING_H

#include <QCryptographicHash>
#include <QString>
class CryptString
{
public:
    CryptString(const QString & data);
    QString generateMD5() const;
private:
    QString _data;
};


#endif // CRYPTSTRING_H
