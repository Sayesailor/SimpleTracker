#ifndef TRACKER_INCLUDE_CAMERABOX_H_
#define TRACKER_INCLUDE_CAMERABOX_H_

#include <mutex>
#include "DetectionResult.h"
#include "filter/FilterDetection.h"
#include "CvvImage.h"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#ifdef _MSC_VER
#ifdef VIPL_EXPORTS
#define VIPL_API __declspec(dllexport)
#else
#define VIPL_API __declspec(dllimport)
#endif
#else
#define VIPL_API
#endif

namespace tracker {

enum CAMBOX_STATUS {
    INIT,
    RUNNING,
    PAUSED,
    STOPED,
};

class VIPL_API CameraBox {
   public:
    CameraBox();
    virtual ~CameraBox();

#ifndef RENDER_WITH_OPENCV
    bool init(HWND);
    const HWND gethWnd() const { return hWnd_; }
#else
    bool init(int);
    const int gethWnd() const { return hWnd_; }
#endif
    const CAMBOX_STATUS getStatus() const { return status_; }
    void draw(const cv::Mat& mat);
    void start();
    void pause();
    void resume();
    void release();
    void result(std::vector<VIPLFaceInfo>& lastresult, int& frameIdx);

   public:
    // for debug
    unsigned long tickDbgGlobal_;
    unsigned long tickDbgLocal_;

   public:
    struct TK_RECT {
        long left;
        long top;
        long right;
        long bottom;
    };

   private:
    void doDetect();
    void doRedetect();
    static void startThreadDetectBackground(CameraBox* p);
    static void startThreadRedetectBackground(CameraBox* p);
    void getTargetRect(const TK_RECT orgrt, const TK_RECT wndrt, TK_RECT& tgrt);

    void renderDetectResult();
    void renderFrmCnt();
    void printDbgInfo(const unsigned long val, const int x, const int y);

    const uint32_t getFrmIdx() const { return nFrameCnt_; }

   private:
#ifndef RENDER_WITH_OPENCV
    HWND hWnd_;
#else
    int hWnd_;
#endif

    cv::Mat frameOrig_;

    cv::Mat frameGlobal_;
    uint32_t frameIdxGlobal_;
    cv::Mat frameLocal_;
    uint32_t frameIdxLocal_;
    cv::Mat frameCanvas_;

#ifndef RENDER_WITH_OPENCV
    CvvImage cvvimage_;
#endif

    int scale_;

    CAMBOX_STATUS status_;
    uint32_t nFrameCnt_;
    std::mutex mtx_;
    std::mutex mtxCpyImg_;
    FilterDetection fd_;
    std::vector<DetectionResult> local_detection_result_;
    std::vector<DetectionResult> global_detection_result_;
    std::vector<DetectionResult> last_detection_result_;

    uint32_t frmcnt_;
    unsigned long tick_;
};
}

#endif
