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

#include "KlsnPayload.h"

KlsnPayload::KlsnPayload()
    : _stream(_stream_buffer, BUFFER_SERIAL_BUFFER_SIZE)
{
}

void KlsnPayload::begin(const uint8_t * key, size_t nonce_size)
{
    _aead.begin();
    _aead.setKey(key);
    _aead.setNonceSize(nonce_size);
}

// void KlsnPayload::authEncrypt(const uint8_t* data, const size_t data_len, uint8_t* cipher, size_t* cipher_len, uint8_t* nonce, size_t* nonce_len)
// {
//     _aead.authEncrypt(data, data_len, cipher, cipher_len, nonce, nonce_len);

// }

void KlsnPayload::pack(const uint8_t* cipher, size_t cipher_len, const uint8_t* nonce, size_t nonce_len, uint8_t* payload, size_t* payload_len)
{
    const uint8_t* s = _stream;

    size_t len = create_packet(cipher, cipher_len, nonce, nonce_len);
    memcpy(payload, s, len);
    *payload_len = len;

}

void KlsnPayload::pack(const uint8_t* cipher, size_t cipher_len, const uint8_t* nonce, size_t nonce_len, const uint8_t* timestamp, size_t timestamp_len, uint8_t* payload, size_t* payload_len)
{
    const uint8_t* s = _stream;

    size_t len = create_packet(cipher, cipher_len, nonce, nonce_len, timestamp, timestamp_len);
    memcpy(payload, s, len);
    *payload_len = len;

}

void KlsnPayload::create(const uint8_t* data, const size_t data_len, uint8_t* payload, size_t* payload_len)
{
    const uint8_t* s = _stream;

    size_t nonce_size = _aead.nonceSize();

    Serial.print("nonce_size: ");
    Serial.println(nonce_size);

    size_t cipher_len = data_len + TAG_SIZE;
    uint8_t cipher[cipher_len], nonce[nonce_size];

    _aead.authEncrypt(data, data_len, cipher, nonce);

    size_t len = create_packet(cipher, cipher_len, nonce, nonce_size);
    memcpy(payload, s, len);
    *payload_len = len;
}

void KlsnPayload::create(const uint8_t* data, size_t data_len, const uint32_t timestamp, uint8_t* payload, size_t* payload_len)
{
    const uint8_t* s = _stream;
    size_t nonce_size = _aead.nonceSize();

    Serial.print("nonce_size: ");
    Serial.println(nonce_size);

    size_t cipher_len = data_len + TAG_SIZE;
    uint8_t cipher[cipher_len], nonce[nonce_size];

    _aead.authEncrypt(data, data_len, cipher, nonce);

    size_t len = create_packet(cipher, cipher_len, nonce, nonce_size, (const uint8_t *)&timestamp, sizeof(timestamp));
    memcpy(payload, s, len);
    *payload_len = len;
}

size_t KlsnPayload::create_packet(const uint8_t* cipher, size_t cipher_len, const uint8_t* nonce, size_t nonce_len)
{
    _stream.flush();

    msgpck_write_map_header(&_stream, 2);
    msgpck_write_string(&_stream, "nonce");
    msgpck_write_bin(&_stream, (byte*)nonce, nonce_len);
    msgpck_write_string(&_stream, "data");
    msgpck_write_bin(&_stream, (byte*)cipher, cipher_len);

    return _stream.available();
}

size_t KlsnPayload::create_packet(const uint8_t* cipher, const size_t cipher_len, const uint8_t* nonce, const size_t nonce_len, const uint8_t* timestamp, const size_t timestamp_len)
{
    _stream.flush();

    msgpck_write_map_header(&_stream, 3);
    msgpck_write_string(&_stream, "nonce");
    msgpck_write_bin(&_stream, (byte*)nonce, nonce_len);
    msgpck_write_string(&_stream, "data");
    msgpck_write_bin(&_stream, (byte*)cipher, cipher_len);
    msgpck_write_string(&_stream, "ts");
    msgpck_write_bin(&_stream, (byte*)timestamp, timestamp_len);

    return _stream.available();
}
