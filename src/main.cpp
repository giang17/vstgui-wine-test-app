// VSTGUI D2D1 Test Application
// Tests VSTGUI rendering operations under Wine
// Cross-compiled with MinGW from Linux

#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

#include <windows.h>
#include <cmath>

#include "vstgui/lib/vstguiinit.h"
#include "vstgui/lib/cframe.h"
#include "vstgui/lib/cview.h"
#include "vstgui/lib/cviewcontainer.h"
#include "vstgui/lib/cdrawcontext.h"
#include "vstgui/lib/cgraphicspath.h"
#include "vstgui/lib/ccolor.h"
#include "vstgui/lib/cfont.h"
#include "vstgui/lib/controls/cknob.h"
#include "vstgui/lib/controls/ctextlabel.h"
#include "vstgui/lib/platform/platformfactory.h"
#include "vstgui/lib/platform/win32/win32factory.h"

using namespace VSTGUI;

static const CColor kColorBlack  = CColor(0, 0, 0, 255);
static const CColor kColorBlue   = CColor(0, 0, 255, 255);
static const CColor kColorRed    = CColor(255, 0, 0, 255);
static const CColor kColorGray   = CColor(180, 180, 180, 255);
static const CColor kColorWhite  = CColor(255, 255, 255, 255);
static const CColor kColorGreen  = CColor(0, 180, 0, 255);

//------------------------------------------------------------------------
// Custom view: draws test lines with different widths and angles
//------------------------------------------------------------------------
class TestLinesView : public CView
{
public:
    TestLinesView(const CRect& size) : CView(size) {}

    void draw(CDrawContext* context) override
    {
        context->setDrawMode(kAntiAliasing);
        auto r = getViewSize();

        // Horizontal lines with increasing width
        for (int i = 0; i < 5; i++)
        {
            CCoord y = r.top + 10 + i * 15;
            CCoord width = 1.0 + i * 1.0;
            context->setLineWidth(width);
            context->setFrameColor(kColorBlack);
            context->drawLine(CPoint(r.left + 5, y), CPoint(r.left + 105, y));
        }

        // Diagonal lines
        context->setLineWidth(2.0);
        context->setFrameColor(kColorBlue);
        for (int i = 0; i < 5; i++)
        {
            CCoord offset = i * 20;
            context->drawLine(
                CPoint(r.left + 120, r.top + 10 + offset),
                CPoint(r.left + 170, r.top + 70 + offset));
        }

        setDirty(false);
    }
};

//------------------------------------------------------------------------
// Custom view: draws a sine waveform using CGraphicsPath
//------------------------------------------------------------------------
class SineWaveformView : public CView
{
public:
    SineWaveformView(const CRect& size) : CView(size) {}

    void draw(CDrawContext* context) override
    {
        context->setDrawMode(kAntiAliasing);
        auto r = getViewSize();

        if (auto path = owned(context->createGraphicsPath()))
        {
            int segments = 40;
            CCoord centerY = r.top + r.getHeight() * 0.5;
            CCoord amplitude = r.getHeight() * 0.4;

            path->beginSubpath(CPoint(r.left, centerY));
            for (int i = 1; i <= segments; i++)
            {
                double t = (double)i / segments;
                CCoord px = r.left + t * r.getWidth();
                CCoord py = centerY - amplitude * sin(t * 2.0 * M_PI);
                path->addLine(CPoint(px, py));
            }

            context->setFrameColor(kColorRed);
            context->setLineWidth(2.0);
            context->drawGraphicsPath(path, CDrawContext::kPathStroked);
        }

        setDirty(false);
    }
};

//------------------------------------------------------------------------
// Custom view: draws a square waveform
//------------------------------------------------------------------------
class SquareWaveformView : public CView
{
public:
    SquareWaveformView(const CRect& size) : CView(size) {}

