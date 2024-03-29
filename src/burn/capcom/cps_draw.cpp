#include "cps.h"
// CPS - Draw

unsigned char CpsRecalcPal = 0;			// Flag - If it is 1, recalc the whole palette

static int LayerCont;
int nStartline, nEndline;
int nRasterline[MAX_RASTER + 2];

int nCpsLcReg = 0;						// Address of layer controller register
int CpsLayEn[6] = {0, 0, 0, 0, 0, 0};	// bits for layer enable
int MaskAddr[4] = {0, 0, 0, 0};

int CpsLayer1XOffs = 0;
int CpsLayer2XOffs = 0;
int CpsLayer3XOffs = 0;
int CpsLayer1YOffs = 0;
int CpsLayer2YOffs = 0;
int CpsLayer3YOffs = 0;

static void Cps1Layers();
static void Cps2Layers();

typedef int  (*CpsObjDrawDoFn)(int,int);
typedef int  (*CpsScrXDrawDoFn)(unsigned char *,int,int);
typedef void (*CpsLayersDoFn)();
typedef int  (*CpsrPrepareDoFn)();
typedef int  (*CpsrRenderDoFn)();

CpsObjDrawDoFn  CpsObjDrawDoX;
CpsScrXDrawDoFn CpsScr1DrawDoX;
CpsScrXDrawDoFn CpsScr3DrawDoX;
CpsLayersDoFn   CpsLayersDoX;
CpsrPrepareDoFn CpsrPrepareDoX;
CpsrRenderDoFn  CpsrRenderDoX;

void DrawFnInit()
{
	if(Cps == 2) {
		CpsLayersDoX   = Cps2Layers;
		CpsScr1DrawDoX = Cps2Scr1Draw;
		CpsScr3DrawDoX = Cps2Scr3Draw;
		CpsObjDrawDoX  = Cps2ObjDraw;
		CpsrPrepareDoX = Cps2rPrepare;
		CpsrRenderDoX  = Cps2rRender;
	} else {
		CpsLayersDoX   = Cps1Layers;
		CpsScr1DrawDoX = Cps1Scr1Draw;
		CpsScr3DrawDoX = Cps1Scr3Draw;
		CpsObjDrawDoX  = Cps1ObjDraw;
		CpsrPrepareDoX = Cps1rPrepare;
		CpsrRenderDoX  = Cps1rRender;
	}
}

static int DrawScroll1(int i)
{
	// Draw Scroll 1
	int nOff, nScrX, nScrY;
	unsigned char *Find;

	nOff = *((unsigned short *)(CpsSaveReg[i] + 0x02));

	// Get scroll coordinates
	nScrX = *((unsigned short *)(CpsSaveReg[i] + 0x0c)); // Scroll 1 X
	nScrY = *((unsigned short *)(CpsSaveReg[i] + 0x0e)); // Scroll 1 Y

	nScrX += 0x40;

//	bprintf(PRINT_NORMAL, _T("1 %x, %x, %x\n"), nOff, nScrX, nScrY);

	nScrX += CpsLayer1XOffs;
	nScrY += 0x10;
	nScrY += CpsLayer1YOffs;
	nOff <<= 8;
	nOff &= 0xffc000;
	Find = CpsFindGfxRam(nOff, 0x4000);
	if (Find == NULL) {
		return 1;
	}
	CpsScr1DrawDoX(Find, nScrX, nScrY);
	return 0;
}

static int DrawScroll2Init(int i)
{
	// Draw Scroll 2
	int nScr2Off; int n;

	nScr2Off = *((unsigned short *)(CpsSaveReg[i] + 0x04));

	// Get scroll coordinates
	nCpsrScrX= *((unsigned short *)(CpsSaveReg[i] + 0x10)); // Scroll 2 X
	nCpsrScrY= *((unsigned short *)(CpsSaveReg[i] + 0x12)); // Scroll 2 Ytess

	// Get row scroll information
	n = *((unsigned short *)(CpsSaveReg[i] + 0x22));

	nScr2Off <<= 8;

	nCpsrScrX += 0x40;

//	bprintf(PRINT_NORMAL, _T("2 %x, %x, %x\n"), nScr2Off, nCpsrScrX, nCpsrScrY);

	nCpsrScrX += CpsLayer2XOffs;
	nCpsrScrX &= 0x03FF;

	nCpsrScrY += 0x10;
	nCpsrScrY += CpsLayer2YOffs;
	nCpsrScrY &= 0x03FF;

	nScr2Off &= 0xFFC000;
	CpsrBase = CpsFindGfxRam(nScr2Off, 0x4000);
	if (CpsrBase == NULL) {
		return 1;
	}

	CpsrRows = NULL;

	if (n & 1) {
		int nTab, nStart;
		// Find row scroll table:

		nTab = *((unsigned short *)(CpsSaveReg[i] + 0x08));
		nStart = *((unsigned short *)(CpsSaveReg[i] + 0x20));

		nTab <<= 8;
		nTab &= 0xFFF800; // Vampire - Row scroll effect in VS screen background

		CpsrRows = (unsigned short *)CpsFindGfxRam(nTab, 0x0800);

		// Find start offset
		nCpsrRowStart = nStart + 16;
	}

	CpsrPrepareDoX();
	return 0;
}

