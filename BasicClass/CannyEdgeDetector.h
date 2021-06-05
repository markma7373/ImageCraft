#pragma once

#include "use_hylib.h"

class CannyEdgeDetector
{
public:
    int m_width;
    int m_height;
    int m_stride; // for mp_x_derivative and mp_y_derivative, in short units
    HyImage *mp_edge_image;

    short *mp_x_derivative;
    short *mp_y_derivative;

    CannyEdgeDetector();
    ~CannyEdgeDetector();

    bool AllocateMemory(int width, int height, bool is_allocate_edge_image = true);
    void FreeMemory();

    void SetDisableIpp(bool is_disable_ipp);

    void SetThresholds(float low_threshold, float high_threshold)
    {
        m_low_threshold = low_threshold;
        m_high_threshold = high_threshold;
    }

    bool DetectEdge(const HyImage *const p_grey_image, HyImage *p_edge_image = NULL);

private:
    bool m_is_disable_ipp;

    float m_low_threshold;
    float m_high_threshold;

    inline void StackPush(BYTE *p_label);
    inline BYTE *StackPop();
    inline void CannyPushLabel(int &flag, BYTE *p_label, int label_step, bool is_pass_threshold);

    // The stack of labels for edge connection.
    BYTE **mpp_stack; // array of BYTE *
    BYTE **mpp_stack_top;

    ChCritSec m_data_lock;

    void StackResize(int new_size);
    void FreeStack();

    bool DoDetectEdge(const BYTE *const p_grey_image_data, const int grey_image_stride, HyImage *p_edge_image);

    bool DetectEdgeByIpp(const BYTE *const p_grey_image_data, const int grey_image_stride, HyImage *p_edge_image);
    bool DetectEdgeByC(const BYTE *const p_grey_image_data, const int grey_image_stride, HyImage *p_edge_image);

    void FilterSobelBorder(int width, int height,
                           const BYTE *p_src_data, int src_step,
                           short *p_dx_data, int dx_step, short *p_dy_data, int dy_step);
    void CannyEdgeDetect(const short *p_dx_data, int dx_step, const short *p_dy_data, int dy_step,
                         BYTE *p_edge_data, int edge_step, int width, int height, float low_threshold, float high_threshold);
};