    void draw(CDrawContext* context) override
    {
        context->setDrawMode(kAntiAliasing);
        auto r = getViewSize();

        if (auto path = owned(context->createGraphicsPath()))
        {
            CCoord stepWidth = r.getWidth() / 8.0;
            path->beginSubpath(CPoint(r.left, r.bottom));

            for (int i = 0; i < 8; i++)
            {
                CCoord sx = r.left + i * stepWidth;
                CCoord top = r.top;
                CCoord bot = r.bottom;
                if (i % 2 == 0)
                {
                    path->addLine(CPoint(sx, top));
                    path->addLine(CPoint(sx + stepWidth, top));
                }
                else
                {
                    path->addLine(CPoint(sx, bot));
                    path->addLine(CPoint(sx + stepWidth, bot));
                }
            }

            context->setFrameColor(kColorBlue);
            context->setLineWidth(2.0);
            context->drawGraphicsPath(path, CDrawContext::kPathStroked);
        }

        setDirty(false);
    }
};

//------------------------------------------------------------------------
// Custom view: draws a triangle waveform
//------------------------------------------------------------------------
class TriangleWaveformView : public CView
{
public:
    TriangleWaveformView(const CRect& size) : CView(size) {}

    void draw(CDrawContext* context) override
    {
        context->setDrawMode(kAntiAliasing);
        auto r = getViewSize();

        if (auto path = owned(context->createGraphicsPath()))
        {
            CCoord centerY = r.top + r.getHeight() * 0.5;
            CCoord periodW = r.getWidth() / 4.0;
            CCoord quarterW = periodW / 4.0;
            path->beginSubpath(CPoint(r.left, centerY));

            for (int i = 0; i < 4; i++)
            {
                CCoord bx = r.left + i * periodW;
                path->addLine(CPoint(bx + quarterW, r.top));
                path->addLine(CPoint(bx + quarterW * 2.0, centerY));
                path->addLine(CPoint(bx + quarterW * 3.0, r.bottom));
                path->addLine(CPoint(bx + periodW, centerY));
            }

            context->setFrameColor(kColorBlack);
            context->setLineWidth(1.5);
            context->drawGraphicsPath(path, CDrawContext::kPathStroked);
        }

        setDirty(false);
    }
};

//------------------------------------------------------------------------
// Custom view: stress test — dense waveform with many segments
//------------------------------------------------------------------------
class StressWaveformView : public CView
{
public:
    int segments;
    float strokeWidth;
    CColor color;

    StressWaveformView(const CRect& size, int segs, float sw, CColor col)
        : CView(size), segments(segs), strokeWidth(sw), color(col) {}

    void draw(CDrawContext* context) override
    {
        context->setDrawMode(kAntiAliasing);
        auto r = getViewSize();

        if (auto path = owned(context->createGraphicsPath()))
        {
            CCoord centerY = r.top + r.getHeight() * 0.5;
            CCoord amplitude = r.getHeight() * 0.4;
            path->beginSubpath(CPoint(r.left, centerY));

            for (int i = 1; i <= segments; i++)
            {
                double t = (double)i / segments;
                CCoord px = r.left + t * r.getWidth();
                CCoord py = centerY - amplitude * sin(t * 6.0 * M_PI)
                           + 0.3 * sin(t * 47.0 * M_PI);
                path->addLine(CPoint(px, py));
            }

            context->setFrameColor(color);
            context->setLineWidth(strokeWidth);
            context->drawGraphicsPath(path, CDrawContext::kPathStroked);
        }

        setDirty(false);
    }
};

//------------------------------------------------------------------------
// Custom view: zigzag stress test
//------------------------------------------------------------------------
class ZigzagStressView : public CView
{
public:
    int segments;
    float strokeWidth;
    CColor color;

    ZigzagStressView(const CRect& size, int segs, float sw, CColor col)
        : CView(size), segments(segs), strokeWidth(sw), color(col) {}

