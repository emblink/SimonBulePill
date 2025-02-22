#include "notePlayer.h"
#include "tim.h"
#include "generic.h"

static NoteStartCb startedCb = NULL;
static PlaybackCb finishedCb = NULL;

// 1 ms resolution timer
#define NOTE_TIMER TIM1
#define NOTE_TIMER_HANDLE htim1
#define NOTE_TIMER_CHANNEL TIM_CHANNEL_1
#define NOTE_TIMER_CCR (NOTE_TIMER->CCR1)

// 48Mhz clock, CCR1 = 1024 - 1 counter, updates by DMA from DMA_TIMER
#define PWM_TIMER TIM2
#define PWM_TIMER_HANDLE htim2
#define PWM_TIMER_CHANNEL TIM_CHANNEL_2
#define PWM_TIMER_CCR (PWM_TIMER->CCR2)

#define DMA_TIMER_CLOCK 72000000
#define DMA_TIMER TIM4
#define DMA_TIMER_HANDLE htim4
#define DMA_TIMER_CHANNEL_HANDLE hdma_tim4_up

const uint16_t sineLookupTable[] = {
	0, 1, 2, 6, 10, 15, 22, 30,
	39, 49, 60, 73, 86, 101, 116, 133,
	150, 168, 187, 207, 227, 249, 270, 293,
	316, 339, 363, 387, 412, 436, 461, 486,
	512, 537, 562, 587, 611, 636, 660, 684,
	707, 730, 753, 774, 796, 816, 836, 855,
	873, 890, 907, 922, 937, 950, 963, 974,
	984, 993, 1001, 1008, 1013, 1017, 1021, 1022,
	1023, 1022, 1021, 1017, 1013, 1008, 1001, 993,
	984, 974, 963, 950, 937, 922, 907, 890,
	873, 855, 836, 816, 796, 774, 753, 730,
	707, 684, 660, 636, 611, 587, 562, 537,
	512, 486, 461, 436, 412, 387, 363, 339,
	316, 293, 270, 249, 227, 207, 187, 168,
	150, 133, 116, 101, 86, 73, 60, 49,
	39, 30, 22, 15, 10, 6, 2, 1,
};

static const uint16_t fadeOutTable[] = {
	1023, 1022, 1021, 1017, 1013, 1008, 1001, 993,
	984, 974, 963, 950, 937, 922, 907, 890,
	873, 855, 836, 816, 796, 774, 753, 730,
	707, 684, 660, 636, 611, 587, 562, 537,
	512, 486, 461, 436, 412, 387, 363, 339,
	316, 293, 270, 249, 227, 207, 187, 168,
	150, 133, 116, 101, 86, 73, 60, 49,
	39, 30, 22, 15, 10, 6, 2, 1, 0
};

void notePlayerInit(NoteStartCb onStartCb, PlaybackCb onFinishCb)
{
    startedCb = onStartCb;
    finishedCb = onFinishCb;
}

static void XferCpltCallback(DMA_HandleTypeDef *hdma)
{
    HAL_TIM_PWM_Stop(&PWM_TIMER_HANDLE, PWM_TIMER_CHANNEL);
    DMA_TIMER_CHANNEL_HANDLE.Instance->CCR |= DMA_CCR_CIRC_Msk; // enable circular mode
    DMA_TIMER_CHANNEL_HANDLE.Instance->CCR &= ~DMA_CCR_TCIE_Msk; // disable interrupts
    if (finishedCb) {
        finishedCb();
    }
}

static Note *melody = NULL;
static uint32_t melodyLen = 0;
static uint32_t melodyNoteIdx = 0;
static uint32_t DestAddress = (uint32_t) &(PWM_TIMER_CCR);

