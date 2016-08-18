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
 *   mode: 'HW' || 'SHW' || 'NW'
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
    EdlibAlignMode edlibMode = EDLIB_MODE_HW;

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
        else {
            Nan::ThrowTypeError("Invalid edlib mode");
            return;
        }
    }
    // -------------------------------------------------------------- //

    // --------------------- FIND ALIGNMENT ------------------------- //
    EdlibAlignResult edlibResult = edlibAlign(query.c_str(), query.length(), target.c_str(), target.length(),
                                              edlibNewAlignConfig(-1, edlibMode, EDLIB_TASK_LOC));
    // -------------------------------------------------------------- //


    v8::Local<v8::Object> result = Nan::New<v8::Object>();
    result->Set(Nan::New("editDistance").ToLocalChecked(), Nan::New(edlibResult.editDistance));
    if (edlibResult.startLocations || edlibResult.endLocations) {
        v8::Local<v8::Array> locations = Nan::New<v8::Array>(edlibResult.numLocations);
        for (int i = 0; i < edlibResult.numLocations; i++) {
            v8::Local<v8::Object> location = Nan::New<v8::Object>();
            if (edlibResult.startLocations) {
                location->Set(Nan::New("start").ToLocalChecked(), Nan::New(edlibResult.startLocations[i]));
            }
            if (edlibResult.endLocations) {
                location->Set(Nan::New("end").ToLocalChecked(), Nan::New(edlibResult.endLocations[i]));
            }
            locations->Set(i, location);
        }
        result->Set(Nan::New("locations").ToLocalChecked(), locations);
    }

    edlibFreeAlignResult(edlibResult);
    args.GetReturnValue().Set(result);
}


void init(v8::Local<v8::Object> exports) {
    exports->Set(Nan::New("align").ToLocalChecked(),
                 Nan::New<v8::FunctionTemplate>(align)->GetFunction());
}

NODE_MODULE(node_edlib, init)
