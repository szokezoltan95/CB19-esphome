import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart, cover, sensor, text_sensor, binary_sensor, button, number, select
from esphome.const import (
    CONF_ID,
    CONF_NAME,
    CONF_UART_ID,
    ENTITY_CATEGORY_CONFIG,
    ENTITY_CATEGORY_DIAGNOSTIC,
    ICON_COUNTER,
    ICON_PERCENT,
)

AUTO_LOAD = ["cover", "sensor", "text_sensor", "binary_sensor", "button", "number", "select"]
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
CONF_MOTOR1_SPEED = "motor1_speed"
CONF_MOTOR1_LOAD = "motor1_load"
CONF_MOTOR2_SPEED = "motor2_speed"
CONF_MOTOR2_LOAD = "motor2_load"
CONF_LAST_STATE = "last_state"
CONF_LAST_ACK = "last_ack"
CONF_LAST_RS = "last_rs"
CONF_LEARN_STATUS = "learn_status"
CONF_PARAM_CURRENT = "param_current"
CONF_PARAM_PENDING = "param_pending"
CONF_CONFIG_WARNING = "config_warning"
CONF_MOVING = "moving"
CONF_FULLY_OPEN = "fully_open"
CONF_FULLY_CLOSED = "fully_closed"
CONF_PHOTOCELL_ACTIVE = "photocell_active"
CONF_OBSTRUCTION_ACTIVE = "obstruction_active"
CONF_PARAMS_DIRTY = "params_dirty"
CONF_LEARNING_ACTIVE = "learning_active"
CONF_OPENING_START_PERCENT = "opening_start_percent"
CONF_CLOSING_START_PERCENT = "closing_start_percent"
CONF_APPLY_PARAMETERS_BUTTON = "apply_parameters_button"
CONF_RELOAD_PARAMETERS_BUTTON = "reload_parameters_button"
CONF_REVERT_PARAMETERS_BUTTON = "revert_parameters_button"
CONF_FACTORY_RESET_BUTTON = "factory_reset_button"
CONF_AUTO_LEARN_BUTTON = "auto_learn_button"
CONF_REMOTE_LEARN_BUTTON = "remote_learn_button"
CONF_CLEAR_REMOTE_LEARN_BUTTON = "clear_remote_learn_button"

