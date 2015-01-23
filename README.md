# node-edlib

Node wrapper for [edlib](https://github.com/Martinsos/edlib) sequence alignment library.

## Usage
```
var edlib = require('node-edlib');
var result = edlib.align('elephant', 'telephone', { mode: 'NW' });
console.log(result);
```
