#include "miniTown.h"

const char* title = "Title_";
 int nScreenWidth = SCREEN_WIDTH;
 int nScreenHeight = SCREEN_HEIGHT;
HWND hWnd = NULL;
HDC hMemDC = NULL;
HBITMAP hBitmapBuffer = NULL;
HBITMAP hBitmapDevice = NULL;
BYTE* buffer = NULL;
BYTE screen_keys[512] = { 0 };
Player player;

Object* drawList[MaxDrawObjectSum];

//std::vector<Player::_Sound> _listPlay;
std::list<Player::_Sound *> _listPlay;
const int timeout = 500;//播放间隔 对音频外设操作需要时间间隔
const int step = 200;//响应时间
std::mutex m;//互斥锁
Player::Player() {
	;
}

Player::~Player() {
	Player::stopPlayAll();
}

int Player::playSound(const char* szFileNameTemp, unsigned short volume) {
	Player::_Sound* pSound = _MyMalloc();//用于保存该次播放的相关信息
	
	pSound->szFileName= (char*)szFileNameTemp;
	pSound->MciOpen.lpstrElementName = szFileNameTemp;
	pSound->volume = volume;
	pSound->isStop = false;
	pSound->hThread = CreateThread(NULL, 0, Player::_Play, (LPVOID)szFileNameTemp, 0, NULL);//创建线程异步播放
	if (pSound->hThread == NULL) {
		//GetLastError();
		return -1;
	}
	_listPlay.push_back(pSound);//保存信息
	//CloseHandle(pSound->hThread);
	Sleep(timeout);
	return (int)pSound->hThread;//返回一个被强转的线程句柄
}

int Player::playSound(char* szFileNameTemp, unsigned short volume) {
	Player::_Sound* pSound = _MyMalloc();//用于保存该次播放的相关信息

	pSound->szFileName = szFileNameTemp;
	pSound->MciOpen.lpstrElementName = szFileNameTemp;
	pSound->volume = volume;
	pSound->isStop = false;
	pSound->hThread = CreateThread(NULL, 0, Player::_Play, (LPVOID)szFileNameTemp, 0, NULL);//创建线程异步播放
	if (pSound->hThread == NULL) {
		//GetLastError();
		return -1;
	}
	_listPlay.push_back(pSound);//保存信息
	//CloseHandle(pSound->hThread);
	Sleep(timeout);
	return (int)pSound->hThread;//返回一个被强转的线程句柄
}

int Player::playSoundLoop(const char* szFileNameTemp, unsigned short volume) {
	Player::_Sound* pSound = _MyMalloc();//用于保存该次播放的相关信息

	pSound->szFileName = (char*)szFileNameTemp;
	pSound->MciOpen.lpstrElementName = szFileNameTemp;
	pSound->volume = volume;
	pSound->isStop = false;
	pSound->hThread = CreateThread(NULL, 0, Player::_PlayLoop, (LPVOID)szFileNameTemp, 0, NULL);//创建线程异步播放
	if (pSound->hThread == NULL) {
		//GetLastError();
		return -1;
	}
	_listPlay.push_back(pSound);//保存信息
	//CloseHandle(pSound->hThread);
	Sleep(timeout);
	return (int)pSound->hThread;//返回一个被强转的线程句柄
}

