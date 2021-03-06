#ifndef _TXLiveCommon_h_
#define _TXLiveCommon_h_

#ifdef LITEAV_EXPORTS
#define LITEAV_API __declspec(dllexport)
#else
#define LITEAV_API __declspec(dllimport)
#endif

class LITEAV_API TXLiveCommon
{
public:
    static TXLiveCommon * getInstance();
    
    /**
    * \brief：设置透明代理，SDK通过此透明代理server转发到腾讯云
    * \param：ip  - 透明代理IP
    * \return 无
    */
    void setProxy(const char * ip, unsigned short port);
};

#endif