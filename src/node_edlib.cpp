#include <nan.h>

#include <string>
#include <cstdlib>

#include "../edlib/edlib.h"

/**
 * Converts string to sequence (array of unsigned chars).
 * Expects seqC to point to array.
 * Saves mapping of letters to numbers in letterIdx, also updated inAlphabet and alphabetLength.
 * letterIdx, inAlphabet and alphabetLength are also used as inputs.
 */
void convertStringToSequence(std::string seq, unsigned char* seqC,
                             unsigned char letterIdx[], bool inAlphabet[], int &alphabetLength) {
    for (unsigned int i = 0; i < seq.length(); i++) {
        if (seq[i] < 0) {
            //ThrowException(Exception::TypeError(String::New("Character is not in range")));
            continue;
        }
        unsigned char c = (unsigned char)seq[i];
        if (!inAlphabet[c]) {
            inAlphabet[c] = true;
            letterIdx[c] = alphabetLength++;
        }
        seqC[i] = letterIdx[c];
    }
}

/**
 * Arguments expected: {string} query, {string} target,  mode
 * @param {string} query
 * @param {string} target
 * @param {object} params {
 *   mode: 'HW' || 'SHW' || 'NW' || 'OV'
 * }
 * @return {
 *   editDistance: integer,
 *   locations: [{
 *     start: integer,
 *     end: integer
 *   }]
 * }
 */
void align(const Nan::FunctionCallbackInfo<v8::Value>& args) {
    int edlibMode = EDLIB_MODE_HW;

    // TODO: support more input parameters, and return alignment.
    // TODO: understand difference between Handle and Local -> I may be doing something wrong.

    // ------------------------ TRANSFORM INPUT -------------------- //
    if (args.Length() < 2 || args.Length() > 3) {
        Nan::ThrowTypeError("Wrong number of arguments");
        return;
    }
    if (!args[0]->IsString() || !args[1]->IsString()) {
        Nan::ThrowTypeError("First and second argument must be strings");
        return;
    }

    std::string query(*Nan::Utf8String(args[0]->ToString()));
    std::string target(*Nan::Utf8String(args[1]->ToString()));

    v8::Local<v8::Object> params = (args.Length() == 3) ? args[2]->ToObject() : Nan::New<v8::Object>();
    v8::Local<v8::Value> mode = params->Get(Nan::New("mode").ToLocalChecked());
    if (mode->IsString()) {
        std::string modeString(*Nan::Utf8String(mode->ToString()));
        if (modeString == "HW") edlibMode = EDLIB_MODE_HW;
        else if (modeString == "NW") edlibMode = EDLIB_MODE_NW;
        else if (modeString == "SHW") edlibMode = EDLIB_MODE_SHW;
        else if (modeString == "OV") edlibMode = EDLIB_MODE_OV;
        else {
            Nan::ThrowTypeError("Invalid edlib mode");
            return;
        }
    }

    // We construct alphabet from query and target sequence
    unsigned char letterIdx[128]; //!< letterIdx[c] is index of letter c in alphabet
    bool inAlphabet[128]; // inAlphabet[c] is true if c is in alphabet
    for (int i = 0; i < 128; i++) inAlphabet[i] = false;
    int alphabetLength = 0;

    unsigned char* queryC = new unsigned char[query.length()];
    convertStringToSequence(query, queryC, letterIdx, inAlphabet, alphabetLength);
    unsigned char* targetC = new unsigned char[target.length()];
    convertStringToSequence(target, targetC, letterIdx, inAlphabet, alphabetLength);
    // -------------------------------------------------------------- //

    // --------------------- FIND ALIGNMENT ------------------------- //
    int bestScore, numLocations, alignmentLength;
    int* endLocations, *startLocations;
    unsigned char* alignment;
    edlibCalcEditDistance(queryC, query.length(), targetC, target.length(),
                          alphabetLength, -1, edlibMode, false, false,
                          &bestScore, &endLocations, &startLocations, &numLocations,
                          &alignment, &alignmentLength);
    // -------------------------------------------------------------- //



    v8::Local<v8::Object> result = Nan::New<v8::Object>();
    result->Set(Nan::New("editDistance").ToLocalChecked(), Nan::New(bestScore));
    if (startLocations || endLocations) {
        v8::Local<v8::Array> locations = Nan::New<v8::Array>(numLocations);
        for (int i = 0; i < numLocations; i++) {
            v8::Local<v8::Object> location = Nan::New<v8::Object>();
            if (startLocations) {
                location->Set(Nan::New("start").ToLocalChecked(), Nan::New(startLocations[i]));
            }
            if (endLocations) {
                location->Set(Nan::New("end").ToLocalChecked(), Nan::New(endLocations[i]));
            }
            locations->Set(i, location);
        }
        result->Set(Nan::New("locations").ToLocalChecked(), locations);
    }

    delete[] queryC;
    delete[] targetC;
    if (endLocations) free(endLocations);
    if (startLocations) free(startLocations);
    if (alignment) free(alignment);

    args.GetReturnValue().Set(result);
}


void init(v8::Local<v8::Object> exports) {
    exports->Set(Nan::New("align").ToLocalChecked(),
                 Nan::New<v8::FunctionTemplate>(align)->GetFunction());
}

NODE_MODULE(node_edlib, init)
