/**
 * Copyright 2015 TASER International, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "RDigester.hpp"

using namespace com::evidence::tools::digest;
using namespace std;

RDigester::RDigester(ChecksumAlg a) {
    this->alg = a;
    this->digestCtx = nullptr;

    switch (alg) {
        case Md5:
            digestCtx = new MD5_CTX;
            memset(digestCtx, 0, sizeof(MD5_CTX));
            MD5_Init((MD5_CTX *)digestCtx);
            break;
        case Sha1:
            digestCtx = new SHA_CTX;
            memset(digestCtx, 0, sizeof(MD5_CTX));
            SHA1_Init((SHA_CTX *)digestCtx);
            break;
        case Sha256:
            digestCtx = new SHA256_CTX;
            memset(digestCtx, 0, sizeof(MD5_CTX));
            SHA256_Init((SHA256_CTX *)digestCtx);
            break;
        case Sha512:
            digestCtx = new SHA512_CTX;
            memset(digestCtx, 0, sizeof(MD5_CTX));
            SHA512_Init((SHA512_CTX *)digestCtx);
            break;
        default:
            break;
    }
}

RDigester::RDigester(ChecksumAlg a, unsigned char *ctxSerialized)
{
    this->alg = a;
    this->digestCtx = nullptr;
    setCtx(ctxSerialized);
}

void RDigester::freeCtx()
{
    if( digestCtx != nullptr )
    {
        switch (alg) {
            case Md5:
                delete (MD5_CTX *)digestCtx;
                digestCtx = nullptr;
                break;
            case Sha1:
                delete (SHA_CTX *)digestCtx;
                digestCtx = nullptr;
                break;
            case Sha256:
                delete (SHA256_CTX *)digestCtx;
                digestCtx = nullptr;
                break;
            case Sha512:
                delete (SHA512_CTX *)digestCtx;
                digestCtx = nullptr;
                break;
            default:
                break;
        }
    }
}

RDigester::~RDigester() {
    freeCtx();
}

size_t RDigester::serialize(unsigned char *dst, size_t dstSize)
{
    memset(dst, 0, sizeof(unsigned char) * dstSize);
    
    switch (alg) {
        case Md5:
            return serialize(dst, dstSize, (MD5_CTX *)digestCtx);
        case Sha1:
            return serialize(dst, dstSize, (SHA_CTX *)digestCtx);
        case Sha256:
            return serialize(dst, dstSize, (SHA256_CTX *)digestCtx);
        case Sha512:
            return serialize(dst, dstSize, (SHA512_CTX *)digestCtx);
        default:
            return 0;
    }
}

void RDigester::setCtx(unsigned char *ctxSerialized) {
    freeCtx();
    
    switch (alg) {
        case Md5:
            digestCtx = deserializeMd5Ctx(ctxSerialized);
            break;
        case Sha1:
            digestCtx = deserializeSha1Ctx(ctxSerialized);
            break;
        case Sha256:
            digestCtx = deserializeSha256Ctx(ctxSerialized);
            break;
        case Sha512:
            digestCtx = deserializeSha512Ctx(ctxSerialized);
            break;
        default:
            digestCtx = nullptr;
            break;
    }
}

void RDigester::update(const void *data, size_t len)
{
    switch (alg) {
        case Md5:
            MD5_Update((MD5_CTX *)digestCtx, data, len);
            break;
        case Sha1:
            SHA1_Update((SHA_CTX *)digestCtx, data, len);
            break;
        case Sha256:
            SHA256_Update((SHA256_CTX *)digestCtx, data, len);
            break;
        case Sha512:
            SHA512_Update((SHA512_CTX *)digestCtx, data, len);
            break;
        default:
            break;
    }
}

std::string RDigester::finalChecksumHex()
{
    unsigned char *hash = nullptr;
    std::string rv;
    
    switch (alg) {
        case Md5:
            hash = new unsigned char[MD5_DIGEST_LENGTH];
            MD5_Final(hash, (MD5_CTX *)digestCtx);
            rv = toHex(hash, MD5_DIGEST_LENGTH);
            break;
        case Sha1:
            hash = new unsigned char[SHA_DIGEST_LENGTH];
            SHA1_Final(hash, (SHA_CTX *)digestCtx);
            rv = toHex(hash, SHA_DIGEST_LENGTH);
            break;
        case Sha256:
            hash = new unsigned char[SHA256_DIGEST_LENGTH];
            SHA256_Final(hash, (SHA256_CTX *)digestCtx);
            rv = toHex(hash, SHA256_DIGEST_LENGTH);
            break;
        case Sha512:
            hash = new unsigned char[SHA512_DIGEST_LENGTH];
            SHA512_Final(hash, (SHA512_CTX *)digestCtx);
            rv = toHex(hash, SHA512_DIGEST_LENGTH);
            break;
        default:
            break;
    }
    
    
    if( hash != nullptr ) {
        delete[] hash;
    }
    
    return rv;
}

std::string RDigester::toHex(unsigned char *data, int len)
{
    static char hexmap[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
    
    std::string s(len * 2, ' ');
    for (int i = 0; i < len; ++i) {
        s[2 * i]     = hexmap[(data[i] & 0xF0) >> 4];
        s[2 * i + 1] = hexmap[data[i] & 0x0F];
    }
    return s;
}

size_t RDigester::serialize(unsigned char *dst, size_t dstSize, const MD5_CTX *ctx)
{
    if( dstSize < (sizeof ctx->A +
                   sizeof ctx->B +
                   sizeof ctx->C +
                   sizeof ctx->D +
                   sizeof ctx->Nl +
                   sizeof ctx->Nh +
                   sizeof(MD5_LONG) * MD5_LBLOCK +
                   sizeof ctx->num) ) {
        return -1;
    }
    
    size_t i = 0;
    
    // A
    memcpy(&dst[i], &ctx->A, sizeof ctx->A);
    i += sizeof ctx->A;
    
    // B
    memcpy(&dst[i], &ctx->B, sizeof ctx->B);
    i += sizeof ctx->B;
    
    // C
    memcpy(&dst[i], &ctx->C, sizeof ctx->C);
    i += sizeof ctx->C;
    
    // D
    memcpy(&dst[i], &ctx->D, sizeof ctx->D);
    i += sizeof ctx->D;
    
    // Nl
    memcpy(&dst[i], &ctx->Nl, sizeof ctx->Nl);
    i += sizeof ctx->Nl;
    
    // Nh
    memcpy(&dst[i], &ctx->Nh, sizeof ctx->Nh);
    i += sizeof ctx->Nh;
    
    // data
    memcpy(&dst[i], &ctx->data, sizeof(MD5_LONG) * MD5_LBLOCK);
    i += sizeof(MD5_LONG) * MD5_LBLOCK;
    
    // num
    memcpy(&dst[i], &ctx->num, sizeof ctx->num);
    i += sizeof ctx->num;
    
    return i;
}

MD5_CTX* RDigester::deserializeMd5Ctx(unsigned char *src)
{
    size_t i = 0;
    MD5_CTX *ctx = new MD5_CTX();
    
    // A
    memcpy(&ctx->A, &src[i], sizeof ctx->A);
    i += sizeof ctx->A;
    
    // B
    memcpy(&ctx->B, &src[i], sizeof ctx->B);
    i += sizeof ctx->B;
    
    // C
    memcpy(&ctx->C, &src[i], sizeof ctx->C);
    i += sizeof ctx->C;
    
    // D
    memcpy(&ctx->D, &src[i], sizeof ctx->D);
    i += sizeof ctx->D;
    
    // Nl
    memcpy(&ctx->Nl, &src[i], sizeof ctx->Nl);
    i += sizeof ctx->Nl;
    
    // Nh
    memcpy(&ctx->Nh, &src[i], sizeof ctx->Nh);
    i += sizeof ctx->Nh;
    
    // data
    memcpy(&ctx->data, &src[i], sizeof(MD5_LONG) * MD5_LBLOCK);
    i += sizeof(MD5_LONG) * MD5_LBLOCK;
    
    // num
    memcpy(&ctx->num, &src[i], sizeof ctx->num);
    i += sizeof ctx->num;
    
    return ctx;
}

size_t RDigester::serialize(unsigned char *dst, size_t dstSize, const SHA256_CTX *ctx)
{
    if( dstSize < (sizeof(SHA_LONG) * 8 +
                   sizeof ctx->Nl +
                   sizeof ctx->Nh +
                   sizeof(SHA_LONG) * SHA_LBLOCK +
                   sizeof ctx->num +
                   sizeof ctx->md_len) ) {
        return -1;
    }
    
    size_t i = 0;
    
    // h
    memcpy(&dst[i], &ctx->h, sizeof(SHA_LONG) * 8);
    i += sizeof(SHA_LONG) * 8;
    
    // Nl
    memcpy(&dst[i], &ctx->Nl, sizeof ctx->Nl);
    i += sizeof ctx->Nl;
    
    // Nh
    memcpy(&dst[i], &ctx->Nh, sizeof ctx->Nh);
    i += sizeof ctx->Nh;
    
    // data
    memcpy(&dst[i], &ctx->data, sizeof(SHA_LONG) * SHA_LBLOCK);
    i += sizeof(SHA_LONG) * SHA_LBLOCK;
    
    // num
    memcpy(&dst[i], &ctx->num, sizeof ctx->num);
    i += sizeof ctx->num;
    
    // md_len
    memcpy(&dst[i], &ctx->md_len, sizeof ctx->md_len);
    i += sizeof ctx->md_len;
    
    return i;
}

SHA256_CTX* RDigester::deserializeSha256Ctx(unsigned char *src)
{
    size_t i = 0;
    SHA256_CTX *ctx = new SHA256_CTX();
    memset(ctx, 0, sizeof(SHA256_CTX));
    
    // h
    memcpy(&ctx->h, &src[i], sizeof(SHA_LONG) * 8);
    i += sizeof(SHA_LONG) * 8;
    
    // Nl
    memcpy(&ctx->Nl, &src[i], sizeof ctx->Nl);
    i += sizeof ctx->Nl;
    
    // Nh
    memcpy(&ctx->Nh, &src[i], sizeof ctx->Nh);
    i += sizeof ctx->Nh;
    
    // data
    memcpy(&ctx->data, &src[i], sizeof(SHA_LONG) * SHA_LBLOCK);
    i += sizeof(SHA_LONG) * SHA_LBLOCK;
    
    // num
    memcpy(&ctx->num, &src[i], sizeof ctx->num);
    i += sizeof ctx->num;
    
    // md_len
    memcpy(&ctx->md_len, &src[i], sizeof ctx->md_len);
    i += sizeof ctx->md_len;
    
    return ctx;
}

size_t RDigester::serialize(unsigned char *dst, size_t dstSize, const SHA512_CTX *ctx)
{
    if( dstSize < (sizeof(SHA_LONG64) * 8 +
                   sizeof ctx->Nl +
                   sizeof ctx->Nh +
                   sizeof(SHA_LONG64) * SHA_LBLOCK +
                   sizeof(unsigned char) * SHA512_CBLOCK +
                   sizeof ctx->num +
                   sizeof ctx->md_len) ) {
        return -1;
    }
    
    size_t i = 0;
    
    // h
    memcpy(&dst[i], &ctx->h, sizeof(SHA_LONG64) * 8);
    i += sizeof(SHA_LONG64) * 8;
    
    // Nl
    memcpy(&dst[i], &ctx->Nl, sizeof ctx->Nl);
    i += sizeof ctx->Nl;
    
    // Nh
    memcpy(&dst[i], &ctx->Nh, sizeof ctx->Nh);
    i += sizeof ctx->Nh;
    
    // u.d
    memcpy(&dst[i], &ctx->u.d, sizeof(SHA_LONG64) * SHA_LBLOCK);
    i += sizeof(SHA_LONG64) * SHA_LBLOCK;
    
    // u.p
    memcpy(&dst[i], &ctx->u.p, sizeof(unsigned char) * SHA512_CBLOCK);
    i += sizeof(unsigned char) * SHA512_CBLOCK;
    
    // num
    memcpy(&dst[i], &ctx->num, sizeof ctx->num);
    i += sizeof ctx->num;
    
    // md_len
    memcpy(&dst[i], &ctx->md_len, sizeof ctx->md_len);
    i += sizeof ctx->md_len;
    
    return i;
}

SHA512_CTX* RDigester::deserializeSha512Ctx(unsigned char *src)
{
    size_t i = 0;
    SHA512_CTX *ctx = new SHA512_CTX();
    
    // h
    memcpy(&ctx->h, &src[i], sizeof(SHA_LONG64) * 8);
    i += sizeof ctx->h;
    
    // Nl
    memcpy(&ctx->Nl, &src[i], sizeof ctx->Nl);
    i += sizeof ctx->Nl;
    
    // Nh
    memcpy(&ctx->Nh, &src[i], sizeof ctx->Nh);
    i += sizeof ctx->Nh;
    
    // u.d
    memcpy(&ctx->u.d, &src[i], sizeof(SHA_LONG64) * SHA_LBLOCK);
    i += sizeof(SHA_LONG64) * SHA_LBLOCK;
    
    // u.p
    memcpy(&ctx->u.p, &src[i], sizeof(unsigned char) * SHA512_CBLOCK);
    i += sizeof(unsigned char) * SHA512_CBLOCK;
    
    // num
    memcpy(&ctx->num, &src[i], sizeof ctx->num);
    i += sizeof ctx->num;
    
    // md_len
    memcpy(&ctx->md_len, &src[i], sizeof ctx->md_len);
    i += sizeof ctx->md_len;
    
    return ctx;
}

size_t RDigester::serialize(unsigned char *dst, size_t dstSize, const SHA_CTX *ctx)
{
    if( dstSize < (sizeof ctx->h0 +
                   sizeof ctx->h1 +
                   sizeof ctx->h2 +
                   sizeof ctx->h3 +
                   sizeof ctx->h4 +
                   sizeof ctx->Nl +
                   sizeof ctx->Nh +
                   sizeof(SHA_LONG) * SHA_LBLOCK +
                   sizeof ctx->num) ) {
        return -1;
    }
    
    size_t i = 0;
    
    // h0
    memcpy(&dst[i], &ctx->h0, sizeof ctx->h0);
    i += sizeof ctx->h0;
    
    // h1
    memcpy(&dst[i], &ctx->h1, sizeof ctx->h1);
    i += sizeof ctx->h1;
    
    // h2
    memcpy(&dst[i], &ctx->h2, sizeof ctx->h2);
    i += sizeof ctx->h2;
    
    // h3
    memcpy(&dst[i], &ctx->h3, sizeof ctx->h3);
    i += sizeof ctx->h3;
    
    // h4
    memcpy(&dst[i], &ctx->h4, sizeof ctx->h4);
    i += sizeof ctx->h4;
    
    // Nl
    memcpy(&dst[i], &ctx->Nl, sizeof ctx->Nl);
    i += sizeof ctx->Nl;
    
    // Nh
    memcpy(&dst[i], &ctx->Nh, sizeof ctx->Nh);
    i += sizeof ctx->Nh;
    
    // data
    memcpy(&dst[i], &ctx->data, sizeof(SHA_LONG) * SHA_LBLOCK);
    i += sizeof(SHA_LONG) * SHA_LBLOCK;
    
    // num
    memcpy(&dst[i], &ctx->num, sizeof ctx->num);
    i += sizeof ctx->num;
    
    return i;
}

SHA_CTX* RDigester::deserializeSha1Ctx(unsigned char *src)
{
    size_t i = 0;
    SHA_CTX *ctx = new SHA_CTX();
    
    // h0
    memcpy(&ctx->h0, &src[i], sizeof ctx->h0);
    i += sizeof ctx->h0;
    
    // h1
    memcpy(&ctx->h1, &src[i], sizeof ctx->h1);
    i += sizeof ctx->h1;
    
    // h2
    memcpy(&ctx->h2, &src[i], sizeof ctx->h2);
    i += sizeof ctx->h2;
    
    // h3
    memcpy(&ctx->h3, &src[i], sizeof ctx->h3);
    i += sizeof ctx->h3;
    
    // h4
    memcpy(&ctx->h4, &src[i], sizeof ctx->h4);
    i += sizeof ctx->h4;
    
    // Nl
    memcpy(&ctx->Nl, &src[i], sizeof ctx->Nl);
    i += sizeof ctx->Nl;
    
    // Nh
    memcpy(&ctx->Nh, &src[i], sizeof ctx->Nh);
    i += sizeof ctx->Nh;
    
    // data
    memcpy(&ctx->data, &src[i], sizeof(SHA_LONG) * SHA_LBLOCK);
    i += sizeof(SHA_LONG) * SHA_LBLOCK;
    
    // num
    memcpy(&ctx->num, &src[i], sizeof ctx->num);
    i += sizeof ctx->num;
    
    return ctx;
}
