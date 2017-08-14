#include "stdafx.h"
#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "VIPLStruct.h"
#include "include/filter/Detectors.h"
#include "include/filter/FilterDetection.h"
#include "include/DetectionResult.h"
#include "include/GlobalDef.h"
#include <cstdlib>

namespace tracker {

#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))

void FilterDetection::doDetect(const cv::Mat& img, const uint32_t crtFrmIdx,
                               std::vector<DetectionResult>& drs) {
    VIPLFaceDetector* pdetector_global =
        Detectors::getInstance()->get_detector_global();
    if (pdetector_global) {
        VIPLImageData img_data(img.cols, img.rows, img.channels());
        img_data.data = img.data;
        std::vector<VIPLFaceInfo> faces = pdetector_global->Detect(img_data);

        drs.clear();
        for (const auto it : faces) {
            DetectionResult dr;
            dr.setFi(it);
            dr.setMEFi(it);
            dr.setFrmIdx(crtFrmIdx);
            drs.push_back(dr);
        }
    }
}

void FilterDetection::doRedetect(const cv::Mat& img, const uint32_t crtFrmIdx,
                                 const std::vector<DetectionResult>& drsGlobal,
                                 std::vector<DetectionResult>& drsLocal) {
    addNewTracker(crtFrmIdx, drsGlobal);
    doTrackDetect(img, crtFrmIdx, drsLocal);
}

void FilterDetection::doTrackDetect(const cv::Mat& img,
                                    const uint32_t curFrmIdx,
                                    std::vector<DetectionResult>& drs) {
    if (img.cols <= 0) return;

    std::vector<DetectionResult> drsResult;
#ifdef _WIN32
    unsigned long tick = GetTickCount();
#elif __linux__
    unsigned long tick = 0;
#endif
    for (auto& it : trackers_) {
        DetectionResult last = it.second.cur;

        VIPLFaceInfo tmpfi = last.getMEFi();
        extenMotionEstimation(tmpfi, MOTION_ESTIMATION_EXPAND_W,
                              MOTION_ESTIMATION_EXPAND_H);

        int32_t x = tmpfi.x;
        int32_t y = tmpfi.y;
        uint32_t w = tmpfi.width;
        uint32_t h = tmpfi.height;

        VIPLImageData tmpLocalImg = genPartImgData(img, x, y, w, h);
        if (w == 0 || h == 0) continue;

        VIPLFaceDetector* pdetector_local =
            Detectors::getInstance()->get_detector_local();
        if (tmpLocalImg.data && pdetector_local) {
            std::vector<VIPLFaceInfo> faces =
                pdetector_local->Detect(tmpLocalImg);

            float maxRatio = 0;
            std::vector<VIPLFaceInfo>::iterator itFaceMax = faces.begin();
            for (std::vector<VIPLFaceInfo>::iterator itface = faces.begin();
                 itface != faces.end(); ++itface) {
                DetectionResult dr;
                VIPLFaceInfo fi = *itface;
                fi.x += x;
                fi.y += y;
                dr.setFi(fi);
                float s = calcIntersectionTotalRatio(dr, last);
                if (s > maxRatio) {
                    itFaceMax = itface;
                    maxRatio = s;
                }
            }
            if (maxRatio > TRACKER_THRESHOLD &&
                curFrmIdx - last.getFrmIdx() < FRAME_THRESHOLD) {
                DetectionResult dr(last);
                VIPLFaceInfo fi = *itFaceMax;
                fi.x += x;
                fi.y += y;
                dr.setFi(fi);
                dr.setFrmIdx(curFrmIdx);
                int32_t vx = 0;
                int32_t vy = 0;
                calcVelocity(dr, last, vx, vy);
                dr.setDeltaFrmCnt(dr.getFrmIdx() - last.getFrmIdx());
                dr.setVxVy(vx, vy);
                it.second.pre = it.second.cur;
                it.second.cur = dr;

                drsResult.push_back(dr);
            }

            delete[] tmpLocalImg.data;
        }
    }
#ifdef _WIN32
    tickDbgFd_ = GetTickCount() - tick;
#elif __linux__
    tickDbgFd_ = 1 - tick;
#endif
    if (drsResult.size()) drs = drsResult;
}

float FilterDetection::calcIntersectionTotalRatio(
    const DetectionResult& curDR, const DetectionResult& preDR) {
    int32_t minx = 0;
    int32_t miny = 0;
    int32_t maxx = 0;
    int32_t maxy = 0;

    minx = max(curDR.getFi().x, preDR.getFi().x);
    miny = max(curDR.getFi().y, preDR.getFi().y);
    maxx = min(curDR.getFi().x + curDR.getFi().width,
               preDR.getFi().x + preDR.getFi().width);
    maxy = min(curDR.getFi().y + curDR.getFi().height,
               preDR.getFi().y + preDR.getFi().height);

    if ((minx > maxx) || (miny > maxy)) {
        return 0;
    }

    int32_t area1 = curDR.getFi().width * curDR.getFi().height;
    int32_t area2 = preDR.getFi().width * preDR.getFi().height;

    float total_area = static_cast<float>(area1 + area2);
    float intersection = static_cast<float>((maxx - minx) * (maxy - miny));

    return intersection / (total_area - intersection);
}

void FilterDetection::calcVelocity(const DetectionResult& curDR,
                                   const DetectionResult& preDR, int& vx,
                                   int& vy) {
    if (curDR.getFrmIdx() - preDR.getFrmIdx() > 0) {
        vx = ((curDR.getFi().x - preDR.getFi().x) * CALC_SPEED_MULTI) /
             static_cast<int32_t>(curDR.getFrmIdx() - preDR.getFrmIdx());
        vy = ((curDR.getFi().y - preDR.getFi().y) * CALC_SPEED_MULTI) /
             static_cast<int32_t>(curDR.getFrmIdx() - preDR.getFrmIdx());
    } else {
        vx = 0;
        vy = 0;
    }
}

void FilterDetection::addNewTracker(
    const uint32_t curFrmIdx, const std::vector<DetectionResult>& drsGlobal) {
    // global detect has no result
    if (drsGlobal.empty()) return;

    std::vector<DetectionResult> tmpDrs;
    for (auto& itTracker : trackers_) tmpDrs.push_back(itTracker.second.cur);

    for (auto itLast : drsGlobal) {
        if (!findInTrackers(tmpDrs, itLast)) {
            VIPLFaceInfo fi = itLast.getFi();
            extenMotionEstimation(fi, MOTION_ESTIMATION_EXPAND_NEWEXTRA,
                                  MOTION_ESTIMATION_EXPAND_NEWEXTRA);
            DetectionResult new_dr(itLast);
            new_dr.setFi(fi);
            int32_t id = object_id_iter_++;
            new_dr.setObjId(id);
            trackers_[id].pre =
                trackers_[id].cur.getObjId() == -1 ? new_dr : trackers_[id].cur;
            trackers_[id].cur = new_dr;
        }
    }
}

bool FilterDetection::findInTrackers(const std::vector<DetectionResult>& drs,
                                     const DetectionResult& dr) {
    for (auto itdrs : drs) {
        float s = calcIntersectionTotalRatio(itdrs, dr);
        if (s > TRACK_MATCH_THRESHOLD) return true;
    }
    return false;
}

VIPLImageData FilterDetection::genPartImgData(const cv::Mat& img, int32_t& x,
                                              int32_t& y, uint32_t& w,
                                              uint32_t& h) {
    VIPLImageData resultImg;
    const uint32_t origW = img.cols;
    const uint32_t origH = img.rows;
    const uint32_t origChannels = img.channels();
    uint8_t* origData = reinterpret_cast<uint8_t*>(img.data);
    do {
        w = (w >= origW - x) ? (origW - x) : w;
        h = (h >= origH - y) ? (origH - y) : h;

        if (origData == NULL || x >= static_cast<int32_t>(origW) || x < 0 ||
            y >= static_cast<int32_t>(origH) || y < 0 || w == 0 || h == 0) {
            x = y = w = h = 0;
            break;
        }

        uint8_t* src_data = origData + (y * origW + x) * origChannels;
        uint8_t* dst_data = new uint8_t[w * h * origChannels];
        if (!dst_data) break;

        uint8_t* p = dst_data;

        for (uint32_t i = 0; i < h; ++i) {
            memcpy(p, src_data, w * origChannels);
            p += w * origChannels;
            src_data += origW * origChannels;
        }

        resultImg.width = w;
        resultImg.height = h;
        resultImg.channels = origChannels;
        resultImg.data = dst_data;

    } while (0);

    return resultImg;
}

void FilterDetection::extenMotionEstimation(VIPLFaceInfo& fi,
                                            const float multiW,
                                            const float multiH) {
    uint32_t origW = fi.width;
    uint32_t origH = fi.height;
    fi.width = static_cast<int32_t>(fi.width * multiW);
    fi.height = static_cast<int32_t>(fi.height * multiH);
    fi.x -= static_cast<int32_t>((fi.width - origW) / 2.0f);
    fi.y -= static_cast<int32_t>((fi.height - origH) / 2.0f);
    if (fi.x < 0) fi.x = 0;
    if (fi.y < 0) fi.y = 0;
}

}  // end namespace tracker
