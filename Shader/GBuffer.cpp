//
// Created by why on 2024/1/8.
//

#include "GBuffer.h"

GBuffer::~GBuffer() {
    for(auto [KeyName, BufferPtr]: Buffers)
    {
        delete BufferPtr;
    }
}




