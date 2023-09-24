#include <inttypes.h>

namespace ConversionTools {
/*
 * Returns base 16 value of a given integer
 *
 * Ex. 4200 -> 0x4200 (16896)
 *
 * Note: max convertable value is 9999
 */
uint16_t dec16ToHex(uint16_t input);

/*
 * Returns base 16 value of a given integer
 *
 * Ex. 4200 -> 0x4200 (16896)
 *
 * Note: max convertable value is 99999999
 */
uint32_t dec32ToHex(uint32_t input);

} // namespace ConversionTools