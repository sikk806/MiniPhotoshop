
// 201611256View.cpp: CMy201611256View 클래스의 구현
//

#include <cmath>
#include <algorithm>
#include <iostream>

using namespace std;

#include "pch.h"
#include "framework.h"
// SHARED_HANDLERS는 미리 보기, 축소판 그림 및 검색 필터 처리기를 구현하는 ATL 프로젝트에서 정의할 수 있으며
// 해당 프로젝트와 문서 코드를 공유하도록 해 줍니다.
#ifndef SHARED_HANDLERS
#include "201611256.h"
#endif

#include "201611256Doc.h"
#include "201611256View.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// histogram
#define BAR_WIDTH 2
#define BAR_HEIGHT 1

// CMy201611256View

RGBQUAD** rgbBuffer; //이미지를 저장할 변수
int imgHeight; //이미지 높이 정보
int imgWidth; //이미지 너비 정보
BITMAPINFOHEADER bmpInfo; //BMP 정보 헤더
BITMAPFILEHEADER bmpHeader; //BMP 파일 헤더

float** hueBuffer;
float** satuBuffer;
float** intenBuffer;

float** transBuffer;
float** histoBuffer;

float** smoothBuffer;
float** sharpenBuffer;

RGBQUAD** smoothBuffer_c;

float** edgeBuffer;

float drawHisto[256] = { 0, };
float lookUpTable[256] = { 0, };

int viewType;
int flag = 0;

IMPLEMENT_DYNCREATE(CMy201611256View, CView)

BEGIN_MESSAGE_MAP(CMy201611256View, CView)
	// 표준 인쇄 명령입니다.
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CView::OnFilePrintPreview)\
	//load Img
	ON_COMMAND(ID_IMG_LOAD_BMP, &CMy201611256View::OnImgLoadBmp)
	ON_COMMAND(ID_IMG_LOAD_JPEG, &CMy201611256View::OnImgLoadJpeg)
	//load HSI
	ON_COMMAND(ID_LOAD_HSI, &CMy201611256View::OnLoadHsi)
	//DO Grey Transformation
	ON_COMMAND(ID_GREYLVTRANS_NEGATIVE, &CMy201611256View::OnGreylvtransNegative)
	ON_COMMAND(ID_GREYLVTRANS_GAMMA2, &CMy201611256View::OnGreylvtransGamma2)
	ON_COMMAND(ID_GREYLVTRANS_GAMMAHALF, &CMy201611256View::OnGreylvtransGammahalf)
	ON_COMMAND(ID_GREYLVTRANS_GAMMA0DOT1, &CMy201611256View::OnGreylvtransGamma0dot1)
	// DO HE
	ON_COMMAND(ID_ORIGINAL_HISTO, &CMy201611256View::OnOriginalHisto)
	ON_COMMAND(ID_HISTOGRAM_EQUALIZATION, &CMy201611256View::OnHistogramEqualization)
	ON_COMMAND(ID_GREYLVTRANS_GAMMA10, &CMy201611256View::OnGreylvtransGamma10)
	// Average
	ON_COMMAND(ID_AVERAGE_3, &CMy201611256View::OnAverage3)
	ON_COMMAND(ID_AVERAGE_5, &CMy201611256View::OnAverage5)
	ON_COMMAND(ID_AVERAGE_7, &CMy201611256View::OnAverage7)
	// Median
	ON_COMMAND(ID_MEDIAN_3, &CMy201611256View::OnMedian3)
	ON_COMMAND(ID_MEDIAN_5, &CMy201611256View::OnMedian5)
	ON_COMMAND(ID_MEDIAN_7, &CMy201611256View::OnMedian7)
	// Gaussian
	ON_COMMAND(ID_GAUSSIAN_3, &CMy201611256View::OnGaussian3)
	ON_COMMAND(ID_GAUSSIAN_5, &CMy201611256View::OnGaussian5)
	ON_COMMAND(ID_GAUSSIAN_7, &CMy201611256View::OnGaussian7)
	// High-Boost
	ON_COMMAND(ID_MASK4_A, &CMy201611256View::OnMask4A12)
	ON_COMMAND(ID_MASK4_A32813, &CMy201611256View::OnMask4A15)
	ON_COMMAND(ID_MASK8_A, &CMy201611256View::OnMask8A12)
	ON_COMMAND(ID_MASK8_A32815, &CMy201611256View::OnMask8A15)
	// Color Median
	ON_COMMAND(ID_MEDIAN_4, &CMy201611256View::OnColorMedian3)
	ON_COMMAND(ID_MEDIAN_6, &CMy201611256View::OnColorMedian5)
	ON_COMMAND(ID_MEDIAN_8, &CMy201611256View::OnColorMedian7)
	// Color Average
	ON_COMMAND(ID_AVERAGE_9, &CMy201611256View::OnColorAverage3)
	ON_COMMAND(ID_AVERAGE_10, &CMy201611256View::OnColorAverage5)
	ON_COMMAND(ID_AVERAGE_11, &CMy201611256View::OnColorAverage7)
	// ColorGaussian
	ON_COMMAND(ID_GAUSSIAN_4, &CMy201611256View::OnColorGaussian3)
	ON_COMMAND(ID_GAUSSIAN_6, &CMy201611256View::OnColorGaussian5)
	ON_COMMAND(ID_GAUSSIAN_8, &CMy201611256View::OnColorGaussian7)
	// EdgeDetector
	ON_COMMAND(ID_EDGEDETECTOR_PREWITT, &CMy201611256View::OnEdgedetectorPrewitt)
	ON_COMMAND(ID_EDGEDETECTOR_SOBEL, &CMy201611256View::OnEdgedetectorSobel)
	ON_COMMAND(ID_LOG_3, &CMy201611256View::OnLog3)
	ON_COMMAND(ID_LOG_5, &CMy201611256View::OnLog5)
	ON_COMMAND(ID_LOG_9, &CMy201611256View::OnLog9)
	ON_COMMAND(ID_LOAD_AVI, &CMy201611256View::OnLoadAvi)
END_MESSAGE_MAP()

// CMy201611256View 생성/소멸

CMy201611256View::CMy201611256View() noexcept
{
	// TODO: 여기에 생성 코드를 추가합니다.
	rgbBuffer = nullptr;

}

CMy201611256View::~CMy201611256View()
{
	if (rgbBuffer != nullptr)
	{
		for (int i = 0; i < imgHeight; i++)
			delete[] rgbBuffer[i];
		delete[] rgbBuffer;
	}
}

BOOL CMy201611256View::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: CREATESTRUCT cs를 수정하여 여기에서
	//  Window 클래스 또는 스타일을 수정합니다.

	return CView::PreCreateWindow(cs);
}

// CMy201611256View 그리기

// For JPEG
BYTE* LoadJpegFromOpenFile(FILE* fp, BITMAPINFOHEADER* pbh, UINT* pWidth, UINT* pHeight) {
	//파일로부터 JPG 정보 획득
	if (pWidth == NULL || pHeight == NULL) return NULL;
	BYTE* tmp = JpegFile::OpenFileToRGB(fp, pWidth, pHeight);
	if (!tmp) return NULL;
	JpegFile::BGRFromRGB(tmp, *pWidth, *pHeight);
	UINT dw;
	BYTE* pbuf = JpegFile::MakeDwordAlignedBuf(tmp, *pWidth, *pHeight, &dw);
	delete[] tmp;
	if (!pbuf) return NULL;
	JpegFile::VertFlipBuf(pbuf, dw, *pHeight);
	pbh->biSize = sizeof(BITMAPINFOHEADER);
	pbh->biWidth = *pWidth;
	pbh->biHeight = *pHeight;
	pbh->biPlanes = 1;
	pbh->biBitCount = 24;
	pbh->biCompression = BI_RGB;
	pbh->biSizeImage = 0;
	pbh->biXPelsPerMeter = 0;
	pbh->biYPelsPerMeter = 0;
	pbh->biClrUsed = 0;
	pbh->biClrImportant = 0;
	return pbuf;
}

