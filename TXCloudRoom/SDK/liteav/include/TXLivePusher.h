#ifndef _TXLivePusher_h_
#define _TXLivePusher_h_

#ifdef LITEAV_EXPORTS
#define LITEAV_API __declspec(dllexport)
#else
#define LITEAV_API __declspec(dllimport)
#endif

#include "TXLiveSDKTypeDef.h"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <memory>

#define MIC_DEVICE_NAME_MAX_SIZE    (512)

class TXLivePusherCallback;
class TXLivePusherImpl;

class LITEAV_API TXLivePusher
{
public:
	TXLivePusher();
	virtual ~TXLivePusher();

	/**
	* \brief：设置回调 TXLivePusher 的回调代理，监听推流事件
	* \param：callback  - TXLivePusherCallback 类型的代理指针
	* \param：pUserData - 一般情况下传 NULL 就可以了，透传用户数据到 TXLivePusherCallback 的回调函数
	* \return 无
	*/
	void setCallback(TXLivePusherCallback* callback, void* pUserData);

	/**
	* \brief：枚举当前的摄像头，如果一台Windows同时安装了多个摄像头，那么此函数获取可用的摄像头数量和名称
	* \param：camerasName - 每个摄像头的名字
	* \param: capacity - camerasName 数组的大小
	* \return：当前Windows可用的 摄像头的数量
	* \attention: 该函数可以分两次调用，第一次调用设置 camerasName 为 NULL，可以获得摄像头数量，第二次调用时就可以创建一个大小刚刚合适的 camerasName
	*/
	static int enumCameras(wchar_t **camerasName = NULL, size_t capacity = 0);

    /**
    * \brief：查询可用的麦克风设备的数量
    * \return：若查询成功，则返回值>=0；若查询失败，则返回值为-1
    */
    int micDeviceCount() const;

    /**
    * \brief：查询麦克风设备的名称
    * \param：index - 麦克风设备的索引，要求index值小于 micDeviceCount 接口的返回值
    * \param：name - 用于存放麦克风设备的名称的字符串数组，数组大小要求 <= MIC_DEVICE_NAME_MAX_SIZE，查询得到的字符编码格式是UTF-8
    * \return：若查询成功，则返回值true；若查询失败或者index非法，则返回值为false
    */
    bool micDeviceName(unsigned int index, char name[MIC_DEVICE_NAME_MAX_SIZE]);

    /**
    * \brief：选择指定的麦克风作为录音设备，不调用该接口时，默认选择索引为0的麦克风
    * \param：index - 麦克风设备的索引，要求index值小于 micDeviceCount 接口的返回值
    */
    void selectMicDevice(unsigned int index);

    /**
    * \brief：查询已选择麦克风的音量
    * \return：音量值，范围是[0, 65535]
    */
    unsigned int micVolume();

    /**
    * \brief：设置已选择麦克风的音量
    * \param：volume - 设置的音量大小，范围是[0, 65535]
    */
    void setMicVolume(unsigned int volume);

	/**
	* \brief：关闭麦克风
	* \param：enable - false为关闭麦克风采集，true为启用麦克风采集
	*/
	void enableMic(bool enable);

#ifndef _WIN64

	/**
	@brief 打开系统声音采集。
	@details 采集系统声音。
	@param [in] szPlayerPath 播放器地址;如果用户此参数填空或不填，表示采集系统中的所有声音;
			    如果填入exe程序(如:酷狗、QQ音乐)所在路径,将会启动此程序，并只采集此程序的声音;
	*/
	void openSystemVoiceInput(const char* szPlayerPath = nullptr);

	/**
	@brief 关闭系统声音采集。
	*/
	void closeSystemVoiceInput();

	/**
	@brief 设置系统声音采集的音量。
	@param [in] value 设置目标音量,取值范围[0,100].
	*/
	void setSystemVoiceInputVolume(int value);

#endif

    /**
    * \brief：启动音频采集，SDK内部采用48K采样率，单声道，16位宽，实现很低延迟的实时音频通话的效果
    * \param：srcType - 音频数据源类型
    */
    void startAudioCapture(TXEAudioCaptureSrcType srcType = TXE_AUDIO_SRC_SDK_DATA);

    /**
    * \brief：关闭音频采集
    */
    void stopAudioCapture();

