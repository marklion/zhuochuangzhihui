//用于接入VZ的车牌识别设备（包括智能识别一体机和智能识别终端等）的应用程序接口
#ifndef _VZ_LPRC_SDK_H_
#define _VZ_LPRC_SDK_H_
#include "VzLPRClientSDKDefine.h"

/**
* @defgroup group_global 全局操作函数
* @defgroup group_device 单个设备操作函数
* @defgroup group_callback 回调函数
*/

#ifdef  LINUX
#ifdef __cplusplus
extern "C"
{
#endif
#endif

/**
*  @brief 全局初始化
*  @return 0表示成功，-1表示失败
*  @note 在所有接口调用之前调用
*  @ingroup group_global
*/
VZ_LPRC_API int __STDCALL VzLPRClient_Setup();

/**
*  @brief 全局释放
*  @note 在程序结束时调用，释放SDK的资源
*  @ingroup group_global
*/
VZ_LPRC_API void __STDCALL VzLPRClient_Cleanup();

/**
*  @brief 通过该回调函数获得找到的设备基本信息
*  @param  [IN] pStrDevName 设备名称
*  @param  [IN] pStrIPAddr	设备IP地址
*  @param  [IN] usPort1		设备端口号
*  @param  [IN] usPort2		预留
*  @param  [IN] SL          设备序列号低位字节
*  @param  [IN] SH			设备序列号高位字节
*  @param  [IN] pUserData	回调函数上下文
*  @ingroup group_callback
*/
typedef void (__STDCALL *VZLPRC_FIND_DEVICE_CALLBACK)(const char *pStrDevName, const char *pStrIPAddr, WORD usPort1, WORD usPort2, unsigned int SL,unsigned int SH, void *pUserData);

/**
*  @brief 开始查找设备
*  @param  [IN] func 找到的设备通过该回调函数返回
*  @param  [IN] pUserData 回调函数中的上下文
*  @return 0表示成功，-1表示失败
*  @ingroup group_global
*/
VZ_LPRC_API int __STDCALL VZLPRClient_StartFindDevice(VZLPRC_FIND_DEVICE_CALLBACK func, void *pUserData);

/**
*  @brief 通过该回调函数获得找到的设备基本信息
*  @param  [IN] pStrDevName 设备名称
*  @param  [IN] pStrIPAddr	设备IP地址
*  @param  [IN] usPort1		设备端口号
*  @param  [IN] usPort2		预留
*  @param  [IN] SL          设备序列号低位字节
*  @param  [IN] SH			设备序列号高位字节
*  @param  [IN] pUserData	回调函数上下文
*  @ingroup group_callback
*/
typedef void(__STDCALL *VZLPRC_FIND_DEVICE_CALLBACK_EX)(const char *pStrDevName, const char *pStrIPAddr, WORD usPort1, WORD usType, unsigned int SL, unsigned int SH, char* netmask, char* gateway, void *pUserData);

/**
*  @brief 开始查找设备
*  @param  [IN] func 找到的设备通过该回调函数返回
*  @param  [IN] pUserData 回调函数中的上下文
*  @return 0表示成功，-1表示失败
*  @ingroup group_global
*/
VZ_LPRC_API int __STDCALL VZLPRClient_StartFindDeviceEx(VZLPRC_FIND_DEVICE_CALLBACK_EX func, void *pUserData);


/**
*  @brief 通过该回调函数获得找到的设备基本信息
*  @param  [IN] pStrDevName 设备名称
*  @param  [IN] pStrIPAddr	设备IP地址
*  @param  [IN] usPort1		  设备端口号
*  @param  [IN] usPort2		  预留
*  @param  [IN] SL            设备序列号低位字节
*  @param  [IN] SH			  设备序列号高位字节
*  @param  [IN] ext_json_data 扩展json格式数据
*  @param  [IN] data_len	  数据长度
*  @param  [IN] pUserData	  回调函数上下文
*  @ingroup group_callback
*/
typedef void(__STDCALL *VZLPRC_FIND_DEVICE_CALLBACK_EX2)(const char *pStrDevName, const char *pStrIPAddr, WORD usPort1, WORD usType, unsigned int SL, unsigned int SH, char* netmask, char* gateway, char* ext_json_data, int data_len, void *pUserData);

/**
*  @brief 开始查找设备
*  @param  [IN] func 找到的设备通过该回调函数返回
*  @param  [IN] pUserData 回调函数中的上下文
*  @return 0表示成功，-1表示失败
*  @ingroup group_global
*/
VZ_LPRC_API int __STDCALL VZLPRClient_StartFindDeviceEx2(VZLPRC_FIND_DEVICE_CALLBACK_EX2 func, void *pUserData);

/**
*  @brief 停止查找设备
*  @ingroup group_global
*/
VZ_LPRC_API void __STDCALL VZLPRClient_StopFindDevice();

/**
*  @brief 通过该回调函数获得设备的一般状态信息
*  @param  [IN] handle		由VzLPRClient_Open函数获得的句柄
*  @param  [IN] pUserData	回调函数上下文
*  @param  [IN] eNotify		通用信息反馈类型
*  @param  [IN] pStrDetail	详细描述字符串
*  @ingroup group_callback
*/
typedef void (__STDCALL *VZLPRC_COMMON_NOTIFY_CALLBACK)(VzLPRClientHandle handle, void *pUserData,
													   VZ_LPRC_COMMON_NOTIFY eNotify, const char *pStrDetail);

/**
*  @brief 设置设备连接反馈结果相关的回调函数
*  @param  [IN] func 设备连接结果和状态，通过该回调函数返回
*  @param  [IN] pUserData 回调函数中的上下文
*  @return 0表示成功，-1表示失败
*  @ingroup group_global
*/
VZ_LPRC_API int __STDCALL VZLPRClient_SetCommonNotifyCallBack(VZLPRC_COMMON_NOTIFY_CALLBACK func, void *pUserData);

/**
*  @brief 打开一个设备
*  @param  [IN] pStrIP 设备的IP地址
*  @param  [IN] wPort 设备的端口号
*  @param  [IN] pStrUserName 访问设备所需用户名
*  @param  [IN] pStrPassword 访问设备所需密码
*  @return 返回设备的操作句柄，当打开失败时，返回0
*  @ingroup group_device
*/
VZ_LPRC_API VzLPRClientHandle __STDCALL VzLPRClient_Open(
  const char *pStrIP,
  WORD wPort,
  const char *pStrUserName,
  const char *pStrPassword);

/**
*  @brief 打开一个设备
*  @param  [IN] pStrIP 设备的IP地址
*  @param  [IN] wPort 设备的端口号
*  @param  [IN] pStrUserName 访问设备所需用户名
*  @param  [IN] pStrPassword 访问设备所需密码
*  @param  [IN] wRtspPort 流媒体的端口号,为0表示使用默认
*  @return 返回设备的操作句柄，当打开失败时，返回0
*  @ingroup group_device
*/
VZ_LPRC_API VzLPRClientHandle __STDCALL VzLPRClient_OpenEx(const char *pStrIP, WORD wPort, const char *pStrUserName, const char *pStrPassword,WORD wRtspPort);

/**
*  @brief 打开一个设备
*  @param  [IN] pStrIP 设备的IP地址
*  @param  [IN] wPort 设备的端口号
*  @param  [IN] pStrUserName 访问设备所需用户名
*  @param  [IN] pStrPassword 访问设备所需密码
*  @param  [IN] wRtspPort 流媒体的端口号,为0表示使用默认
*  @param  [IN] wTcpPort tcp端口号,为0表示使用默认
*  @return 返回设备的操作句柄，当打开失败时，返回0
*  @ingroup group_device
*/
VZ_LPRC_API VzLPRClientHandle __STDCALL VzLPRClient_OpenEx2(const char *pStrIP, WORD wPort, const char *pStrUserName, const char *pStrPassword,WORD wRtspPort, WORD wTcpPort);

/**
*  @brief 关闭一个设备
*  @param  [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @return 0表示成功，-1表示失败
*  @ingroup group_device
*/
VZ_LPRC_API int __STDCALL VzLPRClient_Close(VzLPRClientHandle handle);

/**
*  @brief 通过IP地址关闭一个设备
*  @param  [IN] pStrIP 设备的IP地址
*  @return 0表示成功，-1表示失败
*  @ingroup group_device
*/
VZ_LPRC_API int __STDCALL VzLPRClient_CloseByIP(const char *pStrIP);

/**
*  @brief 获取连接状态
*  @param  [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param  [IN/OUT] pStatus 输入获取状态的变量地址，输出内容为 1已连上，0未连上
*  @return 0表示成功，-1表示失败
*  @note   用户可以周期调用该函数来主动查询设备是否断线，以及断线后是否恢复连接
*  @ingroup group_device
*/
VZ_LPRC_API int __STDCALL VzLPRClient_IsConnected(VzLPRClientHandle handle, BYTE *pStatus);

/**
*  @brief 播放实时视频
*  @param  [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param  [IN] hWnd 窗口的句柄
*  @return 播放的句柄，-1表示失败
*  @ingroup group_device
*/
VZ_LPRC_API int __STDCALL VzLPRClient_StartRealPlay(VzLPRClientHandle handle, void *hWnd);

/**
*  @brief 停止播放指定的播放句柄
*  @param  [IN] nPlayHandle 播放的句柄
*  @return 0表示成功，-1表示失败
*  @note   可用来停止播放来自函数VzLPRClient_StartRealPlay和VzLPRClient_StartRealPlayFrameCallBack的播放句柄
*  @note   停止播放以后，该播放句柄就失效了
*  @ingroup group_device
*/
VZ_LPRC_API int __STDCALL VzLPRClient_StopRealPlay(int nPlayHandle);

/**
*  @brief  通过该回调函数获得实时图像数据
*  @param  [IN] handle		由VzLPRClient_Open函数获得的句柄
*  @param  [IN] pUserData	回调函数的上下文
*  @param  [IN] pFrame		图像帧信息，详见结构体定义VzYUV420P
*  @return 0表示成功，-1表示失败
*  @ingroup group_callback
*/
typedef void (__STDCALL *VZLPRC_VIDEO_FRAME_CALLBACK)(VzLPRClientHandle handle, void *pUserData,
													  const VzYUV420P *pFrame);
/**
*  @brief 设置实时图像数据的回调函数
*  @param  [IN] handle		由VzLPRClient_Open函数获得的句柄
*  @param  [IN] func		实时图像数据函数
*  @param  [IN] pUserData	回调函数中的上下文
*  @return 0表示成功，-1表示失败
*  @note   DEPRECATED: 请转换为使用VzLPRClient_StartRealPlayFrameCallBack(handle, NULL, func, pUserData);
*  @ingroup group_device
*/
VZ_DEPRECATED VZ_LPRC_API int __STDCALL VzLPRClient_SetVideoFrameCallBack(VzLPRClientHandle handle, VZLPRC_VIDEO_FRAME_CALLBACK func, void *pUserData);

/**
*  @brief  通过该回调函数获得实时图像数据
*  @param  [IN] handle		由VzLPRClient_Open函数获得的句柄
*  @param  [IN] pUserData	回调函数的上下文
*  @param  [IN] pFrame		图像帧信息，详见结构体定义VZ_LPRC_IMAGE_INFO
*  @ingroup group_callback
*/
typedef void (__STDCALL *VZLPRC_VIDEO_FRAME_CALLBACK_EX)(VzLPRClientHandle handle, void *pUserData,
														 const VZ_LPRC_IMAGE_INFO *pFrame);
/**
*  @brief 获取实时视频帧，图像数据通过回调函数到用户层，用户可改动图像内容，并且显示到窗口
*  @param  [IN] handle		由VzLPRClient_Open函数获得的句柄
*  @param  [IN] hWnd		窗口的句柄，如果为有效值，则视频图像会显示到该窗口上，如果为空，则不显示视频图像
*  @param  [IN] func		实时图像数据函数
*  @param  [IN] pUserData	回调函数中的上下文
*  @return 播放的句柄，-1表示失败
*  @ingroup group_device
*/
VZ_LPRC_API int __STDCALL VzLPRClient_StartRealPlayFrameCallBack(VzLPRClientHandle handle, void *hWnd,
												  VZLPRC_VIDEO_FRAME_CALLBACK_EX func, void *pUserData);

/**
*  @brief  通过该回调函数获得编码数据
*  @param  [IN] handle		由VzLPRClient_Open函数获得的句柄
*  @param  [IN] pUserData	回调函数的上下文
*  @param  [IN] eDataType	数据类型，详见枚举VZ_LPRC_DATA_TYPE
*  @param  [IN] pData		数据帧信息，详见结构体定义VZ_LPRC_DATA_INFO
*  @ingroup group_callback
*/
typedef void (__STDCALL *VZLPRC_VIDEO_DATA_CALLBACK)(VzLPRClientHandle handle, void *pUserData,
													 VZ_LPRC_DATA_TYPE eDataType, const VZ_LPRC_DATA_INFO *pData);

/**
*  @brief 获取实时编码数据
*  @param  [IN] handle		由VzLPRClient_Open函数获得的句柄
*  @param  [IN] func		回调函数
*  @param  [IN] pUserData	回调函数中的上下文
*  @return 播放的句柄，-1表示失败
*  @ingroup group_device
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SetRealDataCallBack(VzLPRClientHandle handle,
												  VZLPRC_VIDEO_DATA_CALLBACK func, void *pUserData);

/**
*  @brief 通过该回调函数获得车牌识别信息
*  @param  [IN] handle			由VzLPRClient_Open函数获得的句柄
*  @param  [IN] pUserData		回调函数的上下文
*  @param  [IN] pResult			车牌信息数组首地址，详见结构体定义 TH_PlateResult
*  @param  [IN] uNumPlates		车牌数组中的车牌个数
*  @param  [IN] eResultType		车牌识别结果类型，详见枚举类型定义VZ_LPRC_RESULT_TYPE
*  @param  [IN] pImgFull		当前帧的图像内容，详见结构体定义VZ_LPRC_IMAGE_INFO
*  @param  [IN] pImgPlateClip	当前帧中车牌区域的图像内容数组，其中的元素与车牌信息数组中的对应
*  @return 0表示成功，-1表示失败
*  @note   如果需要该回调函数返回截图内容 pImgFull 和pImgPlateClip，需要在设置回调函数（VzLPRClient_SetPlateInfoCallBack）时允许截图内容，否则该回调函数返回的这两个指针为空；
*  @note   实时结果（VZ_LPRC_RESULT_REALTIME）的回调是不带截图内容的
*  @ingroup group_callback
*/
typedef int (__STDCALL *VZLPRC_PLATE_INFO_CALLBACK)(VzLPRClientHandle handle, void *pUserData,
													const TH_PlateResult *pResult, unsigned uNumPlates,
													VZ_LPRC_RESULT_TYPE eResultType,
													const VZ_LPRC_IMAGE_INFO *pImgFull,
													const VZ_LPRC_IMAGE_INFO *pImgPlateClip);

/**
*  @brief 通过该回调函数获得车牌识别信息(组网)
*  @param  [IN] handle			由VzLPRClient_Open函数获得的句柄
*  @param  [IN] pUserData		回调函数的上下文
*  @param  [IN] exitEntryInfo	车辆出入口信息 0 为入口，1 为出口
*  @param  [IN] exitIvsInfo		车辆出口信息（当设备为入口设备时该字段信息无效）
*  @param  [IN] entryIvsInfo	车辆入口信息
*  @return 0表示成功，-1表示失败
*  @ingroup group_callback
*/

typedef int (__STDCALL *VZLPRC_PLATE_GROUP_INFO_CALLBACK)(VzLPRClientHandle handle,
														  void *pUserData,
														  int exitEntryInfo,
														  const TH_PlateResult *exitIvsInfo,
														  const TH_PlateResult *entryIvsInfo,
														  const IVS_DG_DEVICE_INFO *exitDGInfo,
														  const IVS_DG_DEVICE_INFO *entryDGInfo);
/**
*  @brief 设置识别结果的回调函数
*  @param  [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param  [IN] func 识别结果回调函数，如果为NULL，则表示关闭该回调函数的功能
*  @param  [IN] pUserData 回调函数中的上下文
*  @param  [IN] bEnableImage 指定识别结果的回调是否需要包含截图信息：1为需要，0为不需要
*  @return 0表示成功，-1表示失败
*  @ingroup group_device
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SetPlateInfoCallBack(VzLPRClientHandle handle, VZLPRC_PLATE_INFO_CALLBACK func, void *pUserData, int bEnableImage);

/**
*  @brief 设置识别结果的回调函数（组网）
*  @param  [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param  [IN] func 识别结果回调函数，如果为NULL，则表示关闭该回调函数的功能（组网）
*  @param  [IN] pUserData 回调函数中的上下文
*  @return 0表示成功，-1表示失败
*  @ingroup group_device
*/

VZ_LPRC_API int __STDCALL VzLPRClient_SetPlateGroupInfoCallBack(VzLPRClientHandle handle, VZLPRC_PLATE_GROUP_INFO_CALLBACK func, void *pUserData);


/**
*  @brief 设置脱机结果的回调函数
*  @param  [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param  [IN] func 识别结果回调函数，如果为NULL，则表示关闭该回调函数的功能
*  @param  [IN] pUserData 回调函数中的上下文
*  @param  [IN] bEnableImage 指定识别结果的回调是否需要包含截图信息：1为需要，0为不需要
*  @return 0表示成功，-1表示失败
*  @ingroup group_device
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SetOfflinePlateInfoCallBack(VzLPRClientHandle handle, VZLPRC_PLATE_INFO_CALLBACK func, void *pUserData, int bEnableImage);


/**
*  @brief 发送软件触发信号，强制处理当前时刻的数据并输出结果
*  @param  [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @return 0表示成功，-1表示失败
*  @note   车牌识别结果通过回调函数的方式返回，如果当前视频画面中无车牌，输出识别结果为"_无_"
*  @ingroup group_device
*/
VZ_LPRC_API int __STDCALL VzLPRClient_ForceTrigger(VzLPRClientHandle handle);

/**
*  @brief  扩展接口，以Tcp方式发送软件触发信号，强制处理当前时刻的数据并输出结果
*  @param  [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @return 0表示成功，-1表示失败
*  @note   车牌识别结果通过回调函数的方式返回，如果当前视频画面中无车牌，输出识别结果为"_无_"
*  @ingroup group_device
*/
VZ_LPRC_API int __STDCALL VzLPRClient_ForceTriggerEx(VzLPRClientHandle handle);

/**
*  @brief 设置虚拟线圈
*  @param  [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param  [IN] pVirtualLoops 虚拟线圈的结构体指针
*  @return 0表示成功，-1表示失败
*  @ingroup group_device
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SetVirtualLoop(VzLPRClientHandle handle, const VZ_LPRC_VIRTUAL_LOOPS *pVirtualLoops);

/**
*  @brief 获取已设置的虚拟线圈
*  @param  [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param  [IN] pVirtualLoops 虚拟线圈的结构体指针
*  @return 0表示成功，-1表示失败
*  @ingroup group_device
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetVirtualLoop(VzLPRClientHandle handle,  VZ_LPRC_VIRTUAL_LOOPS *pVirtualLoops);

/**
*  @brief 设置虚拟线圈，线圈支持更多的顶点
*  @param  [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param  [IN] pVirtualLoops 虚拟线圈的结构体指针
*  @return 0表示成功，-1表示失败
*  @ingroup group_device
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SetVirtualLoopEx(VzLPRClientHandle handle, const VZ_LPRC_VIRTUAL_LOOPS_EX *pVirtualLoops);

/**
*  @brief 获取已设置的虚拟线圈，线圈支持更多的顶点
*  @param  [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param  [IN] pVirtualLoops 虚拟线圈的结构体指针
*  @return 0表示成功，-1表示失败
*  @ingroup group_device
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetVirtualLoopEx(VzLPRClientHandle handle,  VZ_LPRC_VIRTUAL_LOOPS_EX *pVirtualLoops);

/**
*  @brief 设置识别区域
*  @param  [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param  [IN] pROI 识别区域的结构体指针
*  @return 0表示成功，-1表示失败
*  @note   车牌在识别区域中才会被识别；
*  @note   识别区域可用于屏蔽复杂环境的干扰等，提高识别速度和准确性，避免误识别；
*  @ingroup group_device
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SetRegionOfInterest(VzLPRClientHandle handle, const VZ_LPRC_ROI *pROI);

/**
*  @brief 获取已设置的识别区域
*  @param  [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param  [IN] pROI 识别区域的结构体指针
*  @return 0表示成功，-1表示失败
*  @ingroup group_device
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetRegionOfInterest(VzLPRClientHandle handle, VZ_LPRC_ROI *pROI);

/**
*  @brief 设置识别区域
*  @param  [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param  [IN] pROI 识别区域的结构体指针
*  @return 0表示成功，-1表示失败
*  @note   车牌在识别区域中才会被识别；
*  @note   识别区域可用于屏蔽复杂环境的干扰等，提高识别速度和准确性，避免误识别；
*  @ingroup group_device
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SetRegionOfInterestEx(VzLPRClientHandle handle, const VZ_LPRC_ROI_EX *pROI);

/**
*  @brief 获取已设置的识别区域
*  @param  [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param  [IN] pROI 识别区域的结构体指针
*  @return 0表示成功，-1表示失败
*  @ingroup group_device
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetRegionOfInterestEx(VzLPRClientHandle handle, VZ_LPRC_ROI_EX *pROI);
/**
*  @brief 获取已设置的预设省份
*  @param  [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param  [IN] pProvInfo 预设省份信息指针
*  @return 0表示成功，非0表示失败
*  @ingroup group_device
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetSupportedProvinces(VzLPRClientHandle handle, VZ_LPRC_PROVINCE_INFO *pProvInfo);

/**
*  @brief 设置预设省份
*  @param  [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param  [IN] nIndex 设置预设省份的序号，序号需要参考VZ_LPRC_PROVINCE_INFO::strProvinces中的顺序，从0开始，如果小于0，则表示不设置预设省份
*  @return 0表示成功，-1表示失败
*  @ingroup group_device
*/
VZ_LPRC_API int __STDCALL VzLPRClient_PresetProvinceIndex(VzLPRClientHandle handle, int nIndex);

/**
*  @brief 将图像保存为JPEG到指定路径
*  @param  [IN] pImgInfo 图像结构体，目前只支持默认的格式，即ImageFormatRGB
*  @param  [IN] pFullPathName 设带绝对路径和JPG后缀名的文件名字符串
*  @param  [IN] nQuality JPEG压缩的质量，取值范围1~100；
*  @return 0表示成功，-1表示失败
*  @note   给定的文件名中的路径需要存在
*  @ingroup group_global
*/
VZ_LPRC_API int __STDCALL VzLPRClient_ImageSaveToJpeg(const VZ_LPRC_IMAGE_INFO *pImgInfo, const char *pFullPathName, int nQuality);

/**
*  @brief 将图像编码为JPEG，保存到指定内存
*  @param  [IN] pImgInfo 图像结构体，目前只支持默认的格式，即ImageFormatRGB
*  @param  [IN/OUT] pDstBuf JPEG数据的目的存储首地址
*  @param  [IN] uSizeBuf JPEG数据地址的内存的最大尺寸；
*  @param  [IN] nQuality JPEG压缩的质量，取值范围1~100；
*  @return >0表示成功，即编码后的尺寸，-1表示失败，-2表示给定的压缩数据的内存尺寸不够大
*  @ingroup group_global
*/
VZ_LPRC_API int __STDCALL VzLPRClient_ImageEncodeToJpeg(const VZ_LPRC_IMAGE_INFO *pImgInfo, void *pDstBuf, unsigned uSizeBuf, int nQuality);

/**
*  @brief 将JPEG解码到图像内存
*  @param  [IN] pSrcData JPEG数据存储首地址
*  @param  [IN] uSizeData JPEG数据尺寸
*  @param  [OUT] pImgInfo 解码后的图像内容
*  @param  [IN] pBufExt 外部给定一个用于保存解码后的内存的地址；
*  @param  [IN] uSizeBufExt 外部给定内存的尺寸，需要大于等于JPEG图像解码后的的RGB内存尺寸
*  @return 0表示成功，-1表示失败，>0表示外部给定的内存不够大，需要给定大于等于此尺寸的内存地址
*  @note  如果没有预知JPEG解码后的图像分辨率，那么就将uSizeBufExt指定为0，接口将返回所需最小内存尺寸，分配好内存后，再调用该接口进行解码
*  @ingroup group_global
*/
VZ_LPRC_API int __STDCALL VzLPRClient_JpegDecodeToImage(void *pSrcData, unsigned uSizeData, VZ_LPRC_IMAGE_INFO *pImgInfo, void *pBufExt, unsigned uSizeBufExt);

/**
*  @brief 保存正在播放的视频的当前帧的截图到指定路径
*  @param  [IN] hWnd 正在播放视频的窗口的句柄
*  @param  [IN] pFullPathName 设带绝对路径和JPG后缀名的文件名字符串
*  @param  [IN] nQuality JPEG压缩的质量，取值范围1~100；
*  @return 0表示成功，-1表示失败
*  @note   前提条件是指定的hWnd的窗口已经调用了VzLPRClient_StartRealPlay播放实时视频
*  @note   使用的文件名中的路径需要存在
*  @note   DEPRECATED: 请转为使用VzLPRClient_GetSnapShootToJpeg2
*  @ingroup group_device
*/
VZ_DEPRECATED VZ_LPRC_API int __STDCALL VzLPRClient_GetSnapShootToJpeg(void *hWnd, const char *pFullPathName, int nQuality);

/**
*  @brief 保存正在播放的视频的当前帧的截图到指定路径
*  @param  [IN] nPlayHandle 播放的句柄
*  @param  [IN] pFullPathName 设带绝对路径和JPG后缀名的文件名字符串
*  @param  [IN] nQuality JPEG压缩的质量，取值范围1~100；
*  @return 0表示成功，-1表示失败
*  @note   使用的文件名中的路径需要存在
*  @ingroup group_device
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetSnapShootToJpeg2(int nPlayHandle, const char *pFullPathName, int nQuality);

/**
*  @brief 获取IO输出的状态
*  @param  [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param  [IN] uChnId IO输出的通道号，从0开始
*  @param  [OUT] pOutput IO输出的状态，0表示继电器开路，1表示继电器闭路
*  @return 0表示成功，-1表示失败
*  @ingroup group_device
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetIOOutput(VzLPRClientHandle handle, unsigned uChnId, int *pOutput);

/**
*  @brief 设置IO输出的状态
*  @param  [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param  [IN] uChnId IO输出的通道号，从0开始
*  @param  [OUT] nOutput 将要设置的IO输出的状态，0表示继电器开路，1表示继电器闭路
*  @return 0表示成功，-1表示失败
*  @ingroup group_device
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SetIOOutput(VzLPRClientHandle handle, unsigned uChnId, int nOutput);

/**
*  @brief 设置串口参数
*  @param  [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param  [IN] nSerialPort 指定使用设备的串口序号：0表示第一个串口，1表示第二个串口
*  @param  [IN] pParameter 将要设置的串口参数，详见定义 VZ_SERIAL_PARAMETER
*  @return 0表示成功，-1表示失败
*  @ingroup group_device
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SetSerialParameter(VzLPRClientHandle handle, int nSerialPort,
														 const VZ_SERIAL_PARAMETER *pParameter);

/**
*  @brief 获取串口参数
*  @param  [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param  [IN] nSerialPort 指定使用设备的串口序号：0表示第一个串口，1表示第二个串口
*  @param  [OUT] pParameter 将要获取的串口参数，详见定义 VZ_SERIAL_PARAMETER
*  @return 0表示成功，-1表示失败
*  @ingroup group_device
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetSerialParameter(VzLPRClientHandle handle, int nSerialPort,
														 VZ_SERIAL_PARAMETER *pParameter);
/**
*  @brief 通过该回调函数获得透明通道接收的数据
*  @param  [IN] nSerialHandle VzLPRClient_SerialStart返回的句柄
*  @param  [IN] pRecvData	接收的数据的首地址
*  @param  [IN] uRecvSize	接收的数据的尺寸
*  @param  [IN] pUserData	回调函数上下文
*  @ingroup group_callback
*/
typedef void (__STDCALL *VZDEV_SERIAL_RECV_DATA_CALLBACK)(int nSerialHandle, const unsigned char *pRecvData, unsigned uRecvSize, void *pUserData);

/**
*  @brief 开启透明通道
*  @param  [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param  [IN] nSerialPort 指定使用设备的串口序号：0表示第一个串口，1表示第二个串口
*  @param  [IN] func 接收数据的回调函数
*  @param  [IN] pUserData 接收数据回调函数的上下文
*  @return 返回透明通道句柄，0表示失败
*  @ingroup group_device
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SerialStart(VzLPRClientHandle handle, int nSerialPort,
												  VZDEV_SERIAL_RECV_DATA_CALLBACK func, void *pUserData);
/**
*  @brief 透明通道发送数据
*  @param [IN] nSerialHandle 由VzLPRClient_SerialStart函数获得的句柄
*  @param [IN] pData 将要传输的数据块的首地址
*  @param [IN] uSizeData 将要传输的数据块的字节数
*  @return 0表示成功，其他值表示失败
*  @ingroup group_device
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SerialSend(int nSerialHandle, const unsigned char *pData, unsigned uSizeData);

/**
*  @brief 透明通道停止发送数据
*  @param [IN] nSerialHandle 由VzLPRClient_SerialStart函数获得的句柄
*  @return 0表示成功，其他值表示失败
*  @ingroup group_device
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SerialStop(int nSerialHandle);

/**
*  @brief 调整设备镜头的变倍和聚焦
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] eOPT 操作类型，详见定义VZ_LENS_OPT
*  @return 0表示成功，其他值表示失败
*  @ingroup group_device
*/
VZ_LPRC_API int __STDCALL VzLPRClient_CtrlLens(VzLPRClientHandle handle, VZ_LENS_OPT eOPT);

/**
*  @brief 获取LED当前亮度等级和最大亮度等级
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [OUT] pLevelNow 用于输出当前亮度等级的地址
*  @param [OUT] pLevelMax 用于输出最高亮度等级的地址
*  @return 0表示成功，其他值表示失败
*  @ingroup group_device
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetLEDLightStatus(VzLPRClientHandle handle, int *pLevelNow, int *pLevelMax);

/**
*  @brief 设置LED亮度等级
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] nLevel，LED亮度等级
*  @return 0表示成功，其他值表示失败
*  @ingroup group_device
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SetLEDLightLevel(VzLPRClientHandle handle, int nLevel);

/**
*  @brief 获取LED当前控制模式
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [OUT] pCtrl 用于输出当前LED开关控制模式的地址，详见定义 VZ_LED_CTRL
*  @return 0表示成功，其他值表示失败
*  @ingroup group_device
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetLEDLightControlMode(VzLPRClientHandle handle, VZ_LED_CTRL *pCtrl);

/**
*  @brief 设置LED控制模式
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] eCtrl 控制LED开关模式，详见定义 VZ_LED_CTRL
*  @return 返回值为0表示成功，返回其他值表示失败
*  @ingroup group_device
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SetLEDLightControlMode(VzLPRClientHandle handle, VZ_LED_CTRL eCtrl);

/**
*  @brief 写入用户私有数据，可用于二次加密
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] pUserData 用户数据
*  @param [IN] uSizeData 用户数据的长度，最大128字节
*  @return 返回值为0表示成功，返回其他值表示失败
*  @ingroup group_device
*/
VZ_LPRC_API int __STDCALL VzLPRClient_WriteUserData(VzLPRClientHandle handle, const unsigned char *pUserData, unsigned uSizeData);

/**
*  @brief 读出用户私有数据，可用于二次加密
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN/OUT] pBuffer 用于存放读到的用户数据
*  @param [IN] uSizeBuf 用户数据缓冲区的最小尺寸，不小于128字节
*  @return 返回值为实际用户数据的字节数，返回-1表示失败
*  @ingroup group_device
*/
VZ_LPRC_API int __STDCALL VzLPRClient_ReadUserData(VzLPRClientHandle handle, unsigned char *pBuffer, unsigned uSizeBuf);

/**
*  @brief 读出设备序列号，可用于二次加密
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN/OUT] pSN 用于存放读到的设备序列号，详见定义 VZ_DEV_SERIAL_NUM
*  @return 返回值为0表示成功，返回-1表示失败
*  @ingroup group_device
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetSerialNumber(VzLPRClientHandle handle, VZ_DEV_SERIAL_NUM *pSN);

/**
*  @brief 获取设备的日期时间
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN/OUT] pDTInfo 用于存放获取到的设备日期时间信息，详见定义 VZ_DATE_TIME_INFO
*  @return 返回值为0表示成功，返回-1表示失败
*  @ingroup group_device
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetDateTime(VzLPRClientHandle handle, VZ_DATE_TIME_INFO *pDTInfo);

/**
*  @brief 设置设备的日期时间
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] pDTInfo 将要设置的设备日期时间信息，详见定义 VZ_DATE_TIME_INFO
*  @return 返回值为0表示成功，返回-1表示失败
*  @ingroup group_device
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SetDateTime(VzLPRClientHandle handle, const VZ_DATE_TIME_INFO *pDTInfo);

/**
*  @brief 获取设备的存储信息
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN/OUT] pDTInfo 用于存放获取到的设备日期时间信息，详见定义 VZ_DATE_TIME_INFO
*  @return 返回值为0表示成功，返回-1表示失败
*  @ingroup group_device
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetStorageDeviceInfo(VzLPRClientHandle handle, VZ_STORAGE_DEVICES_INFO *pSDInfo);

/**
*  @brief 开始录像功能
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] sFileName 录像文件的路径
*  @return 返回值为0表示成功，返回-1表示失败
*  @ingroup group_device
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SaveRealData(VzLPRClientHandle handle,char *sFileName);

/**
*  @brief 停止录像
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @return 返回值为0表示成功，返回-1表示失败
*  @ingroup group_device
*/
VZ_LPRC_API int __STDCALL VzLPRClient_StopSaveRealData(VzLPRClientHandle handle);

/**
*  @brief 设置视频OSD参数；
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @return 返回值为0表示成功，返回-1表示失败
*  @ingroup group_device
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SetOsdParam(VzLPRClientHandle handle, VZ_LPRC_OSD_Param *pParam);

/**
*  @brief 获取视频OSD参数；
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @return 返回值为0表示成功，返回-1表示失败
*  @ingroup group_device
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetOsdParam(VzLPRClientHandle handle, VZ_LPRC_OSD_Param *pParam);

/**
*  @brief 开启脱机检查，实现脱机自动切换白名单的功能。
*  @param  [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param  [IN] pUserData 接收数据回调函数的上下文
*  @note   有效前提：设备的“输入输出”页面的“白名单验证”TAB页中的“白名单启用条件”设置为“脱机自动启用”
*  @note   调用该接口，设备与调用该接口的客户端之间就会形成绑定，如果客户端与设备之间连接正常，那么设备上的白名单就为停用状态，不会输出IO信号控制道闸抬杆和输出485推送协议；
*  @note   如果客户端与设备设备之间的网络断了，或者客户端关闭，或者调用了关闭脱机检查（CancelOfflineCheck）接口，那么设备上的白名单将启用，设备依据白名单控制道闸抬杆和输出485推送协议；
*  @return 返回值为0表示成功，返回-1表示失败
*  @ingroup group_device
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SetOfflineCheck(VzLPRClientHandle handle);

/**
*  @brief 关闭脱机检查
*  @param  [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param  [IN] pUserData 接收数据回调函数的上下文
*  @note   有效前提：设备的“输入输出”页面的“白名单验证”TAB页中的“白名单启用条件”设置为“脱机自动启用”
*  @note   关闭脱机检查，设备基于白名单来输出IO信号控制道闸抬杆和输出485推送协议。
*  @return 返回值为0表示成功，返回-1表示失败
*  @ingroup group_device
*/
VZ_LPRC_API int __STDCALL VzLPRClient_CancelOfflineCheck(VzLPRClientHandle handle);

/**
*  @brief 根据ID查询记录
*  @param  [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param  [IN] nID 车牌记录在数据中的ID
*  @return 返回值为0表示成功，返回-1表示失败
*  @ingroup group_device
*/
VZ_LPRC_API int __STDCALL VzLPRClient_QueryPlateRecordByID(VzLPRClientHandle handle, int nID);

/**
*  @brief 根据起始时间和车牌关键字查询记录
*  @param  [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param  [IN] pStartTime 起始时间，格式如"2015-01-02 12:20:30"
*  @param  [IN] pEndTime   起始时间，格式如"2015-01-02 19:20:30"
*  @param  [IN] keyword    车牌号关键字, 如"川"
*  @return 返回值为0表示成功，返回-1表示失败
*  @说明   通过回调返回数据，最多返回100条数据，超过时请调用分页查询的接口
*  @ingroup group_device
*/
VZ_LPRC_API int __STDCALL VzLPRClient_QueryRecordByTimeAndPlate(VzLPRClientHandle handle, const char *pStartTime, const char *pEndTime, const char *keyword);


/**
*  @brief 根据时间和车牌号查询记录条数
*  @param  [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param  [IN] pStartTime 起始时间，格式如"2015-01-02 12:20:30"
*  @param  [IN] pEndTime   起始时间，格式如"2015-01-02 19:20:30"
*  @param  [IN] keyword    车牌号关键字, 如"川"
*  @return 返回值为0表示失败，大于0表示记录条数
*  @ingroup group_device
*/
VZ_LPRC_API int __STDCALL VzLPRClient_QueryCountByTimeAndPlate(VzLPRClientHandle handle, const char *pStartTime, const char *pEndTime, const char *keyword);

/**
*  @brief 根据时间和车牌号查询分页记录
*  @param  [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param  [IN] pStartTime 起始时间，格式如"2015-01-02 12:20:30"
*  @param  [IN] pEndTime   起始时间，格式如"2015-01-02 19:20:30"
*  @param  [IN] keyword    车牌号关键字, 如"川"
*  @param  [IN] start      起始位置大于0,小于结束位置
*  @param  [IN] end        结束位置大于0,大于起始位置，获取记录条数不能大于100
*  @return 返回值为0表示成功，返回-1表示失败
*  @ingroup group_device
*/
VZ_LPRC_API int __STDCALL VzLPRClient_QueryPageRecordByTimeAndPlate(VzLPRClientHandle handle, const char *pStartTime, const char *pEndTime, const char *keyword, int start, int end);


/**
*  @brief 设置查询车牌记录的回调函数
*  @param  [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param  [IN] func 识别结果回调函数，如果为NULL，则表示关闭该回调函数的功能
*  @param  [IN] pUserData 回调函数中的上下文
*  @param  [IN] bEnableImage 指定识别结果的回调是否需要包含截图信息：1为需要，0为不需要
*  @return 0表示成功，-1表示失败
*  @ingroup group_device
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SetQueryPlateCallBack(VzLPRClientHandle handle, VZLPRC_PLATE_INFO_CALLBACK func, void *pUserData);


/**
*  @brief 根据ID获取车牌图片
*  @param  [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param  [IN] id     车牌记录的ID
*  @param  [IN] pdata  存储图片的内存
*  @param  [IN][OUT] size 为传入传出值，传入为图片内存的大小，返回的是获取到jpg图片内存的大小
*  @return 返回值为0表示成功，返回-1表示失败
*  @ingroup group_device
*/
VZ_LPRC_API int __STDCALL VzLPRClient_LoadImageById(VzLPRClientHandle handle, int id, void *pdata, int* size);


/**
*  @brief 根据路径获取图片
*  @param  [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param  [IN] img_path 存储在相机里面的图片路径
*  @param  [IN] pdata  存储图片的内存
*  @param  [IN][OUT] size 为传入传出值，传入为图片内存的大小，返回的是获取到jpg图片内存的大小
*  @return 返回值为0表示成功，返回-1表示失败
*  @ingroup group_device
*/
VZ_LPRC_API int __STDCALL VzLPRClient_LoadImageByPath(VzLPRClientHandle handle, const char* img_path, void *pdata, int* size);

/**
*  @brief 获取视频的宽度和高度
*  @param  [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param  [OUT] pWidth		视频图像宽度变量的地址
*  @param  [OUT] pHeight	视频图像高度变量的地址
*  @return 返回值为0表示成功，返回-1表示失败
*  @ingroup group_device
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetVideoFrameSize(VzLPRClientHandle handle, int *pWidth, int *pHeight);

/**
*  @brief 获取视频的编码方式
*  @param  [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param  [OUT] pEncType	返回的编码方式, 0:H264  1:MPEG4  2:JPEG  其他:错误
*  @return 返回值为0表示成功，返回-1表示失败
*  @ingroup group_device
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetVideoEncodeType(VzLPRClientHandle handle, int *pEncType);

/**
*  @brief 获取GPIO的状态
*  @param  [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param  [IN] gpioIn 数据为0或1
*  @param  [OUT] value 0代表短路，1代表开路
*  @return 返回值为0表示成功，返回-1表示失败
*  @ingroup group_device
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetGPIOValue(VzLPRClientHandle handle, int gpioIn, int *value);

/**
*  @brief 根据时间查询脱机记录条数
*  @param  [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param  [IN] pStartTime 起始时间，格式如"2015-01-02 12:20:30"
*  @param  [IN] pEndTime   起始时间，格式如"2015-01-02 19:20:30"
*  @return 返回值为0表示失败，大于0表示记录条数
*  @ingroup group_device
*/
VZ_LPRC_API int __STDCALL VzLPRClient_QueryOfflineCountByTime(VzLPRClientHandle handle, const char *pStartTime, const char *pEndTime);

/**
*  @brief 根据时间和车牌号分页查询脱机记录
*  @param  [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param  [IN] pStartTime 起始时间，格式如"2015-01-02 12:20:30"
*  @param  [IN] pEndTime   起始时间，格式如"2015-01-02 19:20:30"
*  @param  [IN] start      起始位置大于0,小于结束位置
*  @param  [IN] end        结束位置大于0,大于起始位置，获取记录条数不能大于100
*  @return 返回值为0表示成功，返回-1表示失败
*  @ingroup group_device
*/
VZ_LPRC_API int __STDCALL VzLPRClient_QueryPageOfflineRecordByTime(VzLPRClientHandle handle, const char *pStartTime, const char *pEndTime, int start, int end);


/**
*  @brief 获取系统日志
*  @param  [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] iLogType 日志的类型
*  @param [IN/OUT] pstrLogBuffer 日志内容缓冲
*  @param [IN/OUT] iBufferLen 日志内容缓冲区长度,返回日志长度
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetLog(VzLPRClientHandle handle, int iLogType,char* pstrLogBuffer, unsigned int* iBufferLen);

/**
*  @brief 完全恢复默认参数；
*  @param  [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_RestoreConfig(VzLPRClientHandle handle);

/**
*  @brief 部分恢复默认参数，ip端口配置，用户配置，设备名保留；
*  @param  [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_RestoreConfigPartly(VzLPRClientHandle handle);

/**
*  @brief 获取设备版本号，版本号由4个数值组成
*  @param  [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @return 返回值为0表示成功;
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetRemoteSoftWareVersion(VzLPRClientHandle handle, int *ver1, int *ver2, int *ver3, int *ver4);

/**
*  @brief 格式化sd卡
*  @param  [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SDFormat(VzLPRClientHandle handle);

//升级
/**
*  @brief 获得升级文件版本号
*  @param [IN] file_path_name 升级文件路径
*  @param [OUT] ver1 版本号1
*  @param [OUT] ver2 版本号2
*  @param [OUT] ver3 版本号3
*  @param [OUT] ver4 版本号4
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetUpdateFileVersion(char *file_path_name, int *ver1, int *ver2, int *ver3, int *ver4);

/**
*  @brief 开始升级
*  @param  [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] file_path_name 升级文件路径
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_Update(VzLPRClientHandle handle, char *file_path_name);


/**
*  @brief 获取升级的状态
*  @param  [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @return 升级的状态
*			0: 还未升过级
*			1: 升级成功
*			2: 正在升级，上传中，可以取消升级
*			3: 正在升级，写flash中，取消升级不可用
*		    4: 网络问题，状态未知
*		   -1: 升级失败
*			失败或者网络问题时可以通过getlasterror获得升级错误信息，定义在UPDATE_ERROR里
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetUpdateState(VzLPRClientHandle handle);


/**
*  @brief 取消升级，当升级状态在2时，可以取消升级，其余时候无用途
*  @param  [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_StopUpdate(VzLPRClientHandle handle);


/**
*  @brief 获取升级进度，上传完毕为50，升级完成为100，当前会从50直接跳到100，中间进度不会更新，以后解决
*  @param  [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetUpdateProgress(VzLPRClientHandle handle);

/**
*  @brief 设备网络参数配置。
*  @param  [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] netcfg 参见VZ_LPRC_NETCFG结构体定义
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SetNetConfig(VzLPRClientHandle handle, VZ_LPRC_NETCFG* netcfg);

/**
*  @brief 获取设备网络参数配置。
*  @param  [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [OUT] netcfg 参见VZ_LPRC_NETCFG结构体定义
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetNetConfig(VzLPRClientHandle handle, VZ_LPRC_NETCFG* netcfg);


/**
*  @brief 设置IO输出，并自动复位
*  @param  [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param  [IN] uChnId IO输出的通道号，从0开始
*  @param  [IN] nDuration 延时时间，取值范围[500, 5000]毫秒
*  @return 0表示成功，-1表示失败
*  @ingroup group_device
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SetIOOutputAuto(VzLPRClientHandle handle, unsigned uChnId, int nDuration);

/**
* 获取 LastError
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetLastError();

/**
* 重启设备
*/
VZ_LPRC_API int __STDCALL VzLPRClient_RebootDVR( VzLPRClientHandle handle );

/**
*  @brief 自动变焦的功能
*  @param  [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @return 返回值：返回值为0表示成功，返回其他值表示失败
*/
VZ_LPRC_API int __STDCALL VzLPRClient_DoAutoFocus(VzLPRClientHandle handle);

/**
*  @brief 设置需要识别的车牌类型
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] uBitsRecType 需要识别的车牌类型按位或的值，车牌类型位详见定义VZ_LPRC_REC_XXX
*  @return 返回值：返回值为0表示成功，返回其他值表示失败
*  @note  在需要识别特定车牌时，调用该接口来设置，将不同类型的车牌位定义取或，得到的结果作为参数传入；
*  @note  在不必要的情况下，使用最少的车牌识别类型，将最大限度提高识别率；
*  @note  默认识别蓝牌和黄牌；
*  @note  例如，需要识别蓝牌、黄牌、警牌，那么输入参数uBitsRecType = VZ_LPRC_REC_BLUE|VZ_LPRC_REC_YELLOW|VZ_LPRC_REC_POLICE
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SetPlateRecType(VzLPRClientHandle handle, unsigned uBitsRecType);

/**
*  @brief 获取已设置的需要识别的车牌类型位
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [OUT] pBitsRecType 需要识别的车牌类型按位或的变量的地址，车牌类型位详见定义VZ_LPRC_REC_XXX
*  @return 返回值：返回值为0表示成功，返回其他值表示失败
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetPlateRecType(VzLPRClientHandle handle, unsigned *pBitsRecType);

/**
*  @brief 获取系统版本号
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN/OUT] pstrVersion 保存返回版本号的BUF
*  @param [IN] uSizeBuf 保存返回版本号的BUF大小
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetSystemVersion(VzLPRClientHandle handle, char *pstrVersion, unsigned uSizeBuf);

/**
*  @brief 设置允许的车牌识别触发类型
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] uBitsTrigType 允许的车牌识别触发类型按位或的值，允许触发类型位详见定义VZ_LPRC_TRIG_ENABLE_XXX
*  @return 返回值：返回值为0表示成功，返回其他值表示失败
*  @note  如果设置不允许某种类型的触发，那么该种类型的触发结果也不会保存在设备的SD卡中
*  @note  默认输出稳定触发和虚拟线圈触发
*  @note  不会影响手动触发和IO输入触发
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SetPlateTrigType(VzLPRClientHandle handle, unsigned uBitsTrigType);

/**
*  @brief 获取已设置的允许的车牌识别触发类型
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [OUT] pBitsTrigType 允许的车牌识别触发类型按位或的变量的地址，允许触发类型位详见定义VZ_LPRC_TRIG_ENABLE_XXX
*  @return 返回值：返回值为0表示成功，返回其他值表示失败
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetPlateTrigType(VzLPRClientHandle handle, unsigned *pBitsTrigType);


/**
*  @brief 设置车牌图片里是否显示车牌框
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] bShow 是否显示车牌框，输入值(0或1)，1代表显示，0代表不显示
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SetIsShowPlateRect(VzLPRClientHandle handle, int bShow);

/**
*  @brief 获取用户自定义配置
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [OUT] buffer 保存返回配置的buffer
*  @param [IN] buffer_len 保存返回的buffer大小
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetUserDefinedConfig(VzLPRClientHandle handle, char* buffer, int buffer_len);

/**
*  @brief 设置用户自定义配置
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] buffer 配置内容，必需为字符串
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SetUserDefinedConfig(VzLPRClientHandle handle, char* buffer);

/**
*  @brief 修改网络参数
*  @param  [IN] SL        设备序列号低位字节
*  @param  [IN] SH		  设备序列号高位字节
*  @param [IN] strNewIP   新IP     格式如"192.168.3.109"
*  @param [IN] strGateway 网关     格式如"192.168.3.1"
*  @param [IN] strNetmask 子网掩码 格式如"255.255.255.0"
*  @note 可以用来实现跨网段修改IP的功能
*  @ingroup group_global
*/
VZ_LPRC_API int __STDCALL VzLPRClient_UpdateNetworkParam(unsigned int SL, unsigned int SH, const char* strNewIP, const char* strGateway, const char *strNetmask);


/**
*  @brief 获取设备加密类型和当前加密类型
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] pData 加密信息
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetEMS(VzLPRClientHandle handle, VZ_LPRC_ACTIVE_ENCRYPT* pData);

/**
*  @brief 设置设备加密类型
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] pCurrentKey 当前识别密码
*  @param [IN] nEncyptId	修改的加密类型ID
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SetEncrypt(VzLPRClientHandle handle, const void* pCurrentKey, const unsigned nEncyptId);

/**
*  @brief 修改设备识别密码
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] pCurrentKey 当前识别密码
*  @param [IN] pNewKey	新识别密码
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_ChangeEncryptKey(VzLPRClientHandle handle, const void* pCurrentKey, const void* pNewKey);

/**
*  @brief 重置设备识别密码
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] pPrimeKey 当前设备主密码
*  @param [IN] pNewKey	新识别密码
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_ResetEncryptKey(VzLPRClientHandle handle, const void* pPrimeKey, const void* pNewKey);

/**
*  @brief GPIO输入回调函数
*  @param [IN] nGPIOId GPIO输入ID
*  @param [IN] nVal	GPIO输入值
*  @param [IN] pUserData 用户自定义数据
*/
typedef void (__STDCALL *VZLPRC_GPIO_RECV_CALLBACK)(VzLPRClientHandle handle, int nGPIOId, int nVal, void* pUserData);

/**
*  @brief 设置GPIO输入回调函数
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] func GPIO输入回调函数
*  @param [IN] pUserData 用户自定义数据
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SetGPIORecvCallBack(VzLPRClientHandle handle, VZLPRC_GPIO_RECV_CALLBACK func, void* pUserData);


// #ifdef _DEV_ENCRYPT_
/**
*  @brief 获取用户密码
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] pPrimeKey 主密码
*  @param [IN] pUserKey	 加密过的用户密码
*  @return 返回值为0表示成功，返回其他值表示失败。
*  @note 此接口不对外提供
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetEncryptKey(VzLPRClientHandle handle, const void* pPrimeKey, const void* pUserKey);

//#endif

VZ_LPRC_API int __STDCALL VzLPRClient_GetSesID(VzLPRClientHandle handle, char* buffer,int buffer_len);

/**
*  @brief 将图像保存为JPEG到指定路径，可指定图像尺寸的模式
*  @param  [IN] pImgInfo 图像结构体，目前只支持默认的格式，即ImageFormatRGB
*  @param  [IN] pFullPathName 设带绝对路径和JPG后缀名的文件名字符串
*  @param  [IN] nQuality JPEG压缩的质量，取值范围1~100；
*  @param  [IN] mode     图像大小的模式；
*  @return 0表示成功，-1表示失败
*  @note   给定的文件名中的路径需要存在
*/
VZ_LPRC_API int __STDCALL VzLPRClient_ImageSaveToJpegEx(const VZ_LPRC_IMAGE_INFO *pImgInfo, const char *pFullPathName, int nQuality, IMG_SIZE_MODE mode);

/**
*  @brief 将图像编码为JPEG，保存到指定内存，可指定图像尺寸的模式
*  @param  [IN] pImgInfo 图像结构体，目前只支持默认的格式，即ImageFormatRGB
*  @param  [IN/OUT] pDstBuf JPEG数据的目的存储首地址
*  @param  [IN] uSizeBuf JPEG数据地址的内存的最大尺寸；
*  @param  [IN] nQuality JPEG压缩的质量，取值范围1~100；
*  @param  [IN] mode     图像大小的模式；
*  @return >0表示成功，即编码后的尺寸，-1表示失败，-2表示给定的压缩数据的内存尺寸不够大
*/
VZ_LPRC_API int __STDCALL VzLPRClient_ImageEncodeToJpegEx(const VZ_LPRC_IMAGE_INFO *pImgInfo, void *pDstBuf, unsigned uSizeBuf, int nQuality, IMG_SIZE_MODE mode);

/**
*  @brief 设置是否输出实时结果
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param  [IN] bOutput 是否输出
*  @return 0表示成功，-1表示失败
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SetIsOutputRealTimeResult( VzLPRClientHandle handle, bool bOutput );

/**
*  @brief 获取设备硬件信息
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [OUT] status 状态 0 没有取得硬件信息，1有基本信息，2有基本和扩展信息
*  @param [OUT] mac 输出mac地址，输入字符串缓冲长度至少为20
*  @param [OUT] serialno 输出序列号，输入字符串缓冲长度至少为20
*  @param [OUT] device_type 设备类型，参考IVS_TYPE定义
*  @param [OUT] pdata_ex 扩展硬件信息，参考VZ_FS_INFO_EX定义
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetHwInfo( VzLPRClientHandle handle, int* status, char* mac, char* serialno,int *device_type,VZ_FS_INFO_EX *pdata_ex);


/**
*  @brief 保存抓图数据到Jpeg文件
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] pFullPathName 图片路径
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SaveSnapImageToJpeg(VzLPRClientHandle handle, const char *pFullPathName);

/**
*  @brief 保存抓图数据到缓冲区
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] pDstBuf 接收图片的缓冲区
*  @param [OUT] uSizeBuf 接收图片的缓冲区长度
*  @return 返回值为大于0表示接收图片的长度，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetSnapImage(VzLPRClientHandle handle, void  *pDstBuf, int  uSizeBuf);


/**
*  @brief 弃用
*  @brief 获取当前组网的设备信息
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] pData 当前所有已连接的组网设备
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetOvzid(VzLPRClientHandle handle, VZ_OVZID_RESULT* pData);

/**
*  @brief 弃用
*  @brief 配置组网
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] pInfo 组网设备信息
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_EnableDevicegroup(VzLPRClientHandle handle, VZ_OVZID_INFO *pData);

/**
*  @brief 开启/关闭“接受组网识别结果”
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] bEnable true : 开启;false : 关闭
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SetDGResultEnable(VzLPRClientHandle handle, bool bEnable);

/**
*  @brief 获取主码流分辨率；
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [OUT] sizeval 详见VZDEV_FRAMESIZE_宏定义
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetVideoFrameSizeIndex(VzLPRClientHandle handle, int *sizeval);

/**
*  @brief 设置主码流分辨率；
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] sizeval 详见VZDEV_FRAMESIZE_宏定义
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SetVideoFrameSizeIndex(VzLPRClientHandle handle, int sizeval);

/**
*  @brief 获取主码流帧率
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [OUT] Rateval 帧率，范围1-25
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetVideoFrameRate(VzLPRClientHandle handle, int *Rateval);//1-25

/**
*  @brief 设置主码流帧率；
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] Rateval 帧率，范围1-25
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SetVideoFrameRate(VzLPRClientHandle handle, int Rateval);//1-25

/**
*  @brief 获取主码流压缩模式；
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [OUT] modeval 详见VZDEV_VIDEO_COMPRESS_宏定义
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetVideoCompressMode(VzLPRClientHandle handle, int *modeval);//VZDEV_VIDEO_COMPRESS_XXX

/**
*  @brief 设置主码流压缩模式；
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] modeval 详见VZDEV_VIDEO_COMPRESS_宏定义
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SetVideoCompressMode(VzLPRClientHandle handle, int modeval);//VZDEV_VIDEO_COMPRESS_XXX


/**
*  @brief 获取主码流比特率；
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [OUT] rateval 当前视频比特率
*  @param [OUT] ratelist 暂时不用
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetVideoCBR(VzLPRClientHandle handle, int *rateval/*Kbps*/, int *ratelist);