void CMy201611256View::OnDraw(CDC* pDC)
{
	CMy201611256Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;
	if (rgbBuffer != NULL) { //이미지가 들어 있는 경우에 출력
		for (int i = 0; i < imgHeight; i++) { //이미지 사이즈 만큼 돌면서 한 픽셀씩 출력
			for (int j = 0; j < imgWidth; j++) {
				POINT p; //출력할 픽셀의 위치 지정
				p.x = j;
				p.y = i;
				pDC->SetPixel(p, RGB(rgbBuffer[i][j].rgbRed, rgbBuffer[i][j].rgbGreen, rgbBuffer[i][j].rgbBlue));
				//픽셀 위치 p에 RGB값을 출력
			}
		}
	}

	if (rgbBuffer != NULL) {
		for (int i = 0; i < imgHeight; i++) {
			for (int j = 0; j < imgWidth; j++) { //rgbBuffer를 출력했던 2중 For문
				if (viewType == 2) {
					POINT p;
					p.x = j + imgWidth + 10;
					p.y = i;
					pDC->SetPixel(p, RGB(hueBuffer[i][j], hueBuffer[i][j], hueBuffer[i][j]));
					p.x = j + imgWidth + 10;
					p.y = i + imgHeight + 10;
					pDC->SetPixel(p, RGB(satuBuffer[i][j], satuBuffer[i][j], satuBuffer[i][j]));
					p.x = j;
					p.y = i + imgHeight + 10;
					pDC->SetPixel(p, RGB(intenBuffer[i][j], intenBuffer[i][j], intenBuffer[i][j]));
				}
				else if (viewType == 3) {
					POINT p;
					p.x = j + imgWidth + 10;
					p.y = i;
					pDC->SetPixel(p, RGB(transBuffer[i][j], transBuffer[i][j], transBuffer[i][j]));
				}
			}
		}
	}

	if (rgbBuffer != NULL && viewType == 4) {
		if (flag == 1) {
			for (int i = 0; i < imgHeight; i++) {
				for (int j = 0; j < imgWidth; j++) {
					POINT p;
					p.x = j;
					p.y = i;
					pDC->SetPixel(p, RGB(histoBuffer[i][j], histoBuffer[i][j], histoBuffer[i][j]));
				}
			}
		}
		// draw Histogram
		CPaintDC dc(this);
		CClientDC pDC(this);
		pDC.MoveTo(imgWidth + 10, imgHeight+299);
		pDC.LineTo(imgWidth + 10 + 512, imgHeight + 299); // X축
		//pDC.MoveTo(imgWidth + 10, imgHeight + 299); 
		//pDC.LineTo(imgWidth + 10, 299); // Y축

		for (int i = 0; i < 256; i++) {
			CClientDC pDC(this);
			pDC.FillSolidRect(imgWidth + BAR_WIDTH*i + 10, imgHeight + 300 - BAR_HEIGHT*drawHisto[i], BAR_WIDTH - 1, BAR_HEIGHT* drawHisto[i], RGB(0,0,0));
			pDC.SetBkColor(RGB(255, 255, 255));
		}

	}

	if (rgbBuffer != NULL && viewType == 5) {
		for (int i = 0; i < imgHeight; i++) {
			for (int j = 0; j < imgWidth; j++) {
				POINT p; //출력할 픽셀의 위치 지정
				p.x = j + imgWidth + 10;
				p.y = i;
				pDC->SetPixel(p, RGB(smoothBuffer[i][j], smoothBuffer[i][j], smoothBuffer[i][j]));
				//픽셀 위치 p에 RGB값을 출력
			}
		}
	}

	if (rgbBuffer != NULL && viewType == 6) {
		for (int i = 0; i < imgHeight; i++) {
			for (int j = 0; j < imgWidth; j++) {
				POINT p; //출력할 픽셀의 위치 지정
				p.x = j + imgWidth + 10;
				p.y = i;
				pDC->SetPixel(p, RGB(sharpenBuffer[i][j], sharpenBuffer[i][j], sharpenBuffer[i][j]));
				//픽셀 위치 p에 RGB값을 출력
			}
		}
	}

	if (rgbBuffer != NULL && viewType == 7) {
		for (int i = 0; i < imgHeight; i++) {
			for (int j = 0; j < imgWidth; j++) {
				POINT p; //출력할 픽셀의 위치 지정
				p.x = j + imgWidth + 10;
				p.y = i;
				pDC->SetPixel(p, RGB(smoothBuffer_c[i][j].rgbRed, smoothBuffer_c[i][j].rgbGreen, smoothBuffer_c[i][j].rgbBlue));
				//픽셀 위치 p에 RGB값을 출력
			}
		}
	}

	if (rgbBuffer != NULL && viewType == 8) {
		for (int i = 0; i < imgHeight; i++) {
			for (int j = 0; j < imgWidth; j++) {
				POINT p; //출력할 픽셀의 위치 지정
				p.x = j + imgWidth + 10;
				p.y = i;
				pDC->SetPixel(p, RGB(edgeBuffer[i][j], edgeBuffer[i][j], edgeBuffer[i][j]));
				//픽셀 위치 p에 RGB값을 출력
			}
		}
	}

	// TODO: 여기에 원시 데이터에 대한 그리기 코드를 추가합니다.
}


// CMy201611256View 인쇄

BOOL CMy201611256View::OnPreparePrinting(CPrintInfo* pInfo)
{
	// 기본적인 준비
	return DoPreparePrinting(pInfo);
}

void CMy201611256View::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 인쇄하기 전에 추가 초기화 작업을 추가합니다.
}

void CMy201611256View::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 인쇄 후 정리 작업을 추가합니다.
}


// CMy201611256View 진단

#ifdef _DEBUG
void CMy201611256View::AssertValid() const
{
	CView::AssertValid();
}

void CMy201611256View::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CMy201611256Doc* CMy201611256View::GetDocument() const // 디버그되지 않은 버전은 인라인으로 지정됩니다.
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMy201611256Doc)));
	return (CMy201611256Doc*)m_pDocument;
}
#endif //_DEBUG


// CMy201611256View 메시지 처리기


void CMy201611256View::OnImgLoadBmp()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
	//1. 파일 다이얼로그로부터 BMP 파일 입력
	CFileDialog dlg(TRUE, ".bmp", NULL, NULL, "Bitmap File (*.bmp)|*.bmp||");
	if (IDOK != dlg.DoModal())
		return;
	CString filename = dlg.GetPathName();
	if (rgbBuffer != NULL) { //이미 할당된 경우, 메모리 해제
		for (int i = 0; i < imgHeight; i++)
			delete[] rgbBuffer[i];
		delete[] rgbBuffer;
	}
	//2. 파일을 오픈하여 영상 정보 획득
	CFile file;
	file.Open(filename, CFile::modeRead);
	file.Read(&bmpHeader, sizeof(BITMAPFILEHEADER));
	file.Read(&bmpInfo, sizeof(BITMAPINFOHEADER));
	imgWidth = bmpInfo.biWidth;
	imgHeight = bmpInfo.biHeight;
	//3. 이미지를 저장할 버퍼 할당 (2차원 배열) [이미지 높이 * 이미지 너비 만큼 할당] 
	rgbBuffer = new RGBQUAD * [imgHeight];
	for (int i = 0; i < imgHeight; i++)
		rgbBuffer[i] = new RGBQUAD[imgWidth];
	//4. 이미지의 너비가 4의 배수인지 체크
	// BMP조건 가로는 4byte씩 이어야 한다. 
	// 한 픽셀이3바이트(R,G,B)씩이니깐. 가로(m_width) * 3이4의 배수인가 아닌가를 알아야한다. 
	// b4byte : 4byte배수인지 아닌지를 안다. 
	// upbyte : 4byte배수에 모자라는 바이트다.
	bool b4byte = false;
	int upbyte = 0;
	if ((imgWidth * 3) % 4 == 0) {
		// 4의배수로떨어지는경우. 
		b4byte = true;
		upbyte = 0;
	}
	else {
		// 4의배수로떨어지지않는경우. 
		b4byte = false;
		upbyte = 4 - (imgWidth * 3) % 4;
	}
	//5. 픽셀 데이터를 파일로부터 읽어옴
	BYTE data[3];
	for (int i = 0; i < imgHeight; i++) {
		for (int j = 0; j < imgWidth; j++) {
			file.Read(&data, 3);
			//이미지가 거꾸로 저장되어 있기 때문에 거꾸로 읽어옴
			rgbBuffer[imgHeight - i - 1][j].rgbBlue = data[0];
			rgbBuffer[imgHeight - i - 1][j].rgbGreen = data[1];
			rgbBuffer[imgHeight - i - 1][j].rgbRed = data[2];
		}
		if (b4byte == false)
		{
			// 가로가4byte배수가아니면쓰레기값을읽는다. 
			file.Read(&data, upbyte);
		}
	}
	file.Close(); //파일 닫기
	Invalidate(TRUE); //화면 갱신
}