	/**
	* \brief：启动摄像头预览
	* \param：rendHwnd - 承载预览画面的 HWND，目前 SDK 是采用 OpenGL 向 HWND 上绘制图像的
	* \param：rect     - 指定视频图像在 HWND 上的渲染区域，比如 (0，0，width, height) 表示拿整个 HWND 作为渲染画布
	* \param：cameraIndex - 指定要开启哪个摄像头的预览，如果只有一个摄像头，则设置为 0 ，否则，需要先用 enumCameras 获取 cameraIndex
	* \
	* \       cameraIndex ==  0   表示打开当前可用的第一个摄像头
	* \       cameraIndex ==  1   表示打开当前可用的第二个摄像头
	* \       ... ...
	* \       cameraIndex == -1   表示打开当前可用的全部摄像头，但是预览的是第一个，通过 switchCamera 来切换摄像头
	* \
	* \       Windows 下开启一个 USB 摄像头需要很长的电路和驱动启动时间（一般需要 1-3 秒），所以，如果您要实现较好的切换效果
	* \       推荐的做法是：startPreview 时指定 cameraIndex 为 -1 来打开全部摄像头 ，再用 switchCamera 实现瞬间切换摄像头
	*
	* \return: 成功 or 失败，摄像头被占用可能会导致 camera 打开失败
	* \      旧兼容接口，内部被转换调用：bool startPreview(TXEVideoCaptureSrcType srcType, HWND rendHwnd, const RECT &rect);
	*/
	bool startPreview(HWND rendHwnd, const RECT &rect, int cameraIndex);

    /**
    * \brief：启动视频源预览
    * \param：srcType  - 参考 TXLiveSDKTypeDef.h 中定义的 TXEVideoCaptureSrcType 枚举值
    * \param：rendHwnd - 承载预览画面的 HWND，目前 SDK 是采用 OpenGL 向 HWND 上绘制图像的
    * \param：rect     - 指定视频图像在 HWND 上的渲染区域
    * \param：dataFormat - srcType = TXE_VIDEO_SRC_USER_DATA时生效，指定传入的参数
    * \return: 成功 or 失败
    */
    bool startPreview(TXEVideoCaptureSrcType srcType, HWND rendHwnd, const RECT &rect, TXEVideoUserDataFormat dataFormat = TXE_USER_DATA_FORMAT_UNDEFINED);

	/**
	* \brief：重设摄像头预览区域，当您指定的 HWND 的窗口尺寸发生变化时，可以通过这个函数重新调整视频渲染区域
	* \param：rendHwnd - 承载预览画面的 HWND
	* \param：rect     - 指定视频图像在 HWND 上的渲染区域
	* \return 无
	*/
	void updatePreview(HWND rendHwnd, const RECT &rect);

	/**
	* \brief：关闭视频源预览
	* \return 无
	*/
	void stopPreview();

    /**
    * \brief：枚举当前的可以捕抓的窗口，如果桌面同时存在多个窗口，那么此函数获取可采集的窗口句柄和名称
    * \param：windowArray - 每个可采集的窗口的句柄和名字
    * \param: capacity - windowArray 数组的大小
    * \return：当前桌面可采集的窗口数量
    * \attention: 该函数可以分两次调用，第一次调用设置 windowArray 为 NULL，可以获得窗口数量，第二次调用时就可以创建一个大小刚刚合适的 windowArray
    */
    static int enumCaptureWindow(HWND* windowArray = NULL, size_t capacity = 0);

	/**
	* \brief：屏幕区域捕抓参数设置接口，startPreview(srcType = TXE_VIDEO_SRC_SDK_SCREEN..)前调用，默认捕抓整个主屏幕
	* \param：captureHwnd - 指定捕抓窗口，如果captureHwnd不为NULL时，捕抓整个captureHwnd窗口大小,此时captureRect设置会失效。
	* \                     如果captureHwnd为NULL时，则captureRect有效，捕抓主屏幕指定的区域：captureRect
	* \param：captureRect - 指定捕抓主屏幕区域RC，captureHwnd为NULL时生效，captureRect为{0}时，则捕抓整个主屏幕。
	* \param：bFollowWndRect - 指定跟随窗口抓屏，在某些特殊窗口（如opengl渲染的窗口）是无法直接获取窗口内容的，需要跟随窗口坐标来截取屏幕。
	*/
	void setScreenCaptureParam(HWND captureHwnd, const RECT &rect, bool bFollowWndRect = false);

	/**
	* \brief：推流的图像截图到本地
	* \param：filePath - 存储路径
	* \param: length - 路径字符长度
	* \return：0成功，非0失败：-1:失败，-2路径非法，-3文件存在，-4未推流
	*/
	int captureVideoSnapShot(const wchar_t * filePath, unsigned int length);

	/**
	* \brief：启动推流 (在 startPush 之前需要先 startPreview 启动摄像头预览，否则推送出去的数据流里只有音频)
	* \param：url - 一个合法的推流地址，腾讯云的推流 URL 都要求带有 txSecret 和 txTime 防盗链签名，如果您发现推流推不上去，请检查这两个签名是否合法。
	* \return：成功 or 失败，内存分配、资源申请失败等原因可能会导致返回失败
	*/
	bool startPush(const char * url);

	/**
	* \brief：停止推流，注意推流 url 有排他性，也就是一个推流 Url 同时只能有一个推流端向上推流
	* \param：无
	* \return:无
    * \attention: 若通过 startPreview 接口开启了预览，在调用这个接口前，请先调用 stopPreview 接口
	*/
	void stopPush();

	/**
	* \brief：切换摄像头，支持在推流中动态切换
	* \param：cameraIndex : 摄像头需要，取值返回：  0 ~ (摄像头个数 - 1)
	* \return:无
	*/
	void switchCamera(int cameraIndex);

