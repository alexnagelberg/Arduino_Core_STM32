/*
 * Copyright (c) 2010 by Cristian Maglie <c.maglie@arduino.cc>
 * Copyright (c) 2014 by Paul Stoffregen <paul@pjrc.com> (Transaction API)
 * SPI Master library for arduino.
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of either the GNU General Public License version 2
 * or the GNU Lesser General Public License version 2.1, both as
 * published by the Free Software Foundation.
 */

#include "SPI.h"

SPIClass SPI;

/**
  * @brief  Default constructor. Uses pin configuration of variant.h.
  */
SPIClass::SPIClass()
{
  _spi.pin_miso = digitalPinToPinName(MISO);
  _spi.pin_mosi = digitalPinToPinName(MOSI);
  _spi.pin_sclk = digitalPinToPinName(SCK);
  _spi.pin_ssel = NC;
}

/**
  * @brief  Constructor to create another SPI instance attached to another SPI
  *         peripheral different of the default SPI. All pins must be attached to
  *         the same SPI peripheral. See datasheet of the microcontroller.
  * @param  mosi: SPI mosi pin. Accepted format: number or Arduino format (Dx)
  *         or ST format (Pxy).
  * @param  miso: SPI miso pin. Accepted format: number or Arduino format (Dx)
  *         or ST format (Pxy).
  * @param  sclk: SPI clock pin. Accepted format: number or Arduino format (Dx)
  *         or ST format (Pxy).
  * @param  ssel: SPI ssel pin (optional). Accepted format: number or
  *         Arduino format (Dx) or ST format (Pxy). By default is set to NC.
  *         This pin must correspond to a hardware CS pin which can be managed
  *         by the SPI peripheral itself. See the datasheet of the microcontroller
  *         or look at PinMap_SPI_SSEL[] inside the file PeripheralPins.c
  *         corresponding to the board. If you configure this pin you can't use
  *         another CS pin and don't pass a CS pin as parameter to any functions
  *         of the class.
  */
SPIClass::SPIClass(uint32_t mosi, uint32_t miso, uint32_t sclk, uint32_t ssel)
{
  _spi.pin_miso = digitalPinToPinName(miso);
  _spi.pin_mosi = digitalPinToPinName(mosi);
  _spi.pin_sclk = digitalPinToPinName(sclk);
  _spi.pin_ssel = digitalPinToPinName(ssel);
}

/**
  * @brief  Initialize the SPI instance.
  */
void SPIClass::begin(void)
{
  _spi.handle.State = HAL_SPI_STATE_RESET;
  spi_init(&_spi, _spiSettings.clk,
           _spiSettings.dMode,
           _spiSettings.bOrder);
}

/**
  * @brief  This function should be used to configure the SPI instance in case you
  *         don't use the default parameters set by the begin() function.
  * @param  settings: SPI settings(clock speed, bit order, data mode).
  */
void SPIClass::beginTransaction(SPISettings settings)
{
  _spiSettings.clk = settings.clk;
  _spiSettings.dMode = settings.dMode;
  _spiSettings.bOrder = settings.bOrder;
  _spiSettings.noReceive = settings.noReceive;

  spi_init(&_spi, _spiSettings.clk,
           _spiSettings.dMode,
           _spiSettings.bOrder);
}

/**
  * @brief  End the transaction after beginTransaction usage
  */
void SPIClass::endTransaction(void)
{

}

/**
  * @brief  Deinitialize the SPI instance and stop it.
  */
void SPIClass::end()
{
  spi_deinit(&_spi);
}

/**
  * @brief  Deprecated function.
  *         Configure the bit order: MSB first or LSB first.
  * @param  _bitOrder: MSBFIRST or LSBFIRST
  */
void SPIClass::setBitOrder(BitOrder bitOrder)
{
  _spiSettings.bOrder = bitOrder;

  spi_init(&_spi, _spiSettings.clk,
           _spiSettings.dMode,
           _spiSettings.bOrder);
}

/**
  * @brief  Deprecated function.
  *         Configure the data mode (clock polarity and clock phase)
  * @param  _mode: SPI_MODE0, SPI_MODE1, SPI_MODE2 or SPI_MODE3
  * @note
  *         Mode          Clock Polarity (CPOL)   Clock Phase (CPHA)
  *         SPI_MODE0             0                     0
  *         SPI_MODE1             0                     1
  *         SPI_MODE2             1                     0
  *         SPI_MODE3             1                     1
  */
void SPIClass::setDataMode(uint8_t _mode)
{
  if (SPI_MODE0 == _mode) {
    _spiSettings.dMode = SPI_MODE_0;
  } else if (SPI_MODE1 == _mode) {
    _spiSettings.dMode = SPI_MODE_1;
  } else if (SPI_MODE2 == _mode) {
    _spiSettings.dMode = SPI_MODE_2;
  } else if (SPI_MODE3 == _mode) {
    _spiSettings.dMode = SPI_MODE_3;
  }

  spi_init(&_spi, _spiSettings.clk,
           _spiSettings.dMode,
           _spiSettings.bOrder);
}

/**
  * @brief  Deprecated function.
  *         Configure the clock speed
  * @param  _divider: the SPI clock can be divided by values from 1 to 255.
  *         If 0, default SPI speed is used.
  */
