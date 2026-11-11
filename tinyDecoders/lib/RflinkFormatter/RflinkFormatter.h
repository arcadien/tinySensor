#pragma once

#include <stdint.h>

#include <LacrosseDecoder.h>
#include <OregonDecoder.h>
#include <X10Decoder.h>

// Longest line plus '\0'
const uint8_t RflinkLineSize = 64;

// Renders the frame the decoder just completed as one RFLink line
// "20;NN;Name;ID=XXXX;KEY=value;" plus '\0', without "\r\n", and returns its
// length (FUNC-DECODE-008). No printf, no dynamic allocation.
uint8_t FormatRflink(const X10Decoder &decoder, uint8_t counter,
                     char line[RflinkLineSize]);
uint8_t FormatRflink(const LacrosseDecoder &decoder, uint8_t counter,
                     char line[RflinkLineSize]);
uint8_t FormatRflink(const OregonDecoder &decoder, uint8_t counter,
                     char line[RflinkLineSize]);
