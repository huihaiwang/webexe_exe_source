#include "DialogPushPlay.h"
#include <QMouseEvent>
#include "json.h"
#include "DialogMessage.h"
#include "log.h"
#include "Application.h"
#include "TXLiveCommon.h"

DialogPushPlay::DialogPushPlay(QWidget *parent)
	: QDialog(parent)
	, m_cameraCount(0)
	, m_pushBegin(false)
	, m_bUserIsResizing(false)
{
	ui.setupUi(this);
	setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog | Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint);
	setWindowIcon(QIcon(":/PushPlay/live.ico")); 

	show();
	m_player.setCallback(this, reinterpret_cast<void*>(1));
	m_pusher.setCallback(this, reinterpret_cast<void*>(2));

	qRegisterMetaType<txfunction>("txfunction");
	qApp->installEventFilter(this);
	connect(this, SIGNAL(update_event(int)), this, SLOT(on_update_event(int)));
	connect(this, SIGNAL(dispatch(txfunction)), this, SLOT(handle(txfunction)), Qt::QueuedConnection);
}

DialogPushPlay::~DialogPushPlay()
{
}

void DialogPushPlay::onEventCallback(int eventId, const int paramCount, const char ** paramKeys, const char ** paramValues, void * pUserData)
{
	const int index = reinterpret_cast<int>(pUserData);
	if (index == 2 && eventId != TXE_STATUS_UPLOAD_EVENT)
	{
		std::stringstream eventLog;
		eventLog << "receive sdk event: target: pusher, event:" << eventId;
		for (int i = 0; i < paramCount; ++i)
		{
			eventLog << " " << paramKeys[i] << "=" << paramValues[i];
		}
		LINFO(L"%s", QString::fromStdString(eventLog.str().c_str()).toStdWString().c_str());
	}
	else if (index == 1 && eventId != TXE_STATUS_DOWNLOAD_EVENT)
	{
		std::stringstream eventLog;
		eventLog << "receive sdk event: target: player, event:" << eventId;
		for (int i = 0; i < paramCount; ++i)
		{
			eventLog << " " << paramKeys[i] << "=" << paramValues[i];
		}
		LINFO(L"%s", QString::fromStdString(eventLog.str().c_str()).toStdWString().c_str());
	}

	switch (eventId)
	{
	case PlayEvt::PLAY_ERR_NET_DISCONNECT: ///< 三次重连失败，断开。
		emit update_event(1);
		break;
	case PlayEvt::PLAY_EVT_PLAY_BEGIN: ///< 开始播放
		emit update_event(4);
		break;
	case PushEvt::PUSH_ERR_NET_DISCONNECT:
		emit update_event(2);
		break;
	case PushEvt::PUSH_EVT_PUSH_BEGIN:
		emit update_event(5);
		break;
	case PushEvt::PUSH_ERR_CAMERA_OCCUPY:
		emit update_event(3);
		break;
	default:
		break;
	}

    std::map<std::string, std::string> params;
    for (int i = 0; i < paramCount; ++i)
    {
        params.insert(std::pair<std::string, std::string>(paramKeys[i], paramValues[i]));
    }

    Application::instance().pushSDKEvent(eventId, params);
}

bool DialogPushPlay::eventFilter(QObject* pObj, QEvent* pEvent)
{
	if ((pEvent->type() == QEvent::MouseButtonRelease) || (pEvent->type() == QEvent::NonClientAreaMouseButtonRelease)) {
		QMouseEvent* pMouseEvent = dynamic_cast<QMouseEvent*>(pEvent);
		if ((pMouseEvent->button() == Qt::MouseButton::LeftButton) && m_bUserIsResizing) {
			m_bUserIsResizing = false;
			//if (m_playing)
			//	m_player.updateRenderFrame((HWND)ui.widget_video_play->winId(), RECT{ 0, 0, ui.widget_video_play->width(), ui.widget_video_play->height() });
			//if (m_pushing)
			//	m_pusher.updatePreview((HWND)ui.widget_video_push->winId(), RECT{ 0, 0, ui.widget_video_push->width(), ui.widget_video_push->height() });
		}
	}
	return QObject::eventFilter(pObj, pEvent); // pass it on without eating it
}

void DialogPushPlay::resizeEvent(QResizeEvent *event)
{
	m_bUserIsResizing = true;
	QDialog::resizeEvent(event);
}

