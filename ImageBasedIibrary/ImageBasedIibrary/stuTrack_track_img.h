#ifndef stuTrack_track_img_h__
#define stuTrack_track_img_h__
#include "itctype.h"
#include "itcerror.h"
#include "itcdatastructs.h"
#include "itcCore.h"
#include "stuTrack_settings_parameter.h"

#ifdef  __cplusplus
extern "C" {
#endif

#define STUTRACK_IMG_HEIGHT 264
#define STUTRACK_IMG_WIDTH	480

#define MALLOC_ELEMENT_COUNT 100
typedef struct StuTrack_Stand_t
{
	int direction;
	int count_teack;	//
	int count_up;		//
	int count_down;		//
	int flag_Stand;		//起立标志
	int flag_matching;	//匹配标志
	Track_Point_t centre;
	Track_Rect_t roi;
	clock_t start_tClock;
	clock_t current_tClock;
}StuTrack_Stand_t;

typedef struct StuTrack_BigMoveObj_t
{
	int count_track;
	int flag_bigMove;		//标志
	int dis_threshold;		//认为是移动目标的阈值
	Track_Rect_t roi;
	clock_t start_tClock;
	clock_t current_tClock;
	Track_Point_t origin_position;
	Track_Point_t current_position;
}StuTrack_BigMoveObj_t;

typedef struct _StuITRACK_InteriorParams
{
	int img_size;
	int count_trackObj_stand;
	StuTrack_Stand_t* stuTrack_stand;

	int count_trackObj_bigMove;
	StuTrack_BigMoveObj_t* stuTrack_bigMOveObj;

	int count_stuTrack_rect;
	Track_Rect_t *stuTrack_rect_arr;

	Track_MemStorage_t* stuTrack_storage;

	Itc_Mat_t *tempMat;
	Itc_Mat_t *currMat;
	Itc_Mat_t *lastMat;
	Itc_Mat_t *mhiMat;
	Itc_Mat_t *maskMat;
}StuITRACK_InteriorParams;

int stuTrack_filtrate_contours(StuITRACK_Params *inst, Track_Contour_t** pContour);			//轮廓筛选

int stuTrack_matchingSatnd_ROI(StuITRACK_Params *inst, Track_Rect_t roi);	//匹配roi

void stuTrack_analyze_ROI(StuITRACK_Params *inst );
int stuTrack_judgeStand_ROI(StuITRACK_Params *inst, StuTrack_Stand_t teack_stand);								//判断是否起立

void stuTrack_proStandDown_ROI(StuITRACK_Params *inst);

void stuTrack_initializeTrack(StuITRACK_Params *inst);
void stuTrack_main(StuITRACK_Params *inst, char* imageData);
void stuTrack_stopTrack(StuITRACK_Params *inst);

#ifdef  __cplusplus  
}
#endif  /* end of __cplusplus */ 

#endif