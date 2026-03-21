import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import (
    uart,
    cover,
    sensor,
    text_sensor,
    binary_sensor,
    button,
    number,
)
from esphome.const import (
    CONF_ID,
    CONF_NAME,
    CONF_UART_ID,
    ENTITY_CATEGORY_CONFIG,
    ENTITY_CATEGORY_DIAGNOSTIC,
    ICON_COUNTER,
    ICON_PERCENT,
)

AUTO_LOAD = ["cover", "sensor", "text_sensor", "binary_sensor", "button", "number"]
DEPENDENCIES = ["uart"]

CONF_COVER = "cover"
CONF_PEDESTRIAN_BUTTON = "pedestrian_button"
CONF_MIN_POSITION = "min_position"
CONF_MAX_POSITION = "max_position"
CONF_MOTOR1_RAW = "motor1_raw"
CONF_MOTOR2_RAW = "motor2_raw"
CONF_MOTOR1_PERCENT = "motor1_percent"
CONF_MOTOR2_PERCENT = "motor2_percent"
CONF_OVERALL_PERCENT = "overall_percent"
CONF_LAST_STATE = "last_state"
CONF_LAST_ACK = "last_ack"
CONF_LAST_RS = "last_rs"
CONF_MOVING = "moving"
CONF_FULLY_OPEN = "fully_open"
CONF_FULLY_CLOSED = "fully_closed"
CONF_PHOTOCELL_ACTIVE = "photocell_active"
CONF_OBSTRUCTION_ACTIVE = "obstruction_active"
CONF_OPENING_START_PERCENT = "opening_start_percent"
CONF_CLOSING_START_PERCENT = "closing_start_percent"