    void draw(CDrawContext* context) override
    {
        context->setDrawMode(kAntiAliasing);
        auto r = getViewSize();

        if (auto path = owned(context->createGraphicsPath()))
        {
            path->beginSubpath(CPoint(r.left, r.top));

            for (int i = 1; i <= segments; i++)
            {
                CCoord px = r.left + ((double)i / segments) * r.getWidth();
                CCoord py = (i % 2 == 0) ? r.top : r.bottom;
                path->addLine(CPoint(px, py));
            }

            context->setFrameColor(color);
            context->setLineWidth(strokeWidth);
            context->drawGraphicsPath(path, CDrawContext::kPathStroked);
        }

        setDirty(false);
    }
};

//------------------------------------------------------------------------
// Custom view: knob drawn manually (arc + indicator line)
//------------------------------------------------------------------------
class CustomKnobView : public CView
{
public:
    float value = 0.5f;

    CustomKnobView(const CRect& size) : CView(size) {}

    void draw(CDrawContext* context) override
    {
        context->setDrawMode(kAntiAliasing);
        auto r = getViewSize();
        CCoord centerX = r.left + r.getWidth() * 0.5;
        CCoord centerY = r.top + r.getHeight() * 0.5;
        CCoord radius = std::min(r.getWidth(), r.getHeight()) * 0.45;

        // Background circle
        CRect ellipseRect(centerX - radius, centerY - radius,
                          centerX + radius, centerY + radius);
        context->setFillColor(kColorGray);
        context->drawEllipse(ellipseRect, kDrawFilled);
        context->setFrameColor(kColorBlack);
        context->setLineWidth(2.0);
        context->drawEllipse(ellipseRect, kDrawStroked);

        // Indicator line
        double angle = (120.0 + value * 270.0) * M_PI / 180.0;
        CCoord indX = centerX + (radius - 10) * cos(angle);
        CCoord indY = centerY + (radius - 10) * sin(angle);
        context->setFrameColor(kColorRed);
        context->setLineWidth(3.0);
        context->drawLine(CPoint(centerX, centerY), CPoint(indX, indY));

        // Value arc using CGraphicsPath
        if (auto path = owned(context->createGraphicsPath()))
        {
            CCoord arcRadius = radius - 5;
            double startAngle = 120.0;
            double endAngle = 120.0 + value * 270.0;

            CRect arcRect(centerX - arcRadius, centerY - arcRadius,
                          centerX + arcRadius, centerY + arcRadius);
            path->addArc(arcRect, startAngle, endAngle, true);

            context->setFrameColor(kColorRed);
            context->setLineWidth(4.0);
            context->drawGraphicsPath(path, CDrawContext::kPathStroked);
        }

        setDirty(false);
    }

    CMouseEventResult onMouseDown(CPoint& where, const CButtonState& buttons) override
    {
        if (buttons.isLeftButton())
        {
            lastY = where.y;
            return kMouseEventHandled;
        }
        return kMouseEventNotHandled;
    }

    CMouseEventResult onMouseMoved(CPoint& where, const CButtonState& buttons) override
    {
        if (buttons.isLeftButton())
        {
            float delta = (float)(lastY - where.y) * 0.005f;
            value += delta;
            if (value < 0.f) value = 0.f;
            if (value > 1.f) value = 1.f;
            lastY = where.y;
            invalid();
            return kMouseEventHandled;
        }
        return kMouseEventNotHandled;
    }

private:
    CCoord lastY = 0;
};

//------------------------------------------------------------------------
// Custom view: nine-part rectangle test
//------------------------------------------------------------------------
class NinePartRectView : public CView
{
public:
    NinePartRectView(const CRect& size) : CView(size) {}

