# STM32F746G-DISCO_TouchGFX_Demo-1

This is a basic example on how to use the STM32F746G-DISCO board using the integration between CubeMX and TouchGFX, some of the features included on this demo are:

  - GPIO Control from GUI (LED1, GREEN)
  - GUI Control from GPIO (USER_BTN1)
  - Serial port as debug port (write-only)
    - ST-Link USB-VCP (USART1)
    - Arduino R3 connector Serial (Rx D0 & Tx D1, USART6)
  - FatFs basic example (on-board SD Card slot)
    - Scan files
    - File open/read/write
    - Create FAT volume (disabled by default)

# Enviroment

  - TouchGFX Designer (v4.10.0)
  - MCU: STM32F746NGHx
  - Board: STM32F746G-DISCOVERY
  - STM32CubeMX (v5.0.0)
  - STM32CubeF7 Firmware package (v1.14.0)
  - ST-Link Utility (v4.3.0.0)
  - FreeRTOS (v9.0.0)
  - FatFs (R0.12c)



License
----

MIT


**Free Software, Enjoy!**