void CMy201611256View::OnLoadHsi()
{
	if (rgbBuffer == NULL)
		OnImgLoadJpeg(); //rgbBuffer에 데이터가 없는 경우, 로드 함수를 호출하여 이미지 획득
	hueBuffer = new float* [imgHeight];
	satuBuffer = new float* [imgHeight];
	intenBuffer = new float* [imgHeight];
	for (int i = 0; i < imgHeight; i++)
	{
		hueBuffer[i] = new float[imgWidth];
		satuBuffer[i] = new float[imgWidth];
		intenBuffer[i] = new float[imgWidth];
	}

	for (int i = 0; i < imgHeight; i++) {
		for (int j = 0; j < imgWidth; j++) {
			float r = rgbBuffer[i][j].rgbRed;
			float g = rgbBuffer[i][j].rgbGreen;
			float b = rgbBuffer[i][j].rgbBlue;
			intenBuffer[i][j] = (r + g + b) / (float)(3 * 255); //intensity
			float total = r + g + b;
			r = r / total; g = g / total; b = b / total;
			satuBuffer[i][j] = 1 - 3 * (r > g ? (g > b ? b : g) : (r > b ? b : r));
			if (r == g && g == b) {
				hueBuffer[i][j] = 0; satuBuffer[i][j] = 0;
			}
			else {
				total = (0.5 * (r - g + r - b) / sqrt((r - g) * (r - g) + (r - b) * (g - b)));
				hueBuffer[i][j] = acos((double)total);
				if (b > g) {
					hueBuffer[i][j] = 6.28 - hueBuffer[i][j];
				}
			}
		}
	}

	for (int i = 0; i < imgHeight; i++) {
		for (int j = 0; j < imgWidth; j++) {
			hueBuffer[i][j] = hueBuffer[i][j] * 255 / (3.14 * 2);
			satuBuffer[i][j] = satuBuffer[i][j] * 255;
			intenBuffer[i][j] = intenBuffer[i][j] * 255;
		}
	}

	viewType = 2;
	Invalidate(FALSE);
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
}

void CMy201611256View::OnImgLoadJpeg()
{
	CFileDialog dlg(TRUE, ".jpeg", NULL, NULL, "Jpeg File (*.jpeg)|*.jpeg||");
	if (IDOK != dlg.DoModal()) return;
	CString filename = dlg.GetPathName();
	if (rgbBuffer != NULL) { //이미 할당된 경우, 메모리 해제
		for (int i = 0; i < imgHeight; i++)
			delete[] rgbBuffer[i];
		delete[] rgbBuffer;
	}
	FILE* fp = fopen( filename, "rb");
	BITMAPINFOHEADER pbh;
	UINT w, h;
	BYTE* pbuf = LoadJpegFromOpenFile(fp, &pbh, &w, &h);
	imgWidth = (int)w;
	imgHeight = (int)h;
	rgbBuffer = new RGBQUAD * [imgHeight];
	for (int i = 0; i < imgHeight; i++)
		rgbBuffer[i] = new RGBQUAD[imgWidth];
	int dw = WIDTHBYTES(imgWidth * 24);
	for (int i = 0; i < imgHeight; i++) {
		for (int j = 0; j < imgWidth; j++) {
			rgbBuffer[imgHeight - i - 1][j].rgbBlue = pbuf[i * dw + j * 3 + 0];
			rgbBuffer[imgHeight - i - 1][j].rgbGreen = pbuf[i * dw + j * 3 + 1];
			rgbBuffer[imgHeight - i - 1][j].rgbRed = pbuf[i * dw + j * 3 + 2];
		}
	}
	delete[] pbuf;
	fclose(fp);
	Invalidate(TRUE);

	// TODO: 여기에 명령 처리기 코드를 추가합니다.
}

void CMy201611256View::OnGreylvtransNegative()
{
	if (rgbBuffer == NULL)
		OnImgLoadJpeg();
	transBuffer = new float* [imgHeight];
	for (int i = 0; i < imgHeight; i++)
		transBuffer[i] = new float[imgWidth];

	for (int i = 0; i < imgHeight; i++) {
		for (int j = 0; j < imgWidth; j++) {
			float r = rgbBuffer[i][j].rgbRed;
			float g = rgbBuffer[i][j].rgbGreen;
			float b = rgbBuffer[i][j].rgbBlue;

			transBuffer[i][j] = 255 - (r + g + b)/3;
		}
	}

	viewType = 3;
	Invalidate(FALSE);
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
}

void CMy201611256View::OnGreylvtransGamma10()
{
	if (rgbBuffer == NULL)
		OnImgLoadJpeg();
	transBuffer = new float* [imgHeight];
	for (int i = 0; i < imgHeight; i++)
		transBuffer[i] = new float[imgWidth];

	for (int i = 0; i < imgHeight; i++) {
		for (int j = 0; j < imgWidth; j++) {
			float r = rgbBuffer[i][j].rgbRed;
			float g = rgbBuffer[i][j].rgbGreen;
			float b = rgbBuffer[i][j].rgbBlue;

			float input_r = (r + g + b) / (3 * 255);
			transBuffer[i][j] = pow(input_r, 10);
			transBuffer[i][j] *= 255;
		}
	}
	viewType = 3;
	Invalidate(FALSE);
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
}

void CMy201611256View::OnGreylvtransGamma2()
{
	if (rgbBuffer == NULL)
		OnImgLoadJpeg();
	transBuffer = new float* [imgHeight];
	for (int i = 0; i < imgHeight; i++)
		transBuffer[i] = new float[imgWidth];

	for (int i = 0; i < imgHeight; i++) {
		for (int j = 0; j < imgWidth; j++) {
			float r = rgbBuffer[i][j].rgbRed;
			float g = rgbBuffer[i][j].rgbGreen;
			float b = rgbBuffer[i][j].rgbBlue;

			float input_r = (r + g + b) / (3*255);
			transBuffer[i][j] = input_r * input_r;
			transBuffer[i][j] *= 255.0;
		}
	}

	viewType = 3;
	Invalidate(FALSE);
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
}

void CMy201611256View::OnGreylvtransGammahalf()
{
	if (rgbBuffer == NULL)
		OnImgLoadJpeg();
	transBuffer = new float* [imgHeight];
	for (int i = 0; i < imgHeight; i++)
		transBuffer[i] = new float[imgWidth];

	for (int i = 0; i < imgHeight; i++) {
		for (int j = 0; j < imgWidth; j++) {
			float r = rgbBuffer[i][j].rgbRed;
			float g = rgbBuffer[i][j].rgbGreen;
			float b = rgbBuffer[i][j].rgbBlue;

			float input_r = (r + g + b) / (3 * 255);
			transBuffer[i][j] = pow(input_r, 0.5);
			transBuffer[i][j] *= 255;
		}
	}
	viewType = 3;
	Invalidate(FALSE);
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
}

void CMy201611256View::OnGreylvtransGamma0dot1()
{
	if (rgbBuffer == NULL)
		OnImgLoadJpeg();
	transBuffer = new float* [imgHeight];
	for (int i = 0; i < imgHeight; i++)
		transBuffer[i] = new float[imgWidth];

	for (int i = 0; i < imgHeight; i++) {
		for (int j = 0; j < imgWidth; j++) {
			float r = rgbBuffer[i][j].rgbRed;
			float g = rgbBuffer[i][j].rgbGreen;
			float b = rgbBuffer[i][j].rgbBlue;

			float input_r = (r + g + b) / (3 * 255);
			transBuffer[i][j] = pow(input_r, 0.1);
			transBuffer[i][j] *= 255;
		}
	}

	viewType = 3;
	Invalidate(FALSE);
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
}

void CMy201611256View::OnOriginalHisto()
{
	if (rgbBuffer == NULL)
		OnImgLoadJpeg();

	for (int i = 0; i < imgHeight; i++) {
		for (int j = 0; j < imgWidth; j++) {
			float r = rgbBuffer[i][j].rgbRed;
			float g = rgbBuffer[i][j].rgbGreen;
			float b = rgbBuffer[i][j].rgbBlue;

			float grey = (r + g + b) / 3;
			drawHisto[(int)grey] += 0.02;
		}
	}
	viewType = 4;
	Invalidate(FALSE);
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
}

void CMy201611256View::OnHistogramEqualization()
{
	if (rgbBuffer == NULL)
		OnImgLoadJpeg();

	int sum = imgHeight * imgWidth;
	int cnt_level[256] = { 0, };
	int sum_level[256] = { 0, };

	for (int i = 0; i < imgHeight; i++) {
		for (int j = 0; j < imgWidth; j++) {
			float r = rgbBuffer[i][j].rgbRed;
			float g = rgbBuffer[i][j].rgbGreen;
			float b = rgbBuffer[i][j].rgbBlue;

			float grey = (r + g + b) / 3;
			cnt_level[(int)grey]++;
		}
	}
	sum_level[0] = cnt_level[0];
	for (int i = 1; i < 256; i++) {
		sum_level[i] = sum_level[i - 1] + cnt_level[i];
	}
	for (int i = 0; i < 256; i++) {
		lookUpTable[i] = round(sum_level[i] * 255 / sum);
	}

	histoBuffer = new float* [imgHeight];
	for (int i = 0; i < imgHeight; i++)
		histoBuffer[i] = new float[imgWidth];

	for (int i = 0; i < imgHeight; i++) {
		for (int j = 0; j < imgWidth; j++) {
			float r = rgbBuffer[i][j].rgbRed;
			float g = rgbBuffer[i][j].rgbGreen;
			float b = rgbBuffer[i][j].rgbBlue;

			float grey = (r + g + b) / 3;
			float change_grey = lookUpTable[(int)grey];

			histoBuffer[i][j] = change_grey;
			drawHisto[(int)histoBuffer[i][j]] += 0.02;
		}
	}
	flag = 1;
	viewType = 4;
	Invalidate(FALSE);
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
}