inline static int DrawScroll2Exit()
{
	CpsrBase = NULL;
	nCpsrScrX = 0;
	nCpsrScrY = 0;
	CpsrRows = NULL;
	return 0;
}

inline static int DrawScroll2Do()
{
	if (CpsrBase == NULL) {
		return 1;
	}
	CpsrRenderDoX();
	return 0;
}

static int DrawScroll3(int i)
{
	// Draw Scroll 3
	int nOff, nScrX, nScrY;
	unsigned char *Find;

	nOff = *((unsigned short *)(CpsSaveReg[i] + 0x06));

	// Get scroll coordinates
	nScrX = *((unsigned short *)(CpsSaveReg[i] + 0x14)); // Scroll 3 X
	nScrY = *((unsigned short *)(CpsSaveReg[i] + 0x16)); // Scroll 3 Y

	nScrX += 0x40;

//	bprintf(PRINT_NORMAL, _T("3 %x, %x, %x\n"), nOff, nScrX, nScrY);

	nScrX += CpsLayer3XOffs;
	nScrY += 0x10;
	nScrY += CpsLayer3YOffs;

	nOff <<= 8;
	nOff &= 0xffc000;
	Find=CpsFindGfxRam(nOff, 0x4000);
	if (Find == NULL) {
		return 1;
	}
	CpsScr3DrawDoX(Find, nScrX, nScrY);
	return 0;
}

static int DrawStar(int nLayer)
{
	int nStar, nStarXPos, nStarYPos, nStarColour;
	unsigned char* pStar = CpsStar + (nLayer << 12);

	for (nStar = 0; nStar < 0x1000; nStar++) {
		nStarColour = pStar[nStar];

		if (nStarColour != 0x0F) {
			nStarXPos = (((nStar >> 8) << 5) - *((short*)(CpsSaveReg[0] + 0x18 + (nLayer << 2))) + (nStarColour & 0x1F) - 64) & 0x01FF;
			nStarYPos = ((nStar & 0xFF) - *((short*)(CpsSaveReg[0] + 0x1A + (nLayer << 2))) - 16) & 0xFF;

			if (nStarXPos < 384 && nStarYPos < 224) {
				nStarColour = ((nStarColour & 0xE0) >> 1) + ((GetCurrentFrame() >> 4) & 0x0F);
				PutPix(pBurnDraw + (nBurnPitch * nStarYPos) + (nBurnBpp * nStarXPos), CpsPal[0x0800 + (nLayer << 9) + nStarColour]);
			}
		}
	}

	return 0;
}

