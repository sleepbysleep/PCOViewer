
// PCOViewerDlg.cpp: 구현 파일
//

#include "pch.h"
#include "framework.h"
#include "PCOViewer.h"
#include "PCOViewerDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#include "pco_err.h"
#include "sc2_SDKStructures.h"
#include "SC2_SDKAddendum.h"
#include "SC2_CamExport.h"
#include "SC2_Defs.h"

#include <vector>
#include <string>
#include <locale>
#include <codecvt>

static std::vector<std::wstring> deviceProperties;

HANDLE pcoHandle;
HANDLE pcoBufferEvent;
WORD* pcoBufferAddr;
short pcoBufferNumber;
WORD pcoImageWidth, pcoImageHeight;


void initPCOPanda(void)
{
	int res = PCO_OpenCamera(&pcoHandle, 0);
	if (res != PCO_NOERROR) return;

	PCO_Description standard_desc;
	standard_desc.wSize = sizeof(PCO_Description);
	PCO_GetCameraDescription(pcoHandle, &standard_desc);

	WORD recording_state;
	PCO_GetRecordingState(pcoHandle, &recording_state);
	if (recording_state) {
		res = PCO_SetRecordingState(pcoHandle, 0);
	}

	PCO_ResetSettingsToDefault(pcoHandle);
	PCO_ArmCamera(pcoHandle);

	DWORD camera_warning, camera_error, camera_status;
	PCO_GetCameraHealthStatus(pcoHandle, &camera_warning, &camera_error, &camera_status);
	if (camera_error != 0) {
		PCO_CloseCamera(pcoHandle);
		return;
	}

	PCO_CameraType camera_type;
	camera_type.wSize = sizeof(PCO_CameraType);
	res = PCO_GetCameraType(pcoHandle, &camera_type);
	if (res != PCO_NOERROR) {
		return;
	}

	//int timeouts[3] = { 2000, 3000, 250 }; // command, image, channel timeout in [ms]
	//PCO_SetTimeouts(pcoHandle, timeouts, sizeof(timeouts));

	WORD max_width, max_height;
	PCO_GetSizes(pcoHandle, &pcoImageWidth, &pcoImageHeight, &max_width, &max_height);
	DWORD buffer_size = pcoImageWidth * pcoImageHeight * sizeof(WORD);

	pcoBufferEvent = NULL;
	pcoBufferAddr = NULL;
	pcoBufferNumber = -1;
	PCO_AllocateBuffer(pcoHandle, &pcoBufferNumber, buffer_size, &pcoBufferAddr, &pcoBufferEvent);

	PCO_SetImageParameters(pcoHandle, pcoImageWidth, pcoImageHeight, IMAGEPARAMETERS_READ_WHILE_RECORDING, NULL, 0);

	PCO_SetRecordingState(pcoHandle, 1);
	// camera_type.wCamType == 0x1500 : pco.panda 
	// camera_type.wInterfaceType == 0x0006 : USB 3.0
	// camera_type.dwSerialNumber
	// 

	char buf[256];
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

	PCO_GetInfoString(pcoHandle, INFO_STRING_CAMERA, buf, 255);
	deviceProperties.push_back(converter.from_bytes(buf));
	TRACE("Camera: %s\n", buf);

	PCO_GetInfoString(pcoHandle, INFO_STRING_SENSOR, buf, 255);
	deviceProperties.push_back(converter.from_bytes(buf));
	TRACE("Sensor: %s\n", buf);

	switch (camera_type.wInterfaceType) {
	case 0x0001: snprintf(buf, 255, "FIREWIRE"); break;
	case 0x0002: snprintf(buf, 255, "CAMERALINK"); break;
	case 0x0003: snprintf(buf, 255, "USB"); break;
	case 0x0004: snprintf(buf, 255, "ETHERNET"); break;
	case 0x0005: snprintf(buf, 255, "SERIAL"); break;
	case 0x0006: snprintf(buf, 255, "USB3"); break;
	case 0x0007: snprintf(buf, 255, "CAMERALINKHS"); break;
	case 0x0008: snprintf(buf, 255, "COAXPRESS"); break;
	case 0x0009: snprintf(buf, 255, "USB31_GEN1"); break;
	default: snprintf(buf, 255, "Unknown interface"); break;
	}
	//PCO_GetInfoString(pcoHandle, 0, buf, 255);
	deviceProperties.push_back(converter.from_bytes(buf));
	TRACE("Interface: %s\n", buf);

	PCO_GetInfoString(pcoHandle, INFO_STRING_PCO_MATERIALNUMBER, buf, 255);
	deviceProperties.push_back(converter.from_bytes(buf));
	TRACE("Serial Number: %s\n", buf);

	deviceProperties.push_back(std::to_wstring(pcoImageWidth));
	TRACE("Width: %d\n", pcoImageWidth);
	deviceProperties.push_back(std::to_wstring(pcoImageHeight));
	TRACE("Height: %d\n", pcoImageHeight);
	deviceProperties.push_back(std::to_wstring(16));

	deviceProperties.push_back(std::to_wstring(standard_desc.dwMinExposureDESC));
	TRACE("Min Exposure: %d[ns]\n", standard_desc.dwMinExposureDESC);

	deviceProperties.push_back(std::to_wstring(standard_desc.dwMaxExposureDESC));
	TRACE("Max Exposure: %d[ms]\n", standard_desc.dwMaxExposureDESC);

	// TODO: fixed range of exposure as 10, and 5000 [ms]
	DWORD delay;
	DWORD exposure;
	WORD timebase_delay;
	WORD timebase_exposure;

	PCO_GetDelayExposureTime(pcoHandle, &delay, &exposure, &timebase_delay, &timebase_exposure);
	TRACE("TimeBaseDelay: %d\n", timebase_delay);
	TRACE("TimeBaseExposure: %d\n", timebase_exposure);
	TRACE("Delay: %d\n", delay);
	TRACE("Exposure: %d\n", exposure);

	PCO_SetDelayExposureTime(pcoHandle, 0, 100/*exposure*/, 1, 2/*ms*/);

	PCO_GetDelayExposureTime(pcoHandle, &delay, &exposure, &timebase_delay, &timebase_exposure);
	TRACE("TimeBaseDelay: %d\n", timebase_delay);
	TRACE("TimeBaseExposure: %d\n", timebase_exposure);
	TRACE("Delay: %d\n", delay);
	TRACE("Exposure: %d\n", exposure);
}

