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

#include "Aead.h"

Aead::Aead(const uint8_t * key)
    : _key(key)
{
    _chachapoly.setKey(_key, KEY_SIZE);
}

void Aead::begin(const uint8_t noise_pin)
{
    pinMode(noise_pin, INPUT);
    randomSeed(analogRead(noise_pin));
    for (int i = 0; i < NONCE_SIZE; i++)
    {
        _nonce[i] = (uint8_t)random(256);
    }
}

void Aead::setKey(const uint8_t *key)
{
    _chachapoly.setKey(key, KEY_SIZE);
}

/*
   https://github.com/jedisct1/libsodium/blob/06a523423abeb3993c2ea6f149f10230cda58e8a/src/libsodium/sodium/utils.c#L237
*/
void Aead::increment_iv()
{
    size_t i = 0U;
    uint_fast16_t c = 1U;

    for (; i < NONCE_SIZE; i++)
    {
        c += (uint_fast16_t)_nonce[i];
        _nonce[i] = (unsigned char)c;
        c >>= 8;
    }
}

void Aead::authEncrypt(const uint8_t *msg, const size_t msg_len, uint8_t* cipher, uint8_t* nonce)
{
    _chachapoly.setIV(_nonce, NONCE_SIZE);
    _chachapoly.encrypt(cipher, msg, msg_len);
    _chachapoly.computeTag(_tag, TAG_SIZE);

    memcpy(cipher + msg_len, _tag, TAG_SIZE);
    memcpy(nonce, _nonce, NONCE_SIZE);

    increment_iv();
}