PARAM_KEYS = ["f1","f2","f3","f4","f5","f6","f7","f8","f9","fa","fb","fc","fd","fe","ff","fg","fh","fi","fj","fk"]
PARAM_OPTIONS = {
    "f1": [
        "F1-0 | Normal (default)",
        "F1-1 | Limit switch",
    ],

    "f2": [
        "F2-0 | 2A",
        "F2-1 | 3A (default)",
        "F2-2 | 4A",
        "F2-3 | 5A",
    ],

    "f3": [
        "F3-0 | 2A",
        "F3-1 | 3A (default)",
        "F3-2 | 4A",
        "F3-3 | 5A",
    ],

    "f4": [
        "F4-0 | 40%",
        "F4-1 | 50%",
        "F4-2 | 75% (default)",
        "F4-3 | 100%",
    ],

    "f5": [
        "F5-0 | 40%",
        "F5-1 | 50%",
        "F5-2 | 75% (default)",
        "F5-3 | 100%",
    ],

    "f6": [
        "F6-0 | 40%",
        "F6-1 | 50% (default)",
        "F6-2 | 60%",
        "F6-3 | 70%",
    ],

    "f7": [
        "F7-0 | 75%",
        "F7-1 | 80%",
        "F7-2 | 85%",
        "F7-3 | 90% (default)",
        "F7-4 | 95%",
    ],

    "f8": [
        "F8-0 | 0 s",
        "F8-1 | 2 s (default)",
        "F8-2 | 5 s",
        "F8-3 | 10 s",
        "F8-4 | 15 s",
        "F8-5 | 20 s",
        "F8-6 | 25 s",
        "F8-7 | 35 s",
        "F8-8 | 45 s",
        "F8-9 | 55 s",
    ],

    "f9": [
        "F9-0 | 0 s",
        "F9-1 | 2 s (default)",
        "F9-2 | 5 s",
        "F9-3 | 10 s",
        "F9-4 | 15 s",
        "F9-5 | 20 s",
        "F9-6 | 25 s",
        "F9-7 | 35 s",
        "F9-8 | 45 s",
        "F9-9 | 55 s",
    ],

    "fa": [
        "FA-0 | Disabled (default)",
        "FA-1 | 3 s",
        "FA-2 | 10 s",
        "FA-3 | 20 s",
        "FA-4 | 40 s",
        "FA-5 | 60 s",
        "FA-6 | 120 s",
        "FA-7 | 180 s",
        "FA-8 | 300 s",
    ],

    "fb": [
        "FB-0 | Mode 1 (default)",
        "FB-1 | Mode 2",
        "FB-2 | Mode 3",
        "FB-3 | Mode 4",
        "FB-4 | Mode 5",
        "FB-5 | Mode 6",
        "FB-6 | Mode 7",
    ],

    "fc": [
        "FC-0 | Disabled",
        "FC-1 | Enabled (default)",
    ],

    "fd": [
        "FD-0 | Disabled (default)",
        "FD-1 | Enabled",
    ],

    "fe": [
        "FE-0 | Disabled (default)",
        "FE-1 | Enabled",
    ],

    "ff": [
        "FF-0 | Disabled (default)",
        "FF-1 | Enabled",
    ],

    "fg": [
        "FG-0 | Disabled (default)",
        "FG-1 | Enabled",
    ],

    "fh": [
        "FH-0 | Normal (default)",
        "FH-1 | Electric lock",
    ],

    "fi": [
        "FI-0 | Up",
        "FI-1 | Down (default)",
    ],

    "fj": [
        "FJ-0 | Double wing (default)",
        "FJ-1 | Single wing",
    ],

    "fk": [
        "FK-0 | Disabled (default)",
        "FK-1 | 0.1 s",
        "FK-2 | 0.2 s",
        "FK-3 | 0.3 s",
        "FK-4 | 0.4 s",
        "FK-5 | 0.5 s",
        "FK-6 | 0.6 s",
    ],
}

cb19_ns = cg.esphome_ns.namespace("cb19_gate")
CB19GateComponent = cb19_ns.class_("CB19GateComponent", cg.Component, uart.UARTDevice)
CB19GateCover = cb19_ns.class_("CB19GateCover", cover.Cover)
CB19PedestrianButton = cb19_ns.class_("CB19PedestrianButton", button.Button)
CB19ApplyParametersButton = cb19_ns.class_("CB19ApplyParametersButton", button.Button)
CB19ReloadParametersButton = cb19_ns.class_("CB19ReloadParametersButton", button.Button)
CB19RevertParametersButton = cb19_ns.class_("CB19RevertParametersButton", button.Button)
CB19FactoryResetButton = cb19_ns.class_("CB19FactoryResetButton", button.Button)
CB19AutoLearnButton = cb19_ns.class_("CB19AutoLearnButton", button.Button)
CB19RemoteLearnButton = cb19_ns.class_("CB19RemoteLearnButton", button.Button)
CB19ClearRemoteLearnButton = cb19_ns.class_("CB19ClearRemoteLearnButton", button.Button)
CB19ParameterSelect = cb19_ns.class_("CB19ParameterSelect", select.Select)
CB19OpeningStartNumber = cb19_ns.class_("CB19OpeningStartNumber", number.Number)
CB19ClosingStartNumber = cb19_ns.class_("CB19ClosingStartNumber", number.Number)

