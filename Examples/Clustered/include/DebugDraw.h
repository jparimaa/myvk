#pragma once

#include "Helpers.h"

class DebugDraw
{
public:
    DebugDraw(){};
    ~DebugDraw(){};

    void initialize(const Buffers& buffers);
    void writeImages(const Matrices& matrices);

private:
    Buffers m_buffers;

    void writeLights(const Matrices& matrices);
    void writeTiles();
};
