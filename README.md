# PoC for Project 1: Implement MultiLang Parser

This project implements a multilanguage parser for 3 languages (python, javascript, ruby) as part of the Proof of Concept for MetaCall's Project 1. The aim was take in source files and parse them using Tree-sitter API, create AST and reduced IR. Adapting the generated IR to a unified standard format. It also explores dependency graphs (minimal for this PoC) which is necessary for Function Mesh and Intellisense. 

# How to Use

[NOTE] This PoC is currently tested on a MacOS environment and might cause errors on a different setup

```
# build (Ninja)
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build -j

# for help
./build/polygot_parser -h

# parse a single file
./build/polygot_parser -f examples/example.js

# parse a directory recursively
./build/polygot_parser -d examples/

```

##  CLI Reference
```
  polygot_parser -f <file>           parse one file
  polygot_parser -f <file1> <file2>  parse more than one file
  polygot_parser -d <directory>      parse all supported files in a directory

Other options:
  -o <output.json>        write JSON to file (default: stdout)
  -h, --help              show this help
```
The output is the unified JSON format which can be consumed by the VS Code extension (Intellisense) and the Function Mesh.
 

## FileTree Structure
```
├─ CMakeLists.txt
├─ README.md
├─ src/
│  ├─ main.c                      # main cli logic is defined here
│  ├─ parser.c                    # the parser logic is handled here
│  └─ parser.h
├─ adapters/                       # extracts language specific queries
│  ├─ adapters.h
│  ├─ adapters.c
│  ├─ python_adapter.c           
│  ├─ js_adapter.c              
│  └─ ruby_adapter.c        
├─ ir/
│  ├─ ir.h                  
│  └─ ir.c                  # creates the Intermediate Representation(IR)
├─ graph/
│  ├─ graph.h
│  └─ graph.c              # creates a minimal dependency graph
├─ exporter/
│  ├─ mc_export.h   
│  └─ mc_export.c        # exports to a json output which can be consumed later
├─ tests/               
└─ examples/        # some example files 
```

CMakeLists.txt - Build configuration and dependencies
src/main.c - Main CLI entry point
src/parser.c - parsing flow and file traversal
adapters - tree-sitter extration for languages(py,js,rb)
ir - normalized IR for symbols and exports
graph- dependency graph builder with a type associated with its edges and nodes
exporter - JSON export of IR and graph


## Architecture

<img width="600" height="688" alt="image" src="https://github.com/user-attachments/assets/b38ff257-4c71-4ea8-9b79-b3702c1245ce" />



## JSON Schema

```
{
  "languages": {
    "<lang>": {
      "functions": [
        { "name": "sum", "args": ["a","b"], "exported": true }
      ],
      "classes": [
        { "name": "Calculator", "args": [], "exported": true }
      ],
      "objects": [
        { "name": "CONFIG", "exported": true }
      ]
    }
  },
  "graph": {
    "edges": [
      {
        "from": "examples/example.js",
        "from_kind": "file",        // file | symbol | module
        "to": "examples/example.py",
        "to_kind": "module",       // file | symbol | module
        "type": "require",         // import | require | define | export | member_of
        "lang": "js"
      }
    ]
  }
}
```

## Example Output

### HELP Command
<img width="861" height="301" alt="image" src="https://github.com/user-attachments/assets/d899acec-1d04-4dc3-8c07-b76a2caeb32a" />

### Output for example.js
<img width="1466" height="795" alt="image" src="https://github.com/user-attachments/assets/79633e9c-0889-4f5d-8177-48587415b641" />

## Integration with Metacall VSCode Extension?

As you can see example.py has different functions defined with arguments
<img width="777" height="448" alt="image" src="https://github.com/user-attachments/assets/14a8fc7c-f574-442e-828a-f1ce0658bc85" />

I called these functions in the JS File and gave it different arguments and as you can see there are the error wriggles! Comes from the VSCode Extension :)

<img width="777" height="448" alt="image" src="https://github.com/user-attachments/assets/fd3b8951-e70b-40f7-b31d-2b224d5acd25" />

This is due to the generated parser output:

<img width="777" height="780" alt="image" src="https://github.com/user-attachments/assets/aea8573e-73aa-4d76-b57a-e886e11cc8da" />