int Player::playSoundLoop(char* szFileNameTemp, unsigned short volume) {
	Player::_Sound* pSound = _MyMalloc();//用于保存该次播放的相关信息

	pSound->szFileName = szFileNameTemp;
	pSound->MciOpen.lpstrElementName = szFileNameTemp;
	pSound->volume = volume;
	pSound->isStop = false;
	pSound->hThread = CreateThread(NULL, 0, Player::_PlayLoop, (LPVOID)szFileNameTemp, 0, NULL);//创建线程异步播放
	if (pSound->hThread == NULL) {
		//GetLastError();
		return -1;
	}
	_listPlay.push_back(pSound);//保存信息
	//CloseHandle(pSound->hThread);
	Sleep(timeout);
	return (int)pSound->hThread;//返回一个被强转的线程句柄
}
void Player::stopPlay(const char* szTargetFileName) {
	Player::_Sound* pSound = NULL;
	m.lock();
	std::list<Player::_Sound*>::iterator it = _listPlay.begin();
	for (; it != _listPlay.end(); ++it) {
		if ((*(*it)) == (char*)szTargetFileName) {
			pSound = (*it);
			break;
		}
	}
	m.unlock();
	if (pSound == NULL) {
		return;
	}

	pSound->stop();//通知线程结束
	WaitForSingleObject(pSound->hThread, step * 2);//等待结束
	CloseHandle(pSound->hThread);
	pSound->hThread = NULL;

	//从列表中移除
	clearPlayList();

	_MyFree(pSound);//释放资源
}
void Player::stopPlay(char* szTargetFileName) {
	Player::_Sound* pSound = NULL;
	m.lock();
	std::list<Player::_Sound*>::iterator it = _listPlay.begin();
	for (; it != _listPlay.end(); ++it) {
		if ((*(*it)) ==szTargetFileName) {
			pSound = (*it);
			break;
		}
	}
	m.unlock();
	if (pSound == NULL) {
		return;
	}

	pSound->stop();//通知线程结束
	WaitForSingleObject(pSound->hThread, step * 2);//等待结束
	CloseHandle(pSound->hThread);
	pSound->hThread = NULL;

	//从列表中移除
	clearPlayList();

	_MyFree(pSound);//释放资源
}
void Player::stopPlay(int hThread) {
	Player::_Sound* pSound = NULL;
	m.lock();
	std::list<Player::_Sound*>::iterator it = _listPlay.begin();
	for (; it != _listPlay.end(); ++it) {
		if ((*(*it)) == hThread) {
			pSound = (*it);
			break;
		}
	}
	m.unlock();
	if (pSound == NULL) {
		return;
	}

	pSound->stop();//通知线程结束
	WaitForSingleObject(pSound->hThread, step * 2);//等待结束
	CloseHandle(pSound->hThread);
	pSound->hThread = NULL;

	//从列表中移除
	clearPlayList();

	_MyFree(pSound);//释放资源
}
void Player::stopPlayAll() {
	Player::_Sound* pSound = NULL;
	m.lock();
	while (!_listPlay.empty()) {
		pSound = _listPlay.front();
		m.unlock();
		stopPlay((int)pSound->hThread);
		m.lock();
	}
	m.unlock();
}


DWORD WINAPI Player::_Play(LPVOID lpParameter) {
	Player::_Sound* pSound = NULL;
	//遍历链表寻找 因由当前线程所维护的播放信息
	
	m.lock();
	std::list<Player::_Sound*>::iterator it = _listPlay.begin();
	for (; it != _listPlay.end(); ++it) {
		if ((*(*it)) == (char*)lpParameter) {
			pSound = (*it);
			break;
		}
	}
	m.unlock();

	if (pSound == NULL) {
		return -1;
	}

	//打开操作要和其他操作在同一线程.
	DWORD ret = mciSendCommand(0, MCI_OPEN,
		MCI_OPEN_ELEMENT, (DWORD_PTR)&pSound->MciOpen);//打开一个设备 准备播放该音效
	if (ret != 0) {
		std::cout <<pSound->szFileName << " MCI_OPEN ret:" << ret << std::endl;
		return ret;
	}
	std::cout <<pSound->szFileName << " id:" <<pSound->MciOpen.wDeviceID << std::endl;

	ret = mciSendCommand(pSound->MciOpen.wDeviceID, MCI_PLAY,
		MCI_NOTIFY, (DWORD_PTR)(LPVOID)&pSound->MciPlay);//开始播放
	std::cout <<pSound->szFileName << " MCI_PLAY ret" <<ret << std::endl;

	pSound->MciStatus.dwItem = MCI_STATUS_LENGTH;//要获取的项 获取长度
	ret = mciSendCommand(pSound->MciOpen.wDeviceID, MCI_STATUS,
		MCI_STATUS_ITEM, (DWORD_PTR)(LPVOID)&pSound->MciStatus);//获取音效时长  单位毫秒
	pSound->nLength = pSound->MciStatus.dwReturn;
	std::cout <<pSound->szFileName << " MCI_STATUS ret" << ret << std::endl;
	std::cout <<pSound->szFileName << " Length:" <<pSound->nLength << std::endl;


	pSound->MciSetAudio.dwItem = MCI_DGV_SETAUDIO_VOLUME;
	pSound->MciSetAudio.dwValue = pSound->volume;
	ret = mciSendCommand(pSound->MciOpen.wDeviceID, MCI_SETAUDIO,
		MCI_DGV_SETAUDIO_VALUE | MCI_DGV_SETAUDIO_ITEM, (DWORD_PTR)&pSound->MciSetAudio);//设置音量
	std::cout <<pSound->szFileName << " MCI_SET ret" << ret << std::endl;

	
	for (int i = 0; (!pSound->isStop) && (i < pSound->nLength); i += step) {//等待播放完毕
		Sleep(step);
	}
	//Sleep((*it).nLength);//等待播放完毕
	ret = mciSendCommand(pSound->MciOpen.wDeviceID, MCI_CLOSE, NULL, NULL);//关闭设备
	std::cout <<pSound->szFileName << " MCI_CLOSE ret" << ret << std::endl;

	return 0L;
}