void CMy201611256View::OnAverage3()
{
	if (rgbBuffer == NULL)
		OnImgLoadJpeg();
	smoothBuffer = new float* [imgHeight];
	for (int i = 0; i < imgHeight; i++)
		smoothBuffer[i] = new float[imgWidth];

	for (int i = 0; i < imgHeight - 2; i++) {
		for (int j = 0; j < imgWidth - 2; j++) {
			float average_r = 0;
			float average_g = 0;
			float average_b = 0;
			for (int k = 0; k < 3; k++) {
				for (int l = 0; l < 3; l++) {
					average_r += rgbBuffer[i + k][j + l].rgbRed;
					average_g += rgbBuffer[i + k][j + l].rgbGreen;
					average_b += rgbBuffer[i + k][j + l].rgbBlue;
				}
			}

			average_r /= 9;
			average_g /= 9;
			average_b /= 9;


			smoothBuffer[i + 1][j + 1] = (average_r + average_g + average_b) / 3;
		}
	}

	viewType = 5;
	Invalidate(FALSE);
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
}

void CMy201611256View::OnAverage5()
{
	if (rgbBuffer == NULL)
		OnImgLoadJpeg();
	smoothBuffer = new float* [imgHeight];
	for (int i = 0; i < imgHeight; i++)
		smoothBuffer[i] = new float[imgWidth];

	for (int i = 0; i < imgHeight - 4; i++) {
		for (int j = 0; j < imgWidth - 4; j++) {
			float average_r = 0;
			float average_g = 0;
			float average_b = 0;
			for (int k = 0; k < 5; k++) {
				for (int l = 0; l < 5; l++) {
					average_r += rgbBuffer[i + k][j + l].rgbRed;
					average_g += rgbBuffer[i + k][j + l].rgbGreen;
					average_b += rgbBuffer[i + k][j + l].rgbBlue;
				}
			}

			average_r /= 25;
			average_g /= 25;
			average_b /= 25;


			smoothBuffer[i + 2][j + 2] = (average_r + average_g + average_b) / 3;
		}
	}

	viewType = 5;
	Invalidate(FALSE);
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
}

void CMy201611256View::OnAverage7()
{
	if (rgbBuffer == NULL)
		OnImgLoadJpeg();
	smoothBuffer = new float* [imgHeight];
	for (int i = 0; i < imgHeight; i++)
		smoothBuffer[i] = new float[imgWidth];

	for (int i = 0; i < imgHeight - 6; i++) {
		for (int j = 0; j < imgWidth - 6; j++) {
			float average_r = 0;
			float average_g = 0;
			float average_b = 0;
			for (int k = 0; k < 7; k++) {
				for (int l = 0; l < 7; l++) {
					average_r += rgbBuffer[i + k][j + l].rgbRed;
					average_g += rgbBuffer[i + k][j + l].rgbGreen;
					average_b += rgbBuffer[i + k][j + l].rgbBlue;
				}
			}

			average_r /= 49;
			average_g /= 49;
			average_b /= 49;


			smoothBuffer[i + 3][j + 3] = (average_r + average_g + average_b) / 3;
		}
	}

	viewType = 5;
	Invalidate(FALSE);
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
}

void CMy201611256View::OnMedian3()
{
	if (rgbBuffer == NULL)
		OnImgLoadJpeg();
	smoothBuffer = new float* [imgHeight];
	for (int i = 0; i < imgHeight; i++)
		smoothBuffer[i] = new float[imgWidth];
	
	for (int i = 0; i < imgHeight - 2; i++) {
		for (int j = 0; j < imgWidth - 2; j++) {
			float average_r;
			float average_g;
			float average_b;
			int a = 0;
			float median[9] = { 0, };
			for (int k = 0; k < 3; k++) {
				for (int l = 0; l < 3; l++) {
					average_r = rgbBuffer[i + k][j + l].rgbRed;
					average_g = rgbBuffer[i + k][j + l].rgbGreen;
					average_b = rgbBuffer[i + k][j + l].rgbBlue;
					median[a] = (average_r + average_g + average_b) / 3;
					a++;
				}
			}
			for (int k = 0; k < 9; k++){
				for (int l = 0; l < 9 - k; l++){
					if (median[l] < median[l + 1]){
						int temp = median[l];
						median[l] = median[l + 1];
						median[l + 1] = temp;
					}
				}
			}
			smoothBuffer[i + 1][j + 1] = median[4];
		}
	}

	viewType = 5;
	Invalidate(FALSE);
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
}

void CMy201611256View::OnMedian5()
{
	if (rgbBuffer == NULL)
		OnImgLoadJpeg();
	smoothBuffer = new float* [imgHeight];
	for (int i = 0; i < imgHeight; i++)
		smoothBuffer[i] = new float[imgWidth];

	for (int i = 0; i < imgHeight - 4; i++) {
		for (int j = 0; j < imgWidth - 4; j++) {
			float average_r;
			float average_g;
			float average_b;
			int a = 0;
			float median[25] = { 0, };
			for (int k = 0; k < 5; k++) {
				for (int l = 0; l < 5; l++) {
					average_r = rgbBuffer[i + k][j + l].rgbRed;
					average_g = rgbBuffer[i + k][j + l].rgbGreen;
					average_b = rgbBuffer[i + k][j + l].rgbBlue;
					median[a] = (average_r + average_g + average_b) / 3;
					a++;
				}
			}
			for (int k = 0; k < 25; k++) {
				for (int l = 0; l < 25 - k; l++) {
					if (median[l] < median[l + 1]) {
						int temp = median[l];
						median[l] = median[l + 1];
						median[l + 1] = temp;
					}
				}
			}
			smoothBuffer[i + 2][j + 2] = median[12];
		}
	}

	viewType = 5;
	Invalidate(FALSE);
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
}

void CMy201611256View::OnMedian7()
{
	if (rgbBuffer == NULL)
		OnImgLoadJpeg();
	smoothBuffer = new float* [imgHeight];
	for (int i = 0; i < imgHeight; i++)
		smoothBuffer[i] = new float[imgWidth];

	for (int i = 0; i < imgHeight - 6; i++) {
		for (int j = 0; j < imgWidth - 6; j++) {
			float average_r;
			float average_g;
			float average_b;
			int a = 0;
			float median[49] = { 0, };
			for (int k = 0; k < 7; k++) {
				for (int l = 0; l < 7; l++) {
					average_r = rgbBuffer[i + k][j + l].rgbRed;
					average_g = rgbBuffer[i + k][j + l].rgbGreen;
					average_b = rgbBuffer[i + k][j + l].rgbBlue;
					median[a] = (average_r + average_g + average_b) / 3;
					a++;
				}
			}
			for (int k = 0; k < 49; k++) {
				for (int l = 0; l < 49 - k; l++) {
					if (median[l] < median[l + 1]) {
						int temp = median[l];
						median[l] = median[l + 1];
						median[l + 1] = temp;
					}
				}
			}
			smoothBuffer[i + 3][j + 3] = median[24];
		}
	}

	viewType = 5;
	Invalidate(FALSE);
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
}

void CMy201611256View::OnGaussian3()
{
	if (rgbBuffer == NULL)
		OnImgLoadJpeg();
	smoothBuffer = new float* [imgHeight];
	for (int i = 0; i < imgHeight; i++)
		smoothBuffer[i] = new float[imgWidth];

	int Gauss_3[3][3] = {
		{1,2,1},
		{2,4,2},
		{1,2,1}
	};

	for (int i = 0; i < imgHeight - 2; i++) {
		for (int j = 0; j < imgWidth - 2; j++) {
			float average_r = 0;
			float average_g = 0;
			float average_b = 0;
			for (int k = 0; k < 3; k++) {
				for (int l = 0; l < 3; l++) {
					average_r += Gauss_3[k][l] * rgbBuffer[i + k][j + l].rgbRed;
					average_g += Gauss_3[k][l] * rgbBuffer[i + k][j + l].rgbGreen;
					average_b += Gauss_3[k][l] * rgbBuffer[i + k][j + l].rgbBlue;
				}
			}


			average_r /= 16;
			average_g /= 16;
			average_b /= 16;

			if (average_r < 0) {
				average_r = 0;
			}
			if (average_g < 0) {
				average_g = 0;
			}
			if (average_b < 0) {
				average_b = 0;
			}
			if (average_r > 255) {
				average_r = 255;
			}
			if (average_g > 255) {
				average_g = 255;
			}
			if (average_b > 255) {
				average_b = 255;
			}

			smoothBuffer[i + 1][j + 1] = (average_r + average_g + average_b) / 3;
		}
	}

	viewType = 5;
	Invalidate(FALSE);
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
}