static void stopPWM()
{
	HAL_DMA_Abort(&DMA_TIMER_CHANNEL_HANDLE);
	uint16_t pwm = PWM_TIMER_CCR;
	if (0 == pwm)
	{
		XferCpltCallback(NULL);
	} else {
		// search the closest smaller pwm
		int i = 0;
		while (i < ELEMENTS(fadeOutTable) && pwm < fadeOutTable[i])
		{
			i++;
		}
		// Start DMA transfer from the found point
		// Disable Bit 5 CIRC: Circular mode
		DMA_TIMER_CHANNEL_HANDLE.Instance->CCR &= ~DMA_CCR_CIRC_Msk;
		// Enable iterrupt on transfer complete
		DMA_TIMER_CHANNEL_HANDLE.Instance->CCR |= DMA_CCR_TCIE_Msk;
		DMA_TIMER_CHANNEL_HANDLE.XferCpltCallback= &XferCpltCallback;
		DMA_TIMER->CNT = 0;
		HAL_DMA_Start_IT(&DMA_TIMER_CHANNEL_HANDLE, (uint32_t)&fadeOutTable[i], DestAddress, ELEMENTS(fadeOutTable) - i);
		HAL_TIM_PWM_Start(&PWM_TIMER_HANDLE, PWM_TIMER_CHANNEL);
	}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim == &NOTE_TIMER_HANDLE) {
    	if (melody) {
    		melodyNoteIdx++;
    		if (melodyNoteIdx >= melodyLen) {
    			// Stop timer
    			melody = NULL;
    			melodyLen = 0;
    			HAL_TIM_OC_Stop_IT(&NOTE_TIMER_HANDLE, NOTE_TIMER_CHANNEL);
    			stopPWM();
    		} else {
    			notePlayerPlayNote(melody[melodyNoteIdx].note, melody[melodyNoteIdx].duration);
    		}
    	} else {
    		HAL_TIM_OC_Stop_IT(&NOTE_TIMER_HANDLE, NOTE_TIMER_CHANNEL);
    		stopPWM();
    	}
    }
}

void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim == &NOTE_TIMER_HANDLE) {
    	stopPWM();
    }
}

void notePlayerPlayNote(uint32_t noteHz, uint32_t durationMs)
{
    if (startedCb) {
        startedCb(noteHz, durationMs);
    }

	// Stop and clear all timers
	DMA_TIMER->CR1 &= ~TIM_CR1_CEN_Msk;
	PWM_TIMER->CR1 &= ~TIM_CR1_CEN_Msk;
	DMA_TIMER->CNT = 0;
	NOTE_TIMER->CNT = 0;
	PWM_TIMER->CNT = 0;

	// Start note duration timer
	NOTE_TIMER->SR &= ~TIM_SR_UIF_Msk; // prevent instant update interrupt after the initial timer config
	NOTE_TIMER_CCR = durationMs;
	NOTE_TIMER->ARR = durationMs * 2;
	HAL_TIM_Base_Start_IT(&NOTE_TIMER_HANDLE);
	HAL_TIM_OC_Start_IT(&NOTE_TIMER_HANDLE, NOTE_TIMER_CHANNEL);

	// Start PWM timer
	if (0 == noteHz) {
		return;
	}

	// Start DMA and PWM timers
	uint32_t sinwaveChunkPeriod = DMA_TIMER_CLOCK / (ELEMENTS(sineLookupTable) * noteHz);
	DMA_TIMER->ARR = sinwaveChunkPeriod - 1;
	__HAL_TIM_ENABLE_DMA(&DMA_TIMER_HANDLE, TIM_DMA_UPDATE);
	HAL_TIM_Base_Start(&DMA_TIMER_HANDLE);
	DMA_TIMER_CHANNEL_HANDLE.Instance->CCR |= DMA_CCR_CIRC_Msk; // enable circular mode
	DMA_TIMER_CHANNEL_HANDLE.Instance->CCR &= ~DMA_CCR_TCIE_Msk; // disable interrupts
	HAL_DMA_Start(&DMA_TIMER_CHANNEL_HANDLE, (uint32_t) sineLookupTable, DestAddress, ELEMENTS(sineLookupTable));
	HAL_TIM_PWM_Start(&PWM_TIMER_HANDLE, PWM_TIMER_CHANNEL);
	DMA_TIMER->CR1 |= TIM_CR1_CEN_Msk;
	PWM_TIMER->CR1 |= TIM_CR1_CEN_Msk;
}

void notePlayerPlayMelody(const Note mel[], uint32_t length)
{
	melody = (Note *) mel;
	melodyLen = length;
	melodyNoteIdx = 0;
	notePlayerPlayNote(melody[melodyNoteIdx].note, melody[melodyNoteIdx].duration);
}

bool notePlayerIsPlaying()
{
    bool isMelodyPlaying = (NULL == melody) ? false : true;
    bool isNotePlaying = NOTE_TIMER->CR1 & TIM_CR1_CEN_Msk;
    return isMelodyPlaying || isNotePlaying;
}

void notePlayerStop()
{
	melody = NULL;
	melodyLen = 0;
	melodyNoteIdx = 0;
	HAL_TIM_OC_Stop_IT(&NOTE_TIMER_HANDLE, NOTE_TIMER_CHANNEL);
	stopPWM();
}