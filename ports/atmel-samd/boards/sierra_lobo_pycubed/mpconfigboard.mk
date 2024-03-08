
USB_VID = 0x04D8
USB_PID = 0xEC44
USB_PRODUCT = "PyCubed"
USB_MANUFACTURER = "Sierra Lobo"

CHIP_VARIANT = SAMD51N20A
CHIP_FAMILY = samd51

QSPI_FLASH_FILESYSTEM = 1
EXTERNAL_FLASH_DEVICES = MR2xH40
LONGINT_IMPL = MPZ

CIRCUITPY_FULL_BUILD = 1

CIRCUITPY_ASYNCIO = 1
CIRCUITPY_ATEXIT = 1
CIRCUITPY_OS_GETENV = 1
CIRCUITPY_TRACEBACK = 1
CIRCUITPY_TRACEBACK = 1
CIRCUITPY_ULAB = 1
CIRCUITPY_BINASCII = 1
CIRCUITPY_SDCARDIO = 1
CIRCUITPY_JSON = 1
CIRCUITPY_RE = 1
CIRCUITPY_MSGPACK = 1
CIRCUITPY_ALARM = 1
CIRCUITPY_USB_CDC = 1
CIRCUITPY_BTREE = 1
# no SAMD51 support... yet ;)
# CIRCUITPY_DUALBANK=1

# Not needed
CIRCUITPY_AUDIOBUSIO = 0
CIRCUITPY_AUDIOCORE = 0
CIRCUITPY_AUDIOIO = 0
CIRCUITPY_AUDIOMIXER = 0
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
CIRCUITPY_TERMINALIO = 0
CIRCUITPY_USB_HID = 0
CIRCUITPY_USB_MIDI = 0
CIRCUITPY_ROTARYIO = 0
CIRCUITPY_COUNTIO = 0
CIRCUITPY_GIFIO = 0

FROZEN_MPY_DIRS += $(TOP)/frozen/Adafruit_CircuitPython_NeoPixel
FROZEN_MPY_DIRS += $(TOP)/frozen/Adafruit_CircuitPython_Register
FROZEN_MPY_DIRS += $(TOP)/frozen/adcs
FROZEN_MPY_DIRS += $(TOP)/frozen/Adafruit_CircuitPython_GPS
FROZEN_MPY_DIRS += $(TOP)/frozen/pycubed_rfm9x
FROZEN_MPY_DIRS += $(TOP)/frozen/Adafruit_CircuitPython_RockBlock
