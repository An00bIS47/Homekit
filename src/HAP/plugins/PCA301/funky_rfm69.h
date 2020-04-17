/**
 * @brief Funky RFM69 Access Library
 *
 * Just another RFM69 library but currently only with enough functionality to
 * setup a ELV PCA301 compatible communication.
 *
 * This library was created by extensively reading source code and forum
 * comments contributed by other authors. Thank you.
 *
 * Warning: Try to avoid accessing the RFM69 registers (IRQFLAGS, FIFO) during
 * the ISR. This led to some unexpected deadlocks during development.
 *
 * Copyright (c) 2017, Sven Bachmann <dev@mcbachmann.de>
 *
 * Licensed under the MIT license, see LICENSE for details.
 */
#ifndef RFM69_LIB_H
#define RFM69_LIB_H

/*****************************************************************************/
/* Defines */
/*****************************************************************************/
#define RFM69_UNIT_KILO                             1000
#define RFM69_UNIT_MEGA                             1000000

#define RFM69_FREQ_FXOSC_HZ                         (32.0 * RFM69_UNIT_MEGA)
#define RFM69_FREQ_FSTEP_HZ                         (RFM69_FREQ_FXOSC_HZ / 524288)

#define RFM69_TIMEOUT_MS                            1000


/*****************************************************************************/
/* SPI */
/*****************************************************************************/
#define SPI_WRITE                                   0x80


/*****************************************************************************/
/* 0x00 RegFifo */
/*****************************************************************************/
#define RFM69_REG_FIFO                              0x00


/*****************************************************************************/
/* 0x01 RegOpMode */
/*****************************************************************************/
#define RFM69_REG_OPMODE                            0x01
#define RFM69_MSK_OPMODE_MODE                       0x07
#define RFM69_SHF_OPMODE_MODE                       2

#define RFM69_OPMODE_STANDBY                        0x01
#define RFM69_OPMODE_TX                             0x03
#define RFM69_OPMODE_RX                             0x04


/*****************************************************************************/
/* 0x03 RegBitrateMsb */
/* 0x04 RegBitrateLsb */
/*****************************************************************************/
#define RFM69_REG_BITRATEMSB                        0x03
#define RFM69_REG_BITRATELSB                        0x04


/*****************************************************************************/
/* 0x05 RegFdevMsb */
/* 0x06 RegFdevLsb */
/*****************************************************************************/
#define RFM69_REG_FDEVMSB                           0x05
#define RFM69_REG_FDEVLSB                           0x06

#define RFM69_MSK_FDEVMSB_FDEV                      0x3f
#define RFM69_SHF_FDEVMSB_FDEV                      0

#define RFM69_MSK_FDEVLSB_FDEV                      0xff
#define RFM69_SHF_FDEVLSB_FDEV                      0


/*****************************************************************************/
/* 0x07 RegFrfMsb */
/* 0x08 RegFrfMid */
/* 0x09 RegFrfLsb */
/*****************************************************************************/
#define RFM69_REG_FRFMSB                            0x07
#define RFM69_REG_FRFMID                            0x08
#define RFM69_REG_FRFLSB                            0x09


/*****************************************************************************/
/* 0x11 RegPaLevel */
/*****************************************************************************/
#define RFM69_REG_PALEVEL                           0x11

#define RFM69_MSK_PALEVEL_PA_ON                     0x07
#define RFM69_SHF_PALEVEL_PA_ON                     5

#define RFM69_MSK_PALEVEL_OUTPUTPOWER               0x1f
#define RFM69_SHF_PALEVEL_OUTPUTPOWER               0

#define RFM69_PA_0_ON                               0x04
#define RFM69_PA_1_ON                               0x02
#define RFM69_PA_2_ON                               0x01
#define RFM69_PA_POUT_MAX                           RFM69_MSK_PALEVEL_OUTPUTPOWER


/*****************************************************************************/
/* 0x13 RegOcp */
/*****************************************************************************/
#define RFM69_REG_OCP                               0x13

#define RFM69_MSK_OCP_OCP_ON                        0x01
#define RFM69_SHF_OCP_OCP_ON                        4


/*****************************************************************************/
/* 0x19 RegRxBw */
/*****************************************************************************/
#define RFM69_REG_RXBW                              0x19

#define RFM69_MSK_RXBW_RXBWEXP                      0x07
#define RFM69_SHF_RXBW_RXBWEXP                      0


