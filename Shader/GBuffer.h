//
// Created by why on 2024/1/8.
//

#ifndef CHILLSOFTWARERENDERER_GBUFFER_H
#define CHILLSOFTWARERENDERER_GBUFFER_H

#include "../Math.h"
#include <map>

class BufferBase
{
public:
    int Width;
    int Height;
};

template<typename T>
class Buffer : public BufferBase
{
 public:
	T* Data;

 public:
	Buffer(Vec2i Size)
	{
		Width = Size.width;
		Height = Size.height;
		Data = new T[Width * Height];
	}
	~Buffer()
	{
		delete[] Data;
		Data = nullptr;
	}

	T* operator [](int k)
	{
		return &Data[k * Width];
	}

};

class GBuffer {
private:
	GBuffer() = default;
	~GBuffer();
	GBuffer(const GBuffer&) = default;
	GBuffer& operator = (const GBuffer&) = delete;

	std::map<std::string, BufferBase*>Buffers;

public:
	static GBuffer& Get()
	{
		static GBuffer Instance;
		return Instance;
	}

	template<typename BufferDataType>
	Buffer<BufferDataType>* GetBuffer(const std::string& BufferName) const{
        auto BufferFound = Buffers.find(BufferName);
        if(BufferFound == Buffers.end()) return nullptr;

        return BufferFound;
    }

    template<typename BufferDataType>
    void AddBuffer(const std::string& BufferName, Vec2i BufferSize) {
        BufferBase* BufferPtr = new Buffer<BufferDataType>(BufferSize);
        Buffers.insert({BufferName, BufferPtr});

    }

    template<typename BufferDataType>
    BufferDataType GetBufferElement(const std::string &BufferName, Vec2i BufferLocation) const{
        Buffer<BufferDataType>& Buffer = *GetBuffer<BufferDataType>(BufferName);
        return *Buffer[BufferLocation.x][BufferLocation.y];
    }
};


#endif //CHILLSOFTWARERENDERER_GBUFFER_H
