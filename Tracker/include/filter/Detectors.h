#ifndef TRACKER_INCLUDE_DETECTORS_H_
#define TRACKER_INCLUDE_DETECTORS_H_

#include "VIPLStruct.h"
#include "VIPLFaceDetector.h"

#ifdef SUPPORT_PT_DETECTOR
#include "FaceAlignment/include/face_alignment.h"
#endif

#ifdef SUPPORT_IDENTIFY
#include "FaceIdentification/include/face_identification.h"
#endif

namespace tracker {

class Detectors {
    Detectors();
    static Detectors* m_pInst;

   public:
    static Detectors* getInstance() {
        if (m_pInst == NULL) {
            m_pInst = new Detectors();
        }
        return m_pInst;
    }

    virtual ~Detectors();

    const int32_t get_status() const { return status_; }
    inline VIPLFaceDetector* get_detector_global() { return detector_global_; }
    inline VIPLFaceDetector* get_detector_local() { return detector_local_; }

#ifdef SUPPORT_PT_DETECTOR
    inline seeta::FaceAlignment* getPtDetector() { return point_detector; }
#endif

#ifdef SUPPORT_IDENTIFY
    inline seeta::FaceIdentification* getIdentifier() {
        return face_recognizer;
    }
#endif

   private:
    VIPLFaceDetector* detector_global_;
    VIPLFaceDetector* detector_local_;

#ifdef SUPPORT_PT_DETECTOR
    seeta::FaceAlignment* point_detector;
#endif

#ifdef SUPPORT_IDENTIFY
    seeta::FaceIdentification* face_recognizer;
#endif

    int32_t status_;
};
}

#endif