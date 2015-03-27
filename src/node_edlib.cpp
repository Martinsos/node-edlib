#include <node.h>
#include <v8.h>

#include <string>
#include <cstdlib>

#include "../edlib/edlib.h"

using namespace v8;

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
void align(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);

    int edlibMode = EDLIB_MODE_HW;

    // TODO: support more input parameters, and return alignment.
    // TODO: understand difference between Handle and Local -> I may be doing something wrong.

    // ------------------------ TRANSFORM INPUT -------------------- //
    if (args.Length() < 2 || args.Length() > 3) {
        isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Wrong number of arguments")));
        return;
    }
    if (!args[0]->IsString() || !args[1]->IsString()) {
        isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "First and second argument must be strings")));
        return;
    }

    std::string query(*String::Utf8Value(args[0]->ToString()));
    std::string target(*String::Utf8Value(args[1]->ToString()));

    Local<Object> params = (args.Length() == 3) ? args[2]->ToObject() : Object::New(isolate);
    Local<Value> mode = params->Get(String::NewFromUtf8(isolate, "mode"));
    if (mode->IsString()) {
        std::string modeString(*String::Utf8Value(mode->ToString()));
        if (modeString == "HW") edlibMode = EDLIB_MODE_HW;
        else if (modeString == "NW") edlibMode = EDLIB_MODE_NW;
        else if (modeString == "SHW") edlibMode = EDLIB_MODE_SHW;
        else if (modeString == "OV") edlibMode = EDLIB_MODE_OV;
        else {
            isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Invalid edlib mode")));
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



    Local<Object> result = Object::New(isolate);
    result->Set(String::NewFromUtf8(isolate, "editDistance"), Integer::New(isolate, bestScore));
    if (startLocations || endLocations) {
        Local<Array> locations = Array::New(isolate, numLocations);
        for (int i = 0; i < numLocations; i++) {
            Local<Object> location = Object::New(isolate);
            if (startLocations) {
                location->Set(String::NewFromUtf8(isolate, "start"), Integer::New(isolate, startLocations[i]));
            }
            if (endLocations) {
                location->Set(String::NewFromUtf8(isolate, "end"), Integer::New(isolate, endLocations[i]));
            }
            locations->Set(i, location);
        }
        result->Set(String::NewFromUtf8(isolate, "locations"), locations);
    }

    delete[] queryC;
    delete[] targetC;
    if (endLocations) free(endLocations);
    if (startLocations) free(startLocations);
    if (alignment) free(alignment);

    args.GetReturnValue().Set(result);
}


void init(Handle<Object> exports) {
    NODE_SET_METHOD(exports, "align", align);
}

NODE_MODULE(node_edlib, init)