schema_dict = {
    cv.GenerateID(): cv.declare_id(CB19GateComponent),
    cv.Required(CONF_UART_ID): cv.use_id(uart.UARTComponent),
    cv.Optional(CONF_MIN_POSITION, default=1): cv.int_range(min=0, max=255),
    cv.Optional(CONF_MAX_POSITION, default=225): cv.int_range(min=1, max=255),
    cv.Optional(CONF_COVER): cover.cover_schema(CB19GateCover).extend({cv.Optional(CONF_NAME, default="CB19 Gate"): cv.string}),
    cv.Optional(CONF_PEDESTRIAN_BUTTON): button.button_schema(CB19PedestrianButton).extend({cv.Optional(CONF_NAME, default="Pedestrian Open"): cv.string}),
    cv.Optional(CONF_APPLY_PARAMETERS_BUTTON): button.button_schema(CB19ApplyParametersButton).extend({cv.Optional(CONF_NAME, default="Apply Parameters"): cv.string}),
    cv.Optional(CONF_RELOAD_PARAMETERS_BUTTON): button.button_schema(CB19ReloadParametersButton).extend({cv.Optional(CONF_NAME, default="Reload Parameters"): cv.string}),
    cv.Optional(CONF_REVERT_PARAMETERS_BUTTON): button.button_schema(CB19RevertParametersButton).extend({cv.Optional(CONF_NAME, default="Revert Pending Parameters"): cv.string}),
    cv.Optional(CONF_FACTORY_RESET_BUTTON): button.button_schema(CB19FactoryResetButton).extend({cv.Optional(CONF_NAME, default="Factory Reset"): cv.string}),
    cv.Optional(CONF_AUTO_LEARN_BUTTON): button.button_schema(CB19AutoLearnButton).extend({cv.Optional(CONF_NAME, default="Auto Learn"): cv.string}),
    cv.Optional(CONF_REMOTE_LEARN_BUTTON): button.button_schema(CB19RemoteLearnButton).extend({cv.Optional(CONF_NAME, default="Remote Learn"): cv.string}),
    cv.Optional(CONF_CLEAR_REMOTE_LEARN_BUTTON): button.button_schema(CB19ClearRemoteLearnButton).extend({cv.Optional(CONF_NAME, default="Clear Remote Learn"): cv.string}),
    cv.Optional(CONF_OPENING_START_PERCENT): number.number_schema(CB19OpeningStartNumber, unit_of_measurement="%", icon=ICON_PERCENT, entity_category=ENTITY_CATEGORY_CONFIG).extend({cv.Optional(CONF_NAME, default="Opening Start Percent"): cv.string, cv.Optional("initial_value", default=56.0): cv.float_range(min=0.0, max=99.0), cv.Optional("min_value", default=0.0): cv.float_range(min=0.0, max=99.0), cv.Optional("max_value", default=99.0): cv.float_range(min=0.0, max=100.0), cv.Optional("step", default=1.0): cv.float_range(min=0.1, max=10.0)}),
    cv.Optional(CONF_CLOSING_START_PERCENT): number.number_schema(CB19ClosingStartNumber, unit_of_measurement="%", icon=ICON_PERCENT, entity_category=ENTITY_CATEGORY_CONFIG).extend({cv.Optional(CONF_NAME, default="Closing Start Percent"): cv.string, cv.Optional("initial_value", default=44.0): cv.float_range(min=1.0, max=100.0), cv.Optional("min_value", default=1.0): cv.float_range(min=0.0, max=100.0), cv.Optional("max_value", default=100.0): cv.float_range(min=1.0, max=100.0), cv.Optional("step", default=1.0): cv.float_range(min=0.1, max=10.0)}),
    cv.Optional(CONF_MOTOR1_RAW): sensor.sensor_schema(accuracy_decimals=0, icon=ICON_COUNTER, entity_category=ENTITY_CATEGORY_DIAGNOSTIC),
    cv.Optional(CONF_MOTOR2_RAW): sensor.sensor_schema(accuracy_decimals=0, icon=ICON_COUNTER, entity_category=ENTITY_CATEGORY_DIAGNOSTIC),
    cv.Optional(CONF_MOTOR1_PERCENT): sensor.sensor_schema(accuracy_decimals=1, unit_of_measurement="%", icon=ICON_PERCENT),
    cv.Optional(CONF_MOTOR2_PERCENT): sensor.sensor_schema(accuracy_decimals=1, unit_of_measurement="%", icon=ICON_PERCENT),
    cv.Optional(CONF_OVERALL_PERCENT): sensor.sensor_schema(accuracy_decimals=1, unit_of_measurement="%", icon=ICON_PERCENT),
    cv.Optional(CONF_MOTOR1_SPEED): sensor.sensor_schema(accuracy_decimals=0, icon=ICON_COUNTER, entity_category=ENTITY_CATEGORY_DIAGNOSTIC),
    cv.Optional(CONF_MOTOR1_LOAD): sensor.sensor_schema(accuracy_decimals=0, icon=ICON_COUNTER, entity_category=ENTITY_CATEGORY_DIAGNOSTIC),
    cv.Optional(CONF_MOTOR2_SPEED): sensor.sensor_schema(accuracy_decimals=0, icon=ICON_COUNTER, entity_category=ENTITY_CATEGORY_DIAGNOSTIC),
    cv.Optional(CONF_MOTOR2_LOAD): sensor.sensor_schema(accuracy_decimals=0, icon=ICON_COUNTER, entity_category=ENTITY_CATEGORY_DIAGNOSTIC),
    cv.Optional(CONF_LAST_STATE): text_sensor.text_sensor_schema(entity_category=ENTITY_CATEGORY_DIAGNOSTIC),
    cv.Optional(CONF_LAST_ACK): text_sensor.text_sensor_schema(entity_category=ENTITY_CATEGORY_DIAGNOSTIC),
    cv.Optional(CONF_LAST_RS): text_sensor.text_sensor_schema(entity_category=ENTITY_CATEGORY_DIAGNOSTIC),
    cv.Optional(CONF_LEARN_STATUS): text_sensor.text_sensor_schema(entity_category=ENTITY_CATEGORY_DIAGNOSTIC),
    cv.Optional(CONF_PARAM_CURRENT): text_sensor.text_sensor_schema(entity_category=ENTITY_CATEGORY_DIAGNOSTIC),
    cv.Optional(CONF_PARAM_PENDING): text_sensor.text_sensor_schema(entity_category=ENTITY_CATEGORY_DIAGNOSTIC),
    cv.Optional(CONF_CONFIG_WARNING): text_sensor.text_sensor_schema(entity_category=ENTITY_CATEGORY_DIAGNOSTIC),
    cv.Optional(CONF_MOVING): binary_sensor.binary_sensor_schema(),
    cv.Optional(CONF_FULLY_OPEN): binary_sensor.binary_sensor_schema(),
    cv.Optional(CONF_FULLY_CLOSED): binary_sensor.binary_sensor_schema(),
    cv.Optional(CONF_PHOTOCELL_ACTIVE): binary_sensor.binary_sensor_schema(),
    cv.Optional(CONF_OBSTRUCTION_ACTIVE): binary_sensor.binary_sensor_schema(),
    cv.Optional(CONF_PARAMS_DIRTY): binary_sensor.binary_sensor_schema(entity_category=ENTITY_CATEGORY_DIAGNOSTIC),
    cv.Optional(CONF_LEARNING_ACTIVE): binary_sensor.binary_sensor_schema(entity_category=ENTITY_CATEGORY_DIAGNOSTIC),
}
for key in PARAM_KEYS:
    schema_dict[cv.Optional(key)] = select.select_schema(CB19ParameterSelect, entity_category=ENTITY_CATEGORY_CONFIG).extend({cv.Optional(CONF_NAME, default=key.upper()): cv.string})
