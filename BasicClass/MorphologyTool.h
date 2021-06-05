#pragma once

#ifdef UNIX_OS
#include "PThreadTool.h"
#else

#ifdef _WINRT_METRO
#include "ThreadTool_Metro.h"
#else
#include "ThreadTool.h"
#endif

#endif

#define MULTITHREAD_MACROBLOCK_SIZE 256 // must be a multiple of 16

class MorphologyTool
{
public:
    MorphologyTool();
    ~MorphologyTool();
    void Initialize(int width, int height);
    void FillHole(BYTE *p_src_dst, int src_stride, int max_iteration);
    void FillHoleBinary(BYTE *p_src_dst, int src_stride, int max_iteration);
    void Close(BYTE *p_src_dst, int src_stride, int close_parameter);
    void CloseFillHole(BYTE *p_src_dst, int src_stride, int close_parameter, int max_iteration, bool is_src_binary = false);
    void BoundGradient(BYTE *p_src_dst, int src_stride, int bound_value);
    void Dilate(BYTE *p_src_dst, int stride, int dilate_parameter);
    void Erode(BYTE *p_src_dst, int stride, int erode_parameter);
    void GradientMap(BYTE *p_src, int src_stride, short *p_dst, int dst_stride, int &max_gradient, int &min_gradient);
    void SetThreadPool(ThreadPool *p_thread_pool);

private:
    int m_width;
    int m_height;
    int m_align_width;
    int m_align_height;
    BYTE *mp_align_buffer;

    // handle the src buffer not align
    BYTE *GetAlignBuffer(BYTE *p_src, int src_stride);
    void FreeAndCopyResult(BYTE *p_dst, int dst_stride);

    // fill water
    BYTE *mp_left_bound;
    BYTE *mp_right_bound;
    BYTE *mp_top_bound;
    BYTE *mp_bottom_bound;
    void FillHole16x16TopLeft(BYTE *p_src, BYTE *p_water, BYTE *p_top, BYTE *p_left);
    void FillHole16x16BottomRight(BYTE *p_src, BYTE *p_water, BYTE *p_bottom, BYTE *p_right);
    void FillHole16nx16mTopLeft(BYTE *p_src, BYTE *p_water, BYTE *p_top, BYTE *p_left, int stride, int num_block_i, int num_block_j);
    void FillHole16nx16mBottomRight(BYTE *p_src, BYTE *p_water, BYTE *p_bottom, BYTE *p_right, int stride, int num_block_i, int num_block_j);
    void FillHoleBinary16x16TopLeft(BYTE *p_src, BYTE *p_water, BYTE *p_top, BYTE *p_left);
    void FillHoleBinary16x16BottomRight(BYTE *p_src, BYTE *p_water, BYTE *p_bottom, BYTE *p_right);
    void BoundGradient16x16TopLeft(BYTE *p_src, BYTE *p_top, BYTE *p_left, int bound_value);
    void BoundGradient16x16BottomRight(BYTE *p_src, BYTE *p_bottom, BYTE *p_right, int bound_value);
    void BoundGradient16nx16mTopLeft(BYTE *p_src, BYTE *p_top, BYTE *p_left, int src_stride, int bound_value, int num_block_i, int num_block_j);
    void BoundGradient16nx16mBottomRight(BYTE *p_src, BYTE *p_bottom, BYTE *p_right, int src_stride, int bound_value, int num_block_i, int num_block_j);

    // Closing
    void LocalMinFilter5x5(BYTE *p_src_buffer, BYTE *p_min_buffer, BYTE *p_temp_buffer, 
                           int height, int width, int stride);
    void LocalMaxFilter5x5(BYTE *p_src_buffer, BYTE *p_max_buffer, BYTE *p_temp_buffer, 
                           int height, int width, int stride);
    void LocalMinFilter3x3(BYTE *p_src_buffer, BYTE *p_min_buffer, BYTE *p_temp_buffer, 
                           int height, int width, int stride);
    void LocalMaxFilter3x3(BYTE *p_src_buffer, BYTE *p_max_buffer, BYTE *p_temp_buffer, 
                           int height, int width, int stride);

    enum ThreadStage
    {
        NONE = 0,
        BOUND_GRADIENT_TOP_LEFT,
        BOUND_GRADIENT_BOTTOM_RIGHT,
        FILL_HOLE_TOP_LEFT,
        FILL_HOLE_BOTTOM_RIGHT
    };
    struct ThreadParameter
    {
        int thread_id;
        MorphologyTool *p_context;

        // for BoundGradient
        int align_width;
        int align_height;
        int block_size;
        int bound_value;
        BYTE *p_align_buffer;
        BYTE *p_top;
        BYTE *p_left;
        BYTE *p_bottom;
        BYTE *p_right;
        HANDLE *p_block_event;

        // for FillHole
        BYTE *p_water;
    };
    int m_stage;
    int m_num_thread;
    ThreadParameter *mp_thread_param;
    ThreadControlShell *mp_thread_control;
    ThreadPool *mp_thread_pool;
    void InitialThread();
    static ThreadFunctionReturnType WINAPI MorphologyToolMultiCore(LPVOID parameter);

    // macro block related
    int m_num_macroblock_i;
    int m_num_macroblock_j;
    HANDLE *mp_macroblock_event;
    void InitializeMacroBlock(int align_width, int align_height);
    void ResetMacroBlockEvent();
    void ReleaseMacroBlockEvent();
};
