/*
** resource header file, used in mme.rc
**
*/
#ifndef RESOURCE_H
#define RESOURCE_H

#define WM_CBDIALOG_CANCEL             WM_APP+0

#define ID_FILE_OPEN                   0x0100
#define ID_FILE_CLOSE                  0x0101
#define ID_FILE_OUTPUT                 0x0102
#define ID_FILE_BENCHMARK              0x0103
#define ID_FILE_GOPLIST                0x0104
#define ID_FILE_END                    0x0105

#define ID_EDIT_IN                     0x0200
#define ID_EDIT_OUT                    0x0201
#define ID_EDIT_UNDO                   0x0202
#define ID_EDIT_COPY                   0x0203

#define ID_MOVE_FRAME                  0x0300
#define ID_MOVE_TIMECODE               0x0301
#define ID_MOVE_FORWARD                0x0302
#define ID_MOVE_BACK                   0x0303
#define ID_MOVE_NEXT_I                 0x0304
#define ID_MOVE_PREVIOUS_I             0x0305
#define ID_MOVE_IN                     0x0306
#define ID_MOVE_OUT                    0x0307
#define ID_MOVE_START                  0x0308
#define ID_MOVE_END                    0x0309

#define ID_HELP_ABOUT                  0x0400

#define IDI_APP_ICON                    101
#define IDD_CALLBACK_DIALOG_JA          110
#define IDD_CALLBACK_DIALOG_EN          111
#define IDD_FRAME_BY_NUMBER_DIALOG_JA   120
#define IDD_FRAME_BY_NUMBER_DIALOG_EN   121
#define IDD_FRAME_BY_TIMECODE_DIALOG_JA 130
#define IDD_FRAME_BY_TIMECODE_DIALOG_EN 131
#define IDC_PROGRESS                    1000
#define IDC_PERCENT_TEXT                1001
#define IDC_FRAME_INDEX                 1002
#define IDC_SLIDE_BAR                   1003
#define IDC_FRAME_SPIN                  1004
#define IDC_MAX_FRAME                   1005
#define IDC_TIMECODE_VIEW               1006
#define IDC_TIMECODE_SPIN               1007
#define IDC_MAX_TIMECODE                1008
#endif
