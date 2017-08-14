// Tracker.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "include/CameraBox.h"
#include "include/GlobalDef.h"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#define MAX_LOADSTRING 100

tracker::CameraBox cambox;

int _tmain(int argc, _TCHAR* argv[]) {
    do {
        if (!cambox.init(0)) break;

        if (cambox.getStatus() != tracker::CAMBOX_STATUS::INIT) break;

        cambox.start();

        cv::VideoCapture v("3.mp4");
        if (!v.isOpened()) break;

        cv::Mat mat;
        while (1) {
            v >> mat;
            if (mat.data) {
                cambox.draw(mat);
            }
            cv::waitKey(33);
        }
    } while (0);

    cambox.release();
    exit(EXIT_SUCCESS);
}
