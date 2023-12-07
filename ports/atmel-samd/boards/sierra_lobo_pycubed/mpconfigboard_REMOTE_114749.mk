
USB_VID = 0x04D8
USB_PID = 0xEC44
USB_PRODUCT = "PyCubed"
USB_MANUFACTURER = "SLI"

CHIP_VARIANT = SAMD51N20A
CHIP_FAMILY = samd51

QSPI_FLASH_FILESYSTEM = 1
EXTERNAL_FLASH_DEVICES = MR2xH40
LONGINT_IMPL = MPZ






CIRCUITPY_ULAB = 1
CIRCUITPY_BINASCII = 1
CIRCUITPY_SDCARDIO = 1
CIRCUITPY_JSON = 1
CIRCUITPY_MSGPACK = 1
CIRCUITPY_ALARM = 1
CIRCUITPY_USB_CDC = 1
# no SAMD51 support... yet ;)
# CIRCUITPY_DUALBANK=1

# Not needed

CIRCUITPY_AUDIOBUSIO = 0
CIRCUITPY_AUDIOCORE = 0
CIRCUITPY_AUDIOIO = 0
CIRCUITPY_AUDIOMIXER = 0
CIRCUITPY_AUDIOMP3 = 0
CIRCUITPY_DISPLAYIO = 0
CIRCUITPY_FRAMEBUFFERIO = 0
CIRCUITPY_PIXELMAP = 0
CIRCUITPY_GETPASS = 0
CIRCUITPY_KEYPAD = 0
CIRCUITPY_RGBMATRIX = 0
CIRCUITPY_PS2IO = 0
CIRCUITPY_BLEIO_HCI = 0
CIRCUITPY_BLEIO = 0
CIRCUITPY_FLOPPYIO = 0
CIRCUITPY_TOUCHIO = 0
CIRCUITPY_USB_HID = 0
CIRCUITPY_USB_MIDI = 0
FROZEN_MPY_DIRS += $(TOP)/frozen/Adafruit_CircuitPython_NeoPixel
FROZEN_MPY_DIRS += $(TOP)/frozen/Adafruit_CircuitPython_Register
FROZEN_MPY_DIRS += $(TOP)/frozen/adcs
FROZEN_MPY_DIRS += $(TOP)/frozen/Adafruit_CircuitPython_GPS
FROZEN_MPY_DIRS += $(TOP)/frozen/pycubed_rfm9x