/**
*  @brief 设置主码流比特率；
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [OUT] rateval 当前视频比特率
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SetVideoCBR(VzLPRClientHandle handle, int rateval/*Kbps*/);

/**
*  @brief 获取智能视频显示模式
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [OUT] pDrawMode 显示模式，参考VZ_LPRC_DRAWMODE
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetDrawMode(VzLPRClientHandle handle, VZ_LPRC_DRAWMODE * pDrawMode);

/**
*  @brief 设置智能视频显示模式
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] pDrawMode 显示模式，参考VZ_LPRC_DRAWMODE
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SetDrawMode(VzLPRClientHandle handle, VZ_LPRC_DRAWMODE * pDrawMode);

/**
*  @brief 获取视频参数；
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [OUT] brt 亮度
*  @param [OUT] cst 对比度
*  @param [OUT] sat 饱和度
*  @param [OUT] hue 色度
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetVideoPara(VzLPRClientHandle handle, int *brt, int *cst, int *sat, int *hue);

/**
*  @brief 设置视频参数；
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] brt 亮度
*  @param [IN] cst 对比度
*  @param [IN] sat 饱和度
*  @param [IN] hue 色度
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SetVideoPara(VzLPRClientHandle handle, int brt, int cst, int sat, int hue);

/**
*  @brief 设置通道主码流编码方式
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [OUT] cmd    返回的编码方式, 0->H264  1->MPEG4  2->JPEG  其他->错误
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SetVideoEncodeType(VzLPRClientHandle handle, int cmd);

/**
*  @brief 获取视频图像质量；
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] levelval //0~6，6最好
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetVideoVBR(VzLPRClientHandle handle, int *levelval);

/**
*  @brief 设置视频图像质量；
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] levelval //0~6，6最好
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SetVideoVBR(VzLPRClientHandle handle, int levelval);


/**
*  @brief 获取视频制式；
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] frequency 0:MaxOrZero, 1: 50Hz, 2:60Hz
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetFrequency(VzLPRClientHandle handle, int *frequency);

/**
*  @brief 设置视频制式；
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] frequency 0:MaxOrZero, 1: 50Hz, 2:60Hz
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SetFrequency(VzLPRClientHandle handle, int frequency);

/**
*  @brief 获取曝光时间；
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] shutter 2:>0~8ms 停车场推荐, 3: 0~4ms, 4:0~2ms 卡口推荐
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetShutter(VzLPRClientHandle handle, int *shutter);

/**
*  @brief 设置曝光时间；
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] shutter 2:>0~8ms 停车场推荐, 3: 0~4ms, 4:0~2ms 卡口推荐
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SetShutter(VzLPRClientHandle handle, int shutter);

/**
*  @brief 获取图像翻转；
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] flip, 0: 原始图像, 1:上下翻转, 2:左右翻转, 3:中心翻转
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetFlip(VzLPRClientHandle handle, int *flip);

/**
*  @brief 设置图像翻转；
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] flip, 0: 原始图像, 1:上下翻转, 2:左右翻转, 3:中心翻转
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SetFlip(VzLPRClientHandle handle, int flip);


/**
*  @brief 设置车牌识别触发延迟时间
*  @param  [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param  [IN] nDelay 触发延迟时间,时间范围[0, 10000)
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SetTriggerDelay(VzLPRClientHandle handle, int nDelay);

/**
*  @brief 获取车牌识别触发延迟时间
*  @param  [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param  [OUT] nDelay 触发延迟时间,时间范围[0, 10000)
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetTriggerDelay(VzLPRClientHandle handle, int* nDelay);

/**
*  @brief 设置白名单验证模式
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] nType 0 脱机自动启用;1 启用;2 不启用
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SetWLCheckMethod(VzLPRClientHandle handle, int nType);

/**
*  @brief 获取白名单验证模式
*  @param  [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [OUT]	nType 0 脱机自动启用;1 启用;2 不启用
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetWLCheckMethod(VzLPRClientHandle handle, int* nType);

/**
*  @brief 设置白名单模糊匹配
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] nFuzzyType 0  精确匹配;1 相似字符匹配;2 普通字符模糊匹配
*  @param [IN] nFuzzyLen  允许误识别长度
*  @param [IN] nFuzzyType 忽略汉字
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SetWLFuzzy(VzLPRClientHandle handle, int nFuzzyType, int nFuzzyLen, bool bFuzzyCC);

/**
*  @brief 获取白名单模糊匹配
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] nFuzzyType 0  精确匹配;1 相似字符匹配;2 普通字符模糊匹配
*  @param [IN] nFuzzyLen  允许误识别长度
*  @param [IN] nFuzzyType 忽略汉字
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetWLFuzzy(VzLPRClientHandle handle, int* nFuzzyType, int* nFuzzyLen, bool* bFuzzyCC);

/**
*  @brief 设置输出配置
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] pOutputConfig 输出配置
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SetOutputConfig(VzLPRClientHandle handle, VZ_OutputConfigInfo* pOutputConfigInfo);

/**
*  @brief 获取输出配置0
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] pOutputConfig 输出配置
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetOutputConfig(VzLPRClientHandle handle, VZ_OutputConfigInfo* pOutputConfigInfo);

/**
*  @brief 获取设备序列号；
*  @param [IN] ip ip统一使用字符串的形式传入
*  @param [IN] port 使用和登录时相同的端口
*  @param [OUT] SerHi 序列号高位
*  @param [OUT] SerLo 序列号低位
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetSerialNo(char *ip, WORD port, unsigned int *SerHi, unsigned int *SerLo);

/**
*  @brief 开始实时图像数据流，用于实时获取图像数据
*  @param  [IN] handle		由VzLPRClient_Open函数获得的句柄
*  @return 返回值为0表示成功，返回其他值表示失败。
*  @ingroup group_device
*/
VZ_LPRC_API int __STDCALL VzLPRClient_StartRealPlayDecData(VzLPRClientHandle handle);