/*****************************************************************************/
/* 0x25 RegDioMapping1 */
/* 0x26 RegDioMapping2 */
/*****************************************************************************/
#define RFM69_REG_DIOMAPPING1                       0x25
#define RFM69_REG_DIOMAPPING2                       0x26
#define RFM69_MSK_DIOMAPPING                        3

#define RFM69_DIO0_RX_CRCOK_TX_PACKETSENT           0x00
#define RFM69_DIO0_RX_PAYLOADREADY_TX_TXREADY       0x01

#define RFM69_MSK_DIOMAPPING2_CLKOUT                0x07
#define RFM69_SHF_DIOMAPPING2_CLKOUT                0
#define RFM69_CLKOUT_OFF                            0x07


/*****************************************************************************/
/* 0x27 RegIrqFlags1 */
/*****************************************************************************/
#define RFM69_REG_IRQFLAGS1                         0x27

#define RFM69_MSK_IRQFLAGS1_MODEREADY               0x01
#define RFM69_SHF_IRQFLAGS1_MODEREADY               7

#define RFM69_MSK_IRQFLAGS1_RXREADY                 0x01
#define RFM69_SHF_IRQFLAGS1_RXREADY                 6

#define RFM69_MSK_IRQFLAGS1_TXREADY                 0x01
#define RFM69_SHF_IRQFLAGS1_TXREADY                 5


/*****************************************************************************/
/* 0x28 RegIrqFlags2 */
/*****************************************************************************/
#define RFM69_REG_IRQFLAGS2                         0x28

#define RFM69_MSK_IRQFLAGS2_FIFOOVERRUN             0x01
#define RFM69_SHF_IRQFLAGS2_FIFOOVERRUN             4

#define RFM69_MSK_IRQFLAGS2_PACKETSENT              0x01
#define RFM69_SHF_IRQFLAGS2_PACKETSENT              3

#define RFM69_MSK_IRQFLAGS2_PAYLOADREADY            0x01
#define RFM69_SHF_IRQFLAGS2_PAYLOADREADY            2


/*****************************************************************************/
/* 0x29 RegRssiThresh */
/*****************************************************************************/
#define RFM69_REG_RSSITHRESH                        0x29


/*****************************************************************************/
/* 0x2E RegSyncConfig */
/* 0x2F RegSyncValue1 */
/*****************************************************************************/
#define RFM69_REG_SYNCCONFIG                        0x2e
#define RFM69_REG_SYNCVALUE1                        0x2f

#define RFM69_MSK_SYNCCONFIG_SYNCON                 0x01
#define RFM69_SHF_SYNCCONFIG_SYNCON                 7

#define RFM69_MSK_SYNCCONFIG_SYNCSIZE               0x07
#define RFM69_SHF_SYNCCONFIG_SYNCSIZE               3


/*****************************************************************************/
/* 0x37 RegPacketConfig1 */
/*****************************************************************************/
#define RFM69_REG_PACKETCONFIG1                     0x37

#define RFM69_MSK_PACKETCONFIG1_PACKETFORMAT        0x01
#define RFM69_SHF_PACKETCONFIG1_PACKETFORMAT        7

#define RFM69_MSK_PACKETCONFIG1_CRCON               0x01
#define RFM69_SHF_PACKETCONFIG1_CRCON               4

#define RFM69_MSK_PACKETCONFIG1_CRCAUTOCLEAROFF     0x01
#define RFM69_SHF_PACKETCONFIG1_CRCAUTOCLEAROFF     3


/*****************************************************************************/
/* 0x38 PayloadLength */
/*****************************************************************************/
#define RFM69_REG_PAYLOADLENGTH                     0x38


/*****************************************************************************/
/* 0x3c RegFifoThresh */
/*****************************************************************************/
#define RFM69_REG_FIFOTHRESH                        0x3c

#define RFM69_MSK_FIFOTHRESH_TXSTARTCONDITION       0x01
#define RFM69_SHF_FIFOTHRESH_TXSTARTCONDITION       7
#define RFM69_FIFO_LEVEL                            0
#define RFM69_FIFO_NOT_EMPTY                        1


/*****************************************************************************/
/* 0x3d RegPacketConfig2 */
/*****************************************************************************/
#define RFM69_REG_PACKETCONFIG2                     0x3d

#define RFM69_MSK_PACKETCONFIG2_RXRESTART           0x01
#define RFM69_SHF_PACKETCONFIG2_RXRESTART           2
#define RFM69_RXRESTART                             1


