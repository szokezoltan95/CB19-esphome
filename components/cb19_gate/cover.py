import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import cover
from esphome.const import CONF_ID

from . import cb19_ns, CB19Gate

CB19Cover = cb19_ns.class_("CB19Cover", cover.Cover)

CONFIG_SCHEMA = cover.COVER_SCHEMA.extend({
    cv.GenerateID(): cv.declare_id(CB19Cover),
    cv.Required("gate_id"): cv.use_id(CB19Gate),
})

async def to_code(config):
    gate = await cg.get_variable(config["gate_id"])
    var = cg.new_Pvariable(config[CONF_ID])

    await cover.register_cover(var, config)
    cg.add(var.set_parent(gate))