/**
*  @brief 停止实时图像数据流
*  @param  [IN] handle		由VzLPRClient_Open函数获得的句柄
*  @return 返回值为0表示成功，返回其他值表示失败。
*  @ingroup group_device
*/
VZ_LPRC_API int __STDCALL VzLPRClient_StopRealPlayDecData(VzLPRClientHandle handle);


/**
*  @brief 从解码流中获取JPEG图像，保存到指定内存
*  @param  [IN] handle		由VzLPRClient_Open函数获得的句柄
*  @param  [IN/OUT] pDstBuf JPEG数据的目的存储首地址
*  @param  [IN] uSizeBuf JPEG数据地址的内存的最大尺寸；
*  @param  [IN] nQuality JPEG压缩的质量，取值范围1~100；
*  @return >0表示成功，即编码后的尺寸，-1表示失败，-2表示给定的压缩数据的内存尺寸不够大
*  @ingroup group_global
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetJpegStreamFromRealPlayDec(VzLPRClientHandle handle, void *pDstBuf, unsigned uSizeBuf, int nQuality);

/**
*  @brief 获取串口推送配置
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] pConfigInfo 串口推送配置
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetRS485PushConfig(VzLPRClientHandle handle, VZ_RS485_Config* pConfigInfo);

/**
*  @brief 设置串口推送配置
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] pConfigInfo 串口推送配置
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SetRS485PushConfig(VzLPRClientHandle handle, VZ_RS485_Config* pConfigInfo);


/**
*  @brief aes解密功能
*  @param [IN] hertextb    待解密16进制的数据
*  @param [IN] password    密码
*  @param [IN] nTextBits   数据长度
*  @param [IN] decryptText 解密字符串
*  @param [IN] decryptLen  传入解密字符串buffer的长度
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_AesCtrDecrypt(const char* hertextb, int nTextBits, const char* password, char *decryptText, int decryptLen);

/**
*  @brief 根据ID获取组网结果的大图片
*  @param  [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param  [IN] id     车牌记录的ID
*  @param  [IN] sn     设备序列号
*  @param  [IN] pdata  存储图片的内存
*  @param  [IN][OUT] size 为传入传出值，传入为图片内存的大小，返回的是获取到jpg图片内存的大小
*  @return 返回值为0表示成功，返回-1表示失败
*  @ingroup group_device
*/
VZ_LPRC_API int __STDCALL VzLPRClient_LoadGroupFullImageById(VzLPRClientHandle handle, int id, const char* sn, void* pdata, int* size);