cb19_ns = cg.esphome_ns.namespace("cb19_gate")
CB19GateComponent = cb19_ns.class_("CB19GateComponent", cg.Component, uart.UARTDevice)
CB19GateCover = cb19_ns.class_("CB19GateCover", cover.Cover)
CB19PedestrianButton = cb19_ns.class_("CB19PedestrianButton", button.Button)
CB19OpeningStartNumber = cb19_ns.class_("CB19OpeningStartNumber", number.Number)
CB19ClosingStartNumber = cb19_ns.class_("CB19ClosingStartNumber", number.Number)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(CB19GateComponent),
        cv.Required(CONF_UART_ID): cv.use_id(uart.UARTComponent),
        cv.Optional(CONF_MIN_POSITION, default=1): cv.int_range(min=0, max=255),
        cv.Optional(CONF_MAX_POSITION, default=225): cv.int_range(min=1, max=255),
        cv.Optional(CONF_COVER): cover.cover_schema(CB19GateCover).extend(
            {
                cv.Optional(CONF_NAME, default="CB19 Gate"): cv.string,
            }
        ),
        cv.Optional(CONF_PEDESTRIAN_BUTTON): button.button_schema(CB19PedestrianButton).extend(
            {
                cv.Optional(CONF_NAME, default="Pedestrian Open"): cv.string,
            }
        ),
        cv.Optional(CONF_OPENING_START_PERCENT): number.number_schema(
            CB19OpeningStartNumber,
            unit_of_measurement="%",
            icon=ICON_PERCENT,
            entity_category=ENTITY_CATEGORY_CONFIG,
        ).extend(
            {
                cv.Optional(CONF_NAME, default="Opening Start Percent"): cv.string,
                cv.Optional("initial_value", default=56.0): cv.float_range(min=0.0, max=99.0),
                cv.Optional("min_value", default=0.0): cv.float_range(min=0.0, max=99.0),
                cv.Optional("max_value", default=99.0): cv.float_range(min=0.0, max=100.0),
                cv.Optional("step", default=1.0): cv.float_range(min=0.1, max=10.0),
            }
        ),
        cv.Optional(CONF_CLOSING_START_PERCENT): number.number_schema(
            CB19ClosingStartNumber,
            unit_of_measurement="%",
            icon=ICON_PERCENT,
            entity_category=ENTITY_CATEGORY_CONFIG,
        ).extend(
            {
                cv.Optional(CONF_NAME, default="Closing Start Percent"): cv.string,
                cv.Optional("initial_value", default=44.0): cv.float_range(min=1.0, max=100.0),
                cv.Optional("min_value", default=1.0): cv.float_range(min=0.0, max=100.0),
                cv.Optional("max_value", default=100.0): cv.float_range(min=1.0, max=100.0),
                cv.Optional("step", default=1.0): cv.float_range(min=0.1, max=10.0),
            }
        ),
        cv.Optional(CONF_MOTOR1_RAW): sensor.sensor_schema(
            accuracy_decimals=0,
            icon=ICON_COUNTER,
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
        ),
        cv.Optional(CONF_MOTOR2_RAW): sensor.sensor_schema(
            accuracy_decimals=0,
            icon=ICON_COUNTER,
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
        ),
        cv.Optional(CONF_MOTOR1_PERCENT): sensor.sensor_schema(
            accuracy_decimals=1,
            unit_of_measurement="%",
            icon=ICON_PERCENT,
        ),
        cv.Optional(CONF_MOTOR2_PERCENT): sensor.sensor_schema(
            accuracy_decimals=1,
            unit_of_measurement="%",
            icon=ICON_PERCENT,
        ),
        cv.Optional(CONF_OVERALL_PERCENT): sensor.sensor_schema(
            accuracy_decimals=1,
            unit_of_measurement="%",
            icon=ICON_PERCENT,
        ),
        cv.Optional(CONF_LAST_STATE): text_sensor.text_sensor_schema(
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC
        ),
        cv.Optional(CONF_LAST_ACK): text_sensor.text_sensor_schema(
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC
        ),
        cv.Optional(CONF_LAST_RS): text_sensor.text_sensor_schema(
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC
        ),
        cv.Optional(CONF_MOVING): binary_sensor.binary_sensor_schema(),
        cv.Optional(CONF_FULLY_OPEN): binary_sensor.binary_sensor_schema(),
        cv.Optional(CONF_FULLY_CLOSED): binary_sensor.binary_sensor_schema(),
        cv.Optional(CONF_PHOTOCELL_ACTIVE): binary_sensor.binary_sensor_schema(),
        cv.Optional(CONF_OBSTRUCTION_ACTIVE): binary_sensor.binary_sensor_schema(),
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    uart_parent = await cg.get_variable(config[CONF_UART_ID])
    var = cg.new_Pvariable(config[CONF_ID], uart_parent)
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)

    cg.add(var.set_position_range(config[CONF_MIN_POSITION], config[CONF_MAX_POSITION]))

    if CONF_COVER in config:
        conf = config[CONF_COVER]
        cov = cg.new_Pvariable(conf[CONF_ID])
        await cover.register_cover(cov, conf)
        cg.add(cov.set_parent(var))
        cg.add(var.set_cover(cov))

    if CONF_PEDESTRIAN_BUTTON in config:
        conf = config[CONF_PEDESTRIAN_BUTTON]
        btn = cg.new_Pvariable(conf[CONF_ID])
        await button.register_button(btn, conf)
        cg.add(btn.set_parent(var))
        cg.add(var.set_pedestrian_button(btn))

    if CONF_OPENING_START_PERCENT in config:
        conf = config[CONF_OPENING_START_PERCENT]
        num = cg.new_Pvariable(conf[CONF_ID])
        await number.register_number(
            num,
            conf,
            min_value=conf["min_value"],
            max_value=conf["max_value"],
            step=conf["step"],
        )
        cg.add(num.set_parent(var))
        cg.add(num.set_initial_value(conf["initial_value"]))
        cg.add(var.set_opening_start_number(num))

    if CONF_CLOSING_START_PERCENT in config:
        conf = config[CONF_CLOSING_START_PERCENT]
        num = cg.new_Pvariable(conf[CONF_ID])
        await number.register_number(
            num,
            conf,
            min_value=conf["min_value"],
            max_value=conf["max_value"],
            step=conf["step"],
        )
        cg.add(num.set_parent(var))
        cg.add(num.set_initial_value(conf["initial_value"]))
        cg.add(var.set_closing_start_number(num))

    if CONF_MOTOR1_RAW in config:
        sens = await sensor.new_sensor(config[CONF_MOTOR1_RAW])
        cg.add(var.set_motor1_raw_sensor(sens))

    if CONF_MOTOR2_RAW in config:
        sens = await sensor.new_sensor(config[CONF_MOTOR2_RAW])
        cg.add(var.set_motor2_raw_sensor(sens))

    if CONF_MOTOR1_PERCENT in config:
        sens = await sensor.new_sensor(config[CONF_MOTOR1_PERCENT])
        cg.add(var.set_motor1_percent_sensor(sens))

    if CONF_MOTOR2_PERCENT in config:
        sens = await sensor.new_sensor(config[CONF_MOTOR2_PERCENT])
        cg.add(var.set_motor2_percent_sensor(sens))

    if CONF_OVERALL_PERCENT in config:
        sens = await sensor.new_sensor(config[CONF_OVERALL_PERCENT])
        cg.add(var.set_overall_percent_sensor(sens))

    if CONF_LAST_STATE in config:
        ts = await text_sensor.new_text_sensor(config[CONF_LAST_STATE])
        cg.add(var.set_last_state_text_sensor(ts))

    if CONF_LAST_ACK in config:
        ts = await text_sensor.new_text_sensor(config[CONF_LAST_ACK])
        cg.add(var.set_last_ack_text_sensor(ts))

    if CONF_LAST_RS in config:
        ts = await text_sensor.new_text_sensor(config[CONF_LAST_RS])
        cg.add(var.set_last_rs_text_sensor(ts))

    if CONF_MOVING in config:
        bs = await binary_sensor.new_binary_sensor(config[CONF_MOVING])
        cg.add(var.set_moving_binary_sensor(bs))

    if CONF_FULLY_OPEN in config:
        bs = await binary_sensor.new_binary_sensor(config[CONF_FULLY_OPEN])
        cg.add(var.set_fully_open_binary_sensor(bs))

    if CONF_FULLY_CLOSED in config:
        bs = await binary_sensor.new_binary_sensor(config[CONF_FULLY_CLOSED])
        cg.add(var.set_fully_closed_binary_sensor(bs))

    if CONF_PHOTOCELL_ACTIVE in config:
        bs = await binary_sensor.new_binary_sensor(config[CONF_PHOTOCELL_ACTIVE])
        cg.add(var.set_photocell_binary_sensor(bs))

    if CONF_OBSTRUCTION_ACTIVE in config:
        bs = await binary_sensor.new_binary_sensor(config[CONF_OBSTRUCTION_ACTIVE])
        cg.add(var.set_obstruction_binary_sensor(bs))