	/**
	* \brief：静音接口
	* \param：mute - 是否静音
	* \return:无
	*/
	void setMute(bool mute);

    /**
    * \brief：设置图像的渲染（填充）模式
    * \param：mode - 参考 TXLiveSDKTypeDef.h 中定义的 TXERenderMode 枚举值
    * \return:无
    */
    void setRenderMode(TXERenderMode mode);

    /**
    * \brief：设置图像的顺时针旋转角度
    * \param：rotation - 参考 TXLiveSDKTypeDef.h 中定义的 TXEVideoRotation 枚举值
    * \return:无
    */
    void setRotation(TXEVideoRotation rotation);

	/**
	* \brief：推流的画面质量预设选项
	* \param：paramType  - 预设类型，参考 TXLiveSDKTypeDef.h 中定义的 TXEVideoQualityParamPreset 枚举值
	* \param：ratio  - 预设画面比例，参考 TXLiveSDKTypeDef.h 中定义的 TXEVideoResolution 枚举值
	* \return:无
	* \startPush前生效。
	*/
	void setVideoQualityParamPreset(TXEVideoQualityParamPreset paramType, TXEVideoRatio ratio = TXE_VIDEO_RATIO_4_3);

	/**
	* \brief：设置视频分辨率
    * \param：resolution  - 视频分辨率，参考 TXLiveSDKTypeDef.h 中定义的 TXEVideoResolution 枚举值
	* \return:无
	*/
    void setVideoResolution(TXEVideoResolution resolution);

	/**
	* \brief：设置美颜和美白效果
	* \param：beautyStyle    - 参考 TXLiveSDKTypeDef.h 中定义的 TXEBeautyStyle 枚举值
	* \param：beautyLevel    - 美颜级别取值范围 1 ~ 9； 0 表示关闭，1 ~ 9值越大，效果越明显
	* \param：whitenessLevel - 美白级别取值范围 1 ~ 9； 0 表示关闭，1 ~ 9值越大，效果越明显
	* \return:无
	*/
	void setBeautyStyle(TXEBeautyStyle beautyStyle, int beautyLevel, int whitenessLevel);

    /**
    * \brief：设置预览渲染的镜像效果
    * \param：mirror - true表示画面左右反转，false表示保持原样
    * \return:无
    */
    void setRenderYMirror(bool mirror);

    /**
    * \brief：设置推流画面的镜像效果
    * \param：mirror - true表示画面左右反转，false表示保持原样
    * \return:无
    */
    void setOutputYMirror(bool mirror);

	/**
	* \brief：设置视频码率，注意，不是分辨率越高画面越清晰，是码率越高画面越清晰
	* \param：bitrate - 视频码率，单位 kbps， 比如 640x360 分辨率需要配合 800kbps 的视频码率
	* \return:无
	*/
	void setVideoBitRate(unsigned long bitrate);

	/**
	* \brief：设置流控策略，即是否允许 SDK 根据当前网络情况调整视频码率，以避免网络上传速度不足导致的画面卡顿
	* \param：strategy - 参考 TXLiveSDKTypeDef.h 中定义的 TXEAutoAdjustStrategy 枚举值
	* \return:无
	*/
	void setAutoAdjustStrategy(TXEAutoAdjustStrategy strategy);

	/**
	* \brief：配合 setAutoAdjustStrategy 使用，当 AutoAdjust 策略指定为 TXE_AUTO_ADJUST_NONE 时，如下的两个函数调用均视为无效
	* \param：videoBitrateMin - 允许 SDK 输出的最小视频码率，比如 640x360 分辨率下这个值适合设置为 300kbps
	* \param：videoBitrateMax - 允许 SDK 输出的最大视频码率, 比如 640x360 分辨率下这个值适合设置为 1000kbps
	* \return:无
	*/
	void setVideoBitRateMin(int videoBitrateMin);
	void setVideoBitRateMax(int videoBitrateMax);

	/**
	* \brief：设置视频帧率
	* \param：fps - 视频帧率，默认值为15，重启后生效
	* \return:无
	*/
	void setVideoFPS(unsigned long fps);

	/**
	* \brief：设置视频暂停
	*/
	void setPauseVideo(bool pause);

	/*视频本地录制*/
	/*
	* 支持不推流录制。
	* szRecordPath:视频录制后存储路径，目前只能传 .mp4 后缀文件。
	* sliceTime:   mp4文件分片时间，单位s，默认10分钟。
	* 返回值：
	*          0 成功；
	*         -1 videoPath 为nil；
	*         -2 上次录制未结束，请先stopRecord
	*/
	int startLocalRecord(const char* szRecordPath, uint64_t mp4SliceTime = 10*60);

	/*
	* 结束录制短视频，停止推流后，如果视频还在录制中，SDK内部会自动结束录制
	* 返回值：
	*       0 成功；
	*      -1 不存在录制任务；
	*/
	int stopLocalRecord();


protected:
	TXLivePusherImpl* m_impl;
};

#endif