/**
*  @brief 根据ID获取组网结果的小图片
*  @param  [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param  [IN] id     车牌记录的ID
*  @param  [IN] sn     设备序列号
*  @param  [IN] pdata  存储图片的内存
*  @param  [IN][OUT] size 为传入传出值，传入为图片内存的大小，返回的是获取到jpg图片内存的大小
*  @return 返回值为0表示成功，返回-1表示失败
*  @ingroup group_device
*/
VZ_LPRC_API int __STDCALL VzLPRClient_LoadGroupClipImageById(VzLPRClientHandle handle, int id, const char* sn, void* pdata, int* size);

/**
*  @brief 获取NTP配置
*  @param  [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [OUT] ntp_server_name 服务器地址
*  @param [IN] name_size 服务器地址缓冲区长度
*  @param [OUT] ntp_enable 是否启用NTP
*  @param [OUT] frequency_time 同步的时间
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetNtpConfig(VzLPRClientHandle handle, char* ntp_server_name, int name_size, bool* ntp_enable, int* frequency_time);

/**
*  @brief 设置NTP配置
*  @param  [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] ntp_server_name 服务器地址
*  @param [IN] ntp_enable 是否启用NTP
*  @param [IN] frequency_time 同步的时间
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SetNtpConfig(VzLPRClientHandle handle, const char* ntp_server_name, bool ntp_enable, int frequency_time);


/**
*  @brief 开始二次登录
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] pData 加密信息
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_StartLogin(VzLPRClientHandle handle, VZ_LPRC_ACTIVE_ENCRYPT* pData);


/**
*  @brief 执行二次登录
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] key 用户密码
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_LoginAuth(VzLPRClientHandle handle, const char* key);

/**
*  @brief 弃用
*  @brief 打开
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] key 用户密码
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_OpenAuth(const char *pStrIP, WORD wPort, const char *pStrUserName, const char *pStrPassword, const char* key);


/**
*  @brief 设置当前配置为默认配置
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SetUserDefaultCfg(VzLPRClientHandle handle);


/**
*  @brief 语音播放功能
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] voice 播放的语音文字
*  @param [IN] interval 语音文件的播放间隔(0-5000)ms
*  @param [IN] volume 声音大小(0-100)
*  @param [IN] male 声音类型(男声0，女生1)
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_PlayVoice(VzLPRClientHandle handle, const char *voice, int interval, int volume, int male);

/**
*  @brief 语音播放配置
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] interval 语音文件的播放间隔(0-5000)
*  @param [IN] volume 声音大小(0-100)
*  @param [IN] male 声音类型(男声0，女生1)
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SetVoiceConfig(VzLPRClientHandle handle, int interval, int volume, int male);


/**
*  @brief 语音播放配置
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [OUT] interval 语音文件的播放间隔(0-5000)
*  @param [OUT] volume 声音大小(0-100)
*  @param [OUT] male 声音类型(男声0，女生1)
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetVoiceConfig(VzLPRClientHandle handle, int* interval, int* volume, int* male);


//**************************************************************
// 中心服务器配置
/**
*  @brief 设置中心服务器网络
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] pCenterServerNet  中心服务器信息结构
*  @return 返回值为0表示成功，返回其他值表示失败。
*/

