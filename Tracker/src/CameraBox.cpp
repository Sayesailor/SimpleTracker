#include "stdafx.h"
#include "include/CameraBox.h"
#include <thread>
#include "include/filter/Detectors.h"
#include "include/filter/FilterDetection.h"
#include "include/GlobalDef.h"

namespace tracker {

CameraBox::CameraBox()
    : hWnd_(0),
      scale_(PROCESS_IMAGE_SCALE),
      status_(CAMBOX_STATUS::INIT),
      nFrameCnt_(0),
      frmcnt_(0) {
    Detectors::getInstance();
#ifdef _WIN32
    tick_ = GetTickCount();
#elif __linux__
    tick_ = 0;
#endif
}

CameraBox::~CameraBox() {}

void CameraBox::start() {
    if (status_ == CAMBOX_STATUS::INIT) {
        status_ = CAMBOX_STATUS::RUNNING;

        std::thread global(startThreadDetectBackground, this);
        global.detach();
        std::thread local(startThreadRedetectBackground, this);
        local.detach();
    }
}

void CameraBox::pause() {
    if (status_ == CAMBOX_STATUS::RUNNING) status_ = CAMBOX_STATUS::PAUSED;
}

void CameraBox::resume() {
    if (status_ == CAMBOX_STATUS::PAUSED) status_ = CAMBOX_STATUS::RUNNING;
}

#ifndef RENDER_WITH_OPENCV
bool CameraBox::init(HWND hwnd) {
    hWnd_ = hwnd;
    status_ = CAMBOX_STATUS::INIT;
    return true;
}
#else
bool CameraBox::init(int hwnd) {
    hWnd_ = hwnd;
    status_ = CAMBOX_STATUS::INIT;
    return true;
}
#endif

void CameraBox::getTargetRect(const TK_RECT orgrt, const TK_RECT wndrt,
                              TK_RECT& tgrt) {
    float orgWidth = orgrt.right - orgrt.left * 1.0f;
    float orgHeight = orgrt.bottom - orgrt.top * 1.0f;
    float orgR = orgWidth / orgHeight;

    float targetWidth = (wndrt.right - wndrt.left) * 1.0f;
    float targetHeight = (wndrt.bottom - wndrt.top) * 1.0f;
    float destR = targetWidth / targetHeight;

    tgrt.left = 0;
    tgrt.top = 0;
    tgrt.right = wndrt.right - wndrt.left;
    tgrt.bottom = wndrt.bottom - wndrt.top;

    if (orgR > destR) {
        tgrt.left = 0;
        tgrt.top = static_cast<long>(
            (((tgrt.bottom - tgrt.top) - (1 / orgR) * targetWidth)) / 2.0f);
        tgrt.right = wndrt.right - wndrt.left;
        tgrt.bottom = static_cast<long>(tgrt.top + (1 / orgR) * targetWidth);
    } else {
        tgrt.top = 0;
        tgrt.left = static_cast<long>(
            ((tgrt.right - tgrt.left) - (orgR * targetHeight)) / 2.0f);
        tgrt.right = static_cast<long>(tgrt.left + orgR * targetHeight);
        tgrt.bottom = wndrt.bottom - wndrt.top;
    }
}

void CameraBox::draw(const cv::Mat& mat) {
    if (status_ == CAMBOX_STATUS::RUNNING) {
        mtxCpyImg_.lock();

        cv::Mat imgLarge;
        if (scale_ == 100) {
            frameCanvas_ = mat.clone();
            frameOrig_ = frameCanvas_.clone();
        } else {
            imgLarge = mat.clone();
            cv::resize(
                imgLarge, frameOrig_,
                cv::Size(static_cast<int>(imgLarge.cols * (scale_ / 100.0f)),
                         static_cast<int>(imgLarge.rows * (scale_ / 100.0f))));
            frameCanvas_ = imgLarge.clone();
        }
        nFrameCnt_++;
        mtxCpyImg_.unlock();
    }

    renderDetectResult();
    renderFrmCnt();

#ifndef RENDER_WITH_OPENCV
    IplImage pIplImg = frameCanvas_;
    cvvimage_.CopyOf(&pIplImg, 1);

    RECT orgrt = {0};
    orgrt.right = cvvimage_.Width();
    orgrt.bottom = cvvimage_.Height();

    HDC hDC = GetDC(hWnd_);
    RECT wndrt, tgrt;
    ::GetWindowRect(hWnd_, &wndrt);
    getTargetRect(orgrt, wndrt, tgrt);

    cvvimage_.DrawToHDC(hDC, &tgrt);

#else
    cv::imshow("", frameCanvas_);
#endif
}

void CameraBox::release() {
    status_ = CAMBOX_STATUS::STOPED;
    Sleep(1000);
    delete Detectors::getInstance();
}

void CameraBox::doDetect() {
    if (status_ == CAMBOX_STATUS::RUNNING) {
#ifdef _WIN32
        unsigned long tick = GetTickCount();
#elif __linux__
        unsigned long tick = 0;
#endif
        std::vector<DetectionResult> drs;
        mtxCpyImg_.lock();
        frameGlobal_ = frameOrig_.clone();
        frameIdxGlobal_ = nFrameCnt_;
        mtxCpyImg_.unlock();

        fd_.doDetect(frameGlobal_, frameIdxGlobal_, drs);

        mtx_.lock();
        if (drs.size() > 0)
            global_detection_result_ = drs;
        else
            local_detection_result_.clear();
        mtx_.unlock();
#ifdef _WIN32
        tickDbgGlobal_ = GetTickCount() - tick;
#elif __linux__
        tickDbgGlobal_ = 1 - tick;
#endif
    }
}

void CameraBox::doRedetect() {
    if (status_ == CAMBOX_STATUS::RUNNING) {
#ifdef _WIN32
        unsigned long tick = GetTickCount();
#elif __linux__
        unsigned long tick = 0;
#endif
        mtxCpyImg_.lock();
        frameLocal_ = frameOrig_.clone();
        frameIdxLocal_ = nFrameCnt_;
        mtxCpyImg_.unlock();

        mtx_.lock();
        fd_.doRedetect(frameLocal_, frameIdxLocal_, global_detection_result_,
                       local_detection_result_);
        last_detection_result_ = local_detection_result_;
        mtx_.unlock();
#ifdef _WIN32
        tickDbgLocal_ = GetTickCount() - tick;
#elif __linux__
        tickDbgLocal_ = 1 - tick;
#endif
    }
}

void CameraBox::result(std::vector<VIPLFaceInfo>& lastresult, int& frameIdx) {
    lastresult.clear();
    for (auto it : last_detection_result_) {
        lastresult.push_back(it.getFi());
        frameIdx = it.getFrmIdx();
    }
};

void CameraBox::renderDetectResult() {
    mtx_.lock();
    for (auto itdr : local_detection_result_) {
        itdr.scale(scale_ / 100.0f);
        itdr.render(&frameCanvas_, true  // showId
                    ,
                    false  // showMeBox
                    ,
                    true  // showFiBox
                    ,
                    false  // showName
                    ,
                    cv::Scalar(0, 255, 255));  // rgbDefault
    }
    mtx_.unlock();
}

void CameraBox::renderFrmCnt() {
    char buf[16] = {0};
    sprintf(buf, "%d", nFrameCnt_);
    cv::Scalar scalar(255, 255, 255);

    cv::putText(frameCanvas_, buf, cvPoint(80, 40), 0, 1,
                cv::Scalar(255, 255, 255), 2);

    const uint32_t status = Detectors::getInstance()->get_status();
    sprintf(buf, "%d", status);
    scalar = cv::Scalar(0, status > 0 ? 128 : 0, status > 0 ? 0 : 128);

    cv::putText(frameCanvas_, buf, cvPoint(40, 40), 0, 1,
                cv::Scalar(0, status > 0 ? 128 : 0, status > 0 ? 0 : 128), 2);

    printDbgInfo(tickDbgGlobal_, 800, 40);
    printDbgInfo(fd_.tickDbgFd_, 800, 80);

#ifdef _WIN32
    if (GetTickCount() - tick_ == 10000) {
        tick_ = GetTickCount();
        frmcnt_ = 0;
    }
    printDbgInfo(frmcnt_++ * 1000 / (GetTickCount() - tick_), 800, 120);
#endif
}

void CameraBox::printDbgInfo(const unsigned long val, const int x,
                             const int y) {
    char buf[16] = {0};
    sprintf(buf, "%lu", val);
    cv::Scalar scalar(255, 255, 255);

    cv::putText(frameCanvas_, buf, cvPoint(x, y), 0, 1,
                cv::Scalar(255, 255, 255), 2);
}

void CameraBox::startThreadDetectBackground(CameraBox* pcambox) {
    while (pcambox && pcambox->getStatus() != tracker::CAMBOX_STATUS::STOPED) {
        if (pcambox->getStatus() == tracker::CAMBOX_STATUS::RUNNING) {
            pcambox->doDetect();
        }
    }
}

void CameraBox::startThreadRedetectBackground(CameraBox* p) {
    CameraBox* pcambox = static_cast<CameraBox*>(p);
    while (pcambox && pcambox->getStatus() != tracker::CAMBOX_STATUS::STOPED) {
        if (pcambox->getStatus() == tracker::CAMBOX_STATUS::RUNNING)
            pcambox->doRedetect();
    }
}
}
