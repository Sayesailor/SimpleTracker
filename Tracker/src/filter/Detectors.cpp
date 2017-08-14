#include "stdafx.h"
#include "include/filter/Detectors.h"
#ifdef __linux__
#include <cstdio>
#include <string>
#endif

namespace tracker {

Detectors* Detectors::m_pInst = NULL;

Detectors::Detectors()
    : detector_global_(NULL),
      detector_local_(NULL)
#ifdef SUPPORT_PT_DETECTOR
      ,
      point_detector(NULL)
#endif

#ifdef SUPPORT_IDENTIFY
      ,
      face_recognizer(NULL)
#endif
      ,
      status_(0) {
    FILE* pf = NULL;

    std::string modelfile = "../model/VIPLFaceDetector5.0.0.dat";
    pf = fopen(modelfile.c_str(), "r");
    if (pf) {
        detector_global_ = new VIPLFaceDetector(modelfile.c_str());
        detector_global_->SetMinFaceSize(60);
        detector_global_->SetScoreThresh(0.62f, 0.47f, 0.985f);
        detector_global_->SetImagePyramidScaleFactor(1.414f);
        status_ |= 1;

        detector_local_ = new VIPLFaceDetector(modelfile.c_str());
        detector_local_->SetMinFaceSize(60);
        detector_local_->SetScoreThresh(0.62f, 0.47f, 0.985f);
        detector_local_->SetImagePyramidScaleFactor(1.414f);
        status_ |= 8;
    }

#ifdef SUPPORT_PT_DETECTOR
    pf = fopen("../model/seeta_fa_v1.1.bin", "r");
    if (pf) {
        // Initialize face alignment model
        point_detector = new seeta::FaceAlignment("../model/seeta_fa_v1.1.bin");
        status_ |= 2;
    }
#endif

#ifdef SUPPORT_IDENTIFY
    pf = fopen("../model/seeta_fa_v1.1.bin", "r");
    if (pf) {
        // Initialize face Identification model
        face_recognizer =
            new seeta::FaceIdentification("../model/seeta_fr_v1.0.bin");
        status_ |= 4;
    }
#endif
}

Detectors::~Detectors() {
    if (detector_global_) delete detector_global_;
    if (detector_local_) delete detector_local_;

#ifdef SUPPORT_PT_DETECTOR
    if (point_detector) delete point_detector;
#endif

#ifdef SUPPORT_IDENTIFY
    if (face_recognizer) delete face_recognizer;
#endif
}
}