VZ_LPRC_API int __STDCALL VzLPRClient_SetCenterServerNet(VzLPRClientHandle handle, VZ_LPRC_CENTER_SERVER_NET * pCenterServerNet);

/**
*  @brief 获取中心服务器网络
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] pCenterServerNet  中心服务器信息结构
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetCenterServerNet(VzLPRClientHandle handle, VZ_LPRC_CENTER_SERVER_NET * pCenterServerNet);

/**
*  @brief 设置中心服务器设备注册
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] pCenterServerDeviceReg  中心服务器注册结构
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SetCenterServerDeviceReg(VzLPRClientHandle handle, VZ_LPRC_CENTER_SERVER_DEVICE_REG * pCenterServerDeviceReg);

/**
*  @brief 获取中心服务器设备注册
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] pCenterServerDeviceReg  中心服务器注册结构
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetCenterServerDeviceReg(VzLPRClientHandle handle, VZ_LPRC_CENTER_SERVER_DEVICE_REG * pCenterServerDeviceReg);


/**
*  @brief 设置中心服务器网络车牌推送信息
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] pCenterServerPlate  中心服务器车牌推送信息
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SetCenterServerPlate(VzLPRClientHandle handle, VZ_LPRC_CENTER_SERVER_PLATE * pCenterServerPlate);

/**
*  @brief 获取中心服务器网络车牌推送信息
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] pCenterServerPlate  中心服务器车牌推送信息
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetCenterServerPlate(VzLPRClientHandle handle, VZ_LPRC_CENTER_SERVER_PLATE * pCenterServerPlate);

/**
*  @brief 设置中心服务器网络
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] pCenterServerNet  中心服务器信息
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SetCenterServerGionin(VzLPRClientHandle handle, VZ_LPRC_CENTER_SERVER_GIONIN * pCenterServerGionin);

/**
*  @brief 获取中心服务器网络端口触发信息
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] pCenterServerGionin  中心服务器端口触发信息
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetCenterServerGionin(VzLPRClientHandle handle, VZ_LPRC_CENTER_SERVER_GIONIN * pCenterServerGionin);

/**
*  @brief 设置中心服务器网络串口信息
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] pCenterServerSerial  中心服务器串口信息
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SetCenterServerSerial(VzLPRClientHandle handle, VZ_LPRC_CENTER_SERVER_SERIAL * pCenterServerSerial);

/**
*  @brief 获取中心服务器网络串口信息
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] pCenterServerSerial  中心服务器串口信息
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetCenterServerSerial(VzLPRClientHandle handle, VZ_LPRC_CENTER_SERVER_SERIAL * pCenterServerSerial);

/**
*  @brief 设置中心服务器网络主机备份信息
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] pCenterServerHostBak  中心服务器主机备份信息  例如:"192.168.3.5;192.168.3.6"
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SetCenterServerHostBak(VzLPRClientHandle handle,  char * pCenterServerHostBak);

/**
*  @brief 获取中心服务器网络主机备份信息
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] pCenterServerHostBak  中心服务器主机备份信息  例如:"192.168.3.5;192.168.3.6"
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetCenterServerHostBak(VzLPRClientHandle handle,  char * pCenterServerHostBak);

//**************************************************************
// 设备有效时间
/**
*  @brief 设置设备有效时间
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] status  0 代表不激活超时管理。也就是设备可以不限制时间的使用。1 代表激活超时管理。
*  @param [IN] active_time 代表的是一个以秒为单位的超时时间。如果 active_status 启动了超时管理的话，那么随着设备运行，这个时间会逐步减少，
*              直到为0 ，那么设备就会不再对外输出车牌识别结果了。
*  @param [IN] pCurrentKey 当前识别密码
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SetDeviceActiveStatus( VzLPRClientHandle handle, int status, int times, const char* pCurrentKey );


/**
*  @brief 获取设备有效时间
*  @param [IN]  handle 由VzLPRClient_Open函数获得的句柄
*  @param [OUT] status  0 代表不激活超时管理。也就是设备可以不限制时间的使用。1 代表激活超时管理。
*  @param [OUT] active_time 代表的是一个以秒为单位的超时时间。如果 active_status 启动了超时管理的话，那么随着设备运行，这个时间会逐步减少，
*               直到为0 ，那么设备就会不再对外输出车牌识别结果了。
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetDeviceActiveStatus( VzLPRClientHandle handle, int* status, int* times );


/**
*  @brief 获取设备硬件信息
*  @param [IN]  handle 由VzLPRClient_Open函数获得的句柄
*  @param [OUT] board_version  设备类型
*  @param [OUT] exdataSize 额外数据长度。
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetHwBoardVersion( VzLPRClientHandle handle, int* board_version, long long* exdataSize );


/**
*  @brief 获取设备硬件类型
*  @param [IN]  handle 由VzLPRClient_Open函数获得的句柄
*  @param [OUT] board_type  设备类型(0:3730,1:6446,2:8127)
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetHwBoardType( VzLPRClientHandle handle, int* board_type );


/**
*  @brief 获取特殊车牌配置
*  @param [IN]  handle 由VzLPRClient_Open函数获得的句柄
*  @param [OUT] pCfgBuf配置数据
*  @param [IN] 传入数据的长度
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetSpecialPlatesCfg(VzLPRClientHandle handle, char* pCfgBuf, int len);

/**
*  @brief 设置特殊车牌配置
*  @param [IN]  handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] pCfgBuf配置数据
*  @param [IN] len传入数据长度
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SetSpecialPlatesCfg(VzLPRClientHandle handle, char* pCfgBuf, int len);

/**
*  @brief 获取爆闪控制参数
*  @param [IN] iUserID VZC_Login函数返回的用户ID
*  @param [OUT] comp_enable启用爆闪灯
*  @param [OUT] ivt_time触发延迟时间
*  @param [OUT] dealy_time爆闪持续时间
*  @param [OUT] dealy_out_time爆闪延迟时间
*  @param [OUT] gpio_in地感输入口
*  @param [IN]  in_count数据长度
*  @param [OUT] gpio_out爆闪控制口
*  @param [IN]  out_count数据长度
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetLedComplensating(VzLPRClientHandle handle, bool* comp_enable, int* ivt_time, int* dealy_time, int* dealy_out_time, char* gpio_in, int in_count, char* gpio_out, int out_count);

/**
*  @brief 设置爆闪控制参数
*  @param [IN] iUserID VZC_Login函数返回的用户ID
*  @param [IN] comp_enable启用爆闪灯
*  @param [IN] ivt_time触发延迟时间
*  @param [IN] dealy_time爆闪持续时间
*  @param [IN] dealy_out_time爆闪延迟时间
*  @param [IN] gpio_in地感输入口
*  @param [IN]  in_count数据长度
*  @param [IN] gpio_out爆闪控制口
*  @param [IN]  out_count数据长度
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SetLedComplensating(VzLPRClientHandle handle, bool comp_enable, int ivt_time, int dealy_time, int dealy_out_time, char* gpio_in, int in_count, char* gpio_out, int out_count);


/**
*  @brief 获取图像增强配置
*  @param [IN]  handle 由VzLPRClient_Open函数获得的句柄
*  @param [OUT] mode  设备类型
*  @param [OUT] strength 额外数据长度。
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetDenoise( VzLPRClientHandle handle, int *mode, int *strength );


/**
*  @brief 设置图像增强配置
*  @param [IN]  handle 由VzLPRClient_Open函数获得的句柄
*  @param [OUT] mode  设备类型
*  @param [OUT] strength 额外数据长度。
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SetDenoise( VzLPRClientHandle handle, int mode, int strength );


/**
*  @brief 根据句柄获取设备的IP
*  @param [IN]  handle 由VzLPRClient_Open函数获得的句柄
*  @param [OUT] ip  设备ip
*  @param [IN] max_count 传入IP buffer的长度。
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetDeviceIP( VzLPRClientHandle handle, char* ip, int max_count );

/**
*  @brief 保存抓图数据到缓冲区
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] pDstBuf 接收图片的缓冲区
*  @param [IN] uSizeBuf 接收图片的缓冲区长度，要求传入的BUF大小不小于200k，否则返回失败。
*  @return 返回值为大于0表示图片的长度，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetSnapImageToBuf(VzLPRClientHandle handle, void  *pDstBuf, int  uSizeBuf);


/**
*  @brief 获取主码流分辨率；
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [OUT] sizeval
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetVideoFrameSizeIndexEx(VzLPRClientHandle handle, int *sizeval);

/**
*  @brief 设置主码流分辨率；
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] sizeval
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SetVideoFrameSizeIndexEx(VzLPRClientHandle handle, int sizeval);

/**
*  @brief 车牌自动变倍的功能
*  @param  [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @return 返回值：返回值为0表示成功，返回其他值表示失败
*/
VZ_LPRC_API int __STDCALL VzLPRClient_DoFastFocus(VzLPRClientHandle handle);

