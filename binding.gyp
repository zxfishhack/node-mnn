{
  "targets": [
    {
      "target_name": "node-mnn",
      "sources": [
        "src/exports.cpp",
        "src/ss_generator.cpp",
        "src/llm.cpp"
      ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")",
        "<!(node -e \"console.log(require('@378q/mnn-prebuilt').getIncludePath())\")"
      ],
      "libraries": [
        "<!(node -e \"const config = require('@378q/mnn-prebuilt').getGypConfig(); console.log(config.libraries.join(' '));\")"
      ],
      "library_dirs": [
        "<!(node -e \"console.log(require('@378q/mnn-prebuilt').getLibPath())\")"
      ],
      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ],
      "conditions": [
        ["OS=='mac'", {
          "xcode_settings": {
            "GCC_ENABLE_CPP_EXCEPTIONS": "YES",
            "CLANG_CXX_LIBRARY": "libc++",
            "MACOSX_DEPLOYMENT_TARGET": "10.9",
            "OTHER_LDFLAGS": [
              "-Wl,-rpath,@loader_path"
            ]
          }
        }],
        ["OS=='win'", {
          "defines": [
            "WIN32_LEAN_AND_MEAN"
          ]
        }],
      ],
      "msvs_settings": {
        "VCCLCompilerTool": { 
          "ExceptionHandling": 1,
          "AdditionalOptions": ["/utf-8"]
        }
      }
    }
  ]
}
