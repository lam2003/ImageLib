#ifndef stuTrack_settings_parameter_h__
#define stuTrack_settings_parameter_h__

#ifdef  __cplusplus
extern "C" {
#endif

typedef struct 	_StuITRACK_Params
{
	int flag_setting;	//参数是否被设置
	int height;			//图像高度
	int width;			//图像宽度
	int *stuTrack_size_threshold;				//运动目标大小过滤阈值（根据位置不同阈值不同）
	int stuTrack_direct_range;					//起立时允许的角度偏离范围
	int *stuTrack_direct_threshold;				//起立的标准角度
	float stuTrack_move_threshold;				//判定是移动目标的偏离阈值（比值）
	int stuTrack_standCount_threshold;			//判定为起立的帧数阈值
	int stuTrack_sitdownCount_threshold;		//判定为坐下的帧数阈值
	int sturTrack_moveDelayed_threshold;		//移动目标保持跟踪的延时，超过这个时间无运动，则放弃跟踪(单位：毫秒)

	void* callbackmsg_func;						//用于信息输出的函数指针
	void* interior_params;						//内部参数的指针

}StuITRACK_Params;

#ifdef  __cplusplus  
}
#endif  /* end of __cplusplus */ 

#endif