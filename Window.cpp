#include "Window.h"

static const uint32_t SCR_WIDTH    = GetSystemMetrics(SM_CXSCREEN);
static const uint32_t SCR_HEIGHT   = GetSystemMetrics(SM_CYSCREEN);

Window* Window::mpWindow = nullptr;


LRESULT CALLBACK MainWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    return Window::GetWindowPointer()->WindowProc(
        hwnd, msg, wParam, lParam
    );
}

LRESULT Window::WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
        case WM_CLOSE: {
            mIsRun = false;
            DestroyWindow(hwnd);
        }
        case WM_DESTROY: {
            PostQuitMessage(0);
            break;
        }
        case WM_KEYDOWN: 
            mKeyStates[wParam & 511] = true;
            break;
        case WM_KEYUP: 
            mKeyStates[wParam & 511] = false;
            if (wParam == VK_ESCAPE)
            {
                mIsRun = false;
                PostQuitMessage(0);
            }
            break; 
        case WM_MOUSEMOVE:
            mPosX = static_cast<float>(GET_X_LPARAM(lParam));
            mPosY = static_cast<float>(GET_Y_LPARAM(lParam));
            break;
        case WM_MOUSEWHEEL:
            mDeltaZ = static_cast<float>(GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA);
            break;
        case WM_LBUTTONDOWN:
            mButtonStates[VK_LBUTTON] = true;
            break;
        case WM_LBUTTONUP:
            mButtonStates[VK_LBUTTON] = false;
            break;
        case WM_RBUTTONDOWN:
            mButtonStates[VK_RBUTTON] = true;
            break;
        case WM_RBUTTONUP:
            mButtonStates[VK_RBUTTON] = false;
            break;
        // case WM_LBUTTONDOWN:
        //     mButtonStates[VK_LBUTTON] = true;
        //     POINT point;
        //     GetCursorPos(&point);
        //     ScreenToClient(hwnd, &point);
        //     mPosX = point.x;
        //     mPosY = point.y;
        //     break;
        // case WM_LBUTTONUP:
        //     mButtonStates[VK_LBUTTON] = false;
        //     break;
        default: return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return true;
}

Window::Window(uint32_t width, uint32_t height)
    : mWidth(width), mHeight(height)
{

    WNDCLASSEX wc{ };
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = L"SoftWareRenderer";
    wc.lpfnWndProc = MainWindowProc;

    if (!RegisterClassEx(&wc)) {
        throw std::runtime_error("Failed to register WNDCLASSEX!");
    }

    RECT rect{ 0, 0, static_cast<LONG>(mWidth), static_cast<LONG>(mHeight) };

    AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW, FALSE, WS_EX_OVERLAPPEDWINDOW);

    uint32_t leftTopX   = SCR_WIDTH / 2 - mWidth / 2;
    uint32_t leftTopY   = SCR_HEIGHT / 2 - mHeight / 2;
    
    mHwnd = CreateWindowEx(
        WS_EX_OVERLAPPEDWINDOW,
        wc.lpszClassName,
        L"SoftwareRenderer",
        WS_OVERLAPPEDWINDOW,
        // CW_USEDEFAULT,
        // CW_USEDEFAULT,
        leftTopX,
        leftTopY,
        mWidth,
        mHeight,
        nullptr,
        nullptr,
        nullptr,
        this
    );

    if (!mHwnd) {
        throw std::runtime_error("Failed to initialize Window!");
    }
    
    ShowWindow(mHwnd, SW_SHOW);

    mIsRun = true;

    mColorBuffer.resize(mWidth * mHeight);

    mpWindow = this;
}


bool Window::Run()
{
    HandleMessage();
    UpdateWindowColor();

    return mIsRun;
}

void Window::HandleMessage()
{
    MSG msg{};
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) 
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}


void Window::SwapColorBuffer(std::vector<uint32_t>& colorBuffer)
{
    std::copy(colorBuffer.begin(), colorBuffer.end(), mColorBuffer.begin());
    // colorBuffer.swap(mColorBuffer);
}

void Window::UpdateWindowColor()
{
    mBitmapInfo.bmiHeader.biSize        = sizeof(mBitmapInfo.bmiHeader);
    mBitmapInfo.bmiHeader.biWidth       = mWidth;
    mBitmapInfo.bmiHeader.biHeight      = mHeight;
    mBitmapInfo.bmiHeader.biPlanes      = 1;
    mBitmapInfo.bmiHeader.biBitCount    = 32;
    mBitmapInfo.bmiHeader.biCompression = BI_RGB;

    HDC hdc = GetDC(mHwnd);

    StretchDIBits(hdc,
                  0,
                  0,
                  mWidth,
                  mHeight,
                  0,
                  0,
                  mWidth,
                  mHeight,
                  mColorBuffer.data(),
                  &mBitmapInfo,
                  DIB_RGB_COLORS,
                  SRCCOPY);
                  
    ReleaseDC(mHwnd, hdc);
}

Window::~Window()
{
    ShowWindow(mHwnd, SW_HIDE);

    // if (m_hdc != nullptr) {
    //     DeleteDC(m_hdc);
    //     m_hdc = nullptr;
    // }
    if (mHwnd != nullptr) {
        DestroyWindow(mHwnd);
        mHwnd = nullptr;
    }

}

void Window::SetPixel(int x, int y, uint32_t color) {
    mColorBuffer[y*mWidth + x] = color;
}
