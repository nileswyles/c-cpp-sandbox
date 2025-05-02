#ifndef WYLESLIBS_BASE64_H
#define WYLESLIBS_BASE64_H

#include <vector>
#include <string>
#include <iostream>

namespace WylesLibs {
    static  std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    // for collections of type char or uint8_t
    template<typename T, typename RT>
    RT base64Decode(T& input) {
        if (input.size() == 0) {
            return RT();
        }
        RT decoded;
        int i = 0;
        int j = 0;
        std::vector<int> char_array_4(4), char_array_3(3);

        for (char encoded_char : input) {
            if (encoded_char == '=') {
                break;
            }

            int char_index = base64_chars.find(encoded_char);
            if (char_index == std::string::npos) {
                continue;
            }

            char_array_4[i++] = char_index;
            if (i == 4) {
                char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
                char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
                char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

                for (int k = 0; k < 3; k++) {
                    decoded += static_cast<char>(char_array_3[k]);
                }
                i = 0;
            }
        }

        if (i) {
            for (int k = i; k < 4; k++) {
                char_array_4[k] = 0;
            }

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);

            for (int k = 0; (k < i - 1); k++) {
                decoded += static_cast<char>(char_array_3[k]);
            }
        }

        return decoded;
    }

    template<typename T>
    T base64Decode(T& input) {
        return base64Decode<T, T>(input);
    }

    template<typename T, typename RT>
    RT base64Encode(T& input) {
        RT encoded_string;
        int i = 0;
        unsigned char char_array_3[3];
        unsigned char char_array_4[4];
        int input_length = input.size();

        while (input_length--) {
            char_array_3[i++] = input[input.size() - input_length - 1];
            if (i == 3) {
                char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
                char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
                char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
                char_array_4[3] = char_array_3[2] & 0x3f;

                for (i = 0; (i < 4); i++)
                    encoded_string += base64_chars[char_array_4[i]];
                i = 0;
            }
        }

        if (i) {
            for (int j = i; j < 3; j++)
                char_array_3[j] = '\0';

            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for (int j = 0; (j < i + 1); j++)
                encoded_string += base64_chars[char_array_4[j]];

            while ((i++ < 3))
                encoded_string += '=';
        }

        return encoded_string;
    }

    // LOL - also this
    template<typename T>
    T base64Encode(T& input) {
        return base64Encode<T, T>(input);
    }
};

#endif