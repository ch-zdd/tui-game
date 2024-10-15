#include <memory.h>
#include <fftw3.h>
#include <assert.h>
#include "ofdm_common.h"

struct{
    int m_fft_length;
    int fft_map[64];
    fftw_complex*  m_fftw_in_forward;
    fftw_complex* m_fftw_out_forward;
    fftw_complex* m_fftw_in_inverse;
    fftw_complex* m_fftw_out_inverse;
    fftw_plan m_fftw_plan_forward;
    fftw_plan m_fftw_plan_inverse;
}fm_fft_para;

//ofdm专供常驻内存
uint8_t fm_ofdm_memory[fm_process_last][fm_ofdm_mem_idx_last][FM_OFDM_MEM_MAX_SIZE];

uint8_t* fm_ofdm_mem_get(int32_t process_name, int32_t mem_idx)
{
    if(mem_idx >= fm_ofdm_mem_idx_last || mem_idx < fm_ofdm_mem_idx_data_in){
        return NULL;
    }

    if(process_name >= fm_process_last || process_name < fm_process_frame_detector){
        return NULL;
    }

    memset(fm_ofdm_memory[process_name][mem_idx], 0x00, FM_OFDM_MEM_MAX_SIZE);

    return fm_ofdm_memory[process_name][mem_idx];
}

void fm_fft_init(int fft_length)
{
    static int initialed_flag = 0;

    fm_fft_para.m_fft_length = fft_length;

    if(initialed_flag == 0){
        int map[64] =
        {
        32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31
        };
        memcpy(fm_fft_para.fft_map, map, sizeof(fm_fft_para.fft_map));

        // Allocate the FFT buffers
        fm_fft_para.m_fftw_in_forward = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * fm_fft_para.m_fft_length);
        fm_fft_para.m_fftw_out_forward = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * fm_fft_para.m_fft_length);
        fm_fft_para.m_fftw_in_inverse = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * fm_fft_para.m_fft_length);
        fm_fft_para.m_fftw_out_inverse = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * fm_fft_para.m_fft_length);
        fm_fft_para.m_fftw_plan_forward = fftw_plan_dft_1d(fm_fft_para.m_fft_length, fm_fft_para.m_fftw_in_forward, fm_fft_para.m_fftw_out_forward, FFTW_FORWARD, FFTW_MEASURE);
        fm_fft_para.m_fftw_plan_inverse = fftw_plan_dft_1d(fm_fft_para.m_fft_length, fm_fft_para.m_fftw_in_inverse, fm_fft_para.m_fftw_out_inverse, FFTW_BACKWARD, FFTW_MEASURE);
        initialed_flag=1;
    }
}

/*!
 * This function performs a single in-place 64 point FFT on the 64 complex data samples.
 * The user must loop over time-domain signal and pass each 64 sample symbol to this function
 * individually.
 * This function handles the shifting from all positive (0-63) indexing to
 * positive & negative frequency indexing.
 */
void fm_fft_forward(cd_t data[64])
{
    int s = 0;

    memcpy(fm_fft_para.m_fftw_in_forward, &data[0], fm_fft_para.m_fft_length * sizeof(cd_t));
    fftw_execute(fm_fft_para.m_fftw_plan_forward);

    for(s = 0; s < 64; s++)
    {
        memcpy(&data[s], &fm_fft_para.m_fftw_out_forward[fm_fft_para.fft_map[s]], sizeof(cd_t));
    }
}

/*!
 * This function loops over the input vector (which must be an integer multiple of 64)
 * and performs in-place 64 point IFFTs on each consecutive 64 sample chunk of the input vector.
 * This function handles the shifting from positive & negative frequency (-32 to 31) indexing to
 * all positive (0 to 63) indexing.
 * This function also scales the output by 1/64 to be consistent with the IFFT function.
 */
void fm_fft_inverse(cd_t* data, size_t data_size)
{
    assert(data_size % fm_fft_para.m_fft_length == 0);

    // Run the IFFT on each m_fft_length samples
    for(int x = 0; x < data_size; x += fm_fft_para.m_fft_length)
    {
        if(fm_fft_para.m_fft_length == 64)
        {
            for(int s = 0; s < 64; s++)
            {
                memcpy(&fm_fft_para.m_fftw_in_inverse[s], &data[x + fm_fft_para.fft_map[s]], sizeof(cd_t));
            }
        }
        else
        {
            memcpy(&fm_fft_para.m_fftw_in_inverse[0], &data[x], fm_fft_para.m_fft_length * sizeof(cd_t));
        }

        fftw_execute(fm_fft_para.m_fftw_plan_inverse);
        memcpy(&data[x], fm_fft_para.m_fftw_out_inverse, fm_fft_para.m_fft_length * sizeof(cd_t));
    }

    // Scale by 1/fft_length
    for(int x = 0; x < data_size; x++)
    {
        data[x] /= fm_fft_para.m_fft_length;
    }
}