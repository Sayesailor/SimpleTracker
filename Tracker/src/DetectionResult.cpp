#include "stdafx.h"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "include/DetectionResult.h"
#include "include/GlobalDef.h"
#include <sstream>

namespace tracker {

DetectionResult::DetectionResult()
    : objId_(INIT_OBJ_ID), frmIdx_(0), deltaFrmCnt_(0), vx_(0), vy_(0) {}

DetectionResult::~DetectionResult() {}

#ifdef SUPPORT_PT_DETECTOR
std::vector<seeta::FacialLandmark>& DetectionResult::getFmks() { return fmks_; }
#endif

std::string& DetectionResult::getName() { return name_; }

void DetectionResult::setVxVy(int32_t vx, int32_t vy) {
    vx_ = vx;
    vy_ = vy;

    if (deltaFrmCnt_ > 0) {
        mefi_ = fi_;
        mefi_.x = fi_.x + static_cast<int32_t>(deltaFrmCnt_) * vx_ / 100;
        mefi_.y = fi_.y + static_cast<int32_t>(deltaFrmCnt_) * vy_ / 100;
    }
}

void DetectionResult::scale(float s) {
    vx_ = static_cast<int32_t>((vx_ * 1.0f) / s);
    vy_ = static_cast<int32_t>((vy_ * 1.0f) / s);
    fi_.x = static_cast<int32_t>((fi_.x * 1.0f) / s);
    fi_.y = static_cast<int32_t>((fi_.y * 1.0f) / s);
    fi_.width = static_cast<int32_t>((fi_.width * 1.0f) / s);
    fi_.height = static_cast<int32_t>((fi_.height * 1.0f) / s);
    mefi_.x = static_cast<int32_t>((mefi_.x * 1.0f) / s);
    mefi_.y = static_cast<int32_t>((mefi_.y * 1.0f) / s);
    mefi_.width = static_cast<int32_t>((mefi_.width * 1.0f) / s);
    mefi_.height = static_cast<int32_t>((mefi_.height * 1.0f) / s);
}

void DetectionResult::render(cv::Mat* pmat, bool showId, bool showMeBox,
                             bool showFiBox, bool showName,
                             cv::Scalar rgbDefault) {
    if (pmat) {
        if (showFiBox) {
            cv::Rect rt;
            rt.x = fi_.x;
            rt.y = fi_.y;
            rt.width = fi_.width;
            rt.height = fi_.height;
            cv::Scalar s(0, 255, 0);
            cv::rectangle(*pmat, rt, s);
        }

        if (showName) {
            cv::Scalar s(255, 0, 0);
            cv::putText(*pmat, "", cv::Point(fi_.x, fi_.y), 0, 1.0f, s);
        }

#ifdef SUPPORT_PT_DETECTOR
        for (auto it : fmks_)
            cv::circle(*pmat, cv::Point(it.x, it.y), 2, cv::Scalar(0, 255, 0));
#endif

        if (showId && objId_ > 0) {
            char buf[16] = {0};
            sprintf(buf, "id: %d", objId_);
            cv::Scalar s(255, 0, 255);
            cv::putText(*pmat, buf, cv::Point(fi_.x, fi_.y), 0, 1.0f, s);
        }

    }
}
}