void CMy201611256View::OnGaussian5()
{
	if (rgbBuffer == NULL)
		OnImgLoadJpeg();
	smoothBuffer = new float* [imgHeight];
	for (int i = 0; i < imgHeight; i++)
		smoothBuffer[i] = new float[imgWidth];

	int Gauss_5[5][5] = {
		{1, 4, 7, 4, 1},
		{4, 16, 26, 16, 4},
		{7, 26, 41, 26, 7},
		{4, 16, 26, 16, 4},
		{1, 4, 7, 4, 1}
	};

	for (int i = 0; i < imgHeight - 4; i++) {
		for (int j = 0; j < imgWidth - 4; j++) {
			float average_r = 0;
			float average_g = 0;
			float average_b = 0;
			for (int k = 0; k < 5; k++) {
				for (int l = 0; l < 5; l++) {
					average_r += Gauss_5[k][l] * rgbBuffer[i + k][j + l].rgbRed;
					average_g += Gauss_5[k][l] * rgbBuffer[i + k][j + l].rgbGreen;
					average_b += Gauss_5[k][l] * rgbBuffer[i + k][j + l].rgbBlue;
				}
			}


			average_r /= 273;
			average_g /= 273;
			average_b /= 273;

			if (average_r < 0) {
				average_r = 0;
			}
			if (average_g < 0) {
				average_g = 0;
			}
			if (average_b < 0) {
				average_b = 0;
			}
			if (average_r > 255) {
				average_r = 255;
			}
			if (average_g > 255) {
				average_g = 255;
			}
			if (average_b > 255) {
				average_b = 255;
			}

			smoothBuffer[i + 2][j + 2] = (average_r + average_g + average_b) / 3;
		}
	}

	viewType = 5;
	Invalidate(FALSE);
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
}

void CMy201611256View::OnGaussian7()
{
	if (rgbBuffer == NULL)
		OnImgLoadJpeg();
	smoothBuffer = new float* [imgHeight];
	for (int i = 0; i < imgHeight; i++)
		smoothBuffer[i] = new float[imgWidth];

	int Gauss_7[7][7] = {
		{0, 0, 1, 2, 1, 0, 0},
		{0, 3, 13, 22, 13, 3, 0},
		{1, 13, 59, 97, 59, 13, 1},
		{2, 22, 97, 159, 97, 22, 2},
		{1, 13, 59, 97, 59, 13, 1},
		{0, 3, 13, 22, 13, 3, 0},
		{0, 0, 1, 2, 1, 0, 0},
	};

	for (int i = 0; i < imgHeight - 6; i++) {
		for (int j = 0; j < imgWidth - 6; j++) {
			float average_r = 0;
			float average_g = 0;
			float average_b = 0;
			for (int k = 0; k < 7; k++) {
				for (int l = 0; l < 7; l++) {
					average_r += Gauss_7[k][l] * rgbBuffer[i + k][j + l].rgbRed;
					average_g += Gauss_7[k][l] * rgbBuffer[i + k][j + l].rgbGreen;
					average_b += Gauss_7[k][l] * rgbBuffer[i + k][j + l].rgbBlue;
				}
			}


			average_r /= 1003;
			average_g /= 1003;
			average_b /= 1003;

			if (average_r < 0) {
				average_r = 0;
			}
			if (average_g < 0) {
				average_g = 0;
			}
			if (average_b < 0) {
				average_b = 0;
			}
			if (average_r > 255) {
				average_r = 255;
			}
			if (average_g > 255) {
				average_g = 255;
			}
			if (average_b > 255) {
				average_b = 255;
			}

			smoothBuffer[i + 3][j + 3] = (average_r + average_g + average_b) / 3;
		}
	}

	viewType = 5;
	Invalidate(FALSE);
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
}

void CMy201611256View::OnMask4A12()
{
	if (rgbBuffer == NULL)
		OnImgLoadJpeg();
	sharpenBuffer = new float* [imgHeight];
	for (int i = 0; i < imgHeight; i++)
		sharpenBuffer[i] = new float[imgWidth];

	float Mask4[3][3] = {
		{0, -1, 0},
		{-1, 5.2, -1},
		{0, -1, 0}
	};

	for (int i = 0; i < imgHeight - 2; i++) {
		for (int j = 0; j < imgWidth - 2; j++) {
			float average_r = 0;
			float average_g = 0;
			float average_b = 0;
			for (int k = 0; k < 3; k++) {
				for (int l = 0; l < 3; l++) {
					average_r += Mask4[k][l] * rgbBuffer[i + k][j + l].rgbRed;
					average_g += Mask4[k][l] * rgbBuffer[i + k][j + l].rgbGreen;
					average_b += Mask4[k][l] * rgbBuffer[i + k][j + l].rgbBlue;
				}
			}

			if (average_r < 0) {
				average_r = 0;
			}
			if (average_g < 0) {
				average_g = 0;
			}
			if (average_b < 0) {
				average_b = 0;
			}
			if (average_r > 255) {
				average_r = 255;
			}
			if (average_g > 255) {
				average_g = 255;
			}
			if (average_b > 255) {
				average_b = 255;
			}
			sharpenBuffer[i + 1][j + 1] = (average_r + average_g + average_b) / 3;
		}
	}

	viewType = 6;
	Invalidate(FALSE);

	// TODO: 여기에 명령 처리기 코드를 추가합니다.
}

void CMy201611256View::OnMask4A15()
{
	if (rgbBuffer == NULL)
		OnImgLoadJpeg();
	sharpenBuffer = new float* [imgHeight];
	for (int i = 0; i < imgHeight; i++)
		sharpenBuffer[i] = new float[imgWidth];

	float Mask4[3][3] = {
		{0, -1, 0},
		{-1, 5.5, -1},
		{0, -1, 0}
	};

	for (int i = 0; i < imgHeight - 2; i++) {
		for (int j = 0; j < imgWidth - 2; j++) {
			float average_r = 0;
			float average_g = 0;
			float average_b = 0;
			for (int k = 0; k < 3; k++) {
				for (int l = 0; l < 3; l++) {
					average_r += Mask4[k][l] * rgbBuffer[i + k][j + l].rgbRed;
					average_g += Mask4[k][l] * rgbBuffer[i + k][j + l].rgbGreen;
					average_b += Mask4[k][l] * rgbBuffer[i + k][j + l].rgbBlue;
				}
			}

			if (average_r < 0) {
				average_r = 0;
			}
			if (average_g < 0) {
				average_g = 0;
			}
			if (average_b < 0) {
				average_b = 0;
			}
			if (average_r > 255) {
				average_r = 255;
			}
			if (average_g > 255) {
				average_g = 255;
			}
			if (average_b > 255) {
				average_b = 255;
			}

			sharpenBuffer[i + 1][j + 1] = (average_r + average_g + average_b) / 3;
		}
	}

	viewType = 6;
	Invalidate(FALSE);
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
}

void CMy201611256View::OnMask8A12()
{
	if (rgbBuffer == NULL)
		OnImgLoadJpeg();
	sharpenBuffer = new float* [imgHeight];
	for (int i = 0; i < imgHeight; i++)
		sharpenBuffer[i] = new float[imgWidth];

	float Mask8[3][3] = {
		{-1, -1, -1},
		{-1, 9.2, -1},
		{-1, -1, -1}
	};

	for (int i = 0; i < imgHeight - 2; i++) {
		for (int j = 0; j < imgWidth - 2; j++) {
			float average_r = 0;
			float average_g = 0;
			float average_b = 0;
			for (int k = 0; k < 3; k++) {
				for (int l = 0; l < 3; l++) {
					average_r += Mask8[k][l] * rgbBuffer[i + k][j + l].rgbRed;
					average_g += Mask8[k][l] * rgbBuffer[i + k][j + l].rgbGreen;
					average_b += Mask8[k][l] * rgbBuffer[i + k][j + l].rgbBlue;
				}
			}

			if (average_r < 0) {
				average_r = 0;
			}
			if (average_g < 0) {
				average_g = 0;
			}
			if (average_b < 0) {
				average_b = 0;
			}
			if (average_r > 255) {
				average_r = 255;
			}
			if (average_g > 255) {
				average_g = 255;
			}
			if (average_b > 255) {
				average_b = 255;
			}

			sharpenBuffer[i + 1][j + 1] = (average_r + average_g + average_b) / 3;
		}
	}

	viewType = 6;
	Invalidate(FALSE);
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
}

