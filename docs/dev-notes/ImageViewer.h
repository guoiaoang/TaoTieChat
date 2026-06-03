/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#pragma once

#include <QMainWindow>
#include <QImage>
#if defined(QT_PRINTSUPPORT_LIB)
#  include <QtPrintSupport/qtprintsupportglobal.h>

#  if QT_CONFIG(printer)
#    include <QPrinter>
#  endif
#endif

QT_BEGIN_NAMESPACE
class QAction;
class QLabel;
class QMenu;
class QScrollArea;
class QScrollBar;
class QWheelEvent;
QT_END_NAMESPACE

//! [0]
class ImageViewer : public QMainWindow {
    Q_OBJECT

public:
    static ImageViewer *instance() {
        static ImageViewer s_image_viewer;
        return &s_image_viewer;
    }
    ~ImageViewer();

private:
    ImageViewer(QWidget *parent = nullptr);
    ImageViewer              (const ImageViewer & ) = delete;
    ImageViewer              (      ImageViewer &&) = delete;
    ImageViewer & operator = (const ImageViewer & ) = delete;
    ImageViewer & operator = (      ImageViewer &&) = delete;

public:
    bool loadFile(const QString &);

private slots:
    void open();
    void saveAs();
    void print();
    void copy();
    void paste();
    void zoomIn();
    void zoomOut();
    void normalSize();
    void fitToWindow();
    void about();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void wheelEvent (QWheelEvent  *event) override;

private:
    void createActions();
    void createMenus();
    void updateActions();
    bool saveFile(const QString &fileName);
    void setImage(const QImage &newImage);
    void scaleImage(double factor);
    void adjustScrollBar(QScrollBar *scrollBar, double factor);

#if defined(QT_PRINTSUPPORT_LIB) && QT_CONFIG(printer)
    QPrinter printer;
#endif

    double       scaleFactor    = 1;
    QImage       image;
    QLabel      *imageLabel     = nullptr;
    QScrollArea *scrollArea     = nullptr;
    QAction     *saveAsAct      = nullptr;
    QAction     *printAct       = nullptr;
    QAction     *copyAct        = nullptr;
    QAction     *zoomInAct      = nullptr;
    QAction     *zoomOutAct     = nullptr;
    QAction     *normalSizeAct  = nullptr;
    QAction     *fitToWindowAct = nullptr;
};
//! [0]

// TODO: ~~1. 修改初始大小~~
// TODO: ~~2. 默认居中缩放图像~~
// TODO: ~~3. 默认保持比例缩放并适应窗口大小~~
// TODO: 析构时崩溃
// TODO: 窗口已经显示时，图像打开不会重置缩放选项
// TDOO: 原始大小菜单无效

// 图片支持鼠标缩放，视频不支持鼠标缩放