static void Cps1Layers()
{
  int Draw[4]={-1,-1,-1,-1};
  int nDrawMask=0;
  int i=0;

  nDrawMask=1; // Sprites always on
  LayerCont = *((unsigned short *)(CpsSaveReg[0] + nCpsLcReg));
  // Get correct bits from Layer Controller
  if (LayerCont & CpsLayEn[1]) nDrawMask|=2;
  if (LayerCont & CpsLayEn[2]) nDrawMask|=4;
  if (LayerCont & CpsLayEn[3]) nDrawMask|=8;
  nDrawMask&=nBurnLayer;   // User choice of layers to display
  
  // Layer control:
  Draw[0]=(LayerCont>>12)&3; // top layer
  Draw[1]=(LayerCont>>10)&3;
  Draw[2]=(LayerCont>> 8)&3;
  Draw[3]=(LayerCont>> 6)&3; // bottom layer (most covered up)
  
  // Check for repeated layers and if there are any, the lower layer is omitted
#define CRP(a,b) if (Draw[a]==Draw[b]) Draw[b]=-1;
  CRP(0,1) CRP(0,2) CRP(0,3) CRP(1,2) CRP(1,3) CRP(2,3)
#undef CRP

  for (i = 0; i < 2; i++) {
	  if (LayerCont & CpsLayEn[4 + i]) {
		  CpsStarPalUpdate(CpsSavePal, i, CpsRecalcPal);
		  DrawStar(i);
	  }
  }

  // prepare layer 2
  DrawScroll2Init(0);

  // draw layers, bottom -> top
  for (i=3;i>=0;i--)
  {
    int n=Draw[i]; // Find out which layer to draw

    if (n==0) {
	  if (nDrawMask & 1)  CpsObjDrawDoX(0,7);

	  if (!Mercs && !Sf2jc && !Qad) {
		nBgHi=1;
		switch (Draw[i+1]) {
			case 1:
				if (nDrawMask & 2) 	DrawScroll1(0);
				break;
			case 2:
				if (nDrawMask & 4)  DrawScroll2Do();
				break;
			case 3:
				if (nDrawMask & 8)  DrawScroll3(0);
				break;
		}
		nBgHi=0;
	  }
	}

    // Then Draw the scroll layer on top
    switch (n) {
		case 1:
			if (nDrawMask & 2) DrawScroll1(0);
			break;
		case 2:
			if (nDrawMask & 4) DrawScroll2Do();
			break;
		case 3:
			if (nDrawMask & 8) DrawScroll3(0);
			break;
	}
  }

  DrawScroll2Exit();
}

static void Cps2Layers()
{
	int Draw[MAX_RASTER][4];
	int Prio[MAX_RASTER][4];
	int nDrawMask[MAX_RASTER];

	CpsObjDrawInit();

	int nSlice = 0;
	do {
		LayerCont = *((unsigned short *)(CpsSaveReg[nSlice] + nCpsLcReg));

		// Determine which layers are enabled
		nDrawMask[nSlice] = 1;								// Sprites always on
		if (LayerCont & CpsLayEn[1]) nDrawMask[nSlice] |= 2;
		if (LayerCont & CpsLayEn[2]) nDrawMask[nSlice] |= 4;
		if (LayerCont & CpsLayEn[3]) nDrawMask[nSlice] |= 8;
		nDrawMask[nSlice] &= nBurnLayer;					// User choice of layers to display

		// Determine layer� priority:
		Draw[nSlice][3] = (LayerCont >> 12) & 3;			// top layer
		Draw[nSlice][2] = (LayerCont >> 10) & 3;
		Draw[nSlice][1] = (LayerCont >> 8) & 3;
		Draw[nSlice][0] = (LayerCont >> 6) & 3;				// bottom layer (most covered up)

		// Determine layer-sprite priority (layer >= sprites -> layer on top)
		int nLayPri = (CpsSaveFrg[nSlice][4] << 8) | CpsSaveFrg[nSlice][5];	// Layer priority register at word (400004)
		Prio[nSlice][3] = (nLayPri >> 12) & 7;
		Prio[nSlice][2] = (nLayPri >> 8) & 7;
		Prio[nSlice][1] = (nLayPri >> 4) & 7;
		Prio[nSlice][0] = 0;

		// Check for repeated layers (if found, discard the lower layer)
#define CRP(a, b) if (Draw[nSlice][a] == Draw[nSlice][b]) Draw[nSlice][b] = -1;
		CRP(3, 2) CRP(3, 1) CRP(2, 1) CRP(3, 0) CRP(2, 0) CRP(1, 0)
#undef CRP

		// Pre-process priorities
		// Higher priority layers must have higher layer-sprite priorities
		// N.B. If this is not the case, masking effects may occur (not emulated)
#if 0
		// Raise sprite priorities of top layers if needed
		int nHighPrio = 0;
		for (int i = 0; i < 4; i++) {
			if (Draw[nSlice][i] > 0) {
				if (Prio[nSlice][Draw[nSlice][i]] < nHighPrio) {
					Prio[nSlice][Draw[nSlice][i]] = nHighPrio;
				} else {
					nHighPrio = Prio[nSlice][Draw[nSlice][i]];
				}
			}
		}
#else
		// Lower sprite priorities of bottom layers if needed
		int nHighPrio = 9999;
		for (int i = 3; i >= 0; i--) {
			if (Draw[nSlice][i] > 0) {
				if (Prio[nSlice][Draw[nSlice][i]] > nHighPrio) {
					Prio[nSlice][Draw[nSlice][i]] = nHighPrio;
				} else {
					nHighPrio = Prio[nSlice][Draw[nSlice][i]];
				}
			}
		}
#endif
		nSlice++;
	} while (nSlice < MAX_RASTER && nRasterline[nSlice]);

	int nPrevPrio = -1;
	for (int nCurrPrio = 0; nCurrPrio < 8; nCurrPrio++) {

		nSlice = 0;
		do {
			for (int i = 0; i < 4; i++) {

				if (Prio[nSlice][Draw[nSlice][i]] == nCurrPrio) {

					// Render sprites between the previous layer and this one
					if ((nDrawMask[0] & 1) && (nPrevPrio < nCurrPrio)) {
						CpsObjDrawDoX(nPrevPrio + 1, nCurrPrio);
						nPrevPrio = nCurrPrio;
					}

					nStartline = nRasterline[nSlice];
					nEndline = nRasterline[nSlice + 1];
					if (!nEndline) {
						nEndline = 224;
					}

					// Render layer
					switch (Draw[nSlice][i]) {
						case 1:
							if (nDrawMask[nSlice] & 2) {
								DrawScroll1(nSlice);
							}
							break;
						case 2:
							if (nDrawMask[nSlice] & 4) {
								DrawScroll2Init(nSlice);
								DrawScroll2Do();
								DrawScroll2Exit();
							}
							break;
						case 3:
							if (nDrawMask[nSlice] & 8) {
								DrawScroll3(nSlice);
							}
							break;
					}
				}
			}
			nSlice++;
		}  while (nSlice < MAX_RASTER && nRasterline[nSlice]);
	}

	// Render highest priority sprites
	if ((nDrawMask[0] & 1) && (nPrevPrio < 7)) {
		CpsObjDrawDoX(nPrevPrio + 1, 7);
	}
}

