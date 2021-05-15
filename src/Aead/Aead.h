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

#ifndef KiotlogSN_Payload_Aead_h
#define KiotlogSN_Payload_Aead_h

#include <Arduino.h>

#include <Crypto.h>
#include <ChaChaPoly.h>

#define KEY_SIZE 32
#define MAX_NONCE_SIZE 12
#define TAG_SIZE 16

class Aead {
    public:
        Aead() = default;
        Aead(const uint8_t * key);

        void begin(const uint8_t pin = A0);
        void setKey(const uint8_t *key);
        void authEncrypt(const uint8_t * msg, const size_t msg_len, uint8_t* cipher, uint8_t* nonce);
        void setNonceSize(size_t nonce_size) { _nonce_size = nonce_size; }
        size_t nonceSize() { return _nonce_size; }

    protected:
        ChaChaPoly _chachapoly;

        const uint8_t * _key;
        uint8_t _tag[TAG_SIZE];
        size_t _nonce_size = MAX_NONCE_SIZE;
        uint8_t _nonce[MAX_NONCE_SIZE];

        void increment_iv();
};

#endif
