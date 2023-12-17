import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor

# from esphome.components import spi
from esphome import pins
from esphome.const import (
    CONF_DEVICE,
    CONF_DATA_PINS,
    CONF_INCLUDES,
    CONF_MODE,
    CONF_FREQUENCY,
    CONF_CS_PIN,
    CONF_POWER,
    CONF_RESET_PIN,
    CONF_CLK_PIN,
    CONF_MISO_PIN,
    CONF_ID,
    CONF_MOSI_PIN,
    CONF_CURRENT,
    DEVICE_CLASS_BATTERY,
    DEVICE_CLASS_CURRENT,
    DEVICE_CLASS_POWER,
    STATE_CLASS_MEASUREMENT,
    UNIT_AMPERE,
    UNIT_PERCENT,
    UNIT_WATT,
    CONF_BATTERY_LEVEL,
)

# from esphome.core import CORE

# DEPENDENCIES = ["spi"]
CODEOWNERS = ["@Dianlight"]
MULTI_CONF = False

rtl_433_ns = cg.esphome_ns.namespace("rtl_433")
RTL433Component = rtl_433_ns.class_("RTL433Component", cg.Component)  # , spi.SPIDevice

RF_DEVICES = {
    "SX1278": "SX1278",
    "SX1276": "SX1276",
    "CC1101": "CC1101",
}

RF_MODES = {"FSK": "false", "OOK": "true"}

CONF_LED_PIN = "led_pin"
CONF_THRESHOLD_RSSI = "rssi_threshold"
CONF_SIGNAL_RSSI = "signal_rssi"


CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(RTL433Component),
            cv.Required(CONF_DEVICE): cv.enum(RF_DEVICES, upper=True),
            cv.Optional(CONF_INCLUDES, default=[]): cv.ensure_list(),
            # SPI BASED
            cv.Required(CONF_CLK_PIN): pins.gpio_output_pin_schema,
            cv.Optional(CONF_MISO_PIN): pins.gpio_input_pin_schema,
            cv.Optional(CONF_MOSI_PIN): pins.gpio_output_pin_schema,
            cv.Required(CONF_CS_PIN): pins.gpio_output_pin_schema,
            # END
            cv.Optional(CONF_LED_PIN): pins.gpio_output_pin_schema,
            cv.Optional(CONF_RESET_PIN): pins.gpio_input_pin_schema,
            cv.Required(CONF_DATA_PINS): cv.All(
                [pins.internal_gpio_input_pin_number], cv.Length(min=2, max=3)
            ),
            cv.Required(CONF_MODE): cv.enum(RF_MODES, upper=True),
            cv.Optional(CONF_THRESHOLD_RSSI): cv.positive_not_null_int,
            cv.Optional(CONF_FREQUENCY, default="433.33MHz"): cv.All(
                cv.frequency, cv.Range(min=3e8, max=1e9)
            ),
            cv.Optional(CONF_CURRENT): sensor.sensor_schema(
                unit_of_measurement=UNIT_AMPERE,
                accuracy_decimals=3,
                device_class=DEVICE_CLASS_CURRENT,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_POWER): sensor.sensor_schema(
                unit_of_measurement=UNIT_WATT,
                accuracy_decimals=2,
                device_class=DEVICE_CLASS_POWER,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_BATTERY_LEVEL): sensor.sensor_schema(
                unit_of_measurement=UNIT_PERCENT,
                accuracy_decimals=0,
                device_class=DEVICE_CLASS_BATTERY,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
        }
    )
    #    .extend(spi.spi_device_schema(True, "2MHz"))
    .extend(cv.COMPONENT_SCHEMA)
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    # await spi.register_spi_device(var, config)

    # Exclude unsupported devices
    # define RADIOLIB_EXCLUDE_CC1101
    cg.add_build_flag("-DRADIOLIB_EXCLUDE_NRF24")
    cg.add_build_flag("-DRADIOLIB_EXCLUDE_RF69")
    cg.add_build_flag("-DRADIOLIB_EXCLUDE_SX1231")
    cg.add_build_flag("-DRADIOLIB_EXCLUDE_SI443X")
    cg.add_build_flag("-DRADIOLIB_EXCLUDE_RFM2X")
    # define RADIOLIB_EXCLUDE_SX127X
    cg.add_build_flag("-DRADIOLIB_EXCLUDE_RFM9X")
    # define RADIOLIB_EXCLUDE_SX126X
    # define RADIOLIB_EXCLUDE_STM32WLX   // dependent on RADIOLIB_EXCLUDE_SX126X
    cg.add_build_flag("-DRADIOLIB_EXCLUDE_SX128X")
    cg.add_build_flag("-DRADIOLIB_EXCLUDE_AFSK")
    # cg.add_build_flag("-DRADIOLIB_EXCLUDE_APRS")
    # cg.add_build_flag("-DRADIOLIB_EXCLUDE_PAGER")
    # cg.add_build_flag("-DRADIOLIB_EXCLUDE_AX25")
    cg.add_build_flag("-DRADIOLIB_EXCLUDE_HELLSCHREIBER")
    cg.add_build_flag("-DRADIOLIB_EXCLUDE_MORSE")
    # cg.add_build_flag("-DRADIOLIB_EXCLUDE_RTTY")
    cg.add_build_flag("-DRADIOLIB_EXCLUDE_SSTV")
    # cg.add_build_flag("-DRADIOLIB_EXCLUDE_DIRECT_RECEIVE")

    cg.add_build_flag("-std=gnu++17")

    cg.add_build_flag(f'-DRF_MODULE_MOSI={config.get(CONF_MOSI_PIN)["number"]}')
    cg.add_build_flag(f'-DRF_MODULE_MISO={config.get(CONF_MISO_PIN)["number"]}')
    cg.add_build_flag(f'-DRF_MODULE_SCK={config.get(CONF_CLK_PIN)["number"]}')

    # Clean RTL433 DEBUG
    # cg.add_build_flag("-DRTL_DEBUG=3")
    cg.add_build_flag("-DRAW_SIGNAL_DEBUG=false")
    #

    # cg.add_define("STR_MODULE",config.get(CONF_DEVICE))
    # match config.get(CONF_DEVICE):
    #    case "CC1101":
    #        cg.add_define("RF_MODULE_RECEIVER_GPIO",config.get(CONF_DATA_PINS, [])[0])
    #        cg.add_define("RADIO_LIB_MODULE",f'new Module({config.get(CONF_CS_PIN)["number"]},{config.get(CONF_DATA_PINS, [])[0]},RADIOLIN_NC,{config.get(CONF_DATA_PINS, [])[1]},newSPI)')
    #    case "SX1276" "SX1278":
    #        cg.add_define("RF_MODULE_RECEIVER_GPIO",config.get(CONF_DATA_PINS, [])[2])
    #        cg.add_define("RADIO_LIB_MODULE",f'new Module({config.get(CONF_CS_PIN)["number"]},{config.get(CONF_DATA_PINS, [])[0]},{config.get(CONF_RESET_PIN)["number"]},{config.get(CONF_DATA_PINS, [])[1]},newSPI)')
    #    case _:

    device = config.get(CONF_DEVICE)
    cg.add_build_flag(f'-DRF_{device}="{device}"')
    if led_pin := config.get(CONF_LED_PIN):
        cg.add_build_flag(f'-DONBOARD_LED={led_pin["number"]}')
    cg.add_build_flag("-DPUBLISH_UNPARSED=true")
    if rssi_threshold := config.get(CONF_THRESHOLD_RSSI):
        cg.add_build_flag(f"-DRSSI_THRESHOLD={rssi_threshold}")
    cg.add_build_flag(f'-DOOK_MODULATION={str(config.get(CONF_MODE) == "OOK").lower()}')
    cg.add_build_flag("-DSIGNAL_RSSI=true")

    if config.get(CONF_DEVICE) == "CC1101":
        cg.add_build_flag(f"-DRF_MODULE_GDO0={config.get(CONF_DATA_PINS, [])[0]}")
        cg.add_build_flag(f"-DRF_MODULE_GDO2={config.get(CONF_DATA_PINS, [])[1]}")
    else:
        for idx, conf in enumerate(config.get(CONF_DATA_PINS, [])):
            cg.add_build_flag(f"-DRF_MODULE_DIO{idx}={conf}")

    if CONF_RESET_PIN in config:
        cg.add_build_flag(f'-DRF_MODULE_RST={config.get(CONF_RESET_PIN)["number"]}')

    cg.add_build_flag("-DRF_MODULE_INIT_STATUS=true")
    cg.add_build_flag(f'-DRF_MODULE_CS={config.get(CONF_CS_PIN)["number"]}')
    cg.add_build_flag(f"-DRF_MODULE_FREQUENCY={config.get(CONF_FREQUENCY) / 1000000}")
    # cg.add_define('RADIOLIB_DEBUG','true')

    cg.add_define("JSON_MSG_BUFFER", 512)

    cg.add_library("SPI", None)

    #    cg.add_library("https://github.com/NorthernMan54/rtl_433_ESP.git#3fea1cf",None)
    #    cg.add_library("jgromes/RadioLib", "5.6.0")

    #    cg.add_library("file:///Users/ltarantino/Documents/Sources/rtl_433_ESP",None)

    cg.add_library(
        "git@github.com:dianlight/rtl_433_ESP-esphome.git#esphome_port", None
    )

    if CONF_CURRENT in config:
        conf = config[CONF_CURRENT]
        sens = await sensor.new_sensor(conf)
        cg.add(var.set_current_sensor(sens))
    if CONF_POWER in config:
        conf = config[CONF_POWER]
        sens = await sensor.new_sensor(conf)
        cg.add(var.set_power_sensor(sens))
    if CONF_BATTERY_LEVEL in config:
        conf = config[CONF_BATTERY_LEVEL]
        sens = await sensor.new_sensor(conf)
        cg.add(var.set_battery_sensor(sens))

    for conf in config.get(CONF_INCLUDES, []):
        var.add_include_code(conf)
