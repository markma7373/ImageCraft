#pragma once

#include "Common.h"

class SumImageBoxFilter
{
public:
    SumImageBoxFilter()
    {

    }

    ~SumImageBoxFilter()
    {

    }

    void FilterBox(const BYTE *p_src_data, int src_step, BYTE *p_dst_data, int dst_step,
                   int width, int height, int radius, int *p_external_buffer = NULL);

    void FilterBox_C4R(const BYTE *p_src_data, int src_step, BYTE *p_dst_data, int dst_step,
                       int width, int height, int radius, int *p_external_buffer = NULL);
    void GetSumImage(const BYTE *p_src_data, int src_stride, int src_step,
                     int *p_dst_data, int dst_stride, int width, int height);
};
