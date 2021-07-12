
// PCOViewerDlg.h: 헤더 파일
//

#pragma once

#include "PictureCtrl.h"

// CPCOViewerDlg 대화 상자
class CPCOViewerDlg : public CDialogEx
{
// 생성입니다.
public:
	CPCOViewerDlg(CWnd* pParent = nullptr);	// 표준 생성자입니다.

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PCOVIEWER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.


// 구현입니다.
protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:
	CWinThread* cameraThread = NULL;
	bool needCameraThread = true;
	afx_msg void OnDestroy();
	LRESULT onDeviceInitialized(WPARAM wParam, LPARAM lParam);
	LRESULT onImageReceived(WPARAM wParam, LPARAM lParam);
	CPictureCtrl m_streamCtrl;
};

#define WM_DEVICE_INITIALIZED  (WM_USER + 101)
#define WM_IMAGE_RECEIVED  (WM_USER + 102)
