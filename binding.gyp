{
  "targets": [{
    "target_name": "node_edlib",
    "sources": [
      "src/node_edlib.cpp",
      "edlib/edlib.cpp"
    ],
    "cflags!": ["-O2"],
    "cflags+": ["-O3"]
  }]
}
