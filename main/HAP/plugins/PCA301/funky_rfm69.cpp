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

#include <Arduino.h>
#include <SPI.h>
#include <limits.h>
#include "funky_rfm69.h"


/*****************************************************************************/
/* Local variables */
/*****************************************************************************/
static uint8_t rfm69_pin_spi_ss = 0;            /**< SPI slave select pin */
static bool rfm69_var_len = 0;                  /**< variable length flag */
static uint8_t rfm69_opmode = 0xff;             /**< operation mode */
static uint64_t rfm69_ts64 = 0;                 /**< 64-bit timestamp */
static bool rfm69_flg_isr = false;              /**< ISR flag */
static bool rfm69_flg_is_hw = false;            /**< RFM69HW flag */
static uint8_t rfm69_dio_mapping_rx_dio = 0xff; /**< RX DIO selector */
static uint8_t rfm69_dio_mapping_rx_val;        /**< RX DIO value */
static uint8_t rfm69_dio_mapping_tx_dio = 0xff; /**< TX DIO selector */
static uint8_t rfm69_dio_mapping_tx_val;        /**< TX DIO value */

//uninitalised pointers to SPI objects
static SPIClass * vspi = NULL;

/*****************************************************************************/
/* Local prototypes */
/*****************************************************************************/
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


/*****************************************************************************/
/** RFM69 SPI Initialization
 */
void rfm69_init(
    uint8_t spi_bus,                            /**< SPI busnum             (VSPI)  */
    int8_t pin_spi_sck,                         /**< SPI CLOCK pin          (18)    */
    int8_t pin_spi_miso,                        /**< SPI MISO pin           (19)    */
    int8_t pin_spi_mosi,                        /**< SPI MOSI pin           (23)    */
    uint8_t pin_spi_ss,                         /**< SPI SLAVE SELECT pin   (5)     */
    uint8_t flg_is_rfm69hw                      /**< output power flag              */
)
{
    vspi = new SPIClass(spi_bus);

    /* store pin for communication */
    rfm69_pin_spi_ss = pin_spi_ss;

    /* store variant for output power control */
    rfm69_flg_is_hw = flg_is_rfm69hw;

    /* configure SPI */
    pinMode(rfm69_pin_spi_ss, OUTPUT);
    vspi->begin(pin_spi_sck, pin_spi_miso, pin_spi_mosi, rfm69_pin_spi_ss);
    vspi->setBitOrder(MSBFIRST);

    /* configure power amplifiers in regard to the used variant */
    if (flg_is_rfm69hw) {
        rfm69_pa_sel(RFM69_PA_1_ON | RFM69_PA_2_ON);
        rfm69_ocp(false);
    } else {
        rfm69_pa_sel(RFM69_PA_0_ON);
    }
}


/*****************************************************************************/
/** RFM69 Read Full Register
 */
uint8_t rfm69_reg_read_raw(
    uint8_t addr                                /**< register address */
)
{
    uint8_t val;

    vspi->beginTransaction(SPISettings());
    digitalWrite(SS, LOW);
    vspi->transfer(0x00 | addr);
    val = vspi->transfer(0);
    digitalWrite(SS, HIGH);
    vspi->endTransaction();

    return val;
}


/*****************************************************************************/
/** RFM69 Write Full Register
 */
void rfm69_reg_write_raw(
    uint8_t addr,                               /**< register address */
    uint8_t val                                 /**< value */
)
{
    vspi->beginTransaction(SPISettings());
    digitalWrite(SS, LOW);
    vspi->transfer(SPI_WRITE | addr);
    vspi->transfer(val);
    digitalWrite(SS, HIGH);
    vspi->endTransaction();
}


/*****************************************************************************/
/** RFM69 Read Register Value
 */
uint8_t rfm69_reg_read(
    uint8_t addr,                               /**< register address */
    uint8_t mask,                               /**< value mask */
    uint8_t shift                               /**< value shift */
)
{
    return (rfm69_reg_read_raw(addr) >> shift) & mask;
}


/*****************************************************************************/
/** RFM69 Update Register Value
 *
 * Reads a register, updates the value and writes it back.
 */