void CMy201611256View::OnMask8A15()
{
	if (rgbBuffer == NULL)
		OnImgLoadJpeg();
	sharpenBuffer = new float* [imgHeight];
	for (int i = 0; i < imgHeight; i++)
		sharpenBuffer[i] = new float[imgWidth];

	float Mask8[3][3] = {
		{-1, -1, -1},
		{-1, 9.5, -1},
		{-1, -1, -1}
	};

	for (int i = 0; i < imgHeight - 2; i++) {
		for (int j = 0; j < imgWidth - 2; j++) {
			float average_r = 0;
			float average_g = 0;
			float average_b = 0;
			for (int k = 0; k < 3; k++) {
				for (int l = 0; l < 3; l++) {
					average_r += Mask8[k][l] * rgbBuffer[i + k][j + l].rgbRed;
					average_g += Mask8[k][l] * rgbBuffer[i + k][j + l].rgbGreen;
					average_b += Mask8[k][l] * rgbBuffer[i + k][j + l].rgbBlue;
				}
			}

			if (average_r < 0) {
				average_r = 0;
			}
			if (average_g < 0) {
				average_g = 0;
			}
			if (average_b < 0) {
				average_b = 0;
			}
			if (average_r > 255) {
				average_r = 255;
			}
			if (average_g > 255) {
				average_g = 255;
			}
			if (average_b > 255) {
				average_b = 255;
			}

			sharpenBuffer[i + 1][j + 1] = (average_r + average_g + average_b) / 3;
		}
	}

	viewType = 6;
	Invalidate(FALSE);
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
}

void CMy201611256View::OnColorMedian3()
{
	if (rgbBuffer == NULL)
		OnImgLoadJpeg();
	smoothBuffer_c = new RGBQUAD* [imgHeight];
	for (int i = 0; i < imgHeight; i++)
		smoothBuffer_c[i] = new RGBQUAD[imgWidth];

	float median_r[9] = { 0, };
	float median_g[9] = { 0, };
	float median_b[9] = { 0, };

	for (int i = 0; i < imgHeight - 2; i++) {
		for (int j = 0; j < imgWidth - 2; j++) {
			float average_r;
			float average_g;
			float average_b;
			int a = 0;
			for (int k = 0; k < 3; k++) {
				for (int l = 0; l < 3; l++) {
					average_r = rgbBuffer[i + k][j + l].rgbRed;
					average_g = rgbBuffer[i + k][j + l].rgbGreen;
					average_b = rgbBuffer[i + k][j + l].rgbBlue;
					median_r[a] = average_r;
					median_g[a] = average_g;
					median_b[a] = average_b;
					a++;
				}
			}
			for (int k = 0; k < 9; k++) {
				for (int l = 0; l < 9 - k; l++) {
					if (median_r[l] < median_r[l + 1]) {
						float temp = median_r[l];
						median_r[l] = median_r[l + 1];
						median_r[l + 1] = temp;
					}
				}
			}
			for (int k = 0; k < 9; k++) {
				for (int l = 0; l < 9 - k; l++) {
					if (median_g[l] < median_g[l + 1]) {
						float temp = median_g[l];
						median_g[l] = median_g[l + 1];
						median_g[l + 1] = temp;
					}
				}
			}
			for (int k = 0; k < 9; k++) {
				for (int l = 0; l < 9 - k; l++) {
					if (median_b[l] < median_b[l + 1]) {
						float temp = median_b[l];
						median_b[l] = median_b[l + 1];
						median_b[l + 1] = temp;
					}
				}
			}
			smoothBuffer_c[i + 1][j + 1].rgbRed = median_r[4];
			smoothBuffer_c[i + 1][j + 1].rgbGreen = median_g[4];
			smoothBuffer_c[i + 1][j + 1].rgbBlue = median_b[4];
		}
	}

	viewType = 7;
	Invalidate(FALSE);
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
}

void CMy201611256View::OnColorMedian5()
{
	if (rgbBuffer == NULL)
		OnImgLoadJpeg();
	smoothBuffer_c = new RGBQUAD * [imgHeight];
	for (int i = 0; i < imgHeight; i++)
		smoothBuffer_c[i] = new RGBQUAD[imgWidth];

	for (int i = 0; i < imgHeight - 4; i++) {
		for (int j = 0; j < imgWidth - 4; j++) {
			float average_r;
			float average_g;
			float average_b;
			int a = 0;
			float median_r[25] = { 0, };
			float median_g[25] = { 0, };
			float median_b[25] = { 0, };
			for (int k = 0; k < 5; k++) {
				for (int l = 0; l < 5; l++) {
					average_r = rgbBuffer[i + k][j + l].rgbRed;
					average_g = rgbBuffer[i + k][j + l].rgbGreen;
					average_b = rgbBuffer[i + k][j + l].rgbBlue;
					median_r[a] = average_r;
					median_g[a] = average_g;
					median_b[a] = average_b;
					a++;
				}
			}
			for (int k = 0; k < 25; k++) {
				for (int l = 0; l < 25 - k; l++) {
					if (median_r[l] < median_r[l + 1]) {
						float temp = median_r[l];
						median_r[l] = median_r[l + 1];
						median_r[l + 1] = temp;
					}
				}
			}
			for (int k = 0; k < 25; k++) {
				for (int l = 0; l < 25 - k; l++) {
					if (median_g[l] < median_g[l + 1]) {
						float temp = median_g[l];
						median_g[l] = median_g[l + 1];
						median_g[l + 1] = temp;
					}
				}
			}
			for (int k = 0; k < 25; k++) {
				for (int l = 0; l < 25 - k; l++) {
					if (median_b[l] < median_b[l + 1]) {
						float temp = median_b[l];
						median_b[l] = median_b[l + 1];
						median_b[l + 1] = temp;
					}
				}
			}
			smoothBuffer_c[i + 2][j + 2].rgbRed = median_r[12];
			smoothBuffer_c[i + 2][j + 2].rgbGreen = median_g[12];
			smoothBuffer_c[i + 2][j + 2].rgbBlue = median_b[12];
		}
	}

	viewType = 7;
	Invalidate(FALSE);
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
}

void CMy201611256View::OnColorMedian7()
{
	if (rgbBuffer == NULL)
		OnImgLoadJpeg();
	smoothBuffer_c = new RGBQUAD * [imgHeight];
	for (int i = 0; i < imgHeight; i++)
		smoothBuffer_c[i] = new RGBQUAD[imgWidth];

	for (int i = 0; i < imgHeight - 6; i++) {
		for (int j = 0; j < imgWidth - 6; j++) {
			float average_r;
			float average_g;
			float average_b;
			int a = 0;
			float median_r[49] = { 0, };
			float median_g[49] = { 0, };
			float median_b[49] = { 0, };
			for (int k = 0; k < 7; k++) {
				for (int l = 0; l < 7; l++) {
					average_r = rgbBuffer[i + k][j + l].rgbRed;
					average_g = rgbBuffer[i + k][j + l].rgbGreen;
					average_b = rgbBuffer[i + k][j + l].rgbBlue;
					median_r[a] = average_r;
					median_g[a] = average_g;
					median_b[a] = average_b;
					a++;
				}
			}
			for (int k = 0; k < 49; k++) {
				for (int l = 0; l < 49 - k; l++) {
					if (median_r[l] < median_r[l + 1]) {
						float temp = median_r[l];
						median_r[l] = median_r[l + 1];
						median_r[l + 1] = temp;
					}
				}
			}
			for (int k = 0; k < 49; k++) {
				for (int l = 0; l < 49 - k; l++) {
					if (median_g[l] < median_g[l + 1]) {
						float temp = median_g[l];
						median_g[l] = median_g[l + 1];
						median_g[l + 1] = temp;
					}
				}
			}
			for (int k = 0; k < 49; k++) {
				for (int l = 0; l < 49 - k; l++) {
					if (median_b[l] < median_b[l + 1]) {
						float temp = median_b[l];
						median_b[l] = median_b[l + 1];
						median_b[l + 1] = temp;
					}
				}
			}
			smoothBuffer_c[i + 3][j + 3].rgbRed = median_r[24];
			smoothBuffer_c[i + 3][j + 3].rgbGreen = median_g[24];
			smoothBuffer_c[i + 3][j + 3].rgbBlue = median_b[24];
		}
	}

	viewType = 7;
	Invalidate(FALSE);
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
}

void CMy201611256View::OnColorAverage3()
{
	if (rgbBuffer == NULL)
		OnImgLoadJpeg();
	smoothBuffer_c = new RGBQUAD* [imgHeight];
	for (int i = 0; i < imgHeight; i++)
		smoothBuffer_c[i] = new RGBQUAD[imgWidth];

	for (int i = 0; i < imgHeight - 2; i++) {
		for (int j = 0; j < imgWidth - 2; j++) {
			float average_r = 0;
			float average_g = 0;
			float average_b = 0;
			for (int k = 0; k < 3; k++) {
				for (int l = 0; l < 3; l++) {
					average_r += rgbBuffer[i + k][j + l].rgbRed;
					average_g += rgbBuffer[i + k][j + l].rgbGreen;
					average_b += rgbBuffer[i + k][j + l].rgbBlue;
				}
			}

			average_r /= 9;
			average_g /= 9;
			average_b /= 9;


			smoothBuffer_c[i + 1][j + 1].rgbRed = average_r;
			smoothBuffer_c[i + 1][j + 1].rgbGreen = average_g;
			smoothBuffer_c[i + 1][j + 1].rgbBlue = average_b;
		}
	}

	viewType = 7;
	Invalidate(FALSE);
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
}

