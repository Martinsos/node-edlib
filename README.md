# node-edlib

Node wrapper for [edlib](https://github.com/Martinsos/edlib) sequence alignment library.

## Installation
`npm install node-edlib --save`

## Usage
```javascript
var edlib = require('node-edlib');
var result = edlib.align('elephant', 'telephone', { mode: 'NW' });
console.log(result);
```

## Test
`npm test` should run with no errors.

## Developing
When developing, first run make to download edlib, and then run `npm install` to install node-edlib
Then run `npm test` to test that all is ok.