void DialogPushPlay::startPush(QString url)
{
	stopPush();
	if (url.isEmpty() || !url.toLower().contains("rtmp://"))
	{
		DialogMessage::exec(QStringLiteral("请输入合法的RTMP地址!"), DialogMessage::OK);
		return;
	}

	if (m_cameraCount <= 0)
	{
		m_cameraCount = m_pusher.enumCameras(); //重新检查摄像头
		if (m_cameraCount <= 0)
		{
			DialogMessage::exec(QStringLiteral("请先接入摄像头!"), DialogMessage::OK);
			return;
		}
	}

	m_pusher.setRenderMode(TXE_RENDER_MODE_ADAPT);
	m_pusher.setAutoAdjustStrategy(TXE_AUTO_ADJUST_REALTIME_VIDEOCHAT_STRATEGY);
	m_pusher.startPreview((HWND)ui.widget_video_push->winId(), RECT{ 0, 0, ui.widget_video_push->width(), ui.widget_video_push->height() }, 0);
	m_pusher.startPush(url.toLocal8Bit());
	m_pusher.startAudioCapture();
	m_pushing = true;
}

void DialogPushPlay::startPlay(QString url)
{
	stopPlay();
	if (url.isEmpty() || !url.toLower().contains("rtmp://"))
	{
		DialogMessage::exec(QStringLiteral("请输入合法的RTMP地址!"), DialogMessage::OK);
		return;
	}

	m_player.setRenderYMirror(false);
	m_player.setRenderMode(TXE_RENDER_MODE_ADAPT);
	m_player.setRenderFrame((HWND)ui.widget_video_play->winId(), RECT{ 0, 0, ui.widget_video_play->width(), ui.widget_video_play->height() });
	m_player.startPlay(url.toLocal8Bit(), PLAY_TYPE_LIVE_RTMP_ACC);
	m_playing = true;
}

void DialogPushPlay::stopPush()
{
    m_pusher.setCallback(nullptr, nullptr);
	m_pusher.stopAudioCapture();
	m_pusher.stopPreview();
	m_pusher.stopPush();

	ui.widget_video_push->update();
	ui.widget_video_push->setUpdatesEnabled(true);
	m_pushing = false;
	m_pushBegin = false;
}

void DialogPushPlay::stopPlay()
{
    m_player.setCallback(nullptr, nullptr);
	m_player.stopPlay();
	m_player.closeRenderFrame();
	ui.widget_video_play->update();
	ui.widget_video_play->setUpdatesEnabled(true);
	m_playing = false;
}

void DialogPushPlay::setTitle(QString title)
{
	ui.label_title->setText(title);
}

void DialogPushPlay::setLogo(QString logo)
{
    // todo
}

void DialogPushPlay::setProxy(const std::string& ip, unsigned short port)
{
    if (false == ip.empty())
    {
        TXLiveCommon::getInstance()->setProxy(ip.c_str(), port);
    }
}

void DialogPushPlay::quit()
{
    stopPlay();
    stopPush();
}

void DialogPushPlay::on_btn_min_clicked()
{
	showMinimized();
}

void DialogPushPlay::on_update_event(int status)
{
	switch (status)
	{
	case 1:
	{
		DialogMessage::exec(QStringLiteral("播放连接失败!"), DialogMessage::OK);
		stopPlay();
	}
	break;
	case 2:
	{
		DialogMessage::exec(QStringLiteral("推流连接失败!"), DialogMessage::OK);
		stopPush();
	}
	break;
	case 3:
	{
		DialogMessage::exec(QStringLiteral("摄像头已被占用!"), DialogMessage::OK);
		stopPush();
	}
	break; 
	case 4:
	{
		ui.widget_video_play->setUpdatesEnabled(false);
	}
	break;
	case 5:
	{
		m_pushBegin = true;
		ui.widget_video_push->setUpdatesEnabled(false);
	}
	break;
	default:
		break;
	}
}

void DialogPushPlay::handle(txfunction func)
{
	func();
}

void DialogPushPlay::mousePressEvent(QMouseEvent *e)
{
	mousePressedPosition = e->globalPos();
	windowPositionAsDrag = pos();
}

void DialogPushPlay::mouseReleaseEvent(QMouseEvent *e)
{
	Q_UNUSED(e) 
		mousePressedPosition = QPoint();
}

void DialogPushPlay::mouseMoveEvent(QMouseEvent *e)
{
	if (!mousePressedPosition.isNull()) {
		QPoint delta = e->globalPos() - mousePressedPosition;
		move(windowPositionAsDrag + delta);
	}
}

void DialogPushPlay::on_btn_close_clicked()
{
	if (m_playing)
	{
		m_player.setCallback(NULL, NULL);
		m_player.stopPlay();
		m_player.closeRenderFrame();
	}
	if (m_pushing)
	{
		m_pusher.setCallback(NULL, NULL);
		m_pusher.stopAudioCapture();
		m_pusher.stopPreview();
		m_pusher.stopPush();
	}

	this->hide();
    Application::instance().quit(0);
}
