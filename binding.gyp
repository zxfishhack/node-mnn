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
      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ],
      "conditions": [
        ["OS=='mac'", {
          "libraries": [
            "<!(node -e \"const config = require('@378q/mnn-prebuilt').getGypConfig(); console.log(config.libraries.join(' '));\")"
          ],
          "library_dirs": [
            "<!(node -e \"console.log(require('@378q/mnn-prebuilt').getLibPath())\")"
          ],
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
          "libraries": [
            "<!(node -e \"const config = require('@378q/mnn-prebuilt').getGypConfig(); console.log(config.libraries.join(' '));\")"
          ],
          "library_dirs": [
            "<!(node -e \"console.log(require('@378q/mnn-prebuilt').getLibPath())\")"
          ],
          "defines": [
            "WIN32_LEAN_AND_MEAN"
          ]
        }],
        ["OS=='linux'", {
          "libraries": [
            "<!(node -e \"const config = require('@378q/mnn-prebuilt').getGypConfig(); console.log(config.libraries.join(' '));\")"
          ],
          "library_dirs": [
            "<!(node -e \"console.log(require('@378q/mnn-prebuilt').getLibPath())\")"
          ]
        }]
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