void rfm69_reg_rw(
    uint8_t addr,                               /**< register address */
    uint8_t mask,                               /**< value mask */
    uint8_t shift,                              /**< value shift */
    uint8_t val                                 /**< value */
)
{
    rfm69_reg_write_raw(addr, (rfm69_reg_read_raw(addr) & ~(mask << shift)) | ((val & mask) << shift));
}


/*****************************************************************************/
/** RFM69 Interrupt Handler
 */
void rfm69_isr(
    void
)
{
    rfm69_flg_isr = true;
}


/*****************************************************************************/
/** RFM69 Get Operation Mode
 */
uint8_t rfm69_opmode_get(
    void
)
{
    /* check if mode is already known */
    if (0xff == rfm69_opmode) {

        /* get mode */
        rfm69_opmode = rfm69_reg_read(RFM69_REG_OPMODE,
                                      RFM69_MSK_OPMODE_MODE,
                                      RFM69_SHF_OPMODE_MODE);
    }

    return rfm69_opmode;
}


/*****************************************************************************/
/** RFM69 Set Operation Mode
 */
void rfm69_opmode_set(
    uint8_t mode                                /**< transceiver mode */
)
{
    uint64_t ts64;                              /* timeout timestamp */

    /* configure DIO mapping if set */
    if (RFM69_OPMODE_RX == mode) {
        if (rfm69_dio_mapping_rx_dio != 0xff) {
            rfm69_dio_mapping(rfm69_dio_mapping_rx_dio, rfm69_dio_mapping_rx_val);
        }
    }
    else if (RFM69_OPMODE_TX == mode) {
        if (rfm69_dio_mapping_tx_dio != 0xff) {
            rfm69_dio_mapping(rfm69_dio_mapping_tx_dio, rfm69_dio_mapping_tx_val);
        }
    }

    /* clear ISR flag */
    rfm69_flg_isr = false;

    /* set mode */
    rfm69_reg_rw(RFM69_REG_OPMODE,
                 RFM69_MSK_OPMODE_MODE,
                 RFM69_SHF_OPMODE_MODE,
                 mode);

    /* wait until mode is ready */
    ts64 = rfm69_ts64 + RFM69_TIMEOUT_MS;
    while (!rfm69_reg_read(RFM69_REG_IRQFLAGS1,
                           RFM69_MSK_IRQFLAGS1_MODEREADY,
                           RFM69_SHF_IRQFLAGS1_MODEREADY)) {

        rfm69_timer_loop();
        if (rfm69_ts64 >= ts64) {
            Serial.println("opmode: timeout");
            break;
        }
    }

    /* enable high power output for RFM69HW if mode is TX */
    if (rfm69_flg_is_hw) {
        if (RFM69_OPMODE_TX == mode) {
            rfm69_high_power_pa(true);
        } else {
            rfm69_high_power_pa(false);
        }
    }

    /* restart RX if mode is RX */
    if (RFM69_OPMODE_RX == mode) {
        rfm69_reg_rw(RFM69_REG_PACKETCONFIG2,
                     RFM69_MSK_PACKETCONFIG2_RXRESTART,
                     RFM69_SHF_PACKETCONFIG2_RXRESTART,
                     RFM69_RXRESTART);
    }

    /* update global opmode */
    rfm69_opmode = mode;
}


/*****************************************************************************/
/** RFM69 Carrier Frequency in kHz
 *
 * Example: 868000 for 868 MHz.
 */
void rfm69_freq_carrier_khz(
    uint32_t freq_khz                           /**< carrier frequency in kHz */
)
{
    uint32_t frf;                               /* carrier frequency register value */

    frf = freq_khz / (RFM69_FREQ_FSTEP_HZ / RFM69_UNIT_KILO);

    rfm69_reg_write_raw(RFM69_REG_FRFMSB, (uint8_t) (frf >> 16));
    rfm69_reg_write_raw(RFM69_REG_FRFMID, (uint8_t) (frf >> 8));
    rfm69_reg_write_raw(RFM69_REG_FRFLSB, (uint8_t) frf);
}


/*****************************************************************************/
/** RFM69 Bitrate in b/s
 *
 * Example: 6631 for 6.631 kb/s.
 */
