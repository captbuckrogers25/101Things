#include "PWMAudio.h"
#include <stdio.h>

PWMAudio :: PWMAudio(int audio_pin, int audio_sample_rate)
{
    //audio output pin
    gpio_set_function(audio_pin, GPIO_FUNC_PWM);
    gpio_set_drive_strength(audio_pin, GPIO_DRIVE_STRENGTH_12MA);

    //configure PWM
    const uint16_t pwm_max = 4096;//12 bit pwm at 30517Hz
    audio_pwm_slice_num = pwm_gpio_to_slice_num(audio_pin);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 1.f); //125MHz
    pwm_config_set_wrap(&config, pwm_max);    
    pwm_init(audio_pwm_slice_num, &config, true);

    //configure DMA for audio transfers
    pwm_dma = dma_claim_unused_channel(true);
    audio_cfg = dma_channel_get_default_config(pwm_dma);
    channel_config_set_transfer_data_size(&audio_cfg, DMA_SIZE_16);
    channel_config_set_read_increment(&audio_cfg, true);
    channel_config_set_write_increment(&audio_cfg, false);
    
    int dma_timer = dma_claim_unused_timer(true);
    dma_timer_set_fraction(dma_timer, 1, 125000000/audio_sample_rate);
    channel_config_set_dreq(&audio_cfg, dma_get_timer_dreq(dma_timer));    

gpio_init(1);
gpio_set_dir(1, GPIO_OUT);
gpio_init(2);
gpio_set_dir(2, GPIO_OUT);
}

void PWMAudio :: OutputSamples(uint16_t samples[], uint16_t len)
{
    if(!dma_channel_is_busy(pwm_dma)) printf("not ready for end of dma \n");
    dma_channel_wait_for_finish_blocking(pwm_dma);
    dma_channel_configure(pwm_dma, &audio_cfg, &pwm_hw->slice[audio_pwm_slice_num].cc, samples, len, true);
}
