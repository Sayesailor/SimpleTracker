#ifndef TRACKER_INCLUDE_GLOBALDEF
#define TRACKER_INCLUDE_GLOBALDEF

namespace tracker {

#ifdef _DEBUG
// faster
// const int DEBUG_FRAME_PER_SECOND = 20;
// slower
const int DEBUG_FRAME_PER_SECOND = 10;
#else
// best release
const int RELEASE_FRAME_PER_SECOND = 16;
#endif
const float TRACK_MATCH_THRESHOLD = 0.2f;
const float TRACKER_THRESHOLD = 0.4f;
const int FRAME_THRESHOLD = 10;
const int INIT_OBJ_ID = -1;
const int CALC_SPEED_MULTI = 100;
const float MOTION_ESTIMATION_EXPAND_W = 1.8f;
const float MOTION_ESTIMATION_EXPAND_H = 1.5f;
const float MOTION_ESTIMATION_EXPAND_NEWEXTRA = 1.2f;
const int PROCESS_IMAGE_SCALE = 50;
}

#endif