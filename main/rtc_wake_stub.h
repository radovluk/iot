/**
 * @brief Wake-up stub function executed during wake-up from deep sleep.
 *
 * This function runs in the minimal RTC fast memory environment and handles
 * minimal tasks required during wake-up, such as determining the wake-up cause,
 * handling sensor triggers, and managing deep sleep cycles.
 *
 * @note Logging and use of standard library functions are limited in this environment.
 */
void wake_stub(void);

/**
 * @brief Stores a PIR event with the current timestamp and device information.
 *
 * Calculates the actual timestamp based on RTC time and synchronization data,
 * then stores the event in the PIR events array.
 */
void store_pir_event(void);

/**
 * @brief Retrieves the current RTC time in microseconds.
 *
 * This function reads the RTC time registers to obtain the current time.
 * It operates in the wake-up stub environment and uses low-level register access.
 *
 * @return Current RTC time in microseconds.
 */
RTC_IRAM_ATTR uint64_t my_rtc_time_get_us(void);

