{
  "targets": [{
    "target_name": "node_edlib",
    "sources": [
      "src/node_edlib.cpp",
      "lib/edlib.cpp"
    ],
    "cflags!": ["-O2"],
    "cflags+": ["-O3"]
  }]
}
