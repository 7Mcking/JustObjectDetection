//
// Created by Naren Sadhwani on 18.04.24.
//

#include <QObject>
#include <QApplication>
#include <QDateTime>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>
#include "../Headers/utilities.h"

QString utilities::getDataPath() {
    auto userPicturesPath =  QStandardPaths::standardLocations(QStandardPaths::PicturesLocation)[0];
    QDir picturesDir(userPicturesPath);
    picturesDir.mkpath("JustObjectDetection");
    return picturesDir.absoluteFilePath("JustObjectDetection");
}

QString utilities::newPhotoName() {
    QDateTime time = QDateTime::currentDateTime();
    return time.toString("yyyy-MM-dd+HH:mm:ss");
}

QString utilities::getPhotoPath(QString name, QString postfix) {
    return QString("%1/%2.%3").arg(utilities::getDataPath(), name, postfix);
}
