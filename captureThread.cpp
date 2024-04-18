//
// Created by Naren Sadhwani on 18.04.24.
//

#include <QTime>
#include <QDebug>
#include "captureThread.h"
#include "utilities.h"

captureThread::captureThread(int camera, QMutex *lock):
running(false),
cameraID(camera),
videoPath(""),
dataLock(lock)
{
    frameWidth=frameHeight=0;
    takingPhoto = false;
}

captureThread::captureThread(QString videoPath, QMutex *lock):
running(false),
cameraID(-1),
videoPath(videoPath),
dataLock(lock)
{
    frameWidth=frameHeight=0;
    takingPhoto = false;
}

captureThread::~captureThread() = default;


void captureThread::run() {
    running = true;
    cv::VideoCapture cap(cameraID);
    // Add for video
    cv::Mat tempFrame;

    frameWidth = cap.get(cv::CAP_PROP_FRAME_WIDTH);
    frameHeight = cap.get(cv::CAP_PROP_FRAME_HEIGHT);

    classifier = new cv::CascadeClassifier(std::string(OPENCV_DATA_DIR) + "haarcascades/haarcascade_frontalcatface_extended.xml");

    while(running){
        cap >> tempFrame;
        if(tempFrame.empty()){
            break;
        }

        if (takingPhoto){
            takePhoto(tempFrame);
        }

        detectObjects(tempFrame);

        cv::cvtColor(tempFrame, tempFrame, cv::COLOR_BGR2RGB);
        dataLock->lock();
        frame = tempFrame;
        dataLock->unlock();
        emit frameCaptured(&frame);
    }

    cap.release();
    delete classifier;
    classifier = nullptr;
    running = false;

}

void captureThread::takePhoto(cv::Mat &frame) {
    QString photoName =  utilities::newPhotoName();
    auto photoPath =  utilities::getPhotoPath(photoName, "jpg");
    cv::imwrite(photoPath.toStdString(), frame);
    emit photoTaken(photoPath);
    takingPhoto = false;
}

void captureThread::detectObjects(cv::Mat &frame) {
    vector<cv::Rect> objects;
    classifier->detectMultiScale(frame, objects);

    cv::Scalar color = cv::Scalar(255, 0, 0);

    // Draw rectangles
    for (auto &object : objects){
        cv::rectangle(frame, object, color, 2);
    }

}