/**
*  @brief 通过该回调函数获得TCp server通道发送来的数据
*  @param  [IN] handle		由VzLPRClient_Open函数获得的句柄
*  @param  [IN] pUserData	回调函数上下文
*  @param  [IN] pData		传递的数据
*  @param  [IN] dataLen	    数据长度
*  @ingroup
*/
typedef void (__STDCALL *VZLPRC_TCPSERVER_RECV_CALLBACK)(VzLPRClientHandle handle, const char* pData,int dataLen,void *pUserData);

/**
*  @brief 设置tcpserver通道传递的数据回调
*  @param [IN]  handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] pCfgBuf配置数据
*  @param [IN] len传入数据长度
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SetTcpServerRecvCallBack(VzLPRClientHandle handle,VZLPRC_TCPSERVER_RECV_CALLBACK func, void *pUserData);


/**
*  @brief 注册tcpserver通道(才能收到TCp返回的数据)
*  @param [IN]  handle 由VzLPRClient_Open函数获得的句柄
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_RegisterTcpServerChannal( VzLPRClientHandle handle);

/**
*  @brief 向TCPServer通道发送数据
*  @param [IN]  handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] pData  向tcp发送的数据
*  @param [IN] len    发送的数据长度
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SendTcpServerData( VzLPRClientHandle handle,void* pData, int len);

/**
*  @brief 弃用
*  @brief 设置输出"不在线"的识别记录的回调函数
*  @param  [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param  [IN] func 识别结果回调函数，如果为NULL，则表示关闭该回调函数的功能
*  @param  [IN] pUserData 回调函数中的上下文
*  @return 0表示成功，-1表示失败
*  @ingroup group_device
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SetDisconnectPlatesCallBack(VzLPRClientHandle handle, VZLPRC_PLATE_INFO_CALLBACK func, void *pUserData);


/**
*  @brief 弃用
*  @brief 获取"不在线"的识别记录
*  @param  [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @return 0表示成功，-1表示失败
*  @note   在线记录的定义：  首先客户端调用VzLPRClient_SetOfflineCheck接口与相机形成绑定。如果客户端与相机之间连接正常，SDK收到识别结果后会向相机回复一个消息，相机收到消息后，会在相机的"车牌记录"数据库中将这条记录标记为"在线"。
*  @note   不在线记录的定义："车牌记录"数据库中没有把记录标记为"在线"状态的记录，都为"不在线"的记录。有以下几种情况：
1. 客户端没有调用VzLPRClient_SetOfflineCheck这个接口，所有的识别记录都不会标记为"在线"。
2. 客户端调用了VzLPRClient_SetOfflineCheck这个接口，但是客户端与相机网络出现异常时。客户端没有接收到识别结果，或者已经接收到结果，但是SDK给相机回复消息，相机没有收到。这时不会把这条记录标记为"在线"。
3. 客户端程序崩溃，或者关闭掉客户端。所有的记录都不会标记为"在线"。

*  @note   使用这个接口可能出现的情况：
1. 相机里面可以保存12万条记录，如果之前客户端没有调这个VzLPRClient_SetOfflineCheck接口或者一直是脱机运行，使用这个接口可能最多会输出12万条车牌记录。
2. 如果客户端已经收到识别结果，但是SDK给相机回复消息，相机没有收到。这条记录的标记是"不在线"，会导致车牌记录出现重复的情况，这种情况在现场网络不稳定的情况下偶尔会出现。可以在客户端的数据库保存车牌记录id,根据id来过虑重复数据。
3. 回调一次输出一条数据，最多连续输出100条数据。取完100条数据后，会再输出下100条数据。如果不在线的记录较多，输出结果会花较长时间。

*  @note   接口不会阻塞，即使"不在线"记录很多的情况下。
*  @note   调用后输出所有"不在线"的记录，请在客户端启动的时候调用。之后再产生"不在线"记录，不会再输出，需要重新调用这个接口。
*  @return 返回值为0表示成功，返回-1表示失败
*  @ingroup group_device
*/
VZ_LPRC_API int __STDCALL VzLPRClient_LoadDisconnectPlates(VzLPRClientHandle handle);

/**
*  @brief 获取R相机的编码参数；
*  @param  [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] iChannel 通道号
*  @param [IN] stream 0主码流 1子码流
*  @param [OUT] param 编码参数
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_RGet_Encode_Param(VzLPRClientHandle handle, int stream, VZ_LPRC_R_ENCODE_PARAM *param);

/**
*  @brief 设置R相机的编码参数；
*  @param  [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] iChannel 通道号
*  @param [IN] stream 0主码流 1子码流
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_RSet_Encode_Param(VzLPRClientHandle handle, int stream, VZ_LPRC_R_ENCODE_PARAM *param);

/**
*  @brief 获取R相机支持的编码参数；
*  @param  [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] iChannel 通道号
*  @param [IN] stream 0主码流 1子码流
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_RGet_Encode_Param_Property(VzLPRClientHandle handle, VZ_LPRC_R_ENCODE_PARAM_PROPERTY *param);


/**
*  @brief 获取R相机的视频参数；
*  @param  [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [OUT] param 视频参数
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_RGet_Video_Param(VzLPRClientHandle handle, VZ_LPRC_R_VIDEO_PARAM *param);

/**
*  @brief 获取R相机的视频参数；
*  @param  [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] iChannel 通道号
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_RSet_Video_Param(VzLPRClientHandle handle, VZ_LPRC_R_VIDEO_PARAM *param);


/**
*  @brief 获取X流程状态
*  @param  [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] status流程状态
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_XGet_RegistrationStatusParam(VzLPRClientHandle handle, int *status);

/**
*  @brief 发送X相机配准请求
*  @param  [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] request_status(0：开始生成配准文件,1：开始校验配准文件)
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_XSet_RegistrationParam(VzLPRClientHandle handle, int request_status);


/**
*  @brief 设置正式版校验模型
*  @param  [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] distance 3x3棋盘格到相机的真是距离，以毫米为单位
*  @param [IN] deltax 匹配误差，以像素为单位
*  @param [IN] error_ratio 距离错误比例，百分比
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_XSet_ModelCheckStatus(VzLPRClientHandle handle, int distance, float deltax, float error_ratio);

/**
*  @brief 获取正式版校验模型状态
*  @param  [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] status   s-3 : 没有找到模型（重新配准） -2 : 没有找到棋盘格（调整相机姿态）-1 : 模型参数不准确（重新配准）0:未开始 1:模型参数准确（PASS） 2: 校验中
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_XGet_ModelCheckStatus(VzLPRClientHandle handle, int* status);

/**
*  @brief 获取设备硬件信息
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [OUT] oem_info 定制信息
*  @param [IN]  oem_len  定制信息长度,长度不小于8
*  @param [OUT] company_info 公司信息
*  @param [IN]  company_len 公司信息长度，长度不小于32
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetOEMInfo(VzLPRClientHandle handle, char* oem_info, int oem_len, char* company_info, int company_len);

/**
*  @brief 设置x相机双目图像切换
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] image_mode  模式：0广角  1长焦
*  @param [IN] small_image 叠加小图 1叠加，其他值不叠加
*  @param [IN] deep_image  叠加深度图 1叠加，其他值不叠加
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_XSet_ImgModePrm(VzLPRClientHandle handle, int image_mode, int small_image, int deep_image);

/**
*  @brief 获取x相机双目图像切换
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [OUT] image_mode  模式：0广角  1长焦
*  @param [OUT] small_image 叠加小图 1叠加，其他值不叠加
*  @param [OUT] deep_image  叠加深度图 1叠加，其他值不叠加
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_XGet_ImgModePrm(VzLPRClientHandle handle, int* image_mode, int* small_image, int* deep_image);


/**
*  @brief 清空Flash里面的数据
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_CleanFlash(VzLPRClientHandle handle);


/**
*  @brief 开启透明通道(64位接口)
*  @param  [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param  [IN] nSerialPort 指定使用设备的串口序号：0表示第一个串口，1表示第二个串口
*  @param  [IN] func 接收数据的回调函数
*  @param  [IN] pUserData 接收数据回调函数的上下文
*  @return 返回透明通道句柄，0表示失败
*  @ingroup group_device
*/
VZ_LPRC_API long long __STDCALL VzLPRClient_SerialStart_V2(VzLPRClientHandle handle, int nSerialPort,
	VZDEV_SERIAL_RECV_DATA_CALLBACK func, void *pUserData);
/**
*  @brief 透明通道发送数据(64位接口)
*  @param [IN] nSerialHandle 由VzLPRClient_SerialStart函数获得的句柄
*  @param [IN] pData 将要传输的数据块的首地址
*  @param [IN] uSizeData 将要传输的数据块的字节数
*  @return 0表示成功，其他值表示失败
*  @ingroup group_device
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SerialSend_V2(long long nSerialHandle, const unsigned char *pData, unsigned uSizeData);

/**
*  @brief 透明通道停止发送数据(64位接口)
*  @param [IN] nSerialHandle 由VzLPRClient_SerialStart函数获得的句柄
*  @return 0表示成功，其他值表示失败
*  @ingroup group_device
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SerialStop_V2(long long nSerialHandle);

/**
*  @brief 根据时间和车牌号查询记录条数
*  @param  [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param  [IN] count 记录条数 大于等于1、小于等于100
*  @return 返回值为0表示失败，大于0表示记录条数
*  @ingroup group_device
*/
VZ_LPRC_API int __STDCALL VzLPRClient_QueryNearestCountRecord(VzLPRClientHandle handle, int count);

/**
*  @brief 开始喊话
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @parmm [IN] client_win_size 客户端窗口大小
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_StartTalk(VzLPRClientHandle handle, int client_win_size);

/**
*  @brief 喊话广播消息
*  @param [OUT] state 状态值
*  @param [OUT] error_msg	错误信息
*  @param [OUT] pUserData 用户自定义数据
*/
typedef void (__STDCALL *VZLPRC_REQUEST_TALK_CALLBACK)(VzLPRClientHandle handle, int state, const char* error_msg, void* pUserData);

/**
*  @brief 设置GPIO输入回调函数
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] func GPIO输入回调函数
*  @param [IN] pUserData 用户自定义数据
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SetRequestTalkCallBack(VzLPRClientHandle handle, VZLPRC_REQUEST_TALK_CALLBACK func, void* pUserData);


/**
*  @brief 停止喊话
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @parmm [IN] device_ip 设备IP
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_StopTalk(VzLPRClientHandle handle);


/**
*  @brief 开始录音
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @parmm [IN] file_path 音频文件路径
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_StartRecordAudio(VzLPRClientHandle handle, const char* file_path);


/**
*  @brief 停止录音
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_StopRecordAudio(VzLPRClientHandle handle);


/**
*  @brief 获取定焦版本相机安装距离
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [OUT] reco_dis安装距离 0:2-4米, 2: 4-6米, 1: 6-8米
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetAlgResultParam(VzLPRClientHandle handle, int* reco_dis);


/**
*  @brief 设置定焦版本相机安装距离
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] reco_dis安装距离 0:2-4米, 2: 4-6米, 1: 6-8米
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SetAlgResultParam(VzLPRClientHandle handle, int reco_dis);


/**
*  @brief LED控制
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] ctrl_param控制参数
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SetLedCtrl(VzLPRClientHandle handle, VZ_LPRC_CTRL_PARAM* ctrl_param);

/**
*  @brief 获取LED控制参数
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [OUT] ctrl_param控制参数
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetLedCtrl(VzLPRClientHandle handle, VZ_LPRC_CTRL_PARAM* ctrl_param);

/**
*  @brief 设置x相机的OSD参数
*  @param [IN] iUserID VZC_Login函数返回的用户ID
*  @param [IN] iChannel 通道号
*  @param [IN] pParam  OSD参数
*  @param [IN] pShowParam  显示的参数
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_XSet_OSDParam(VzLPRClientHandle handle, VZ_LPRC_OSD_Param *pParam, VZ_LPRC_REALTIME_SHOW_PARAM* pShowParam);

/**
*  @brief 获取x相机的OSD参数
*  @param [IN] iUserID VZC_Login函数返回的用户ID
*  @param [IN] iChannel 通道号
*  @param [IN] pParam  OSD参数
*  @param [IN] pShowParam  显示的参数
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_XGet_OSDParam(VzLPRClientHandle handle, VZ_LPRC_OSD_Param *pParam, VZ_LPRC_REALTIME_SHOW_PARAM* pShowParam);

/**
*  @brief 获取设备硬件信息
*  @param [IN] ip ip统一使用字符串的形式传入
*  @param [IN] port 使用和登录时相同的端口
*  @param [OUT] status 状态 0 没有取得硬件信息，1有基本信息，2有基本和扩展信息
*  @param [OUT] mac 输出mac地址，输入字符串缓冲长度至少为20
*  @param [OUT] serialno 输出序列号，输入字符串缓冲长度至少为20
*  @param [OUT] device_type 设备类型，参考IVS_TYPE定义
*  @param [OUT] pdata_ex 扩展硬件信息，参考VZ_FS_INFO_EX定义
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetHwInfoEx( const char *ip, WORD port, int* status, char* mac, char* serialno,int *device_type,VZ_FS_INFO_EX *pdata_ex);

/**
*  @brief 打开一个设备，支持外网访问设备
*  @param  [IN] pStrIP 设备的IP地址
*  @param  [IN] wPort 设备的端口号
*  @param  [IN] pStrUserName 访问设备所需用户名
*  @param  [IN] pStrPassword 访问设备所需密码
*  @param  [IN] wHttpPort 流媒体的端口号,默认为80(如果为0表示使用默认端口）
*  @param  [IN] network_type 网络类型(0局域网,1外网-PDNS方式)
*  @param  [IN] sn 设备序列号
*  @return 返回设备的操作句柄，当打开失败时，返回0
*  @ingroup group_device
*/
VZ_LPRC_API VzLPRClientHandle __STDCALL VzLPRClient_OpenV2(const char *pStrIP, WORD wPort, const char *pStrUserName, const char *pStrPassword,WORD wHttpPort, int network_type, char* sn);


/**
*  @brief 打开一个设备V4
*  @param  [IN] pLoginParam 登录信息参数
*  @return 返回设备的操作句柄，当打开失败时，返回0
*  @ingroup group_device
*/
VZ_LPRC_API VzLPRClientHandle __STDCALL VzLPRClient_OpenV4(VZ_LPRC_LOGIN_PARAM * pLoginParam);

