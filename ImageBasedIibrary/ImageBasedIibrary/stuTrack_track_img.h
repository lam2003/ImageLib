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

#define HEIGHT_STUTRACK_IMG_ 264
#define WIDTH_STUTRACK_IMG_	480

#define COUNT_STUTRACK_MALLOC_ELEMENT 10

#define _PRINTF											\
if(interior_params_p->callbackmsg_func==NULL)			\
{	interior_params_p->callbackmsg_func = printf;}		\
((callbackmsg)(interior_params_p->callbackmsg_func))

#define ITC_RETURN
#define JUDEGE_STUREACK_IF_NULL(p,r)			\
if ((p) == NULL)								\
{												\
	stuTrack_stopTrack(inst, interior_params_p);\
	return r;									\
}

#include <time.h>
typedef struct StuTrack_Stand_t
{
	int direction;
	unsigned int count_teack;	//
	unsigned int count_up;		//
	unsigned int count_down;	//
	int flag_Stand;		//起立标志
	int flag_matching;	//匹配标志
	Track_Point_t centre;
	Track_Rect_t roi;
	clock_t start_tClock;
	clock_t current_tClock;
}StuTrack_Stand_t;

typedef struct StuTrack_BigMoveObj_t
{
	unsigned int count_track;
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
	BOOL initialize_flag;
	unsigned int _count;	//统计帧数
	size_t img_size;		//图像大小w*h

	int result_flag;							//当前帧变化状态

	unsigned int count_trackObj_stand;			//起立区域计数
	StuTrack_Stand_t* stuTrack_stand;

	unsigned int count_trackObj_bigMove;		//移动目标计数
	StuTrack_BigMoveObj_t* stuTrack_bigMOveObj;

	unsigned int count_stuTrack_rect;			//运动区域计数
	Track_Rect_t *stuTrack_rect_arr;

	Track_MemStorage_t* stuTrack_storage;

	int *stuTrack_size_threshold;				//运动目标大小过滤阈值（根据位置不同阈值不同）
	int *stuTrack_direct_threshold;				//起立的标准角度,大小为width
	Itc_Mat_t *tempMat;
	Itc_Mat_t *currMat;
	Itc_Mat_t *lastMat;
	Itc_Mat_t *mhiMat;
	Itc_Mat_t *maskMat;

	callbackmsg callbackmsg_func;						//用于信息输出的函数指针
}StuITRACK_InteriorParams;

//默认参数值
#define THRESHOLD_STUTRACK_MOVE_DEFALUT_PARAMS			1.2
#define THRESHOLD_STUTRACK_STANDCOUNT_DEFALUT_PARAMS	5
#define THRESHOLD_STUTRACK_SITDOWNCOUNT_DEFALUT_PARAMS	5
#define THRESHOLD_STUTRACK_MOVEDELAYED_DEFALUT_PARAMS	500
#define RANGE_STUTRACK_STANDDIRECT_DEFALUT_PARAMS		9

//用于计算筛选阈值的线性方程参数
#define A_STUTRACK_SIZE_THRESHOLD_PARAMS	(0.25)
#define B_STUTRACK_SIZE_THRESHOLD_PARAMS	(-6.0)
#define A_STUTRACK_DIRECT_THRESHOLD_PARAMS	(0.125)
#define B_STUTRACK_DIRECT_THRESHOLD_PARAMS	(240.0)
#define MINTHRESHOLD_STUTRACK_SIZE_THRESHOLD_PARAMS		20
#define MAXTHRESHOLD_STUTRACK_SIZE_THRESHOLD_PARAMS		55
#define MINTHRESHOLD_STUTRACK_DIRECT_THRESHOLD_PARAMS	225
#define MAXTHRESHOLD_STUTRACK_DIRECT_THRESHOLD_PARAMS	315

#define COMPUTER_STUTRACK_SIZE_THRESHOLD_PARAMS(n,a,b)  (ITC_MIN(ITC_MAX(((a *n + b)), MINTHRESHOLD_STUTRACK_SIZE_THRESHOLD_PARAMS), MAXTHRESHOLD_STUTRACK_SIZE_THRESHOLD_PARAMS))
#define COMPUTER_STUTRACK_DIRECT_THRESHOLD_PARAMS(n,a,b)  (ITC_MIN(ITC_MAX(((a *n + b)), MINTHRESHOLD_STUTRACK_DIRECT_THRESHOLD_PARAMS), MAXTHRESHOLD_STUTRACK_DIRECT_THRESHOLD_PARAMS))

void stuTrack_initializeTrack(StuITRACK_Params *inst, StuITRACK_InteriorParams* interior_params_p);
void stuTrack_process(StuITRACK_Params *inst, StuITRACK_InteriorParams* interior_params_p, StuITRACK_OutParams_t* return_params, char* imageData);
void stuTrack_stopTrack(StuITRACK_Params *inst, StuITRACK_InteriorParams* interior_params_p);

#ifdef  __cplusplus  
}
#endif  /* end of __cplusplus */ 

#endif