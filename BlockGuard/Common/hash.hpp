//
// Created by srai on 4/17/19.
//

#ifndef Hash_hpp
#define Hash_hpp


#include <openssl/sha.h>

std::string sha256(const std::string& str){
	unsigned char digest[SHA256_DIGEST_LENGTH];

	SHA256_CTX ctx;
	SHA256_Init(&ctx);
	SHA256_Update(&ctx, str.c_str(), str.length());
	SHA256_Final(digest, &ctx);

	std::stringstream hash;
	for(unsigned char i : digest){
		hash<< std::hex << std::setw(2) << std::setfill('0') << (int)i;
	}
	return hash.str();
}


#endif //Hash_hpp
