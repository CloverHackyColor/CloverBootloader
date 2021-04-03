//
//  base64.h
//  cpp_tests
//
//  Created by Jief on 24/08/2020.
//  Copyright © 2020 jief_machack. All rights reserved.
//

#ifndef base64_h
#define base64_h

char *base64_encode(const unsigned char *data,
size_t input_length,
size_t *output_length);

#endif /* base64_h */
