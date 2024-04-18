//
// Created by Naren Sadhwani on 18.04.24.
//

#include "MainWindow.h"
#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QPixmap>
#include <QCameraInfo>
#include <QGridLayout>
#include <QIcon>
#include <QStandardItem>
#include <QSize>
#include <opencv2/videoio.hpp>
#include "utilities.h"




MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    fileMenu(nullptr),
    cameraThread(nullptr)
{
    initUI();
    dataLock =  new QMutex();
}

MainWindow::~MainWindow() = default;


void MainWindow::initUI() {
    this->resize(1000,800);
    fileMenu =  menuBar()->addMenu(tr("&File"));

    // Main Area
    auto *mainLayout = new QGridLayout();
    imageScene = new QGraphicsScene();
    imageView = new QGraphicsView(imageScene);
    mainLayout->addWidget(imageView, 0, 0, 12, 1);

    //tools
    auto *toolsLayout = new QGridLayout();
    mainLayout->addLayout(toolsLayout, 12, 0,1, 1);

    //Shutter Button
    shutterButton = new QPushButton("Take Photo");
    toolsLayout->addWidget(shutterButton, 0, 0, Qt::AlignHCenter);
    connect(shutterButton, SIGNAL(clicked(bool)), this, SLOT(takePhoto()));

    //list of saved photos
    savedList = new QListView();
    savedList->setViewMode(QListView::IconMode);
    savedList->setResizeMode(QListView::Adjust);
    savedList->setSpacing(10);
    savedList->setWrapping(false);

    savedListModel = new QStandardItemModel();
    savedList->setModel(savedListModel);
    mainLayout->addWidget(savedList, 13, 0, 4, 1);

    auto *widget = new QWidget();
    widget->setLayout(mainLayout);
    setCentralWidget(widget);

    //status bar
    statusBar = new QStatusBar();
    mainStatusLabel = new QLabel();
    statusBar->addPermanentWidget(mainStatusLabel);
    mainStatusLabel->setText("Ready");

    createActions();
    populateSavedList();
}

void MainWindow::createActions() {
    cameraInfoAction = new QAction(tr("Camera Info"), this);
    fileMenu->addAction(cameraInfoAction);
    openCameraAction = new QAction(tr("Open Camera"), this);
    fileMenu->addAction(openCameraAction);
    exitAct = new QAction(tr("E&xit"), this);
    fileMenu->addAction(exitAct);

    // Connect signals and slots
    connect(cameraInfoAction, SIGNAL(triggered(bool)), this, SLOT(showCameraInfo()));
    connect(openCameraAction, SIGNAL(triggered(bool)), this, SLOT(openCamera()));
    connect(exitAct, SIGNAL(triggered(bool)), QApplication::instance(), SLOT(quit()));



}

void MainWindow::showCameraInfo() {
    QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    QString info = QString("Available Cameras: \n");
    foreach (const QCameraInfo &cameraInfo, cameras){
        info += QString("Name: %1\n").arg(cameraInfo.deviceName());
    }

    QMessageBox::information(this, "Camera Info", info);

}

void MainWindow::openCamera() {
    if (cameraThread != nullptr){
        cameraThread->setRunning(false);
        disconnect(cameraThread, &captureThread::frameCaptured, this, &MainWindow::updateFrame);
        disconnect(cameraThread, &captureThread::photoTaken, this, &MainWindow::appendSavedPhoto);
        // For deleting the thread
        connect(cameraThread, &captureThread::finished, cameraThread, &QObject::deleteLater);
    }

    int cameraId = 0;
    cameraThread = new captureThread(cameraId, dataLock);
    connect(cameraThread, &captureThread::frameCaptured, this , &MainWindow::updateFrame);
    connect(cameraThread, &captureThread::photoTaken, this, &MainWindow::appendSavedPhoto);
    cameraThread->start();
    mainStatusLabel->setText(QString("Capturing Camera %1").arg(cameraId));
}


void MainWindow::updateFrame(cv::Mat *mat) {
    dataLock->lock();
    currentFrame = *mat;
    dataLock->unlock();

    QImage frame(
            currentFrame.data,
            currentFrame.cols,
            currentFrame.rows,
            QImage::Format_RGB888
            );

    QPixmap pixmap = QPixmap::fromImage(frame);
    imageScene->clear();
    imageView->resetTransform();
    imageScene->addPixmap(pixmap);
    imageScene->update();
    imageView->setSceneRect(pixmap.rect());

}

void MainWindow::takePhoto() {
    if (cameraThread != nullptr){
        cameraThread->takePhoto();
    }
}

void MainWindow::populateSavedList() {
    QDir dir(utilities::getDataPath());
    QStringList nameFilters;
    nameFilters << "*.jpg";
    QFileInfoList files = dir.entryInfoList(nameFilters, QDir::Files, QDir::Time);
    foreach (QFileInfo photo, files){
        auto name = photo.fileName();
        auto *item = new QStandardItem();
        savedListModel->appendRow(item);
        auto index =  savedListModel->indexFromItem(item);
        savedListModel->setData(index, QPixmap(photo.absoluteFilePath()).scaledToHeight(100), Qt::DecorationRole);
        savedListModel->setData(index, name, Qt::DisplayRole);
    }

}

void MainWindow::appendSavedPhoto() {
    QString photoPath = utilities::getPhotoPath(utilities::newPhotoName(), "jpg");
    auto *item = new QStandardItem();
    savedListModel->appendRow(item);
    auto index = savedListModel->indexFromItem(item);
    savedListModel->setData(index, QPixmap(photoPath).scaledToHeight(100), Qt::DecorationRole);
    savedListModel->setData(index, photoPath, Qt::DisplayRole);
    savedList->scrollTo(index);

}