DWORD WINAPI Player::_PlayLoop(LPVOID lpParameter) {
	Player::_Sound* pSound = NULL;
	//遍历链表寻找 因由当前线程所维护的播放信息

	m.lock();
	std::list<Player::_Sound*>::iterator it = _listPlay.begin();
	for (; it != _listPlay.end(); ++it) {
		if ((*(*it)) == (char*)lpParameter) {
			pSound = (*it);
			break;
		}
	}
	m.unlock();

	if (pSound == NULL) {
		return -1;
	}

	DWORD ret;
	//循环播放
	while (!pSound->isStop) {
		ret = mciSendCommand(0, MCI_OPEN,
			MCI_OPEN_ELEMENT, (DWORD_PTR)(LPVOID)&pSound->MciOpen);//打开一个设备 准备播放该音效
		if (ret != 0) {
			return ret;
		}
		std::cout <<pSound->szFileName << " id:" <<pSound->MciOpen.wDeviceID << std::endl;

		ret = mciSendCommand(pSound->MciOpen.wDeviceID, MCI_PLAY,
			MCI_NOTIFY, (DWORD_PTR)(LPVOID)&pSound->MciPlay);//开始播放
		std::cout <<pSound->szFileName << " MCI_PLAY ret" << ret << std::endl;

		pSound->MciStatus.dwItem = MCI_STATUS_LENGTH;//要获取的项 获取长度
		ret = mciSendCommand(pSound->MciOpen.wDeviceID, MCI_STATUS,
			MCI_STATUS_ITEM, (DWORD_PTR)(LPVOID)&pSound->MciStatus);//获取音效时长  单位毫秒
		pSound->nLength = pSound->MciStatus.dwReturn;
		std::cout <<pSound->szFileName << " MCI_STATUS ret" << ret << std::endl;
		std::cout <<pSound->szFileName << " Length:" <<pSound->MciStatus.dwReturn << std::endl;

		pSound->MciSetAudio.dwItem = MCI_DGV_SETAUDIO_VOLUME;
		pSound->MciSetAudio.dwValue = pSound->volume;
		ret = mciSendCommand(pSound->MciOpen.wDeviceID, MCI_SETAUDIO,
			MCI_DGV_SETAUDIO_VALUE | MCI_DGV_SETAUDIO_ITEM, (DWORD_PTR)&pSound->MciSetAudio);//设置音量
		std::cout <<pSound->szFileName << " MCI_SET ret" << ret << std::endl;

		for (int i = 0; (!pSound->isStop) && (i < pSound->nLength); i += step) {
			Sleep(step);
		}

		ret = mciSendCommand(pSound->MciOpen.wDeviceID, MCI_CLOSE, NULL, NULL);//关闭设备
		std::cout <<pSound->szFileName << " MCI_CLOSE ret" << ret << std::endl;
	}
	return 0L;
}

