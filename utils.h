uint16_t bytes_to_big_endian16(const uint8_t bytes[2]);
uint16_t bytes_to_big_endian16(const uint8_t byte1, const uint8_t byte2);
uint32_t bytes_to_big_endian32(const uint8_t bytes[4]);
uint8_t encode_big_endian8(uint8_t v, uint8_t* bytes);
uint8_t encode_big_endian16(uint16_t v, uint8_t* bytes);
uint8_t encode_big_endian32(uint32_t v, uint8_t* bytes);
uint8_t decode_varint(const uint8_t* bytes, uint64_t& result);
uint8_t encode_varint(uint64_t v, uint8_t* bytes);
uint8_t get_n_bytes_in_varint(uint64_t v);
void print_binary(uint64_t v);
void print_binary(uint8_t v);
void print_bytes(uint8_t* start, uint8_t* end, char last = '\n');
void print_uint8_t(uint8_t v, char last = '\n');

void print_uint8_t(uint8_t v, char last) {
    std::cout << static_cast<int>(v) << last;
}

void print_bytes(uint8_t* start, uint8_t* end, char last) {
    while (start != end) {
        std::cout << *start;
        ++start;
    }
    std::cout << last;
}

void print_binary(uint64_t v) {
    for (int i = 63; i >= 0; --i) {
        uint64_t mask = 1ULL << i;
        std::cout << ((v & mask) ? '1' : '0');

        if (i % 8 == 0) {
            std::cout << ' ';
        }
    }

    std::cout << "\n";
}

void print_binary(uint8_t v) {
    for (int i = 7; i >= 0; --i) {
        uint8_t mask = 1 << i;
        std::cout << ((v & mask) ? '1' : '0');
    }

    std::cout << "\n";
}

// The header size varint and serial type varints will usually consist of a single byte
// The serial type varints for large strings and BLOBs might extend to two or three byte varints,
// but that is the exception rather than the rule
// The varint format is very efficient at coding the record header
// Thats why they have this scary code

/*
** The variable-length integer encoding is as follows:
**
** KEY:
**         A = 0xxxxxxx    7 bits of data and one flag bit
**         B = 1xxxxxxx    7 bits of data and one flag bit
**         C = xxxxxxxx    8 bits of data
**
**  7 bits - A
** 14 bits - BA
** 21 bits - BBA
** 28 bits - BBBA
** 35 bits - BBBBA
** 42 bits - BBBBBA
** 49 bits - BBBBBBA
** 56 bits - BBBBBBBA
** 64 bits - BBBBBBBBC
*/

uint8_t decode_varint(const uint8_t* bytes, uint64_t& result) {
    result = 0;
    uint8_t cnt = 1;

    for (; (*bytes & 0b10000000) && cnt <= 8; ++bytes, ++cnt) {
        result |= static_cast<uint64_t>(*bytes & 0b01111111);
        result <<= 7;
    }
    if (cnt == 9) {
        result |= static_cast<uint64_t>(*bytes);
        return 9;
    }
    result |= static_cast<uint64_t>(*bytes & 0b01111111);
    return cnt;
}

uint8_t encode_varint(uint64_t v, uint8_t* bytes) {
    if (v <= 0x7f) {
        bytes[0] = v & 0x7f;
        return 1;
    }
    if (v <= 0x3fff) {
        bytes[0] = ((v >> 7) & 0x7f) | 0x80;
        bytes[1] = v & 0x7f;
        return 2;
    }

    int i, j, n;
    uint8_t buf[10];
    if (v & (((uint64_t) 0xff000000) << 32)) {
        bytes[8] = (uint8_t) v;
        v >>= 8;
        for (i = 7; i >= 0; --i) {
            bytes[i] = (uint8_t) ((v & 0x7f) | 0x80);
            v >>= 7;
        }
        return 9;
    }

    n = 0;
    do {
        buf[n++] = (uint8_t)((v & 0x7f) | 0x80);
        v >>= 7;
    } while (v != 0);
    buf[0] &= 0x7f;

    //assert (n <= 9);

    for (i = 0, j = n - 1; j >= 0; j--, i++) {
        bytes[i] = buf[j];
    }
    return n;
}

uint8_t get_n_bytes_in_varint(uint64_t v) {
    if (v <= 0x7f) {
        return 1;
    }
    if (v <= 0x3fff) {
        return 2;
    }

    int i, n;
    uint8_t buf[10];
    if (v & (((uint64_t) 0xff000000) << 32)) {
        v >>= 8;
        for (i = 7; i >= 0; --i) {
            v >>= 7;
        }
        return 9;
    }

    n = 0;
    do {
        buf[n++] = (uint8_t)((v & 0x7f) | 0x80);
        v >>= 7;
    } while (v != 0);
    buf[0] &= 0x7f;
    return n;
}

uint16_t bytes_to_big_endian16(const uint8_t bytes[2]) {
    return (static_cast<uint16_t>(bytes[0]) << 8) | static_cast<uint16_t>(bytes[1]);
}

uint16_t bytes_to_big_endian16(const uint8_t byte1, const uint8_t byte2) {
    return (static_cast<uint16_t>(byte1) << 8) | static_cast<uint16_t>(byte2);
}

uint32_t bytes_to_big_endian32(const uint8_t bytes[4]) {
    return (static_cast<uint32_t>(bytes[0]) << 24) |
           (static_cast<uint32_t>(bytes[1]) << 16) |
           (static_cast<uint32_t>(bytes[2]) << 8) |
           static_cast<uint32_t>(bytes[3]);
}

uint8_t encode_big_endian8(uint8_t v, uint8_t* bytes) {
    bytes[0] = v;
    return 1;
}

uint8_t encode_big_endian16(uint16_t v, uint8_t* bytes) {
    bytes[0] = static_cast<uint8_t>((v >> 8) & 0xFF);
    bytes[1] = static_cast<uint8_t>(v & 0xFF);
    return 2;
}

uint8_t encode_big_endian32(uint32_t v, uint8_t* bytes) {
    bytes[0] = static_cast<uint8_t>((v >> 24) & 0xFF);
    bytes[1] = static_cast<uint8_t>((v >> 16) & 0xFF);
    bytes[2] = static_cast<uint8_t>((v >> 8) & 0xFF);
    bytes[3] = static_cast<uint8_t>(v & 0xFF);
    return 4;
}