void CMy201611256View::OnColorAverage5()
{
	if (rgbBuffer == NULL)
		OnImgLoadJpeg();
	smoothBuffer_c = new RGBQUAD * [imgHeight];
	for (int i = 0; i < imgHeight; i++)
		smoothBuffer_c[i] = new RGBQUAD[imgWidth];

	for (int i = 0; i < imgHeight - 4; i++) {
		for (int j = 0; j < imgWidth - 4; j++) {
			float average_r = 0;
			float average_g = 0;
			float average_b = 0;
			for (int k = 0; k < 5; k++) {
				for (int l = 0; l < 5; l++) {
					average_r += rgbBuffer[i + k][j + l].rgbRed;
					average_g += rgbBuffer[i + k][j + l].rgbGreen;
					average_b += rgbBuffer[i + k][j + l].rgbBlue;
				}
			}

			average_r /= 25;
			average_g /= 25;
			average_b /= 25;


			smoothBuffer_c[i + 2][j + 2].rgbRed = average_r;
			smoothBuffer_c[i + 2][j + 2].rgbGreen = average_g;
			smoothBuffer_c[i + 2][j + 2].rgbBlue = average_b;
		}
	}

	viewType = 7;
	Invalidate(FALSE);
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
}

void CMy201611256View::OnColorAverage7()
{
	if (rgbBuffer == NULL)
		OnImgLoadJpeg();
	smoothBuffer_c = new RGBQUAD * [imgHeight];
	for (int i = 0; i < imgHeight; i++)
		smoothBuffer_c[i] = new RGBQUAD[imgWidth];

	for (int i = 0; i < imgHeight - 6; i++) {
		for (int j = 0; j < imgWidth - 6; j++) {
			float average_r = 0;
			float average_g = 0;
			float average_b = 0;
			for (int k = 0; k < 7; k++) {
				for (int l = 0; l < 7; l++) {
					average_r += rgbBuffer[i + k][j + l].rgbRed;
					average_g += rgbBuffer[i + k][j + l].rgbGreen;
					average_b += rgbBuffer[i + k][j + l].rgbBlue;
				}
			}

			average_r /= 49;
			average_g /= 49;
			average_b /= 49;


			smoothBuffer_c[i + 3][j + 3].rgbRed = average_r;
			smoothBuffer_c[i + 3][j + 3].rgbGreen = average_g;
			smoothBuffer_c[i + 3][j + 3].rgbBlue = average_b;
		}
	}

	viewType = 7;
	Invalidate(FALSE);
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
}

void CMy201611256View::OnColorGaussian3()
{
	if (rgbBuffer == NULL)
		OnImgLoadJpeg();
	smoothBuffer_c = new RGBQUAD* [imgHeight];
	for (int i = 0; i < imgHeight; i++)
		smoothBuffer_c[i] = new RGBQUAD[imgWidth];

	int Gauss_3[3][3] = {
		{1,2,1},
		{2,4,2},
		{1,2,1}
	};

	for (int i = 0; i < imgHeight - 2; i++) {
		for (int j = 0; j < imgWidth - 2; j++) {
			float average_r = 0;
			float average_g = 0;
			float average_b = 0;
			for (int k = 0; k < 3; k++) {
				for (int l = 0; l < 3; l++) {
					average_r += Gauss_3[k][l] * rgbBuffer[i + k][j + l].rgbRed;
					average_g += Gauss_3[k][l] * rgbBuffer[i + k][j + l].rgbGreen;
					average_b += Gauss_3[k][l] * rgbBuffer[i + k][j + l].rgbBlue;
				}
			}

			average_r /= 16;
			average_g /= 16;
			average_b /= 16;

			if (average_r < 0) {
				average_r = 0;
			}
			if (average_g < 0) {
				average_g = 0;
			}
			if (average_b < 0) {
				average_b = 0;
			}
			if (average_r > 255) {
				average_r = 255;
			}
			if (average_g > 255) {
				average_g = 255;
			}
			if (average_b > 255) {
				average_b = 255;
			}

			smoothBuffer_c[i + 1][j + 1].rgbRed = average_r;
			smoothBuffer_c[i + 1][j + 1].rgbGreen = average_g;
			smoothBuffer_c[i + 1][j + 1].rgbBlue = average_b;
		}
	}

	viewType = 7;
	Invalidate(FALSE);
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
}

void CMy201611256View::OnColorGaussian5()
{
	if (rgbBuffer == NULL)
		OnImgLoadJpeg();
	smoothBuffer_c = new RGBQUAD * [imgHeight];
	for (int i = 0; i < imgHeight; i++)
		smoothBuffer_c[i] = new RGBQUAD[imgWidth];

	int Gauss_5[5][5] = {
		{1, 4, 7, 4, 1},
		{4, 16, 26, 16, 4},
		{7, 26, 41, 26, 7},
		{4, 16, 26, 16, 4},
		{1, 4, 7, 4, 1}
	};

	for (int i = 0; i < imgHeight - 4; i++) {
		for (int j = 0; j < imgWidth - 4; j++) {
			float average_r = 0;
			float average_g = 0;
			float average_b = 0;
			for (int k = 0; k < 5; k++) {
				for (int l = 0; l < 5; l++) {
					average_r += Gauss_5[k][l] * rgbBuffer[i + k][j + l].rgbRed;
					average_g += Gauss_5[k][l] * rgbBuffer[i + k][j + l].rgbGreen;
					average_b += Gauss_5[k][l] * rgbBuffer[i + k][j + l].rgbBlue;
				}
			}


			average_r /= 273;
			average_g /= 273;
			average_b /= 273;

			if (average_r < 0) {
				average_r = 0;
			}
			if (average_g < 0) {
				average_g = 0;
			}
			if (average_b < 0) {
				average_b = 0;
			}
			if (average_r > 255) {
				average_r = 255;
			}
			if (average_g > 255) {
				average_g = 255;
			}
			if (average_b > 255) {
				average_b = 255;
			}

			smoothBuffer_c[i + 2][j + 2].rgbRed = average_r;
			smoothBuffer_c[i + 2][j + 2].rgbGreen = average_g;
			smoothBuffer_c[i + 2][j + 2].rgbBlue = average_b;
		}
	}

	viewType = 7;
	Invalidate(FALSE);
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
}

void CMy201611256View::OnColorGaussian7()
{
	if (rgbBuffer == NULL)
		OnImgLoadJpeg();
	smoothBuffer_c = new RGBQUAD * [imgHeight];
	for (int i = 0; i < imgHeight; i++)
		smoothBuffer_c[i] = new RGBQUAD[imgWidth];

	int Gauss_7[7][7] = {
		{0, 0, 1, 2, 1, 0, 0},
		{0, 3, 13, 22, 13, 3, 0},
		{1, 13, 59, 97, 59, 13, 1},
		{2, 22, 97, 159, 97, 22, 2},
		{1, 13, 59, 97, 59, 13, 1},
		{0, 3, 13, 22, 13, 3, 0},
		{0, 0, 1, 2, 1, 0, 0},
	};

	for (int i = 0; i < imgHeight - 6; i++) {
		for (int j = 0; j < imgWidth - 6; j++) {
			float average_r = 0;
			float average_g = 0;
			float average_b = 0;
			for (int k = 0; k < 7; k++) {
				for (int l = 0; l < 7; l++) {
					average_r += Gauss_7[k][l] * rgbBuffer[i + k][j + l].rgbRed;
					average_g += Gauss_7[k][l] * rgbBuffer[i + k][j + l].rgbGreen;
					average_b += Gauss_7[k][l] * rgbBuffer[i + k][j + l].rgbBlue;
				}
			}


			average_r /= 1003;
			average_g /= 1003;
			average_b /= 1003;

			if (average_r < 0) {
				average_r = 0;
			}
			if (average_g < 0) {
				average_g = 0;
			}
			if (average_b < 0) {
				average_b = 0;
			}
			if (average_r > 255) {
				average_r = 255;
			}
			if (average_g > 255) {
				average_g = 255;
			}
			if (average_b > 255) {
				average_b = 255;
			}

			smoothBuffer_c[i + 3][j + 3].rgbRed = average_r;
			smoothBuffer_c[i + 3][j + 3].rgbGreen = average_g;
			smoothBuffer_c[i + 3][j + 3].rgbBlue = average_b;
		}
	}

	viewType = 7;
	Invalidate(FALSE);
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
}

