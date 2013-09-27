#include "message.h"

static char *msg_table_ja[] = {
	"クリップボードが開けませんでした",
	"クリップボードの所有権が取得できませんでした",
	"出力中 %d %%",
	"エラーなし",
	"%s を開くのに失敗しました",
	"%s のファイル情報を取得できませんでした",
	"%s はビデオストリームを持ちません",
	"%s のビデオストリーム情報を取得できませんでした",
	"メモリが足りません",
	"フレーム %I64d の読み込みに失敗しました",
	"先頭フレーム",
	"最終フレーム",
	"I ピクチャを発見できませんでした",
	"フレーム %I64d は I ピクチャまたは closed_gop が指定された B ピクチャではありません",
	"フレーム %I64d は B ピクチャです",
	"%I64d まで",
	"%s まで",
	"%s の出力に失敗しました",
};
		
static char *msg_table_en[] = {
	"Failed to open clipboard",
	"Failed to get clipboard ownership",
	"Now working.. %d %%",
	"No error",
	"Failed to open %s",
	"Failed to get %s information",
	"%s has no video stream",
	"Failed to get %s video stream information",
	"No enough memory",
	"Failed to load frame data",
	"First frame",
	"Last frame",
	"Failed to find I-picture",
	"Frame %I64d is not I-picture nor closed-gop B-picture",
	"Frame %I64d is B-picture",
	"Max: %I64d",
	"Max: %s",
	"Failed to store %s",
};

char **msg_table[] = {
	msg_table_ja,
	msg_table_en,
};