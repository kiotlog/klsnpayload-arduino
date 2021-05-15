/*
 * Copyright (C) 2017 Giampaolo Mancini, Trampoline SRL.
 *
 * This file is part of KiotlogSN for Arduino.
 *
 * KiotlogSN for Arduino is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * KiotlogSN for Arduino is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with KiotlogSN for Arduino.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef KiotlogSN_Payload_h
#define KiotlogSN_Payload_h

#include <Arduino.h>
#include <msgpck.h>
#include "Aead/Aead.h"
#include "BufferStream/BufferStream.h"


class KlsnPayload
{
    public:
        KlsnPayload();
        void begin(const uint8_t* key, const size_t nonce_size = 12);
        void pack(const uint8_t* cipher, const size_t cipher_len, const uint8_t* nonce, const size_t nonce_len,                                                       uint8_t* payload, size_t* payload_len);
        void pack(const uint8_t* cipher, const size_t cipher_len, const uint8_t* nonce, const size_t nonce_len, const uint8_t* timestamp, const size_t timestamp_len, uint8_t* payload, size_t* payload_len);
        void create(const uint8_t* data, const size_t data_len,                           uint8_t* payload, size_t* payload_len);
        void create(const uint8_t* data, const size_t data_len, const uint32_t timestamp, uint8_t* payload, size_t* payload_len);

    private:
        const static size_t BUFFER_SERIAL_BUFFER_SIZE = 256;
        uint8_t _stream_buffer[BUFFER_SERIAL_BUFFER_SIZE];
        BufferStream _stream;
        Aead _aead;

        size_t create_packet(const uint8_t* cipher, const size_t cipher_len, const uint8_t* nonce, const size_t nonce_len);
        size_t create_packet(const uint8_t* cipher, const size_t cipher_len, const uint8_t* nonce, const size_t nonce_len, const uint8_t* timestamp, const size_t timestamp_len);
};

#endif
