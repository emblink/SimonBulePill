
void batteryManagerInit()
{

}

uint32_t batteryManagerGetCharge()
{
    /* Start single measurement */
    HAL_ADC_Start(&hadc1);
    /* Wait until measurement is completed */
    HAL_ADC_PollForConversion(&hadc1, 100);
    /* Obtain the measured value*/
    uint32_t value = HAL_ADC_GetValue(&hadc1);

    return value;
}
