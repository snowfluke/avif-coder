/*
 * MIT License
 *
 * Copyright (c) 2023 Radzivon Bartoshyk
 * avif-coder [https://github.com/awxkee/avif-coder]
 *
 * Created by Radzivon Bartoshyk on 07/09/2023
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#ifndef AVIF_PERCEPTUALQUANTINIZER_H
#define AVIF_PERCEPTUALQUANTINIZER_H

#include <cstdint>

enum PQGammaCorrection {
    Rec2020, DCIP3
};

class PerceptualQuantinizer {
public:
    PerceptualQuantinizer(uint8_t *rgbaData, int stride, int width, int height, bool U16,
                          bool halfFloats, int bitDepth, PQGammaCorrection gammaCorrection) {
        this->bitDepth = bitDepth;
        this->halfFloats = halfFloats;
        this->rgbaData = rgbaData;
        this->stride = stride;
        this->width = width;
        this->height = height;
        this->U16 = U16;
        this->gammaCorrection = gammaCorrection;
    }

    void transfer();

private:
    bool halfFloats;
    int stride;
    int bitDepth;
    int width;
    int height;
    uint8_t *rgbaData;
    PQGammaCorrection gammaCorrection;
    bool U16;
protected:
};


#endif //AVIF_PERCEPTUALQUANTINIZER_H
