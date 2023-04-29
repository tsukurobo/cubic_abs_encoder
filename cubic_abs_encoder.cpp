#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"

#define SPI_PORT0 spi0
#define PIN_MOSI0 0
#define PIN_SS0   1
#define PIN_SCK0  2
#define PIN_MISO0 3

#define SPI_PORT1 spi1
#define PIN_MOSI1 11
const uint8_t PIN_SS1[] = {6, 24, 17, 10, 4, 22, 27, 19};
#define PIN_SCK1  26
#define PIN_MISO1 8

#define SPI_FREQ0 4000000
#define SPI_FREQ1 2000000

#define ENCODER_NUM 8
const uint8_t PIN_LED[] = {7, 25, 18, 12, 5, 23, 16, 9};
const uint8_t *READ_COMMAND = 0x00;
#define TIME_BYTES 3
#define ENCODER_BITS 16

#define ERR_VAL 0x7fff

#define DELAY_US 100

void print_vals(int16_t *enc_vals) {
    for (int i = 0; i < ENCODER_NUM; i++) {
        printf("%d ", enc_vals[i]);
    }
    printf("\n");
}

void read_encoder(uint16_t *enc_val, uint8_t enc_num) {
    // スレーブ選択
    gpio_put(PIN_SS1[enc_num], 0);
    sleep_us(TIME_BYTES);
    // データ送信
    spi_write_read_blocking(SPI_PORT1, READ_COMMAND, (uint8_t*)enc_val + 1, 1);
    sleep_us(TIME_BYTES);
    spi_write_read_blocking(SPI_PORT1, READ_COMMAND, (uint8_t*)enc_val, 1);
    sleep_us(TIME_BYTES);
    // スレーブ解除
    gpio_put(PIN_SS1[enc_num], 1);
}

void set_zero_point(uint8_t enc_num){
    // スレーブ選択
    gpio_put(PIN_SS1[enc_num], 0);
    sleep_us(TIME_BYTES);
    // データ送信
    spi_write_blocking(SPI_PORT1, READ_COMMAND, 1);
    sleep_us(TIME_BYTES);
    spi_write_blocking(SPI_PORT1, (const uint8_t *)0x70, 1);
    sleep_us(TIME_BYTES);
    sleep_us(250000);
    // スレーブ解除
    gpio_put(PIN_SS1[enc_num], 1);
}

// パリティチェックを行う
// | K1 K0 D14 D13 D12 D11 D10 D9 | D8 D7 D6 D5 D4 D3 D2 D1 | の16ビットのうち，K0とK1はパリティビット
// K1 = !(D14 xor D12 xor D10 xor D8 xor D6 xor D4 xor D2)
// K0 = !(D13 xor D11 xor D9 xor D7 xor D5 xor D3 xor D1)
bool parity_check(int16_t enc_val) {
    bool bit[16];
    for (int i = 0; i < 16; i++) {
        bit[i] = (enc_val >> i) & 1;
    }

    if (bit[15] == !(bit[13] ^ bit[11] ^ bit[9] ^ bit[7] ^ bit[5] ^ bit[3] ^ bit[1]) &&
        bit[14] == !(bit[12] ^ bit[10] ^ bit[8] ^ bit[6] ^ bit[4] ^ bit[2] ^ bit[0])) {
        return true;
    }
    return false;
}

// パリティビットを除去する
uint16_t remove_parity_bit(uint16_t enc_val) {
    return enc_val & 0x3fff;
}

#define PIN_RS 14

uint16_t enc_vals[ENCODER_NUM];

int main()
{
    stdio_init_all();
    // SPIピンの設定
    gpio_set_function(PIN_MOSI0, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SS0,   GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK0,  GPIO_FUNC_SPI);
    gpio_set_function(PIN_MISO0, GPIO_FUNC_SPI);

    // SPI初期化(周波数を4MHzに設定)
    spi_init(SPI_PORT0, SPI_FREQ0);
    spi_set_format(SPI_PORT0, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
    // スレーブでSPI通信開始
    spi_set_slave(SPI_PORT0, true);

    spi_init(SPI_PORT1, SPI_FREQ1);
    gpio_set_function(PIN_MOSI1, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK1,  GPIO_FUNC_SPI);
    gpio_set_function(PIN_MISO1, GPIO_FUNC_SPI);

    for (int i = 0; i < ENCODER_NUM; i++) {
        gpio_init(PIN_SS1[i]);
        gpio_set_dir(PIN_SS1[i], GPIO_OUT);
        gpio_put(PIN_SS1[i], 1);
        gpio_init(PIN_LED[i]);
        gpio_set_dir(PIN_LED[i], GPIO_OUT);
        gpio_put(PIN_LED[i], 0);
    }

    for (int i = 0; i < ENCODER_NUM; i++) {
        // set_zero_point(i);s
    }

    while(true) {
        // エンコーダの値を読み込む
        for (int i = 0; i < ENCODER_NUM; i++) {
            read_encoder(&enc_vals[i], i);
            //printf("%d ", parity_check(enc_vals[i]));
            if (parity_check(enc_vals[i])) {
                // LEDを点灯する
                gpio_put(PIN_LED[i], 1);
            } else {
                // LEDを消灯する
                gpio_put(PIN_LED[i], 0);
                // エラー値を代入する
                enc_vals[i] = ERR_VAL;
            }
            //enc_vals[i] = remove_parity_bit(enc_vals[i]);
        }
        //printf(" ");
        // usb通信は遅いため普段はコメントアウト
        //print_vals((int16_t*)enc_vals);

        spi_write_blocking(SPI_PORT0, (uint8_t*)enc_vals, ENCODER_BITS);

        sleep_us(DELAY_US);
    }

    return 0;
}