/*****************************************************************************/
/* 0x5a RegTestPa1 */
/* 0x5c RegTestPa2 */
/*****************************************************************************/
#define RFM69_REG_TESTPA1                           0x5a
#define RFM69_REG_TESTPA2                           0x5c

#define RFM69_PA20DBM1_NORMAL                       0x55
#define RFM69_PA20DBM1_20DBM_MODE                   0x5d

#define RFM69_PA20DBM2_NORMAL                       0x70
#define RFM69_PA20DBM2_20DBM_MODE                   0x7c


/*****************************************************************************/
/* Prototypes */
/*****************************************************************************/
void rfm69_init(
    uint8_t spi_bus,                            /**< SPI busnum (VSPI)*/
    int8_t pin_spi_sck,                         /**< SPI CLOCK pin (18)*/
    int8_t pin_spi_miso,                        /**< SPI MISO pin (19)*/
    int8_t pin_spi_mosi,                        /**< SPI MOSI pin (19)*/
    uint8_t pin_spi_ss,                         /**< SPI slave select pin */
    uint8_t flg_is_rfm69hw                      /**< output power flag */
);

uint8_t rfm69_reg_read_raw(
    uint8_t addr                                /**< register address */
);

void rfm69_reg_write_raw(
    uint8_t addr,                               /**< register address */
    uint8_t val                                 /**< value */
);

uint8_t rfm69_reg_read(
    uint8_t addr,                               /**< register address */
    uint8_t mask,                               /**< value mask */
    uint8_t shift                               /**< value shift */
);

void rfm69_reg_rw(
    uint8_t addr,                               /**< register address */
    uint8_t mask,                               /**< value mask */
    uint8_t shift,                              /**< value shift */
    uint8_t val                                 /**< value */
);

uint8_t rfm69_opmode_get(
    void
);

void rfm69_opmode_set(
    uint8_t mode                                /**< transceiver mode */
);

void rfm69_freq_carrier_khz(
    uint32_t freq_khz                           /**< carrier frequency in kHz */
);

void rfm69_bitrate_bs(
    uint16_t bitrate_bs                         /**< bitrate in b/s */
);

void rfm69_dio_mapping_rx(
    uint8_t dio,                                /**< DIO number */
    uint8_t val                                 /**< map value */
);

void rfm69_dio_mapping_tx(
    uint8_t dio,                                /**< DIO number */
    uint8_t val                                 /**< map value */
);

void rfm69_dio_mapping(
    uint8_t dio,                                /**< DIO number */
    uint8_t val                                 /**< map value */
);

void rfm69_clkout(
    uint8_t clkout                              /**< CLKOUT config */
);

void rfm69_crc_on(
    bool on                                     /**< CRC calculation on flag */
);

void rfm69_crc_auto_clear_off(
    bool off                                    /**< CRC auto clear off flag */
);

void rfm69_payload_length(
    uint8_t len                                 /**< payload length */
);

void rfm69_sync_on(
    bool on                                     /**< sync on flag */
);

void rfm69_sync_word(
    uint8_t size,                               /**< sync word size */
    uint8_t *values                             /**< sync values */
);

void rfm69_rx_bw_exp(
    uint8_t exp                                 /**< exponent */
);

void rfm69_rssi_threshold(
    uint8_t threshold                           /**< threshold */
);

void rfm69_fifo_clear(
    void
);

void rfm69_isr(
  void
);

bool rfm69_fifo_data_avail(
    void
);

uint8_t rfm69_fifo_data(
    void
);

void rfm69_send(
    uint8_t len,                                /**< data length */
    uint8_t *data                               /**< data */
);

void rfm69_packet_format_var_len(
    bool var_len                                /**< variable length flag */
);

void rfm69_tx_start_cond(
    uint8_t val                                 /**< TX start condition */
);

void rfm69_timer_loop(
    void
);

void rfm69_int_enable(
    void
);

void rfm69_int_disable(
    void
);

void rfm69_fdev_hz(
    uint16_t fdev_hz                            /**< value in Hz */
);

void rfm69_pa_sel(
    uint8_t pa_sel                              /**< power amplifier mask */
);

void rfm69_output_power(
    uint8_t val                                 /**< output power in percent */
);

bool rfm69_rx_avail(
    void
);

void rfm69_ocp(
    bool on                                     /**< OCP on flag */
);

void rfm69_high_power_pa(
    bool on                                     /**< high power PA */
);


#endif /* RFM69_LIB_H */
