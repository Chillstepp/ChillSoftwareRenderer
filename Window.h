#pragma once

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

#ifndef UNICODE
#define UNICODE
#endif 

#include <windows.h>
#include <Windowsx.h>
#include <wingdi.h>
#include <cstdint>
#include <iostream>
#include <cassert>
#include <memory>
#include <vector>


class Window {
public:

    explicit Window(uint32_t width, uint32_t height);

    ~Window();

    static Window* GetWindowPointer()
    {
        return mpWindow;
    }

    bool Run(); 

    void SwapColorBuffer(std::vector<uint32_t>& colorBuffer);

    inline HWND GetWindowHandle() const
    {
        return mHwnd;
    }

    inline bool GetKeyState(uint32_t key) const
    {
        return mKeyStates[key];
    }

    inline bool GetButtonState(uint32_t button) const
    {
        return mButtonStates[button];
    }

    inline float GetMousePosX() const
    {
        return mPosX;
    }

    inline float GetMousePosY() const
    {
        return mPosY;
    }

    inline float GetDeltaZ() const
    {
        return mDeltaZ;
    }

    inline void SetDeltaZ(float deltaZ)
    {
        mDeltaZ = deltaZ;
    }

    inline void SetMousePosX(float posX)
    {
        mPosX = posX;
    }

    inline void SetMousePosY(float posY)
    {
        mPosY = posY;
    }


    LRESULT WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);    

protected:

    static Window* mpWindow;
    
private:

    void HandleMessage();

    void UpdateWindowColor();

    HWND mHwnd;

    std::vector<uint32_t> mColorBuffer;

    BITMAPINFO mBitmapInfo;

    uint32_t mWidth{ 1 };

    uint32_t mHeight{ 1 };


private:

    bool mIsRun{ false };
    bool mKeyStates[512]{ false };
    bool mButtonStates[3]{ false };

    float mPosX{ 0.0f };
    float mPosY{ 0.0f };
    float mDeltaZ{ 0.0f };

};