void SPIClass::setClockDivider(uint8_t _divider)
{
  if (_divider == 0) {
    _spiSettings.clk = SPI_SPEED_CLOCK_DEFAULT;
  } else {
    /* Get clk freq of the SPI instance and compute it */
    _spiSettings.clk = spi_getClkFreq(&_spi) / _divider;
  }

  spi_init(&_spi, _spiSettings.clk,
           _spiSettings.dMode,
           _spiSettings.bOrder);
}

/**
  * @brief  Transfer one byte on the SPI bus.
  *         begin() or beginTransaction() must be called at least once before.
  * @param  data: byte to send.
  * @return byte received from the slave.
  */
byte SPIClass::transfer(uint8_t data)
{
  uint8_t rx_buffer = 0;
  spi_transfer(&_spi, &data, &rx_buffer, sizeof(uint8_t), SPI_TRANSFER_TIMEOUT, _spiSettings.noReceive);
  return rx_buffer;
}

/**
  * @brief  Transfer two bytes on the SPI bus in 16 bits format.
  *         begin() or beginTransaction() must be called at least once before.
  * @param  data: bytes to send.
  * @return bytes received from the slave in 16 bits format.
  */
uint16_t SPIClass::transfer16(uint16_t data)
{
  uint16_t rx_buffer = 0;
  uint16_t tmp;

  if (_spiSettings.bOrder) {
    tmp = ((data & 0xff00) >> 8) | ((data & 0xff) << 8);
    data = tmp;
  }
  spi_transfer(&_spi, (uint8_t *)&data, (uint8_t *)&rx_buffer, sizeof(uint16_t),
               SPI_TRANSFER_TIMEOUT, _spiSettings.noReceive);

  if (_spiSettings.bOrder) {
    tmp = ((rx_buffer & 0xff00) >> 8) | ((rx_buffer & 0xff) << 8);
    rx_buffer = tmp;
  }

  return rx_buffer;
}

/**
  * @brief  Transfer several bytes. Only one buffer used to send and receive data.
  *         begin() or beginTransaction() must be called at least once before.
  * @param  _buf: pointer to the bytes to send. The bytes received are copy in
  *         this buffer.
  * @param  _count: number of bytes to send/receive.
  */
void SPIClass::transfer(void *_buf, size_t _count)
{
  if ((_count != 0) && (_buf != NULL)) {
    spi_transfer(&_spi, ((uint8_t *)_buf), ((uint8_t *)_buf), _count,
                 SPI_TRANSFER_TIMEOUT, _spiSettings.noReceive);
  }
}

/**
  * @brief  Transfer several bytes. One buffer contains the data to send and
  *         another one will contains the data received. begin() or
  *         beginTransaction() must be called at least once before.
  * @param  _bufout: pointer to the bytes to send.
  * @param  _bufin: pointer to the bytes received.
  * @param  _count: number of bytes to send/receive.
  */
void SPIClass::transfer(void *_bufout, void *_bufin, size_t _count)
{
  if ((_count != 0) && (_bufout != NULL) && (_bufin != NULL)) {
    spi_transfer(&_spi, ((uint8_t *)_bufout), ((uint8_t *)_bufin), _count,
                 SPI_TRANSFER_TIMEOUT, _spiSettings.noReceive);
  }
}

/**
  * @brief  Not implemented.
  */
void SPIClass::usingInterrupt(uint8_t interruptNumber)
{
  UNUSED(interruptNumber);
}

/**
  * @brief  Not implemented.
  */
void SPIClass::attachInterrupt(void)
{
  // Should be enableInterrupt()
}

/**
  * @brief  Not implemented.
  */
void SPIClass::detachInterrupt(void)
{
  // Should be disableInterrupt()
}

#if defined(SUBGHZSPI_BASE)
void SUBGHZSPIClass::begin()
{
  SPIClass::begin();
}

void SUBGHZSPIClass::beginTransaction(SPISettings settings)
{
  SPIClass::beginTransaction(settings);
}

byte SUBGHZSPIClass::transfer(uint8_t _data)
{
  byte res;
  res = SPIClass::transfer(_data);
  return res;
}

uint16_t SUBGHZSPIClass::transfer16(uint16_t _data)
{
  uint16_t rx_buffer = 0;
  rx_buffer = SPIClass::transfer16(_data);
  return rx_buffer;
}

void SUBGHZSPIClass::transfer(void *_buf, size_t _count)
{
  SPIClass::transfer(_buf, _count);
}

void SUBGHZSPIClass::transfer(void *_bufout, void *_bufin, size_t _count)
{
  SPIClass::transfer(_bufout, _bufin, _count);
}

void SUBGHZSPIClass::enableDebugPins(uint32_t mosi, uint32_t miso, uint32_t sclk, uint32_t ssel)
{
  /* Configure SPI GPIO pins */
  pinmap_pinout(digitalPinToPinName(mosi), PinMap_SPI_MOSI);
  pinmap_pinout(digitalPinToPinName(miso), PinMap_SPI_MISO);
  pinmap_pinout(digitalPinToPinName(sclk), PinMap_SPI_SCLK);
  pinmap_pinout(digitalPinToPinName(ssel), PinMap_SPI_SSEL);
}
#endif
