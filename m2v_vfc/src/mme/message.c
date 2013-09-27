#include "message.h"

static char *msg_table_ja[] = {
	"�N���b�v�{�[�h���J���܂���ł���",
	"�N���b�v�{�[�h�̏��L�����擾�ł��܂���ł���",
	"�o�͒� %d %%",
	"�G���[�Ȃ�",
	"%s ���J���̂Ɏ��s���܂���",
	"%s �̃t�@�C�������擾�ł��܂���ł���",
	"%s �̓r�f�I�X�g���[���������܂���",
	"%s �̃r�f�I�X�g���[�������擾�ł��܂���ł���",
	"������������܂���",
	"�t���[�� %I64d �̓ǂݍ��݂Ɏ��s���܂���",
	"�擪�t���[��",
	"�ŏI�t���[��",
	"I �s�N�`���𔭌��ł��܂���ł���",
	"�t���[�� %I64d �� I �s�N�`���܂��� closed_gop ���w�肳�ꂽ B �s�N�`���ł͂���܂���",
	"�t���[�� %I64d �� B �s�N�`���ł�",
	"%I64d �܂�",
	"%s �܂�",
	"%s �̏o�͂Ɏ��s���܂���",
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