void rfm69_bitrate_bs(
    uint16_t bitrate_bs                         /**< bitrate in b/s */
)
{
    uint16_t bitrate;                           /* bitrate register value */

    bitrate = RFM69_FREQ_FXOSC_HZ / bitrate_bs;

    rfm69_reg_write_raw(RFM69_REG_BITRATEMSB, (uint8_t) (bitrate >> 8));
    rfm69_reg_write_raw(RFM69_REG_BITRATELSB, (uint8_t) bitrate);
}


/*****************************************************************************/
/** RFM69 DIO Pin Mapping for RX
 */
void rfm69_dio_mapping_rx(
    uint8_t dio,                                /**< DIO number */
    uint8_t val                                 /**< map value */
)
{
    rfm69_dio_mapping_rx_dio = dio;
    rfm69_dio_mapping_rx_val = val;
}


/*****************************************************************************/
/** RFM69 DIO Pin Mapping for TX
 */
void rfm69_dio_mapping_tx(
    uint8_t dio,                                /**< DIO number */
    uint8_t val                                 /**< map value */
)
{
    rfm69_dio_mapping_tx_dio = dio;
    rfm69_dio_mapping_tx_val = val;
}


/*****************************************************************************/
/** RFM69 DIO Pin Mapping
 */
void rfm69_dio_mapping(
    uint8_t dio,                                /**< DIO number */
    uint8_t val                                 /**< map value */
)
{
    uint8_t reg;
    uint8_t shift;

    if (4 > dio) {
        reg = RFM69_REG_DIOMAPPING1;
        shift = 6 - (dio * 2);
    } else {
        reg = RFM69_REG_DIOMAPPING2;
        shift = 6 - ((dio - 4) * 2);
    }

    rfm69_reg_rw(reg, RFM69_MSK_DIOMAPPING, shift, val);
}


/*****************************************************************************/
/** RFM69 Control CLKOUT
 */
void rfm69_clkout(
    uint8_t clkout                              /**< CLKOUT config */
)
{
    rfm69_reg_rw(RFM69_REG_DIOMAPPING2,
                 RFM69_MSK_DIOMAPPING2_CLKOUT,
                 RFM69_SHF_DIOMAPPING2_CLKOUT,
                 clkout);
}


/*****************************************************************************/
/** RFM69 CRC Calculation Control
 */
void rfm69_crc_on(
    bool on                                     /**< CRC calculation on flag */
)
{
    rfm69_reg_rw(RFM69_REG_PACKETCONFIG1,
                 RFM69_MSK_PACKETCONFIG1_CRCON,
                 RFM69_SHF_PACKETCONFIG1_CRCON,
                 (on) ? 1 : 0);
}


/*****************************************************************************/
/** RFM69 CRC Auto Clear Control
 */
void rfm69_crc_auto_clear_off(
    bool off                                    /**< CRC auto clear off flag */
)
{
    rfm69_reg_rw(RFM69_REG_PACKETCONFIG1,
                 RFM69_MSK_PACKETCONFIG1_CRCAUTOCLEAROFF,
                 RFM69_SHF_PACKETCONFIG1_CRCAUTOCLEAROFF,
                 (off) ? 1 : 0);
}


/*****************************************************************************/
/** RFM69 Payload Length
 */
void rfm69_payload_length(
    uint8_t len                                 /**< payload length */
)
{
    rfm69_reg_write_raw(RFM69_REG_PAYLOADLENGTH, len);
}


/*****************************************************************************/
/** RFM69 Sync Word Generation And Detection
 */
void rfm69_sync_on(
    bool on                                     /**< sync on flag */
)
{
    rfm69_reg_rw(RFM69_REG_SYNCCONFIG,
                 RFM69_MSK_SYNCCONFIG_SYNCON,
                 RFM69_SHF_SYNCCONFIG_SYNCON,
                 (on) ? 1 : 0);
}


/*****************************************************************************/
/** RFM69 Sync Word Size
 */
void rfm69_sync_word(
    uint8_t size,                               /**< sync word size */
    uint8_t *values                             /**< sync values */
)
{
    unsigned int cnt;                           /* counter */

    /* sync size always add +1 so decrement size here */
    rfm69_reg_rw(RFM69_REG_SYNCCONFIG,
                 RFM69_MSK_SYNCCONFIG_SYNCSIZE,
                 RFM69_SHF_SYNCCONFIG_SYNCSIZE,
                 size - 1);

    /* fill sync values */
    for (cnt = 0; cnt < size; cnt++) {
        rfm69_reg_write_raw(RFM69_REG_SYNCVALUE1 + cnt, values[cnt]);
    }
}


