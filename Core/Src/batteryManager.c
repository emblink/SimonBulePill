#include "batteryManager.h"
#include "adc.h"
#include "generic.h"

#define ADC_VREF_MV 3335
#define R1 19820
#define R2 50000

typedef struct {
    int32_t percent;
    int32_t voltageMv;
} BatteryLevel;

static const BatteryLevel batteryCurve[] = {
    {0,   3312},
    {10,  3670},
    {30,  3754},
    {50,  3795},
    {70,  3905},
    {90,  4043},
    {100, 4150},
};

static int32_t interpolate(BatteryLevel startPoint, BatteryLevel endPoint, int32_t currentVoltage)
{
    // (y - y0)   (y1 - y0)
    //  ------  =  ------- 
    // (x - x0)   (x1 - x0)

    // x = x0 + (y - y0) * (x1 - x0) / (y1 - y0)

    int32_t currentPercent = startPoint.percent + (currentVoltage - startPoint.voltageMv) * 
            (endPoint.percent - startPoint.percent) / (endPoint.voltageMv - startPoint.voltageMv);

    return currentPercent;
}

int32_t batteryManagerGetPercent()
{
    /* Start single measurement */
    HAL_ADC_Start(&hadc1);
    /* Wait until measurement is completed */
    HAL_ADC_PollForConversion(&hadc1, 100);
    /* Obtain the measured value*/
    int32_t adcVal = HAL_ADC_GetValue(&hadc1);
    int64_t vol = ADC_VREF_MV * adcVal / 4095;
    vol = vol * (R1 + R2) / R2;

    if (vol < batteryCurve[0].voltageMv) {
        return 0;
    }

    if (vol > batteryCurve[ELEMENTS(batteryCurve) - 1].voltageMv) {
        return 100;
    }

    for (int i = 0; i < ELEMENTS(batteryCurve); i++) {
        if (vol < batteryCurve[i].voltageMv) {
            return interpolate(batteryCurve[i - 1], batteryCurve[i], vol);
        }
    }

    return 0;
}