/**
*  @brief 语音回环控制
*  @param [IN] iUserID VZC_Login函数返回的用户ID
*  @param [IN] control  //1：开始回环；0：结束回环
*  @param [IN] pShowParam  显示的参数
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SetRecycleControl(VzLPRClientHandle handle, int control);

/**
*  @brief 获取设备名称；
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [OUT] name 设备名
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetDeviceName(VzLPRClientHandle handle, char *name);

/**
*  @brief 设置设备名称；
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] name  设备名
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SetDeviceName(VzLPRClientHandle handle, const char *name);

/**
*  @brief 设置IO输出，并自动复位[此接口会等待相机返回是否开闸成功]
*  @param  [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param  [IN] uChnId IO输出的通道号，从0开始
*  @param  [IN] nDuration 延时时间，取值范围[500, 5000]毫秒
*  @return 0表示成功，-1表示失败
*  @ingroup group_device
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SetIOOutputAutoResp(VzLPRClientHandle handle, unsigned uChnId, int nDuration);

/**
*  @brief 设置用户自定义OSD配置(支持三行)
*  @param [IN] iUserID VZC_Login函数返回的用户ID
*  @param [IN] iChannel 通道号
*  @param [IN] pOsdParam 用户自定义osd配置
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SetUserOsdParam(VzLPRClientHandle handle, VZ_LPRC_USER_OSD_PARAM* pOsdParam);

/**
*  @brief 获取用户自定义OSD配置(支持三行)
*  @param [IN] iUserID VZC_Login函数返回的用户ID
*  @param [IN] iChannel 通道号
*  @param [OUT] pOsdParam 用户自定义osd配置
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetUserOsdParam(VzLPRClientHandle handle, VZ_LPRC_USER_OSD_PARAM* pOsdParam);

/**
*  @brief 设置用户自定义OSD配置(支持四行)
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] pOsdParam 用户自定义osd配置
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SetUserOsdParamEx(VzLPRClientHandle handle, VZ_LPRC_USER_OSD_EX_PARAM* pOsdParam);

/**
*  @brief 获取用户自定义OSD配置(支持四行)
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [OUT] pOsdParam 用户自定义osd配置
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetUserOsdParamEx(VzLPRClientHandle handle, VZ_LPRC_USER_OSD_EX_PARAM* pOsdParam);

/**
*  @brief 播放子码流实时视频
*  @param  [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param  [IN] hWnd 窗口的句柄
*  @return 播放的句柄，-1表示失败
*  @ingroup group_device
*/
VZ_LPRC_API int __STDCALL VzLPRClient_StartSubStreamRealPlay(VzLPRClientHandle handle, void *hWnd);

/**
*  @brief 设置子码流实时图像数据的回调函数
*  @param  [IN] handle		由VzLPRClient_Open函数获得的句柄
*  @param  [IN] func		实时图像数据函数
*  @param  [IN] pUserData	回调函数中的上下文
*  @return 0表示成功，-1表示失败
*  @note   DEPRECATED: 请转换为使用VzLPRClient_StartRealPlayFrameCallBack(handle, NULL, func, pUserData);
*  @ingroup group_device
*/
VZ_DEPRECATED VZ_LPRC_API int __STDCALL VzLPRClient_SetSubStreamVideoFrameCallBack(VzLPRClientHandle handle, VZLPRC_VIDEO_FRAME_CALLBACK func, void *pUserData);

/**
*  @brief 获取相机参数
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] command 命令类型
*  @param [IN] channel 通道号，默认为0
*  @param [OUT] pOutBuffer 返回的数据
*  @param [IN] dwOutBufferSize 数据的长度
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetCameraConfig(VzLPRClientHandle handle, int command, short channel, void *pOutBuffer, int dwOutBufferSize);

/**
*  @brief 设置相机参数
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] command 命令类型
*  @param [IN] channel 通道号，默认为0
*  @param [IN] pInBuffer 返回的数据
*  @param [IN] dwInBufferSize 数据的长度
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SetCameraConfig(VzLPRClientHandle handle, int command, short channel, void *pInBuffer, int dwInBufferSize);

/**
*  @brief  获取云端设备mjpeg流的url
*  @param  [IN] pStrIP 设备的IP地址
*  @param  [IN] wPort 设备的端口号
*  @param  [IN] sn 设备序列号
*  @param  [IN] mjpeg_port 视频流的端口号
*  @param  [OUT] mjpeg_url 返回的url地址
*  @param  [IN] url_len 传入url的长度
*  @param  [IN] app_id 安全验证id
*  @param  [IN] app_key 安全验证key
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetMjpegURL(const char *pStrIP, WORD wPort, char* sn, WORD mjpeg_port, char* mjpeg_url, int url_len, const char* app_id, const char* app_key);

/**
*  @brief 报警输出回调函数
*  @param [OUT] type 报警类型
*  @param [OUT] nVal	GPIO输入值
*  @param [OUT] pUserData 用户自定义数据
*/
typedef void (__STDCALL *VZLPRC_ALARM_PUSH_CALLBACK)(VzLPRClientHandle handle, int type, const char* content, void* pUserData);

/**
*  @brief 设置报警输出回调函数
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] func 报警输出回调函数
*  @param [IN] pUserData 用户自定义数据
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SetAlarmPushCallBack(VzLPRClientHandle handle, VZLPRC_ALARM_PUSH_CALLBACK func, void* pUserData);


/**
*  @brief 播放实时音频
*  @param  [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @return 播放的句柄，-1表示失败
*  @note   音频不允许单独播放，先播放视频，才允许播放音频
*  @ingroup group_device
*/
VZ_LPRC_API int __STDCALL VzLPRClient_StartPlayAudio(VzLPRClientHandle handle);

/**
*  @brief 停止播放音频
*  @param  [IN]handle 由VzLPRClient_Open函数获得的句柄
*  @return 0表示成功，-1表示失败
*  @ingroup group_device
*/
VZ_LPRC_API int __STDCALL VzLPRClient_StopPlayAudio(VzLPRClientHandle handle);


/**
*  @brief 参数导出
*  @param  [IN]handle 由VzLPRClient_Open函数获得的句柄
*  @param  [IN]level 1,不包含网络参数、用户数据、登录密码的基本参数信息  2, 包含网络参数（IP地址等） 4, 包含用户数据 8, 包含登录密码
*  @param  [IN]user_name 用户名
*  @param  [IN]user_pwd 密码
*  @return 0表示成功，-1表示失败
*  @ingroup group_device
*/
VZ_LPRC_API int __STDCALL VzLPRClient_ParaExport(VzLPRClientHandle handle, int level, const char* user_name, const char* user_pwd);


/**
*  @brief 参数导出的回调函数
*  @param [OUT] content     报警类型
*  @param [OUT] content_len	数据长度
*  @param [OUT] end	是否结束
*  @param [OUT] pUserData 用户自定义数据
*/
typedef void (__STDCALL *VZLPRC_PARA_EXPORT_CALLBACK)(VzLPRClientHandle handle, const char* content, int content_len, int end, void* pUserData);

/**
*  @brief 设置报警输出回调函数
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] func 报警输出回调函数
*  @param [IN] pUserData 用户自定义数据
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SetParaExportCallBack(VzLPRClientHandle handle, VZLPRC_PARA_EXPORT_CALLBACK func, void* pUserData);


/**
*  @brief 参数导入
*  @param  [IN]handle 由VzLPRClient_Open函数获得的句柄
*  @param  [IN]level 1,不包含网络参数、用户数据、登录密码的基本参数信息  2, 包含网络参数（IP地址等） 4, 包含用户数据 8, 包含登录密码
*  @param  [IN]user_name 用户名
*  @param  [IN]user_pwd 密码
*  @param  [IN]para_data 参数内容
*  @param  [IN]count 参数个数
*  @return 0表示成功，-1表示失败
*  @ingroup group_device
*/
VZ_LPRC_API int __STDCALL VzLPRClient_ParaImport(VzLPRClientHandle handle, int level, const char* user_name, const char* user_pwd, const char* para_data, int count);

/**
*  @brief  转换出云平台的IP和端口号
*  @param  [IN]ip   云平台的IP地址
*  @param  [IN]port 云平台的端口号
*  @param  [IN]sn   设备序列号
*  @param  [IN]dest_port 目标端口号
*  @param  [OUT]new_ip 通过云平台转换的IP地址
*  @param  [IN]new_port 通过云平台转换的端口
*  @return 0表示成功，-1表示失败
*  @ingroup group_device
*/
VZ_LPRC_API int __STDCALL VzLPRClient_ConvertPDNSNetworkParam(const char *ip, WORD port, const char* sn, WORD dest_port, char* new_ip, WORD* new_port);

/**
*  @brief  重新加载线圈、识别区域规则
*  @param  [IN]handle 由VzLPRClient_Open函数获得的句柄
*  @param  [IN]playhandle 播放句柄
*  @return 0表示成功，-1表示失败
*  @ingroup group_device
*/
VZ_LPRC_API int __STDCALL VzLPRClient_ReloadDrawRules(VzLPRClientHandle handle, int playhandle);


/**
*  @brief IO开闸结果的回调函数
*  @param [OUT]handle 由VzLPRClient_Open函数获得的句柄
*  @param [OUT] io_result GPIO输入ID
*  @param [OUT] pUserData 用户自定义数据
*/
typedef void (__STDCALL *VZLPRC_IOCTRL_RECV_CALLBACK)(VzLPRClientHandle handle, VZ_IO_RESULT* io_result, void* pUserData);

/**
*  @brief 设置IO开闸结果的回调函数
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] func GPIO输入回调函数
*  @param [IN] pUserData 用户自定义数据
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SetIoCtrlRecvCallBack(VzLPRClientHandle handle, VZLPRC_IOCTRL_RECV_CALLBACK func, void* pUserData);

/**
*  @brief 获取设备信息的json数据
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [OUT] data 输出的数据
*  @param [IN]  data_len 数据长度
*  @return 返回值大于0表示成功，返回-2表示传入的内存大小不够，-1表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetHwInfoData(VzLPRClientHandle handle, char *data, int data_len);

/**
*  @brief 根据ID获取车牌和开闸信息
*  @param  [IN]  handle 由VzLPRClient_Open函数获得的句柄
*  @param  [IN]  ids     车牌记录的ID数组
*  @param  [OUT] results 车牌记录和开闸结果的数组
*  @param  [IN]  count   车牌id的个数，范围在(1-20)之间
*  @return 返回值为0表示成功，返回-1表示失败
*  @ingroup group_device
*/
VZ_LPRC_API int __STDCALL VzLPRClient_QueryPlateIoResultByIDs(VzLPRClientHandle handle, int* ids, VZ_PLATE_IO_RESULT* results, int count);

/**
*  @brief 开始录像功能(录制私有文件格式)
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] sFileName 录像文件的路径
*  @return 返回值为0表示成功，返回-1表示失败
*  @ingroup group_device
*/
VZ_LPRC_API int __STDCALL VzLPRClient_StartPVFRecord(VzLPRClientHandle handle,char *sFileName);

/**
*  @brief 停止录像(录制私有文件格式)
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @return 返回值为0表示成功，返回-1表示失败
*  @ingroup group_device
*/
VZ_LPRC_API int __STDCALL VzLPRClient_StopPVFRecord(VzLPRClientHandle handle);

/**
*  @brief 连接wifi
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] wifi  连接信息
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_LinkWifi(VzLPRClientHandle handle, VZ_LINK_WIFI_INFO *wifi);

/**
*  @brief 上传文件
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] file_path 文件的路径(如d:\\test.wav)
*  @param [IN] type 上传文件的类型(0:语音文件 1:配置文件 2:广告资源包 10:自定义广告包文件 11:全屏广告包文件 12:半屏广告包文件)
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_UpdateLoadFile(VzLPRClientHandle handle, const char* file_path, int type);

/**
*  @brief 播放上传语音文件
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] file_name 语音文件名(如:test.wav)
*  @param [IN] volumn    播放音量大小（1-100）
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_PlayWavFile(VzLPRClientHandle handle, const char* file_name, int volumn);

/**
*  @brief 获取上传文件状态)
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @return 上传的状态
*			0: 还未上传
*			1: 上传成功
            2: 正在上传
*		   -1: 上传失败
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetUpdateLoadState(VzLPRClientHandle handle);


/**
*  @brief 路侧停车识别结果
*  @param [OUT] face_result 人脸结果
*  @param [OUT] pUserData 用户自定义数据
*/
typedef void (__STDCALL *VZLPRC_ROAD_SITE_RESULT_CALLBACK)(VzLPRClientHandle handle, TH_RoadSiteResult* face_result, void* pUserData);

/**
*  @brief 设置路侧停车识别结果回调
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] func 回调接口
*  @param [IN] pUserData 用户自定义数据
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SetRoadSiteResultCallBack(VzLPRClientHandle handle, VZLPRC_ROAD_SITE_RESULT_CALLBACK func, void* pUserData);

/**
*  @brief 获取设备硬件信息
*  @param [IN] ip ip统一使用字符串的形式传入
*  @param [IN] port 使用和登录时相同的端口
*  @param [OUT] board_version 设备硬件型号
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetBoardVersionInfo( const char *ip, WORD port, int* board_version);

/**
*  @brief 设置抓拍参数
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] image_quality图片质量(10 ,20, 30 40, 50, 60, 70, 80, 90, 100)
*  @param [IN] img_size图像大小(2:352*288 3: 640*360 5:704*576 9:1280*720 10:1920*1080)
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SetOutputImageSize(VzLPRClientHandle handle, int image_quality, int img_size);

/**
*  @brief 透传http请求
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] url 传入的url地址
*  @param [IN] cmd 传入的命令参数
*  @param [OUT] response 返回的
*  @param [IN] cmd 传入的命令参数
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_HttpTransSend(VzLPRClientHandle handle, const char* url, const char* cmd, char* response, int max_response_len, int type);

/**
*  @brief 全智能相机录像文件列表查询
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] sub_type 查询子类型 1 人脸抓拍 2 人脸识别 0 全部
*  @param [IN] start_time 开始时间，时间格式 YYYY-MM-DD HH24:mm:SS
*  @param [IN] end_time   结束时间，时间格式 YYYY-MM-DD HH24:mm:SS
*  @param [IN] page_num 页码
*  @param [OUT] record_video 文件列表
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzFicClient_QueryRecordVideo(VzLPRClientHandle handle, int sub_type, const char* start_time, const char* end_time, int page_num, VZ_FIC_RECORD_VIDEO_INFO* record_video);


/**
*  @brief 修改登录密码
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] password 登录密码
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SetLoginAccount(VzLPRClientHandle handle, const char* password);

/**
*  @brief 设置播放窗口的大小
*  @param  [IN] nPlayHandle 播放的句柄
*  @param  [IN] width  窗口宽度
*  @param  [IN] height 窗口高度
*  @return 0表示成功，-1表示失败
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SetPlayWndSize(int nPlayHandle, int width, int height);


/**
*  @brief 根据时间查询SD卡中的录像文件
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param  [IN] start_time   开始时间
*  @param  [IN] end_time     结束时间
*  @param  [IN] page_index   当前页索引
*  @param  [IN] record_video 录像文件结果
*  @return 0表示成功，-1表示失败
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SearchRecordFile(VzLPRClientHandle handle, const char* start_time, const char* end_time, int page_index, VZ_FIC_RECORD_VIDEO_INFO* record_video);

/**
*  @brief 按时间段下载录像文件，通过回调返回视频数据
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] pStartTime 播放起始时间 (2018-12-12 10:01:02)
*  @param [IN] pEndTime 播放结束时间 (2018-12-12 10:59:59)
*  @return 播放句柄: -1表示失败,其他值代表成功
*/
VZ_LPRC_API int __STDCALL VzLPRClient_DownLoadFileByTime(VzLPRClientHandle handle, const char* pStartTime, const char* pEndTime, VZLPRC_VIDEO_DATA_CALLBACK func, void *pUserData);

