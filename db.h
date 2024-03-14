#include "rough.h"
#include <fstream>

void _write_uint8_t(uint8_t v, std::fstream& file) {
    file.put(v);
}

void _write_uint16_t(uint16_t v, std::fstream& file) {
    file.put(static_cast<uint8_t>((v >> 8) & 0xFF));
    file.put(static_cast<uint8_t>(v & 0xFF));
}

void _write_uint32_t(uint32_t v, std::fstream& file) {
    file.put(static_cast<uint8_t>((v >> 24) & 0xFF));
    file.put(static_cast<uint8_t>((v >> 16) & 0xFF));
    file.put(static_cast<uint8_t>((v >> 8) & 0xFF));
    file.put(static_cast<uint8_t>(v & 0xFF));
}

uint8_t _read_uint8_t(std::fstream& file) {
    uint8_t value = file.get();
    return value;
}

uint16_t _read_uint16_t(std::fstream& file) {
    uint8_t byte1 = _read_uint8_t(file);
    uint8_t byte2 = _read_uint8_t(file);
    return static_cast<uint16_t>((byte1 << 8) | byte2);
}

uint32_t _read_uint32_t(std::fstream& file) {
    uint8_t byte1 = _read_uint8_t(file);
    uint8_t byte2 = _read_uint8_t(file);
    uint8_t byte3 = _read_uint8_t(file);
    uint8_t byte4 = _read_uint8_t(file);
    return static_cast<uint32_t>((byte1 << 24) | (byte2 << 16) | (byte3 << 8) | byte4);
}

enum class DBPageType: uint8_t {
    BTREE = 0x01,
    OVERFLOW = 0x02
};

struct BTreePage {
    DB* db;
    uint16_t n_cells_on_page = 0;
    uint16_t start_of_cell_content_area = 4096;
};

struct DB {
    std::fstream file;
    bool connected = false;
    uint16_t page_size = 4096;
    uint16_t first_overflow_page = 0;
    uint16_t n_cells_on_page = 0;
    uint16_t start_of_cell_content_area = 4096;

    DB() { }

    void connect(std::string& fn) {
        file.open(fn, std::ios::in | std::ios::out | std::ios::binary);

        if (!file.is_open()) {
            std::cerr << "failed to open the file for writing.\n";
            return;
        }

        connected = true;
    }

    uint32_t max_cell_size() {
        return page_size - 4;
    }

    uint16_t max_cells_on_page() {
        return (page_size - 4) / 6;
    }

    bool _could_be_inserted(uint16_t P) {
        return start_of_cell_content_area - 4 - (n_cells_on_page + 1) * 2 >= P;
    }

    uint16_t _compute_options_size(RoughObjectType type) {
        return (type == RoughObjectType::CIRCLE || type == RoughObjectType::POLYGON) ? 17 : 8;
    }

    uint16_t _compute_pts_size(uint16_t P, RoughObjectType type) {
        return (P - 3 - _compute_options_size(type)) / 2;
    }

    void _insert(RoughObjectType type, RoughObjectOptions& opts, std::vector<uint16_t>& pts) {
        uint16_t P = 2 + 1 + _compute_options_size(type) + pts.size() * 2;
        if (_could_be_inserted(P)) {
            uint16_t offset = start_of_cell_content_area - P;

            n_cells_on_page += 1;
            start_of_cell_content_area = offset;
            file.seekp(0);
            _write_uint16_t(n_cells_on_page, file);
            _write_uint16_t(start_of_cell_content_area, file);

            file.seekp(4 + (n_cells_on_page - 1) * 2);
            _write_uint16_t(offset, file);

            file.seekp(offset);

            if (type == RoughObjectType::LINE || type == RoughObjectType::LINEAR_PATH) {
                _write_uint16_t(static_cast<uint16_t>(P), file);
                _write_uint8_t(static_cast<uint8_t>(type), file);
                _write_uint32_t(opts.stroke, file);
                _write_uint8_t(opts.stroke_width, file);
                _write_uint8_t(opts.roughness, file);
                _write_uint8_t(opts.bowing, file);
                _write_uint8_t(opts.seed, file);

                for (int i = 0; i < pts.size(); ++i) {
                    _write_uint16_t(pts[i], file);
                }
            } else {
                _write_uint16_t(static_cast<uint16_t>(P), file);
                _write_uint8_t(static_cast<uint8_t>(type), file);
                _write_uint32_t(opts.stroke, file);
                _write_uint8_t(opts.stroke_width, file);
                _write_uint8_t(opts.roughness, file);
                _write_uint8_t(opts.bowing, file);
                _write_uint8_t(opts.seed, file);

                _write_uint32_t(opts.fill, file);
                _write_uint8_t(static_cast<uint8_t>(opts.fill_style), file);
                _write_uint8_t(opts.fill_weight, file);
                _write_uint8_t(opts.hachure_angle, file);
                _write_uint8_t(opts.hachure_gap, file);
                _write_uint8_t(opts.disable_multi_stroke, file);

                for (int i = 0; i < pts.size(); ++i) {
                    _write_uint16_t(pts[i], file);
                }
            }
        } else {
            std::cout << "TODO\n";
        }
    }

    RoughObject _read(uint32_t id) {
        if (id > n_cells_on_page) {
            return RoughObject();
        }
        file.seekp(4 + id * 2);
        uint16_t offset = _read_uint16_t(file);
        file.seekp(offset);
        uint16_t P = _read_uint16_t(file);

        RoughObjectType type = static_cast<RoughObjectType>(_read_uint8_t(file));
        RoughObjectOptions opts;
        std::vector<uint16_t> pts;
        if (type == RoughObjectType::LINE || type == RoughObjectType::LINEAR_PATH) {
            opts.stroke = _read_uint32_t(file);
            opts.stroke_width = _read_uint8_t(file);
            opts.roughness = _read_uint8_t(file);
            opts.bowing = _read_uint8_t(file);
            opts.seed = _read_uint8_t(file);

            for (int i = 0; i < _compute_pts_size(P, type); ++i) {
                pts.push_back(_read_uint16_t(file));
            }
        } else {
            opts.stroke = _read_uint32_t(file);
            opts.stroke_width = _read_uint8_t(file);
            opts.roughness = _read_uint8_t(file);
            opts.bowing = _read_uint8_t(file);
            opts.seed = _read_uint8_t(file);

            opts.fill = _read_uint32_t(file);
            opts.fill_style = static_cast<RoughFillStyle>(_read_uint8_t(file));
            opts.fill_weight = _read_uint8_t(file);
            opts.hachure_angle = _read_uint8_t(file);
            opts.hachure_gap = _read_uint8_t(file);
            opts.disable_multi_stroke = _read_uint8_t(file);

            for (int i = 0; i < _compute_pts_size(P, type); ++i) {
                pts.push_back(_read_uint16_t(file));
            }
        }
        return RoughObject(type, opts, pts);
    }

    void disconnect() {
        file.close();
    }
};