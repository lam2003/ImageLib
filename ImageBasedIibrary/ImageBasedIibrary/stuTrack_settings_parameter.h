#ifndef stuTrack_settings_parameter_h__
#define stuTrack_settings_parameter_h__

#ifdef  __cplusplus
extern "C" {
#endif

typedef struct _POINT
{
	int x;
	int y;
}TrackPrarms_Point_t;

typedef struct 	_StuITRACK_Params
{
	int flag_setting;	//参数是否被设置
	int height;			//图像高度
	int width;			//图像宽度

	TrackPrarms_Point_t stuTrack_vertex[4];		//学生区域四个顶点位置
	int stuTrack_direct_standard[4];			//四个顶点位置竖直方向在图像中的角度
	int stuTrack_stuWidth_standard[4];			//四个顶点位置学生在图像中所占的宽度
	int stuTrack_direct_range;					//起立时允许的角度偏离范围
	float stuTrack_move_threshold;				//判定是移动目标的偏离阈值（比值）
	int stuTrack_standCount_threshold;			//判定为起立的帧数阈值
	int stuTrack_sitdownCount_threshold;		//判定为坐下的帧数阈值
	int sturTrack_moveDelayed_threshold;		//移动目标保持跟踪的延时，超过这个时间无运动，则放弃跟踪(单位：毫秒)
}StuITRACK_Params;

//变化状态宏
#define STUTRACK_RETURN_NULL		0
#define	STUTRACK_RETURN_STANDUP		1
#define	STUTRACK_RETURN_SITDOWN		2
#define	STUTRACK_RETURN_MOVE		4
#define STUTRACK_RETURN_STOPMOVE	8

typedef struct _StuITRACK_OutParams
{
	int result_flag;							//当前帧变化状态
	unsigned int count_trackObj_stand;			//起立目标个数
	unsigned int count_trackObj_bigMove;		//移动目标个数
	TrackPrarms_Point_t stand_position;			//起立目标位置
	TrackPrarms_Point_t move_position;			//移动目标位置
	int standObj_size[2];						//起立目标大小（宽，高）
	int moveObj_size[2];						//移动目标大小
}StuITRACK_OutParams_t;

#ifdef  __cplusplus  
}
#endif  /* end of __cplusplus */ 

#endif