/*****************************************************************************/
/** RFM69 Channel Filter Bandwidth Control
 */
void rfm69_rx_bw_exp(
    uint8_t exp                                 /**< exponent */
)
{
    rfm69_reg_rw(RFM69_REG_RXBW,
                 RFM69_MSK_RXBW_RXBWEXP,
                 RFM69_SHF_RXBW_RXBWEXP,
                 exp);
}


/*****************************************************************************/
/** RFM69 RSSI Threshold
 *
 * Default: 228 (0xe4) => 228 / 2 = -114 dBm
 */
void rfm69_rssi_threshold(
    uint8_t threshold                           /**< threshold */
)
{
    rfm69_reg_write_raw(RFM69_REG_RSSITHRESH, threshold);
}


/*****************************************************************************/
/** RFM69 Clear Fifo
 */
void rfm69_fifo_clear(
    void
)
{
    rfm69_reg_write_raw(RFM69_REG_IRQFLAGS2,
                        RFM69_MSK_IRQFLAGS2_FIFOOVERRUN << RFM69_SHF_IRQFLAGS2_FIFOOVERRUN);
}


/*****************************************************************************/
/** RFM69 Fifo Data Available
 */
bool rfm69_fifo_data_avail(
    void
)
{
    return (rfm69_reg_read(RFM69_REG_IRQFLAGS2,
                           RFM69_MSK_IRQFLAGS2_PAYLOADREADY,
                           RFM69_SHF_IRQFLAGS2_PAYLOADREADY)) ? true : false;
}


/*****************************************************************************/
/** RFM69 Fifo Data
 */
uint8_t rfm69_fifo_data(
    void
)
{
    return rfm69_reg_read_raw(RFM69_REG_FIFO);
}


/*****************************************************************************/
/** RFM69 Send Data
 *
 * Send given data and switch back to RX mode.
 */
void rfm69_send(
    uint8_t len,                                /**< data length */
    uint8_t *data                               /**< data */
)
{
    uint64_t ts64;                              /* timeout timestamp */

    /* restart RX to avoid RX deadlocks */
    rfm69_reg_rw(RFM69_REG_PACKETCONFIG2,
                 RFM69_MSK_PACKETCONFIG2_RXRESTART,
                 RFM69_SHF_PACKETCONFIG2_RXRESTART,
                 RFM69_RXRESTART);

    /* disable receiver and interrupts */
    rfm69_opmode_set(RFM69_OPMODE_STANDBY);
    rfm69_fifo_clear();
    rfm69_int_disable();

    /* transfer data */
    digitalWrite(SS, LOW);
    
    vspi->beginTransaction(SPISettings());
    vspi->transfer(SPI_WRITE | RFM69_REG_FIFO);

    for (; len; len--, data++) {
        vspi->transfer(*data);
    }
    digitalWrite(SS, HIGH);
    vspi->endTransaction();

    /* enable interrupts and send frame */
    rfm69_int_enable();
    rfm69_opmode_set(RFM69_OPMODE_TX);

    /* wait until data was sent
     * (ISR flag is cleared at next mode set)
     */
    ts64 = rfm69_ts64 + RFM69_TIMEOUT_MS;
    while (true != rfm69_flg_isr) {

        rfm69_timer_loop();
        if (rfm69_ts64 >= ts64) {
            Serial.println("send: timeout");
            rfm69_fifo_clear();
            break;
        }
    }

    /* switch back to receive mode */
    rfm69_opmode_set(RFM69_OPMODE_STANDBY);
    rfm69_opmode_set(RFM69_OPMODE_RX);
}


/*****************************************************************************/
/** RFM69 Packet Format
 */
void rfm69_packet_format_var_len(
    bool var_len                                /**< variable length flag */
)
{
    rfm69_var_len = var_len;

    rfm69_reg_rw(RFM69_REG_PACKETCONFIG1,
                 RFM69_MSK_PACKETCONFIG1_PACKETFORMAT,
                 RFM69_SHF_PACKETCONFIG1_PACKETFORMAT,
                 (var_len) ? 1 : 0);
}


