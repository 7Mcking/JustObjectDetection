
//
// Created by Naren Sadhwani on 18.04.24.
//

#ifndef JUSTOBJECTDETECTION_MAINWINDOW_H
#define JUSTOBJECTDETECTION_MAINWINDOW_H

#include <QMainWindow>
#include <QMenuBar>
#include <QAction>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QStatusBar>
#include <QLabel>
#include <QListView>
#include <QCheckBox>
#include <QPushButton>
#include <QGraphicsPixmapItem>
#include <QMutex>
#include <QStandardItem>

#include <opencv2/opencv.hpp>
#include "captureThread.h"
#include "utilities.h"

class MainWindow : public QMainWindow{
Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private:
    void initUI();
    void createActions();
    void populateSavedList();

    QMenu *fileMenu;
    QAction *cameraInfoAction;
    QAction *openCameraAction;
    QAction *exitAct;

    QGraphicsScene *imageScene;
    QGraphicsView *imageView;
    QPushButton *shutterButton;

    QListView *savedList;
    QStandardItemModel *savedListModel;

    QStatusBar *statusBar;
    QLabel *mainStatusLabel;

    cv::Mat currentFrame;

    //For thread
    QMutex *dataLock;
    captureThread *cameraThread;


private slots:
    void showCameraInfo();
    void openCamera();
    void updateFrame(cv::Mat *frame);
    void takePhoto();
    void appendSavedPhoto();

};


#endif //JUSTOBJECTDETECTION_MAINWINDOW_H