void Player::clearPlayList(){
	m.lock();
	std::list<Player::_Sound*>::iterator it = _listPlay.begin();
	for (; it != _listPlay.end(); ++it) {
		if ((*it)->hThread == NULL) {
			_listPlay.erase(it);
			break;
		}
	}
	m.unlock();
}

Player::_Sound* Player::_MyMalloc(){
	return new Player::_Sound;
}

void Player::_MyFree(Player::_Sound* ptr){
	delete ptr;
}


int drawSum = 0;
void AddDrawObject(Object* object)
{
	bool findFlag = false;
	for (int i = 0; i < drawSum; i++)
	{
		if (drawList[i] == object)
		{
			findFlag = true;
		}
	}
	if (!findFlag)
	{
		drawList[drawSum] = object;
		drawSum++;
	}
}

void RemoveDrawObecjt(Object* object)
{
	int findIndex = -1;
	for (int i = 0; i < drawSum; i++)
	{
		if (drawList[i] == object)
		{
			findIndex = i;
			break;
		}
	}
	if (findIndex != -1)
	{
		for (int i = findIndex; i < drawSum - 1; i++)
		{
			drawList[i] = drawList[i + 1];
		}
		drawSum--;
	}
}

inline void DrawPoint(int nPos, Color c) {
	if (!c.isTransparent()) {
		buffer[nPos++] = c.b;
		buffer[nPos++] = c.g;
		buffer[nPos++] = c.r;
	}
}

inline void DrawPoint(int nPos, BYTE r, BYTE g, BYTE b, BYTE a = 0) {
	if (!(r == 255 && g == 255 && b == 255)) {
		buffer[nPos++] = b;
		buffer[nPos++] = g;
		buffer[nPos++] = r;
	}
}

inline void DrawPoint(int x, int y, Color c) {
	if (!c.isTransparent()) {
		int nPos = (y * nScreenWidth + x) * 4;
		buffer[nPos++] = c.b;
		buffer[nPos++] = c.g;
		buffer[nPos++] = c.r;
	}
}

inline void DrawPoint(int x, int y, BYTE r, BYTE g, BYTE b) {
	if (!(r == 255 && g == 255 && b == 255)) {
		int nPos = (y * nScreenWidth + x) * 4;
		buffer[nPos++] = b;
		buffer[nPos++] = g;
		buffer[nPos++] = r;
	}
}
inline void DrawRect(int x, int y, int width, int height, BYTE r, BYTE g, BYTE b) {
	for (int i = 0; i < height; ++i) {
		int posBuffer = (nScreenWidth * (y + i) + x) * 4;//计算每行的起点
		for (int j = 0; j < width; ++j) {
			DrawPoint(posBuffer, r, g, b);
			posBuffer += 4;
		}
	}
}

//uncheck
inline void DrawPic(int x, int y, Picture* pic, RECT rect) {
	int xBuffer = 0, yBuffer = 0;
	int xPic, yPic;

	for (yPic = rect.top; yPic < rect.bottom; ++yPic) {
		int posBuffer = (nScreenWidth * (y + yBuffer++) + x) * 4;
		for (xPic = rect.left; xPic < rect.right; ++xPic) {
			int posPic = pic->getWidth() * yPic + xPic;
			DrawPoint(posBuffer, pic->pChannelR[posPic], pic->pChannelG[posPic], pic->pChannelB[posPic]);
			posBuffer += 4;
			++posPic;
		}
	}
}
inline void DrawPic(int x, int y, Picture* pic) {
	RECT rect;
	int _x, _y;
	//纵向检查
	if ((y + pic->getHeight()) < 0) {
		//完全超出上边界
		//不绘制
		return;
	}
	else if (y < 0) {
		//部分超出上边界
		rect.top = -y;
		rect.bottom = pic->getHeight();
		//_x = x;
		_y = 0;
	}
	else if ((y + pic->getHeight()) < nScreenHeight) {
		//完全在窗口内
		rect.top = 0;
		rect.bottom = pic->getHeight();
		//_x = x;
		_y = y;
	}
	else if (y < nScreenHeight) {
		//部分超出下边界
		rect.top = 0;
		rect.bottom = nScreenHeight - y;
		//_x = x;
		_y = y;
	}
	else {
		//完全超出下边界
		//不绘制
		return;
	}

	//横向检查
	if ((x + pic->getWidth()) < 0) {
		//完全超出左边界
		//不绘制
		return;
	}
	else if (x < 0) {
		//部分超出左边界
		rect.left = -x;
		rect.right = pic->getWidth();
		_x = 0;
		//_y = y;
	}
	else if ((x + pic->getWidth()) < nScreenWidth) {
		//完全在窗口内
		rect.left = 0;
		rect.right = pic->getWidth();
		_x = x;
		//_y = y;
	}
	else if (x < nScreenWidth) {
		//部分超出右边界
		rect.left = 0;
		rect.right = nScreenWidth - x;
		_x = x;
		//_y = y;
	}
	else {
		//完全超出下边界
		//不绘制
		return;
	}

	DrawPic(_x, _y, pic, rect);
}