void CMy201611256View::OnEdgedetectorPrewitt()
{
	if (rgbBuffer == NULL)
		OnImgLoadJpeg();
	edgeBuffer = new float* [imgHeight];
	for (int i = 0; i < imgHeight; i++)
		edgeBuffer[i] = new float[imgWidth];

	int prewitt_X[3][3] = {
		{-1,0,1},
		{-1,0,1},
		{-1,0,1}
	};

	int prewitt_Y[3][3] = {
		{1,1,1},
		{0,0,0},
		{-1,-1,-1}
	};

	for (int i = 0; i < imgHeight - 2; i++) {
		for (int j = 0; j < imgWidth - 2; j++) {
			float average_r_x = 0;
			float average_g_x = 0;
			float average_b_x = 0;
			float average_r_y = 0;
			float average_g_y = 0;
			float average_b_y = 0;
			for (int k = 0; k < 3; k++) {
				for (int l = 0; l < 3; l++) {
					average_r_x += prewitt_X[k][l] * rgbBuffer[i + k][j + l].rgbRed;
					average_g_x += prewitt_X[k][l] * rgbBuffer[i + k][j + l].rgbGreen;
					average_b_x += prewitt_X[k][l] * rgbBuffer[i + k][j + l].rgbBlue;
					average_r_y += prewitt_Y[k][l] * rgbBuffer[i + k][j + l].rgbRed;
					average_g_y += prewitt_Y[k][l] * rgbBuffer[i + k][j + l].rgbGreen;
					average_b_y += prewitt_Y[k][l] * rgbBuffer[i + k][j + l].rgbBlue;
				}
			}

			float average_r = sqrt(average_r_x * average_r_y);
			float average_g = sqrt(average_g_x * average_g_y);
			float average_b = sqrt(average_b_x * average_b_y);

			if (average_r > 255) {
				average_r = 255;
			}
			if (average_g > 255) {
				average_g = 255;
			}
			if (average_b > 255) {
				average_b = 255;
			}

			edgeBuffer[i + 1][j + 1] = (average_r + average_g + average_b) / 3;
		}
	}

	viewType = 8;
	Invalidate(FALSE);
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
}

void CMy201611256View::OnEdgedetectorSobel()
{
	if (rgbBuffer == NULL)
		OnImgLoadJpeg();
	edgeBuffer = new float* [imgHeight];
	for (int i = 0; i < imgHeight; i++)
		edgeBuffer[i] = new float[imgWidth];

	int sobel_X[3][3] = {
		{-1,0,1},
		{-2,0,2},
		{-1,0,1}
	};

	int sobel_Y[3][3] = {
		{1,2,1},
		{0,0,0},
		{-1,-2,-1}
	};

	for (int i = 0; i < imgHeight - 2; i++) {
		for (int j = 0; j < imgWidth - 2; j++) {
			float average_r_x = 0;
			float average_g_x = 0;
			float average_b_x = 0;
			float average_r_y = 0;
			float average_g_y = 0;
			float average_b_y = 0;
			for (int k = 0; k < 3; k++) {
				for (int l = 0; l < 3; l++) {
					average_r_x += sobel_X[k][l] * rgbBuffer[i + k][j + l].rgbRed;
					average_g_x += sobel_X[k][l] * rgbBuffer[i + k][j + l].rgbGreen;
					average_b_x += sobel_X[k][l] * rgbBuffer[i + k][j + l].rgbBlue;
					average_r_y += sobel_Y[k][l] * rgbBuffer[i + k][j + l].rgbRed;
					average_g_y += sobel_Y[k][l] * rgbBuffer[i + k][j + l].rgbGreen;
					average_b_y += sobel_Y[k][l] * rgbBuffer[i + k][j + l].rgbBlue;
				}
			}

			float average_r = sqrt(average_r_x * average_r_y);
			float average_g = sqrt(average_g_x * average_g_y);
			float average_b = sqrt(average_b_x * average_b_y);

			if (average_r > 255) {
				average_r = 255;
			}
			if (average_g > 255) {
				average_g = 255;
			}
			if (average_b > 255) {
				average_b = 255;
			}

			edgeBuffer[i + 1][j + 1] = (average_r + average_g + average_b) / 3;
		}
	}

	viewType = 8;
	Invalidate(FALSE);
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
}

void CMy201611256View::OnLog3()
{
	if (rgbBuffer == NULL)
		OnImgLoadJpeg();
	edgeBuffer = new float* [imgHeight];
	for (int i = 0; i < imgHeight; i++)
		edgeBuffer[i] = new float[imgWidth];

	int LoG[3][3] = {
		{-1,-1,-1},
		{-1,8,-1},
		{-1,-1,-1}
	};

	for (int i = 0; i < imgHeight - 2; i++) {
		for (int j = 0; j < imgWidth - 2; j++) {
			float average_r = 0;
			float average_g = 0;
			float average_b = 0;
			
			for (int k = 0; k < 3; k++) {
				for (int l = 0; l < 3; l++) {
					average_r += LoG[k][l] * rgbBuffer[i + k][j + l].rgbRed;
					average_g += LoG[k][l] * rgbBuffer[i + k][j + l].rgbGreen;
					average_b += LoG[k][l] * rgbBuffer[i + k][j + l].rgbBlue;
				}
			}

			if (average_r < 0) {
				average_r = 0;
			}
			if (average_g < 0) {
				average_g = 0;
			}
			if (average_b < 0) {
				average_b = 0;
			}
			if (average_r > 255) {
				average_r = 255;
			}
			if (average_g > 255) {
				average_g = 255;
			}
			if (average_b > 255) {
				average_b = 255;
			}

			edgeBuffer[i + 1][j + 1] = (average_r + average_g + average_b) / 3;
		}
	}

	viewType = 8;
	Invalidate(FALSE);
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
}

void CMy201611256View::OnLog5()
{
	if (rgbBuffer == NULL)
		OnImgLoadJpeg();
	edgeBuffer = new float* [imgHeight];
	for (int i = 0; i < imgHeight; i++)
		edgeBuffer[i] = new float[imgWidth];

	int LoG[5][5] = {
		{0, 0,-1,0, 0},
		{0, -1,-2,-1, 0},
		{-1, -2,16,-2, -1},
		{0, -1,-2,-1, 0},
		{0, 0,-1,0, 0},
	};

	for (int i = 0; i < imgHeight - 4; i++) {
		for (int j = 0; j < imgWidth - 4; j++) {
			float average_r = 0;
			float average_g = 0;
			float average_b = 0;

			for (int k = 0; k < 5; k++) {
				for (int l = 0; l < 5; l++) {
					average_r += LoG[k][l] * rgbBuffer[i + k][j + l].rgbRed;
					average_g += LoG[k][l] * rgbBuffer[i + k][j + l].rgbGreen;
					average_b += LoG[k][l] * rgbBuffer[i + k][j + l].rgbBlue;
				}
			}

			if (average_r < 0) {
				average_r = 0;
			}
			if (average_g < 0) {
				average_g = 0;
			}
			if (average_b < 0) {
				average_b = 0;
			}
			if (average_r > 255) {
				average_r = 255;
			}
			if (average_g > 255) {
				average_g = 255;
			}
			if (average_b > 255) {
				average_b = 255;
			}

			edgeBuffer[i + 2][j + 2] = (average_r + average_g + average_b) / 3;
		}
	}

	viewType = 8;
	Invalidate(FALSE);
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
}

void CMy201611256View::OnLog9()
{
	if (rgbBuffer == NULL)
		OnImgLoadJpeg();
	edgeBuffer = new float* [imgHeight];
	for (int i = 0; i < imgHeight; i++)
		edgeBuffer[i] = new float[imgWidth];

	int LoG[9][9] = {
		{0, -1,-1, -2, -2, -2, -1, -1, 0},
		{-1, -2, -4, -5, -5, -5, -4, -2, -1},
		{-1, -4, -5, -3, 0, -3, -5, -4, -1},
		{-2, -5, -3, 12, 24, 12, -3, -5, -2},
		{-2, -5, 0, 24, 40, 24, 0, -5, -2},
		{-2, -5, -3, 12, 24, 12, -3, -5, -2},
		{-1, -4, -5, -3, 0, -3, -5, -4, -1},
		{-1, -2, -4, -5, -5, -5, -4, -2, -1},
		{0, -1,-1, -2, -2, -2, -1, -1, 0},
	};

	for (int i = 0; i < imgHeight - 8; i++) {
		for (int j = 0; j < imgWidth - 8; j++) {
			float average_r = 0;
			float average_g = 0;
			float average_b = 0;

			for (int k = 0; k < 9; k++) {
				for (int l = 0; l < 9; l++) {
					average_r += LoG[k][l] * rgbBuffer[i + k][j + l].rgbRed;
					average_g += LoG[k][l] * rgbBuffer[i + k][j + l].rgbGreen;
					average_b += LoG[k][l] * rgbBuffer[i + k][j + l].rgbBlue;
				}
			}

			if (average_r < 0) {
				average_r = 0;
			}
			if (average_g < 0) {
				average_g = 0;
			}
			if (average_b < 0) {
				average_b = 0;
			}
			if (average_r > 255) {
				average_r = 255;
			}
			if (average_g > 255) {
				average_g = 255;
			}
			if (average_b > 255) {
				average_b = 255;
			}

			edgeBuffer[i + 4][j + 4] = (average_r + average_g + average_b) / 3;
		}
	}

	viewType = 8;
	Invalidate(FALSE);
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
}




void CMy201611256View::OnLoadAvi()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
}