CONFIG_SCHEMA = cv.Schema(schema_dict).extend(cv.COMPONENT_SCHEMA)

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

    button_map = [
        (CONF_APPLY_PARAMETERS_BUTTON, "set_apply_parameters_button"),
        (CONF_RELOAD_PARAMETERS_BUTTON, "set_reload_parameters_button"),
        (CONF_REVERT_PARAMETERS_BUTTON, "set_revert_parameters_button"),
        (CONF_FACTORY_RESET_BUTTON, "set_factory_reset_button"),
        (CONF_AUTO_LEARN_BUTTON, "set_auto_learn_button"),
        (CONF_REMOTE_LEARN_BUTTON, "set_remote_learn_button"),
        (CONF_CLEAR_REMOTE_LEARN_BUTTON, "set_clear_remote_learn_button"),
    ]
    for conf_key, setter_name in button_map:
        if conf_key in config:
            conf = config[conf_key]
            btn = cg.new_Pvariable(conf[CONF_ID])
            await button.register_button(btn, conf)
            cg.add(btn.set_parent(var))
            cg.add(getattr(var, setter_name)(btn))

    if CONF_OPENING_START_PERCENT in config:
        conf = config[CONF_OPENING_START_PERCENT]
        num = cg.new_Pvariable(conf[CONF_ID])
        await number.register_number(num, conf, min_value=conf["min_value"], max_value=conf["max_value"], step=conf["step"])
        cg.add(num.set_parent(var))
        cg.add(num.set_initial_value(conf["initial_value"]))
        cg.add(var.set_opening_start_number(num))

    if CONF_CLOSING_START_PERCENT in config:
        conf = config[CONF_CLOSING_START_PERCENT]
        num = cg.new_Pvariable(conf[CONF_ID])
        await number.register_number(num, conf, min_value=conf["min_value"], max_value=conf["max_value"], step=conf["step"])
        cg.add(num.set_parent(var))
        cg.add(num.set_initial_value(conf["initial_value"]))
        cg.add(var.set_closing_start_number(num))

    for idx, key in enumerate(PARAM_KEYS):
        if key in config:
            conf = config[key]
            sel = cg.new_Pvariable(conf[CONF_ID])
            await select.register_select(sel, conf, options=PARAM_OPTIONS[key])
            cg.add(sel.set_parent(var))
            cg.add(sel.set_parameter_index(idx))
            cg.add(var.set_parameter_select(idx, sel))

    sensor_map = [
        (CONF_MOTOR1_RAW, "set_motor1_raw_sensor"),
        (CONF_MOTOR2_RAW, "set_motor2_raw_sensor"),
        (CONF_MOTOR1_PERCENT, "set_motor1_percent_sensor"),
        (CONF_MOTOR2_PERCENT, "set_motor2_percent_sensor"),
        (CONF_OVERALL_PERCENT, "set_overall_percent_sensor"),
        (CONF_MOTOR1_SPEED, "set_motor1_speed_sensor"),
        (CONF_MOTOR1_LOAD, "set_motor1_load_sensor"),
        (CONF_MOTOR2_SPEED, "set_motor2_speed_sensor"),
        (CONF_MOTOR2_LOAD, "set_motor2_load_sensor"),
    ]
    for conf_key, setter_name in sensor_map:
        if conf_key in config:
            sens = await sensor.new_sensor(config[conf_key])
            cg.add(getattr(var, setter_name)(sens))

    text_map = [
        (CONF_LAST_STATE, "set_last_state_text_sensor"),
        (CONF_LAST_ACK, "set_last_ack_text_sensor"),
        (CONF_LAST_RS, "set_last_rs_text_sensor"),
        (CONF_LEARN_STATUS, "set_learn_status_text_sensor"),
        (CONF_PARAM_CURRENT, "set_param_current_text_sensor"),
        (CONF_PARAM_PENDING, "set_param_pending_text_sensor"),
        (CONF_CONFIG_WARNING, "set_config_warning_text_sensor"),
    ]
    for conf_key, setter_name in text_map:
        if conf_key in config:
            ts = await text_sensor.new_text_sensor(config[conf_key])
            cg.add(getattr(var, setter_name)(ts))

    binary_map = [
        (CONF_MOVING, "set_moving_binary_sensor"),
        (CONF_FULLY_OPEN, "set_fully_open_binary_sensor"),
        (CONF_FULLY_CLOSED, "set_fully_closed_binary_sensor"),
        (CONF_PHOTOCELL_ACTIVE, "set_photocell_binary_sensor"),
        (CONF_OBSTRUCTION_ACTIVE, "set_obstruction_binary_sensor"),
        (CONF_PARAMS_DIRTY, "set_params_dirty_binary_sensor"),
        (CONF_LEARNING_ACTIVE, "set_learning_active_binary_sensor"),
    ]
    for conf_key, setter_name in binary_map:
        if conf_key in config:
            bs = await binary_sensor.new_binary_sensor(config[conf_key])
            cg.add(getattr(var, setter_name)(bs))