    void draw(CDrawContext* context) override
    {
        auto r = getViewSize();
        CCoord cs = 10; // corner size

        // Corners (red)
        context->setFillColor(kColorRed);
        context->drawRect(CRect(r.left, r.top, r.left + cs, r.top + cs), kDrawFilled);
        context->drawRect(CRect(r.right - cs, r.top, r.right, r.top + cs), kDrawFilled);
        context->drawRect(CRect(r.left, r.bottom - cs, r.left + cs, r.bottom), kDrawFilled);
        context->drawRect(CRect(r.right - cs, r.bottom - cs, r.right, r.bottom), kDrawFilled);

        // Edges (blue)
        context->setFillColor(kColorBlue);
        context->drawRect(CRect(r.left + cs, r.top, r.right - cs, r.top + cs), kDrawFilled);
        context->drawRect(CRect(r.left + cs, r.bottom - cs, r.right - cs, r.bottom), kDrawFilled);
        context->drawRect(CRect(r.left, r.top + cs, r.left + cs, r.bottom - cs), kDrawFilled);
        context->drawRect(CRect(r.right - cs, r.top + cs, r.right, r.bottom - cs), kDrawFilled);

        // Center (gray)
        context->setFillColor(kColorGray);
        context->drawRect(CRect(r.left + cs, r.top + cs, r.right - cs, r.bottom - cs), kDrawFilled);

        // Border
        context->setFrameColor(kColorBlack);
        context->setLineWidth(1.0);
        context->drawRect(r, kDrawStroked);

        setDirty(false);
    }
};

//------------------------------------------------------------------------
// Custom view: shapes test (rounded rect, ellipse, small circles)
//------------------------------------------------------------------------
class ShapesView : public CView
{
public:
    ShapesView(const CRect& size) : CView(size) {}

    void draw(CDrawContext* context) override
    {
        context->setDrawMode(kAntiAliasing);
        auto r = getViewSize();

        // Rounded rectangle
        CRect rrRect(r.left, r.top, r.left + 160, r.top + 60);
        if (auto path = owned(context->createGraphicsPath()))
        {
            CCoord radius = 10;
            path->addRoundRect(rrRect, radius);
            context->setFillColor(kColorGray);
            context->drawGraphicsPath(path, CDrawContext::kPathFilled);
            context->setFrameColor(kColorBlack);
            context->setLineWidth(2.0);
            context->drawGraphicsPath(path, CDrawContext::kPathStroked);
        }

        // Non-circular ellipse
        CRect ellipseRect(r.left + 10, r.top + 80, r.left + 110, r.top + 120);
        context->setFillColor(kColorGray);
        context->drawEllipse(ellipseRect, kDrawFilled);
        context->setFrameColor(kColorBlack);
        context->setLineWidth(2.0);
        context->drawEllipse(ellipseRect, kDrawStroked);

        // Small circles
        context->setFrameColor(kColorBlue);
        context->setLineWidth(2.0);
        for (int i = 0; i < 5; i++)
        {
            CCoord cx = r.left + 130 + i * 30;
            CCoord cy = r.top + 80 + i * 8;
            CRect circ(cx - 12, cy - 12, cx + 12, cy + 12);
            context->drawEllipse(circ, kDrawStroked);
        }

        setDirty(false);
    }
};

//------------------------------------------------------------------------
// Main window container — assembles all test views
//------------------------------------------------------------------------
class MainView : public CViewContainer
{
public:
    MainView(const CRect& size) : CViewContainer(size)
    {
        setBackgroundColor(kColorWhite);

        // Knob (top left)
        addView(new CustomKnobView(CRect(30, 80, 170, 220)));

        // Test lines (top center)
        addView(new TestLinesView(CRect(190, 80, 400, 230)));

        // Nine-part rectangle
        addView(new NinePartRectView(CRect(200, 200, 300, 260)));

        // Square waveform
        addView(new SquareWaveformView(CRect(320, 210, 480, 250)));

        // Shapes (rounded rect, ellipse, circles)
        addView(new ShapesView(CRect(20, 240, 200, 380)));

        // Sine waveform (red)
        addView(new SineWaveformView(CRect(200, 280, 360, 320)));

        // Triangle waveform
        addView(new TriangleWaveformView(CRect(380, 280, 540, 320)));

        // Stress tests
        addView(new StressWaveformView(CRect(20, 340, 220, 370), 800, 2.0f, kColorRed));
        addView(new StressWaveformView(CRect(240, 340, 400, 370), 400, 8.0f, kColorBlue));
        addView(new ZigzagStressView(CRect(420, 340, 570, 365), 40, 3.0f, kColorBlack));
        addView(new StressWaveformView(CRect(20, 400, 570, 440), 400, 1.5f, kColorRed));
    }
};