// Display ID -> Camera Type
// MAC Address -> Sensor Type
// IP Address -> Interface Type
// Serial Number -> Serial Number
// Width
// Height
// Pixel Bits
// Min Exposure
// Max Exposure

void deinitPCOPanda(void)
{
	PCO_SetRecordingState(pcoHandle, 0);
	PCO_FreeBuffer(pcoHandle, pcoBufferNumber);
	PCO_CloseCamera(pcoHandle);
}

static bool needStreaming = true;

UINT cameraStreaming(LPVOID lParam)
{
	CPCOViewerDlg* mainDlg = (CPCOViewerDlg*)lParam;

	initPCOPanda();
	
	PostMessage(mainDlg->m_hWnd, WM_DEVICE_INITIALIZED, (WPARAM)NULL, (LPARAM)&deviceProperties);
	Sleep(10);

	while (mainDlg->needCameraThread) {
		if (needStreaming) {
			int res = PCO_GetImageEx(pcoHandle, 1, 0, 0, pcoBufferNumber, pcoImageWidth, pcoImageHeight, 16);
			if (res == PCO_NOERROR) {
				PostMessage(mainDlg->m_hWnd, WM_IMAGE_RECEIVED, NULL, (LPARAM)pcoBufferAddr);
				needStreaming = false;
			}
		}
		Sleep(10);
	}

	deinitPCOPanda();

	return 0;
}

// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.
class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

// 구현입니다.
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CPCOViewerDlg 대화 상자



CPCOViewerDlg::CPCOViewerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_PCOVIEWER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CPCOViewerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_STREAM, m_streamCtrl);
}

BEGIN_MESSAGE_MAP(CPCOViewerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DESTROY()
	ON_MESSAGE(WM_DEVICE_INITIALIZED, &CPCOViewerDlg::onDeviceInitialized)
	ON_MESSAGE(WM_IMAGE_RECEIVED, &CPCOViewerDlg::onImageReceived)
END_MESSAGE_MAP()


// CPCOViewerDlg 메시지 처리기

BOOL CPCOViewerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.
	cameraThread = AfxBeginThread(cameraStreaming, this, 1);

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CPCOViewerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 애플리케이션의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CPCOViewerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CPCOViewerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CPCOViewerDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	needCameraThread = false;
	Sleep(0);
	if (cameraThread) WaitForSingleObject(cameraThread->m_hThread, 5000);
}

LRESULT CPCOViewerDlg::onDeviceInitialized(WPARAM wParam, LPARAM lParam)
{
	int count = (int)wParam;
	std::vector<std::wstring>* properties = (std::vector<std::wstring> *)lParam;

	TRACE("Device Info.:%d", properties->size());

	return 0;
}

static int imageCount = 0;
static BYTE* imageBuffer = NULL;

LRESULT CPCOViewerDlg::onImageReceived(WPARAM wParam, LPARAM lParam)
{
	WORD* buffer = (WORD*)lParam;

	TRACE("imageCount:%d\n", imageCount);
	++imageCount;
	
	if (imageBuffer == NULL) imageBuffer = new BYTE[pcoImageHeight * pcoImageWidth * 4];

	for (int i = 0; i < pcoImageHeight; ++i) {
		for (int j = 0; j < pcoImageWidth; ++j) {
			uint8_t value = *(buffer + i * pcoImageWidth + j) >> 8; // (uint8_t)((double)(*(buffer + i * w + j) - minValue) / (maxValue - minValue) * 255.0);
			*(imageBuffer + (i * pcoImageWidth + j) * 4 + 3) = 255;
			*(imageBuffer + (i * pcoImageWidth + j) * 4 + 2) = value; // Red
			*(imageBuffer + (i * pcoImageWidth + j) * 4 + 1) = value; // Green
			*(imageBuffer + (i * pcoImageWidth + j) * 4 + 0) = value; // Blue
		}
	}

	Gdiplus::Bitmap bmp(pcoImageWidth, pcoImageHeight, 4 * pcoImageWidth, PixelFormat32bppARGB, imageBuffer);

	//Gdiplus::Graphics* g = Gdiplus::Graphics::FromImage(&bmp);
	//Gdiplus::Pen p(Gdiplus::Color::Red, (float)3);
	//p.SetDashStyle(Gdiplus::DashStyle::DashStyleDot);
	//g->DrawLine(&p, m_ProfileX, 0, m_ProfileX, pcoImageHeight - 1);
	//g->DrawLine(&p, 0, m_ProfileY, m_imageWidth - 1, m_ProfileY);
	//delete g;

	IStream* istream = nullptr;
	CreateStreamOnHGlobal(NULL, TRUE, &istream);

	CLSID clsid_png;
	CLSIDFromString(L"{557cf406-1a04-11d3-9a73-0000f81ef32e}", &clsid_png);
	Gdiplus::Status status = bmp.Save(istream, &clsid_png);
	if (status == Gdiplus::Status::Ok) {
		m_streamCtrl.Load(istream);
	}

	istream->Release();

	needStreaming = true;

	return 0;
}