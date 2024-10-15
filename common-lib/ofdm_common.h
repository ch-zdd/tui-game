#ifndef OFDM_COMMON_H
#define OFDM_COMMON_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "data_handle.h"

#define FM_STS_LENGTH 16
#define FM_LTS_LENGTH 64


//Tweakable Parameters
#define FM_PLATEAU_THRESHOLD 0.9
#define FM_STS_PLATEAU_LENGTH 16

#define CONVERT_TO_COMPLEX(x, y) (x+y*I)

typedef enum sample_tag{
    NONE,           //!< No tag
    STS_START,      //!< Approximate start of STS
    STS_END,        //!< Approximate end of STS
    LTS_START,      //!< Estimated beginning of LTS (i.e. first sample of the LTS Cyclic prefix
    LTS1,           //!< Estimated beginning of first LTS symbol (32 samples after LTS_START
    LTS2,           //!< Estimated beginning of second LTS symbol (64 samples after LTS1
    START_OF_FRAME, //!< Estimated beginning of frame i.e. Signal symbol (64 samples after LTS2)
}fm_sample_tag;

typedef struct fm_tagged_sample
{
    cd_t sample; //!< The complex sample
    fm_sample_tag tag;              //!< The sample's tag
}fm_tagged_sample_t;

typedef struct fm_tagged_vector
{
    cd_t samples[64]; //!< The complex sample
    fm_sample_tag tag;              //!< The sample's tag
}fm_tagged_vector_t;

enum{
    fm_ofdm_mem_idx_data_in = 0,
    fm_ofdm_mem_idx_data_out,
    fm_ofdm_mem_idx_tmp_data_in,
    fm_ofdm_mem_idx_tmp_data_out,
    fm_ofdm_mem_idx_last
};

enum{
    fm_process_frame_detector = 0,
    fm_process_timing_sync,
    fm_process_fft_symbols,
    fm_process_last
};

#define FM_OFDM_MEM_MAX_SIZE 1024*32

uint8_t* fm_ofdm_mem_get(int32_t process_name, int32_t mem_idx);
void fm_fft_init(int fft_length);
void fm_fft_forward(cd_t data[64]);
void fm_fft_inverse(cd_t* data, size_t data_size);

#ifdef __cplusplus
}
#endif

#endif