/*****************************************************************************/
/** RFM69 TX Start Condition
 */
void rfm69_tx_start_cond(
    uint8_t val                                 /**< TX start condition */
)
{
    rfm69_reg_rw(RFM69_REG_FIFOTHRESH,
                 RFM69_MSK_FIFOTHRESH_TXSTARTCONDITION,
                 RFM69_SHF_FIFOTHRESH_TXSTARTCONDITION,
                 val);
}


/*****************************************************************************/
/** Timer Overflow Handler
 */
void rfm69_timer_loop(
    void
)
{
    static unsigned long ts_last = millis();    /* last checked timestamp */
    unsigned long ts = millis();                /* current timestamp */

    if (ts == ts_last) {
        return;
    }
    else if (ts > ts_last) {
        rfm69_ts64 += ts - ts_last;
    }
    else {
        rfm69_ts64 += (ULONG_MAX - ts_last) + ts;
    }

    ts_last = ts;
}


/*****************************************************************************/
/** RFM69 Frequency Deviation in Hz
 */
void rfm69_fdev_hz(
    uint16_t fdev_hz                            /**< value in Hz */
)
{
    uint16_t fdev;                              /* frequency deviation reg val */

    fdev = fdev_hz / RFM69_FREQ_FSTEP_HZ;

    rfm69_reg_write_raw(RFM69_REG_FDEVMSB, (uint8_t) (fdev >> 8));
    rfm69_reg_write_raw(RFM69_REG_FDEVLSB, (uint8_t) fdev);
}


/*****************************************************************************/
/** RFM69 Power Amplifier Selection
 */
void rfm69_pa_sel(
    uint8_t pa_sel                              /**< power amplifier mask */
)
{
    rfm69_reg_rw(RFM69_REG_PALEVEL,
                 RFM69_MSK_PALEVEL_PA_ON,
                 RFM69_SHF_PALEVEL_PA_ON,
                 pa_sel);
}


/*****************************************************************************/
/** RFM69 Output Power Level in Percent
 *
 * RFM69W = -18 .. 13 dBm => 0 = -18 dBm, 50 = -3 dBm, 100 = 13 dBm
 * RFM69HW = +5 .. 20 dBm => 0 = 5 dBm, 50 = 12 dBm, 100 = 20 dBM
 */
void rfm69_output_power(
    uint8_t val                                 /**< output power in percent */
)
{
    if (rfm69_flg_is_hw) {
        val = (val * (20 - 5)) / 100;
    } else {
        val = (val * (13 - (-18))) / 100;
    }

    rfm69_reg_rw(RFM69_REG_PALEVEL,
                 RFM69_MSK_PALEVEL_OUTPUTPOWER,
                 RFM69_SHF_PALEVEL_OUTPUTPOWER,
                 val);
}


/*****************************************************************************/
/** RFM69 Packet Receive Check
 */
bool rfm69_rx_avail(
    void
)
{
    if (RFM69_OPMODE_RX != rfm69_opmode) {
        return false;
    }

    if (true == rfm69_flg_isr) {
        rfm69_flg_isr = false;
        return true;
    }

    return rfm69_fifo_data_avail();
}


/*****************************************************************************/
/** RFM69 Over Current Protection
 */
void rfm69_ocp(
    bool on                                     /**< OCP on flag */
)
{
    rfm69_reg_rw(RFM69_REG_OCP,
                 RFM69_MSK_OCP_OCP_ON,
                 RFM69_SHF_OCP_OCP_ON,
                 !!on);
}


/*****************************************************************************/
/** RFM69 High Power Power Amplifier
 */
void rfm69_high_power_pa(
    bool on                                     /**< high power PA */
)
{
    rfm69_reg_write_raw(RFM69_REG_TESTPA1,
                        (on) ? RFM69_PA20DBM1_20DBM_MODE : RFM69_PA20DBM1_NORMAL);

    rfm69_reg_write_raw(RFM69_REG_TESTPA2,
                        (on) ? RFM69_PA20DBM2_20DBM_MODE : RFM69_PA20DBM2_NORMAL);
}
