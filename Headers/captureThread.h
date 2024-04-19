
//
// Created by Naren Sadhwani on 18.04.24.
//

#ifndef JUSTOBJECTDETECTION_CAPTURETHREAD_H
#define JUSTOBJECTDETECTION_CAPTURETHREAD_H

#include <QString>
#include <QThread>
#include <QMutex>

#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>

using namespace std;


class captureThread: public QThread {
Q_OBJECT
public:
    explicit captureThread(int camera, QMutex *lock);
    explicit captureThread(QString videoPath, QMutex *lock);

    ~captureThread() override;

    void setRunning(bool run){running = run;}
    void takePhoto(){takingPhoto = true;}

protected:
    void run() override;

signals:
    void frameCaptured(cv::Mat *frame);
    void photoTaken(QString path);


private:
    void takePhoto(cv::Mat &frame);
    bool running;
    int cameraID;
    QString videoPath;
    QMutex *dataLock;
    cv::Mat frame;
    int frameWidth, frameHeight;
    bool takingPhoto;

    //DNN Object Detection
    cv::dnn::Net net;
    void detectObjectsDNN(cv::Mat &frame);
    vector<string> objectClasses;

    //Object Detection
    cv::CascadeClassifier *classifier;
    void detectObjects(cv::Mat &frame);

};


#endif //JUSTOBJECTDETECTION_CAPTURETHREAD_H