//Description:
//	P点绕P0顺时针旋转angle
//Paramter: 
//	POINT p		被旋转点
//	POINT p0	中心点
//	float angle	旋转角度
//	POINT& p_	旋转后点
//Return:
//	NONE
inline void spin(Point p, Point p0, float angle, Point& p_) {
	float sinAngle = sin(angle);
	float cosAngle = cos(angle);


	p_.x = p.x * cosAngle + p.y * sinAngle + p0.x * (1 - cosAngle) - p0.y * sinAngle;


	p_.y = -p.x * sinAngle + p.y * cosAngle + p0.y * (1 - cosAngle) + p0.x * sinAngle;
}

inline void DrawObject(Object* obj) {
	//后期考虑 作为object的成员函数
	float angle = obj->getAngle(1);

	if (angle == 0)//应该可以
	{
		//不需要旋转
		DrawPic(obj->point.x, obj->point.y, obj->pic);
	}
	else {
		int nWidth = obj->pic->getWidth();
		int nHeight = obj->pic->getHeight();

		//遍历图像
		for (int i = 0; i < nHeight; ++i) {
			for (int j = 0; j < nWidth; ++j) {
				//计算旋转后坐标
				Point p_;

				//这样计算得到的p_是以当前对象左上角为原点的坐标系 需要映射到窗口坐标系中
				spin({ j,i }, obj->pic->centerPoint, obj->getAngle(1), p_);

				//映射
				p_ += obj->point;

				if (p_.x < 0 || p_.y < 0) {
					continue;//将旋转到窗体外的点抛弃
				}
				else if (p_.x > nScreenWidth || p_.y > nScreenHeight) {
					continue;//将旋转到窗体外的点抛弃
				}
				int posBuffer = (p_.y * nScreenWidth + p_.x) * 4;
				int posPic = i * nWidth + j;

				DrawPoint(posBuffer, obj->pic->pChannelR[posPic], obj->pic->pChannelG[posPic], obj->pic->pChannelB[posPic]);
				posBuffer += 4;
				++posPic;
			}
		}
	}

}

//Description:
//	更新重画窗口
//	接收消息
//Paramter: 
//	
//Return:
//	
void Draw() {
	void getMessage();

	DrawRect(0, 0, nScreenWidth, nScreenHeight, 192, 224, 0);

	/*
	Object obj;
	obj.point.x = 10;
	obj.point.y = 10;
	obj.z = 1;
	obj.setAngle(45);
	obj.pic = &picField;

	Object obj2;
	obj2.point.x = 30;
	obj2.point.y = 30;
	obj2.z = 2;
	obj2.setAngle(45);
	obj2.pic = &picField1;

	Object obj3;
	obj3.point.x = 50;
	obj3.point.y = 50;
	obj3.setAngle(45);
	obj3.z = 0;
	obj3.pic = &picField2;

	Object* _drawList[MaxDrawObjectSum];
	_drawList[0] = &obj;
	_drawList[1] = &obj2;
	_drawList[2] = &obj3;
	std::sort(*_drawList, *(_drawList + 2));
	for (int i = 0; i < 3; ++i) {
		DrawObject(_drawList[i]);
	}*/
	
	for (int i = 0; i < drawSum; i++)
	{
		//DebugPrint("第%d个objetc 开始绘制\t",i);
		Object* drawNowObject = drawList[i];
		//DrawBmp(drawNowObject->x, drawNowObject->y, drawNowObject->pic);
		drawNowObject->updatePoint();
		DrawObject(drawNowObject);
		//DebugPrintln("绘制完成");
	}

	HDC hDC = GetDC(hWnd);
	BitBlt(hDC, 0, 0, nScreenWidth, nScreenHeight, hMemDC, 0, 0, SRCCOPY);
	ReleaseDC(hWnd, hDC);
	getMessage();
}

