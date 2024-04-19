//
// Created by Naren Sadhwani on 18.04.24.
//

#include <QTime>
#include <QDebug>
#include "../Headers/captureThread.h"
#include "../Headers/utilities.h"
#include <fstream>




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

        //detectObjects(tempFrame);
        detectObjectsDNN(tempFrame);

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

static vector<string> getOutputsNames(const cv::dnn::Net &net);
static void decodeOutputLayers(cv::Mat &frame, const vector<cv::Mat> &outs,
                               vector<int> &outputClassIds, vector<float> &outputConfidences,
                               vector<cv::Rect> &outputBoundingBoxes);

void captureThread::detectObjectsDNN(cv::Mat &frame) {
    int inputHeight = 416;
    int inputWidth = 416;

    if(net.empty()){
        try{
            string modelConfiguration = "/Users/mcking/CLionProjects/JustObjectDetection/Yolo/yolov3.cfg";
            string modelWeights = "/Users/mcking/CLionProjects/JustObjectDetection/Yolo/yolov3.weights";
            net = cv::dnn::readNetFromDarknet(modelConfiguration, modelWeights);
            objectClasses.clear();
            string name;
            string classesFile = "/Users/mcking/CLionProjects/JustObjectDetection/Yolo/coco.names";
            ifstream ifs(classesFile.c_str());
            while (getline(ifs, name)) objectClasses.push_back(name);
        }
        catch (cv::Exception &e){
            qDebug() << "Error loading YOLO model";
            return;
        }

    }

    cv::Mat blob= cv::dnn::blobFromImage(frame,
                                         1/255.0,
                                         cv::Size(inputWidth,inputHeight),
                                         cv::Scalar(0,0,0),
                                         true, false);

    net.setInput(blob);

    //forward pass
    vector<cv::Mat> outs;
    net.forward(outs, getOutputsNames(net));

    vector<int> outputClassIds;
    vector<float> outputConfidences;
    vector<cv::Rect> outputBoundingBoxes;

    decodeOutputLayers(frame, outs, outputClassIds, outputConfidences, outputBoundingBoxes);

    for(size_t i = 0; i < outputClassIds.size(); i ++) {
        cv::rectangle(frame, outputBoundingBoxes[i], cv::Scalar(0, 0, 255));

        // get the label for the class name and its confidence
        string label = objectClasses[outputClassIds[i]];
        label += cv::format(":%.2f", outputConfidences[i]);

        // display the label at the top of the bounding box
        int baseLine;
        cv::Size labelSize = cv::getTextSize(label, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);
        int left = outputBoundingBoxes[i].x, top = outputBoundingBoxes[i].y;
        top = max(top, labelSize.height);
        cv::putText(frame, label, cv::Point(left, top), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255,255,255));
    }
}


vector<string> getOutputsNames(const cv::dnn::Net &net){
    static vector<string> names;
    vector<int> outLayers =  net.getUnconnectedOutLayers();
    vector<string> layersNames = net.getLayerNames();
    names.resize(outLayers.size());
    for (size_t i = 0; i < outLayers.size(); ++i){
        names[i] = layersNames[outLayers[i] - 1];
    }
    return names;
}

void decodeOutputLayers(cv::Mat &frame, const vector<cv::Mat> &outs,
                        vector<int> &outputClassIds, vector<float> &outputConfidences,
                        vector<cv::Rect> &outputBoundingBoxes){
    float confidenceThreshold = 0.5;
    float nmsThreshold = 0.4;

    vector<int> classIds;
    vector<float> confidences;
    vector<cv::Rect> boxes;

    for (const auto & out : outs){
        auto *data = (float *) out.data;
        for (int j = 0; j < out.rows; j++, data += out.cols){
            cv::Mat scores = out.row(j).colRange(5, out.cols);
            cv::Point classIdPoint;
            double confidence;
            cv::minMaxLoc(scores, 0, &confidence, 0, &classIdPoint);
            if (confidence > confidenceThreshold){
                int centerX = (int)(data[0] * frame.cols);
                int centerY = (int)(data[1] * frame.rows);
                int width = (int)(data[2] * frame.cols);
                int height = (int)(data[3] * frame.rows);
                int left = centerX - width / 2;
                int top = centerY - height / 2;

                classIds.push_back(classIdPoint.x);
                confidences.push_back((float)confidence);
                boxes.emplace_back(left, top, width, height);
            }
        }
    }

    // Perform non-maximum suppression
    vector<int> indices;
    cv::dnn::NMSBoxes(boxes, confidences, confidenceThreshold, nmsThreshold, indices);

    for (int idx : indices){
        outputBoundingBoxes.push_back(boxes[idx]);
        outputClassIds.push_back(classIds[idx]);
        outputConfidences.push_back(confidences[idx]);
    }

}