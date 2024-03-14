enum class RoughObjectType: uint8_t {
    LINE = 0x01,
    LINEAR_PATH = 0x02,
    CIRCLE = 0x03,
    POLYGON = 0x04
};

enum class RoughFillStyle: uint8_t {
    HACHURE = 0x01,
    SOLID = 0x02,
    ZIGZAG = 0x03,
    CROSS_HATCH = 0x04,
    DOTS = 0x05,
    DASHED = 0x06,
    ZIGZAG_LINE = 0x07
};

struct RoughObjectOptions {
    uint32_t stroke = 0x000000;
    uint8_t stroke_width = 1;
    uint8_t roughness = 1;
    uint8_t bowing = 1;
    uint32_t seed = 42;

    uint32_t fill = 0xffffff;
    RoughFillStyle fill_style = RoughFillStyle::HACHURE;
    uint8_t fill_weight = 1;
    uint8_t hachure_angle = -42;
    uint8_t hachure_gap = 4;
    uint8_t disable_multi_stroke = 0;
};

struct RoughObject {
    RoughObjectType type;
    RoughObjectOptions opts;
    std::vector<uint16_t> pts;

    RoughObject(): type(RoughObjectType::LINE), opts(RoughObjectOptions()), pts(std::vector<uint16_t>({0, 0, 0, 0})) { }
    RoughObject(RoughObjectType type, RoughObjectOptions& opts, std::vector<uint16_t>& pts): type(type), opts(opts), pts(pts) { }
};