void getMessage() {
	MSG msg;
	while (1) {
		if (!PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) break;
		if (!GetMessage(&msg, NULL, 0, 0)) break;
		DispatchMessage(&msg);
	}
}

void freeSomethingForEngine() {
	if (hMemDC) {
		if (hBitmapDevice) {
			SelectObject(hMemDC, hBitmapDevice);
			hBitmapDevice = NULL;
		}
		DeleteDC(hMemDC);
		hMemDC = NULL;
	}
	if (hBitmapBuffer) {
		DeleteObject(hBitmapBuffer);
		hBitmapBuffer = NULL;
	}
	if (hWnd) {
		CloseWindow(hWnd);
		hWnd = NULL;
	}

	Player::stopPlayAll();
}
//处理消息
static LRESULT CALLBACK WinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
	case WM_CLOSE: {
		//DebugPrintln("%d\t%d", wParam, wParam & 511);
		break;
	}
	case WM_KEYDOWN: {
		//DebugPrintln("%d\t%d", wParam, wParam & 511);
		screen_keys[wParam & 511] = 1;
		break;
	}
	case WM_KEYUP: {
		screen_keys[wParam & 511] = 0;
		break;
	}
	default: {
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	}
	return 0;
}

//Description:
//	创建并维护一个窗口,目前不对参数合法性进行检查.
//	该窗口显示的内容依赖于window结构体里所指向的img
//Paramter: 
//	LPVOID window一个指向Window结构体的指针
//Return:
//	0:		正常
//	other:	由GetLastError()返回的错误代码
int _CreateWindow(const char* title, int nWidth, int nHeight) {
	HINSTANCE hInstance = GetModuleHandle(NULL);

	WNDCLASS wc;
	memset(&wc, 0, sizeof(wc));
	wc.style = CS_BYTEALIGNCLIENT;
	wc.lpfnWndProc = (WNDPROC)WinProc;
	wc.hInstance = hInstance;//GetModuleHandle(NULL);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszClassName = (char*)title;
	wc.lpszMenuName = (char*)title;

	RegisterClass(&wc);

	//Create a new window
	hWnd = CreateWindow(
		(char*)title,//class name
		(char*)title,//title
		WS_OVERLAPPEDWINDOW,//style
		CW_USEDEFAULT, CW_USEDEFAULT,//position
		nWidth, nHeight,//size
		NULL,//parent window
		NULL,//menu
		hInstance,//application instance
		NULL//prarameters
	);
	if (hWnd == 0) {
		return GetLastError();
	}

	//创建内存缓冲画板
	HDC hDC = GetDC(hWnd);
	hMemDC = CreateCompatibleDC(hDC);//创建与窗口兼容的内存设备上下文环境(创建内存缓冲画板)
	BITMAPINFO bi = { { sizeof(BITMAPINFOHEADER), nWidth, -nHeight, 1, 32, BI_RGB,
		nWidth * nHeight * 4, 0, 0, 0, 0 } };
	LPVOID ptr;
	hBitmapBuffer = CreateDIBSection(hMemDC, &bi, DIB_RGB_COLORS, &ptr, 0, 0);//创建缓冲区画布
	hBitmapDevice = (HBITMAP)SelectObject(hMemDC, hBitmapBuffer);//选择缓冲区画布到缓冲区画板，并保留原始画布
	buffer = (BYTE*)ptr;
	ReleaseDC(hWnd, hDC);

	ShowWindow(hWnd, SW_NORMAL);
	UpdateWindow(hWnd);

	return 0;
}
