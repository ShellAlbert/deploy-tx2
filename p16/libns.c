#include "../libns/libns.h"
// Initialize the processing
// Input
//      - mode  : the mode to process the data
// Return:
//			- 0 if OK, negative if fail

int ns_init(int mode)
{
    return 0;
}

// Initialize the processing for custom mode
// Input
//      - denoiseAlgorithm  : the algorithm of noise reduction
//      - denoiseLevel  : the level of noise reduction
//      - gainType  		: the type of gains of equalizer
//      - gainLevel  		: the level of gains of equalizer
//      - customBandGains  : the gains of eight bands
//      - preEmphasisFlag  : the flag of preemphasis
// Return:
//			- 0 if OK, negative if fail

int ns_custom_init(int denoiseAlgorithm,
                int denoiseLevel,
                int enhancedType,
                int enhancedLevel,
                char* customBandGains,
                char preEmphasisFlag)
{
    return 0;
}

// processing the noised speech
// Input
//      - pPcmAudio  : input data buffer of the noised speech
//											48KHz/2 Channels/16-bit PCM data
//      - nPcmLength : fixed 960 bytes, length of the data, 10ms
//
// Output:
//      - pPcmAudio : output data buffer of the denoised speech
// Return:
//			- 0 if OK, negative if fail

int ns_processing(char *pPcmAudio,const int nPcmLength)
{
    return 0;
}

// Uninitialize the processing
// Return:
//			- 0 if OK, negative if fail

int ns_uninit(void)
{
    return 0;
}

// Get the software version
// Output:
//      - currentVersion : the string of current version
void ns_getVersion(char* currentVersion)
{
    return 0;
}
