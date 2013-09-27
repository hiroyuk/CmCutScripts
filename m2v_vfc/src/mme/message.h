#ifndef MESSAGE_H
#define MESSAGE_H

#define MSG_FAILED_TO_OPEN_CLIPBOARD          0
#define MSG_FAILED_TO_GET_CLIPBOARD_OWNERSHIP 1
#define MSG_STORE_CALLBACK_PERCENT_TEMPLATE   2
#define MSG_NO_ERROR                          3
#define MSG_FAILED_TO_OPEN_FILE               4
#define MSG_FAILED_TO_GET_FILE_INFORMATION    5
#define MSG_FILE_HAS_NO_VIDEO_STREAM          6
#define MSG_FAILED_TO_GET_VIDEO_INFORMATION   7
#define MSG_NO_ENOUGH_MEMORY                  8
#define MSG_FAILED_TO_LOAD_FRAME              9
#define MSG_FIRST_FRAME                       10
#define MSG_LAST_FRAME                        11
#define MSG_FAILED_TO_FIND_I_FRAME            12
#define MSG_FRAME_TYPE_NOT_STARTABLE          13
#define MSG_FRAME_TYPE_NOT_FINISHABLE         14
#define MSG_MAX_FRAME_TEMPLATE                15
#define MSG_MAX_TIMECODE_TEMPLATE             16
#define MSG_FAILED_TO_STORE_FILE              17
#ifdef __cplusplus
extern "C" {
#endif

extern char **msg_table[];

#ifdef __cplusplus
}
#endif

#endif