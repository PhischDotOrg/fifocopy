/*-
 * $Copyright$
 */

#ifndef _FIFOCOPY_HPP_03F980FB_ECA3_4CAC_98C7_287C37363F1C
#define _FIFOCOPY_HPP_03F980FB_ECA3_4CAC_98C7_287C37363F1C

/******************************************************************************/
namespace stm32 {
/******************************************************************************/

template<
    typename InputT,
    typename OutputT
>
void
copy_from_fifo(const InputT &p_input, OutputT &p_output, size_t p_bytes) {
    static_assert(1 == sizeof(typename OutputT::value_type));

    typename InputT::const_iterator i;
    typename OutputT::iterator o;
    unsigned idx;

    for (idx = 0, i = p_input.cbegin(), o = p_output.begin();
      (idx < p_bytes) && (i != p_input.cend()) && (o < p_output.end());
      idx += InputT::fifo_width, i++, o += InputT::fifo_width) {
        static_assert(
                 (InputT::fifo_width == sizeof(uint8_t))
              || (InputT::fifo_width == sizeof(uint16_t))
              || (InputT::fifo_width == sizeof(uint32_t))
        );

        if constexpr(InputT::fifo_width >= sizeof(uint8_t)) {
            *(o + 0) = ((*i) >> 0) & 0xFF;
        }
        
        if constexpr(InputT::fifo_width >= sizeof(uint16_t)) {
            if ((o + 1) == p_output.end()) break;
            *(o + 1) = ((*i) >> 8) & 0xFF;
        }
          
        if constexpr(InputT::fifo_width >= sizeof(uint32_t)) {
            if ((o + 2) == p_output.end()) break;
            *(o + 2) = ((*i) >> 16) & 0xFF;

            if ((o + 3) == p_output.end()) break;
            *(o + 3) = ((*i) >> 24) & 0xFF;
        }

        if constexpr(InputT::fifo_width > 4) {
            static_assert(InputT::fifo_width == 0, "FIFO Width beyond 32-Bit / 4 Bytes not implemented.");
        }
    }
}

template<
    typename InputT,
    typename OutputT
>
void
copy_to_fifo(const InputT &p_input, OutputT &p_output, size_t p_bytes) {
    static_assert(1 == sizeof(typename InputT::value_type));

    typename InputT::const_iterator i;
    typename OutputT::iterator o;
    typename OutputT::value_type v;
    unsigned idx;


    for (idx = 0, i = p_input.cbegin(), o = p_output.begin();
      (idx < p_bytes) && (i < p_input.cend()) && (o != p_output.end());
      idx += OutputT::fifo_width, i += OutputT::fifo_width, o++) {
        static_assert(
                 (OutputT::fifo_width == sizeof(uint8_t))
              || (OutputT::fifo_width == sizeof(uint16_t))
              || (OutputT::fifo_width == sizeof(uint32_t))
        );

        if constexpr(OutputT::fifo_width >= sizeof(uint8_t)) {
            v = *i;
        }
        
        if constexpr(OutputT::fifo_width >= sizeof(uint16_t)) {
            if ((i + 1) == p_input.end()) break;
            v = ((*(i + 1)) << 8) | (v & 0x00FFu);
        }
          
        if constexpr(OutputT::fifo_width >= sizeof(uint32_t)) {
            if ((i + 2) == p_input.end()) break;
            v = ((*(i + 2)) << 16) | (v & 0x0000FFFFu);

            if ((i + 3) == p_input.end()) break;
            v = ((*(i + 3)) << 24) | (v & 0x00FFFFFFu);
        }

        *o = v;

        if constexpr(OutputT::fifo_width > 4) {
            static_assert(OutputT::fifo_width == 0, "FIFO Width beyond 32-Bit / 4 Bytes not implemented.");
        }
    }
}

/******************************************************************************/
} /* namespace stm32 */
/******************************************************************************/

#endif /* _FIFOCOPY_HPP_03F980FB_ECA3_4CAC_98C7_287C37363F1C */