void CpsClearScreen()
{
	if (Cps == 1) {
		switch (nBurnBpp) {
			case 4: {
				unsigned int* pClear = (unsigned int*)pBurnDraw;
				unsigned int nColour = CpsPal[0xbff ^ 15];
				for (int i = 0; i < 384 * 224 / 8; i++) {
					*pClear++ = nColour;
					*pClear++ = nColour;
					*pClear++ = nColour;
					*pClear++ = nColour;
					*pClear++ = nColour;
					*pClear++ = nColour;
					*pClear++ = nColour;
					*pClear++ = nColour;
				}
				break;
			}

			case 3: {
				unsigned char* pClear = pBurnDraw;
				unsigned char r = CpsPal[0xbff ^ 15];
				unsigned char g = (r >> 8) & 0xFF;
				unsigned char b = (r >> 16) & 0xFF;
				r &= 0xFF;
				for (int i = 0; i < 384 * 224; i++) {
					*pClear++ = r;
					*pClear++ = g;
					*pClear++ = b;
				}
				break;
			}

			case 2: {
				unsigned int* pClear = (unsigned int*)pBurnDraw;
				unsigned int nColour = CpsPal[0xbff ^ 15] | CpsPal[0xbff ^ 15] << 16;
				for (int i = 0; i < 384 * 224 / 16; i++) {
					*pClear++ = nColour;
					*pClear++ = nColour;
					*pClear++ = nColour;
					*pClear++ = nColour;
					*pClear++ = nColour;
					*pClear++ = nColour;
					*pClear++ = nColour;
					*pClear++ = nColour;
				}
				break;
			}
		}
	} else {
		memset(pBurnDraw, 0, 384 * 224 * nBurnBpp);
	}
}

static void DoDraw(int Recalc)
{
	CtvReady();								// Point to correct tile drawing functions

	// Update Palette
	CpsPalUpdate(CpsSavePal, Recalc);		// recalc whole palette if needed
	
	CpsClearScreen();

	CpsLayersDoX();
}

int CpsDraw()
{
	DoDraw(CpsRecalcPal);

	CpsRecalcPal = 0;
	return 0;
}

int CpsRedraw()
{
	DoDraw(1);

	CpsRecalcPal = 0;
	return 0;
}