//------------------------------------------------------------------------
// Minimal VSTGUIEditorInterface stub
//------------------------------------------------------------------------
class EditorStub : public VSTGUIEditorInterface
{
public:
    int64_t getOpenedFrameCount () const { return 1; }
    CFrame* getFrame () const override { return frame; }
    void beginEdit (int32_t) override {}
    void endEdit (int32_t) override {}
    CFrame* frame = nullptr;
};

//------------------------------------------------------------------------
// Windows message loop
//------------------------------------------------------------------------
static CFrame* gFrame = nullptr;
static EditorStub gEditor;

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_ERASEBKGND:
        return 1; // Suppress default erase — VSTGUI child covers entire client area
    case WM_TIMER:
        if (wParam == 1)
        {
            KillTimer(hwnd, 1);
            // Repaint: invalidate VSTGUI frame + all child windows
            if (gFrame)
                gFrame->invalid();
            EnumChildWindows(hwnd, [](HWND child, LPARAM) -> BOOL {
                InvalidateRect(child, nullptr, TRUE);
                return TRUE;
            }, 0);
            InvalidateRect(hwnd, nullptr, TRUE);
            return 0;
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}

//------------------------------------------------------------------------
// Entry point
//------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
    // Initialize VSTGUI
    VSTGUI::init(hInstance);

    // Disable DirectComposition (not implemented in Wine, causes ~10s timeout)
    if (auto win32Factory = getPlatformFactory().asWin32Factory())
        win32Factory->disableDirectComposition();

    // Register window class
    WNDCLASSEX wcex = {};
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = hInstance;
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszClassName = L"VSTGUITestApp";
    RegisterClassEx(&wcex);

    // Create window
    HWND hwnd = CreateWindowEx(
        0, L"VSTGUITestApp", L"VSTGUI D2D1 Test - Wine Rendering",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 620, 500,
        nullptr, nullptr, hInstance, nullptr);

    if (!hwnd)
    {
        VSTGUI::exit();
        return 1;
    }

    // Get actual client area size for CFrame
    RECT clientRect;
    GetClientRect(hwnd, &clientRect);
    CRect frameSize(0, 0, clientRect.right, clientRect.bottom);
    gFrame = new CFrame(frameSize, &gEditor);
    gEditor.frame = gFrame;

    // Add our main content view
    auto mainView = new MainView(frameSize);
    gFrame->addView(mainView);

    // Open frame on native window
    gFrame->open(hwnd);

    // Draw header text using a CTextLabel
    auto titleLabel = new CTextLabel(CRect(20, 10, 420, 35));
    titleLabel->setText("VSTGUI D2D1 Test - Wine Rendering Issues");
    titleLabel->setFontColor(kColorBlack);
    titleLabel->setBackColor(kColorWhite);
    titleLabel->setFrameColor(kColorWhite);
    titleLabel->setHoriAlign(kLeftText);
    mainView->addView(titleLabel);

    auto knobLabel = new CTextLabel(CRect(50, 222, 150, 240));
    knobLabel->setText("Drag to adjust");
    knobLabel->setFontColor(kColorBlack);
    knobLabel->setBackColor(kColorWhite);
    knobLabel->setFrameColor(kColorWhite);
    knobLabel->setHoriAlign(kCenterText);
    mainView->addView(knobLabel);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // Schedule a deferred repaint to handle race condition where the window
    // appears before VSTGUI's child window has finished its initial paint.
    // The timer fires after the message loop starts, ensuring WM_PAINT is processed.
    SetTimer(hwnd, 1, 50, nullptr);

    // Message loop
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Cleanup
    gFrame->close();
    VSTGUI::exit();

    return (int)msg.wParam;
}
