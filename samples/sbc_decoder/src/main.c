#include <misc/printk.h>
#include <logging/sys_log.h>
#include <uart.h>

#ifdef __BEAR_SOC_H_
#include <uart_bear.h>
#endif

#include "formats.h"         // au_header
#include "sbc.h"             // declare sbc_decode function


#define CMD_SEND_CHUNK       0x01
#define CMD_RECEIVE_CHUNK    0x02
#define CMD_BREAK_OFF        0xFF

#define REC_BUF_SIZE         1024  // testing size
#define DEC_BUF_SIZE         1024


static sbc_t sbc;
static volatile bool send_header;
static volatile bool break_off;

static uint8_t rec_buf[REC_BUF_SIZE];
static uint8_t dec_buf[DEC_BUF_SIZE];


static uint32_t extract_frequency(uint32_t code)
{
    uint32_t frequency = 0;

    switch (code) {
        case SBC_FREQ_16000:
            frequency = 16000;
            break;

        case SBC_FREQ_32000:
            frequency = 32000;
            break;

        case SBC_FREQ_44100:
            frequency = 44100;
            break;

        case SBC_FREQ_48000:
            frequency = 48000;
            break;

        default:
            break;
    }

    return frequency;
}


static void poll_out_buf(struct device *dev, uint8_t *buf, uint32_t size)
{
    for (uint32_t i = 0; i < size; i++) {
        uart_poll_out(dev, buf[i]);
    }
}


static void init_au_header(struct au_header *au_hdr, sbc_t *sbc)
{
    uint32_t frequency = extract_frequency(sbc->frequency);
    uint32_t channels = (sbc->mode == SBC_MODE_MONO ? 1 : 2); 

    au_hdr->magic       = AU_MAGIC;
    au_hdr->hdr_size    = sys_cpu_to_be32(24);
    au_hdr->data_size   = sys_cpu_to_be32(0xFFFFFFFF);
    au_hdr->encoding    = sys_cpu_to_be32(AU_FMT_LIN16);
    au_hdr->sample_rate = sys_cpu_to_be32(frequency);
    au_hdr->channels    = sys_cpu_to_be32(channels);
}


static void uart_fifo_force_read(struct device *dev, uint8_t *buf, uint32_t size)
{
    do {
        uint32_t count = uart_fifo_read(dev, buf, size);
        buf += count;
        size -= count;
    } while (size > 0);
}


static void send_chunk(struct device *dev, uint8_t *buf, uint32_t size)
{
    uart_poll_out(dev, CMD_SEND_CHUNK);
    poll_out_buf(dev, (uint8_t *) &size, sizeof(size));
    poll_out_buf(dev, buf, size);
}


static uint32_t receive_chunk(struct device *dev, uint8_t *buf, uint32_t size)
{
    uint32_t expected_size = 0;
    uart_fifo_force_read(dev, (uint8_t *) &expected_size,
                         sizeof(expected_size));
    uart_fifo_force_read(dev, buf, expected_size);  // expected_size
                                                    // must be < size

    return expected_size;
}


static void handle_send_cmd(struct device *dev)
{
    uint32_t rec_size = receive_chunk(dev, rec_buf, REC_BUF_SIZE);
    
    uint32_t len = 0;
    int32_t framelen = sbc_decode(&sbc, rec_buf, rec_size,
                                  dec_buf, DEC_BUF_SIZE, &len);

    if (framelen < 0) {
        uart_poll_out(dev, CMD_BREAK_OFF);
        break_off = true;
        uart_irq_rx_disable(dev);

        return;
    }

    if (!send_header) {
        struct au_header au_hdr;
        init_au_header(&au_hdr, &sbc);
        send_chunk(dev, (uint8_t *) &au_hdr, sizeof(au_hdr));

        send_header = true;
    }

    send_chunk(dev, dec_buf, len);
}


static void decode_callback(struct device *dev)
{
    uart_irq_update(dev);

    uint8_t cmd = 0;
    uart_fifo_force_read(dev, &cmd, 1);

    switch (cmd) {
        case CMD_SEND_CHUNK:
            handle_send_cmd(dev);
            uart_poll_out(dev, CMD_RECEIVE_CHUNK);
            break;

        case CMD_BREAK_OFF:
            break_off = true;
            uart_irq_rx_disable(dev);
            break;
    }
}


static void decode(struct device *dev)
{
    if (!dev) {
        SYS_LOG_ERR("Cannot get UART device");
        return;
    }

    int init = sbc_init(&sbc, 0L);
    if (init) {
        SYS_LOG_ERR("Cannot init sbc struct");
        return;
    }

    sbc.endian = SBC_BE;
    send_header = false;
    break_off = false;

    uart_irq_callback_set(dev, decode_callback);
    uart_irq_rx_enable(dev);

    while (break_off == false) ;

    sbc_finish(&sbc);
}


void main(void)
{
    SYS_LOG_DBG("sbc decoding begin");

    struct device *dev = device_get_binding(CONFIG_UART_BEAR_PORT_0_NAME);
    decode(dev);

    SYS_LOG_DBG("sbc decoding end");

#ifdef __BEAR_SOC_H_ //Send MSI
    while (!(uart_drv_cmd(dev, UART_TX_EMPTY_CMD, 0) && uart_drv_cmd(dev, UART_TX_IDLE_CMD, 0))) ;

    (*((volatile unsigned int *)(BEAR_PCIE_MSI_BASE))) = 1;
#endif
}