#ifndef TRACKER_INCLUDE_DETECTIONRESULT_H_
#define TRACKER_INCLUDE_DETECTIONRESULT_H_

#include <vector>
#include "VIPLStruct.h"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

namespace tracker {

class DetectionResult {
   public:
    DetectionResult();
    ~DetectionResult();

#ifdef SUPPORT_PT_DETECTOR
    std::vector<seeta::FacialLandmark>& getFmks();
#endif

    std::string& getName();

    void setFi(VIPLFaceInfo fi) { fi_ = fi; }
    void setMEFi(VIPLFaceInfo mefi) { mefi_ = mefi; }
    const VIPLFaceInfo& getFi() const { return fi_; }
    const VIPLFaceInfo& getMEFi() const { return mefi_; }

    void setFrmIdx(uint32_t frmidx) { frmIdx_ = frmidx; }
    const uint32_t getFrmIdx() const { return frmIdx_; }

    void setDeltaFrmCnt(uint32_t deltafrm) { deltaFrmCnt_ = deltafrm; }
    const uint32_t getDeltaFrmCnt() const { return deltaFrmCnt_; }

    void setVxVy(int32_t vx, int32_t vy);
    void getVxVy(int32_t& vx, int32_t& vy) {
        vx = vx_;
        vy = vy_;
    }

    void setObjId(int32_t id) { objId_ = id; }
    const int32_t getObjId() const { return objId_; }

    void scale(float s);

    void render(cv::Mat* pmat, bool showId, bool showMeBox, bool showFiBox,
                bool showName, cv::Scalar rgbDefault);

   private:
    VIPLFaceInfo fi_;
    VIPLFaceInfo mefi_;

#ifdef SUPPORT_PT_DETECTOR
    std::vector<seeta::FacialLandmark> fmks_;
#endif
    // -1 init, 0 catch as new obj, > 0 obj's id
    int32_t objId_;
    std::string name_;
    uint32_t frmIdx_;
    uint32_t deltaFrmCnt_;
    int32_t vx_;
    int32_t vy_;
};
}

#endif
