
//
// Created by Naren Sadhwani on 18.04.24.
//

#ifndef JUSTOBJECTDETECTION_UTILITIES_H
#define JUSTOBJECTDETECTION_UTILITIES_H

#include <QString>

class utilities {
public:
    static QString getDataPath();
    static QString newPhotoName();
    static QString getPhotoPath(QString name, QString postfix);

};


#endif //JUSTOBJECTDETECTION_UTILITIES_H
