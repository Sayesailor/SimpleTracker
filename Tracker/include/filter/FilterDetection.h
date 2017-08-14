#ifndef TRACKER_INCLUDE_FILTERDETECTION_H_
#define TRACKER_INCLUDE_FILTERDETECTION_H_

#include "include/DetectionResult.h"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <map>

namespace tracker {

class FilterDetection {
   public:
    FilterDetection() : object_id_iter_(1) {}

    virtual ~FilterDetection() {}

    void doDetect(const cv::Mat& img, const uint32_t crtFrmIdx,
                  std::vector<DetectionResult>& drs);

    void doRedetect(const cv::Mat& img, const uint32_t crtFrmIdx,
                    const std::vector<DetectionResult>& drsGlobal,
                    std::vector<DetectionResult>& drsLocal);

   private:
    void calcVelocity(const DetectionResult& curDR,
                      const DetectionResult& preDR, int& vx, int& vy);

    void doTrackDetect(const cv::Mat& img, const uint32_t curFrmIdx,
                       std::vector<DetectionResult>& drs);

    float calcIntersectionTotalRatio(const DetectionResult& t1,
                                     const DetectionResult& t2);

    void addNewTracker(const uint32_t curFrmIdx,
                       const std::vector<DetectionResult>& drsGlobal);

    bool findInTrackers(const std::vector<DetectionResult>& drs,
                        const DetectionResult& dr);

    // BE CAREFUL, MEMORY ALLOCED IN genPartImgData(), RELEASE img.data AFTER
    // USED
    VIPLImageData genPartImgData(const cv::Mat& img, int32_t& x, int32_t& y,
                                 uint32_t& w, uint32_t& h);

    void extenMotionEstimation(VIPLFaceInfo& fi, const float multiW,
                               const float multiH);

   public:
    unsigned long tickDbgFd_;

   private:
    struct tracker_item {
        DetectionResult cur;
        DetectionResult pre;
    };
    int32_t object_id_iter_;
    std::map<int32_t, tracker_item> trackers_;
};
}

#endif