/**
*  @brief 获取录像下载进度
*  @param [IN] playhandle 由VzLPRClient_DownLoadFileByTime函数获得的播放句柄
*  @return 返回值为当前下载进度百分比 [0, 100];
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetDownLoadProgress(int playhandle);


/**
*  @brief 按时间段下载录像文件, 保存为mp4格式的文件(一台设备一次只能下载一个文件)
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] pStartTime 播放起始时间 (2018-12-12 10:01:02)
*  @param [IN] pEndTime 播放结束时间 (2018-12-12 10:02:59)
*  @param [IN] file_path 文件路径
*  @return 播放句柄: -1表示失败,其他值代表成功
*/
VZ_LPRC_API int __STDCALL VzLPRClient_DownLoadMP4FileByTime(VzLPRClientHandle handle, const char* pStartTime, const char* pEndTime, const char* file_path);


/**
*  @brief 初始化云端参数，用于连接云平台音视频服务器，播放音视频(全局接口，程序启动后调用，只调用一次)
*  @param  [IN] pdns_addr 服务器地址
*  @param  [IN] username  用户名
*  @param  [IN] accesskey_id
*  @param  [IN] accesskey_secret
*  @param  [IN] res       预留参数
*  @return 0表示成功，-1表示失败
*  @ingroup group_global
*/
VZ_LPRC_API int __STDCALL VzClient_CloudSetup(const char *pdns_addr, const char *username, const char *accesskey_id, const char *accesskey_secret, const char *res);

/**
*  @brief 卸载云平台
*  @return 0表示成功，-1表示失败
*  @ingroup group_global
*/
VZ_LPRC_API int __STDCALL VzClient_CloudCleanup();

/**
*  @brief 播放实时视频(通过云平台方式播放视频)
*  @param  [IN] handle 由VzClient_Open函数获得的句柄
*  @param  [IN] hWnd 窗口的句柄
*  @return 播放的句柄，-1表示失败
*  @ingroup group_device
*/
VZ_LPRC_API long __STDCALL VzClient_CloudStartRealPlay(VzLPRClientHandle handle, void *hWnd);

/**
*  @brief 停止播放实时视频(通过云平台方式播放视频)
*  @param  [IN] handle 由VzClient_Open函数获得的句柄
*  @param  [IN] hWnd 窗口的句柄
*  @return 播放的句柄，-1表示失败
*  @ingroup group_device
*/
VZ_LPRC_API long __STDCALL VzClient_CloudStopRealPlay(long nPlayHandle);

/**
*  @brief 开启语音对讲
*  @param  [IN] handle 由VzClient_Open函数获得的句柄
*  @param  [IN] hWnd 窗口的句柄
*  @return 语音对讲的句柄，-1表示失败
*  @ingroup group_device
*/
VZ_LPRC_API long __STDCALL VzClient_CloudStreamTalk(VzLPRClientHandle handle);

/**
*  @brief 发送语音对讲数据
*  @param  [IN] handle 由VzClient_Open函数获得的句柄
*  @param  [IN] hWnd 窗口的句柄
*  @return 语音对讲的句柄，-1表示失败
*  @ingroup group_device
*/
VZ_LPRC_API int __STDCALL VzClient_CloudSendTalkData(long talk_handle, char *buf, int len, unsigned int stamp);

/**
*  @brief 停止语音对讲
*  @param  [IN] handle 由VzClient_Open函数获得的句柄
*  @param  [IN] hWnd 窗口的句柄
*  @return 语音对讲的句柄，-1表示失败
*  @ingroup group_device
*/
VZ_LPRC_API long __STDCALL VzClient_CloudStopTalk(long talk_handle);

/**
*  @brief 云台控制
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] dir 方向，2:向上移动 4:向下移动 8:停止上下移动 16:向左移动 32:向右移动 64:停止左右移动 128:还原
*  @param [IN] degree 角度[0~65535]
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzClient_SetPtzCtrl(VzLPRClientHandle handle, int dir, int degree);

/**
*  @brief 透传tcp请求
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] cmd 传入的命令参数
*  @param [OUT] response 返回的
*  @param [IN] max_response_len 返回值的长度
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_TcpTransSend(VzLPRClientHandle handle, const char* cmd_type, const char* cmd, char* response, int max_response_len);

/**
*  @brief 识别结果回调
*  @param [OUT] type结果类型(0:RG抓拍结果)
*  @param [OUT] pResultInfo结果信息 JSON格式
*  @param [OUT] len结果信息长度
*  @param [OUT] imgs图片信息
*  @param [OUT] count图片个数
*  @param [OUT] pUserData 用户自定义数据
*  @note pResultInfo为json字符串,具体内容说明参考协议文档
*/
typedef void (__STDCALL *VZ_COMMON_RESULT_CALLBACK)(VzLPRClientHandle handle, int type, const char *pResultInfo, int len, VZ_IMAGE_INFO *imgs, int count, void *pUserData);

/**
*  @brief 设置通用识别结果回调
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] type 结果类型(0:车道监控结果)
*  @param [IN] func 回调接口
*  @param [IN] pUserData 用户自定义数据
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzClient_SetCommonResultCallBack(VzLPRClientHandle handle, int type, VZ_COMMON_RESULT_CALLBACK func, void* pUserData);

/**
*  @brief 查询云存储的录像文件结果列表
*  @param [IN]  handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN]  query_info 查询条件
*  @param [OUT] record_list 返回的查询结果
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzClient_QueryCloudRecordList(VzLPRClientHandle handle, VzStream_RecordQueryInfo* query_info, VzStream_RecordList* record_list);

/**
*  @brief 云存录像点播，获取hls文件url
*  @param [IN]  handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN]  file_name 录像文件名
*  @param [OUT] out_code 输出值，200表ok
*  @param [OUT] out_buffer 输出值，hls文件url
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzClient_StreamCloudVod(VzLPRClientHandle handle, char* file_name, int* out_code, char* out_buffer, int out_len);

/**
*  @brief 云存hls文件播放
*  @param [IN]  url 云存录像文件url
*  @param [IN]  seek_time 相对于录像开始时间的偏移，单位毫秒
*  @param [OUT] hWnd 窗口句柄，画面渲染区域。如果设置了数据回调，该值填NULL
*  @param [OUT] out_buffer 输出值，hls文件url
*  @return 返回0失败，非0值表句柄
*  @note 停止播放，调用VzClient_CloudStopRealPlay接口
*/
VZ_LPRC_API long __STDCALL VzClient_StreamOpenHLS(char* url, int seek_time, void* hWnd);


/**
*  @brief 通过云平台登录方式打开一个设备
*  @param  [IN] pStrIP 设备的IP地址
*  @param  [IN] wPort 设备的端口号
*  @param  [IN] pStrUserName 访问设备所需用户名
*  @param  [IN] pStrPassword 访问设备所需密码
*  @param  [IN] wHttpPort http的端口号,默认为80(如果为0表示使用默认端口）
*  @param  [IN] network_type 网络类型(1外网-PDNS方式)
*  @param  [IN] sn 设备序列号
*  @param  [IN] app_id 云平台登录账号
*  @param  [IN] app_key 云平台登录密码
*  @param  [IN] pStrExtData 预留数据
*  @return 返回设备的操作句柄，当打开失败时，返回0
*  @ingroup group_device
*/
VZ_LPRC_API VzLPRClientHandle __STDCALL VzLPRClient_CloudOpen(const char *pStrIP, WORD wPort, const char *pStrUserName, const char *pStrPassword, WORD wHttpPort, int network_type, char* sn, const char* appid, const char *appkey, const char *pStrExtData);


/**
*  @brief RG获取指定伴侣机的配置信息
*  @param [IN]  handle 由VzLPRClient_Open函数获得的句柄
*  @param [OUT] VZ_LPRC_MATE_INFO  RG伴侣机配置信息
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetMateInfo(VzLPRClientHandle handle, VZ_LPRC_MATE_INFO* mateinfo );


/**
*  @brief RG获取伴侣机的Rtsp代理信息
*  @param [IN]  handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN]  sn 指定Rtsp的附属设备sn
*  @param [OUT] VZ_LPRC_RTSP_INFO  RG伴侣机的Rtsp代理信息
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetRtspInfo(VzLPRClientHandle handle,const char *sn,VZ_LPRC_RTSP_INFO * rtspinfo);

/**
*  @brief RG获取伴侣机的语音代理信息
*  @param [IN]  handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN]  sn 指定语音代理的附属设备sn
*  @param [IN]  window_size 服务器窗口大小，每次发送数据包的大
*  @param [OUT] VZ_LPRC_REQUEST_TALK_INFO  RG伴侣机的语音代理信息
*  @return 返回值为0表示成功，返回其他值表示失败。
*/

VZ_LPRC_API int __STDCALL VzLPRClient_GetRequestTalkInfo(VzLPRClientHandle handle,const char *sn,const int window_size,VZ_LPRC_REQUEST_TALK_INFO * talkinfo);

/**
*  @brief 设置Rg人车滞留事件上报使能
*  @param [IN]  handle 由VzLPRClient_Open函数获得的句柄
*  @param [IN] enable  使能开关，0：关闭，1打开
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SetStayEventEnable(VzLPRClientHandle handle,int enable);

/**
*  @brief 设置gpio口锁定状态
*  @param [IN]  handle 由VzLPRClient_Open函数获得的句柄
*  @param  [IN] ioout  ioout输出口 0或者1
*  @param  [IN] status 0 解锁 1高电平锁定 2 低电平锁定
*  @return 0表示成功，-1表示失败
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SetIoLockStatus(VzLPRClientHandle handle, int ioout, int status);


/**
*  @brief 获取gpio口锁定状态
*  @param [IN]  handle 由VzLPRClient_Open函数获得的句柄
*  @param  [IN] ioout  ioout输出口 0或者1
*  @param  [OUT] status 0 解锁 1高电平锁定 2 低电平锁定
*  @return 0表示成功，-1表示失败
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetIoLockStatus(VzLPRClientHandle handle, int ioout, int *status);


/**
*  @brief 通过流媒体地址播放视频流(rtsp地址url)
*  @param  [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @param  [IN] stream_url(rtsp地址)
*  @param  [IN] hWnd 窗口的句柄(可以传null)
*  @param  [IN] pUserData	回调函数中的上下文
*  @return 播放的句柄，-1表示失败
*  @ingroup group_device
*/
VZ_LPRC_API int __STDCALL VzLPRClient_StartPlayUrl(VzLPRClientHandle handle, const char* stream_url, void *hwnd);


/**
*  @brief 开始对讲
*  @param [IN] handle 由VzLPRClient_Open函数获得的句柄
*  @parmm [IN] client_win_size 客户端窗口大小
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_StartTalkProxy(VzLPRClientHandle handle, int server_win_size, int encode_type, int port, int sample_rate, int sample_point);

/**
*  @brief 获取LCD屏显广告配置内容
*  @param [IN]  ad_type(0自定义/1全屏/2半屏广告;-1代表不指定广告类型,获取当前正在使用的广告配置)
*  @param [OUT] config广告配置内容
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_GetLcdAdConfig(VzLPRClientHandle handle, int ad_type, VZ_AD_CONFIG *config);

/**
*  @brief 修改当前LCD屏显广告场景中一个元素的配置内容
*  @param [IN] config获取到的广告配置
*  @param [IN] scene_name场景名称
*  @param [IN] elem_id元素id
*  @param [IN] key元素id
*  @param [IN] value元素id
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SetAdSceneElemItem(VzLPRClientHandle handle, VZ_AD_CONFIG *config, const char* scene_name, int elem_id, const char* key, const char* value);

/**
*  @brief 修改当前LCD屏显广告场景的配置内容
*  @param [IN] config获取到的广告配置
*  @param [IN] scene_name场景名称
*  @param [IN] elem_id元素id
*  @param [IN] key元素id
*  @param [IN] value元素id
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SetAdSceneItem(VzLPRClientHandle handle, VZ_AD_CONFIG *config, const char* scene_name, const char* key, const char* value);

/**
*  @brief 设置LCD屏显广告配置内容
*  @param [IN] config广告配置内容
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzLPRClient_SetLcdAdConfig(VzLPRClientHandle handle, VZ_AD_CONFIG *config);

/**
*  @brief 连接mqtt服务器
*  @param  [IN] url 云服务器url
*  @param  [IN] wPort 设备的端口号
*  @param  [IN] app_id 云平台登录账号
*  @param  [IN] app_key 云平台登录密码
*  @param  [IN] pStrExtData 预留数据
*  @return 返回值为0表示成功，返回其他值表示失败。
*  @ingroup group_device
*/
VZ_LPRC_API int __STDCALL VzClient_MqttConnect(const char *url, WORD wPort, const char* appid, const char *appkey, const char *pStrExtData);


/**
*  @brief 上下线消息
*  @param [IN] userdata 用户数据
*  @parmm [IN] cause 上下线原因
*  @parmm [IN] type  0下线, 1上线
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
typedef void (__STDCALL *MQTTClientConnectLost)(void* userdata, char* cause, int type);

/**
*  @brief 订阅mqtt消息通知
*  @param [IN] userdata 用户数据
*  @parmm [IN] topicName
*  @parmm [IN] payload    消息内容
*  @parmm [IN] payloadlen 消息长度
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
typedef void (__STDCALL *MQTTClientMessageArrived)(void* userdata, char* topicName, int topicLen, void* payload, int payloadlen);

/**
*  @brief 设置mqtt回调
*  @param [IN] sn 设备序列号
*  @parmm [IN] topic 消息主题
*  @return 无
*  @note 不能在回调中执行耗时时间长的业务,否则会卡住回调,影响后续数据输出
*/
VZ_LPRC_API void __STDCALL VzClient_MqttSetCallbacks(MQTTClientConnectLost cl, MQTTClientMessageArrived ma, void *userdata);

/**
*  @brief 订阅mqtt消息
*  @param [IN] sn 设备序列号
*  @parmm [IN] topic 消息主题
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzClient_MqttSubscribe(const char* topic);

/**
*  @brief 发布mqtt消息
*  @parmm [IN] topicName
*  @parmm [IN] payload    消息内容
*  @parmm [IN] payloadlen 消息长度
*  @return 返回值为0表示成功，返回其他值表示失败。
*/
VZ_LPRC_API int __STDCALL VzClient_MqttPublishMessage(const char* topicName, const char* message);


/**
*  @brief 断开与mqtt服务器的连接
*  @return 无
*  @ingroup group_device
*/
VZ_LPRC_API void __STDCALL Vzlient_MqttDisConnect();


VZ_LPRC_API int __STDCALL VzClient_RequireMqttInfo(const char *url, WORD wPort, const char* appid, const char *appkey, char *mqtt_ip, int *mqtt_port,
												 char *client_id, char *username,  char *passwd);

#ifdef  LINUX
#ifdef __cplusplus
}
#endif
#endif

#endif
