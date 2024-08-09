# WIP!
This project is still work-in-progress! It may not work as expected

# Markdown Parser

Simple, efficient, single-header C++ (14) markdown to html converison algorithm. Following features are supported:

- Headings
- Italic text
- Bold Text
- Italic and bold text
- Blockquotes
- Ordered Lists
- Unordered Lists
- Code
- Horizontal Rule
- Link ( \[title](link) form )
- Image
- Fenced Code Block
- Strikethrough
- Html itself

Parser was implemented according to the rules from this website: <https://www.markdownguide.org/cheat-sheet/>

# Pros
- Fast
- Customizable
- Single Header

# Usage
```cpp
#include "markdown_parsing.hpp"

[...]

std::string markdown_snippet = R"(
  ## Foo
  ***Bar***
)";

//Parse using the default html marks set
std::cout << markdown_parsing::markdown_to_html(source);

//Create a custom set of html marks, and then override html marks for italic text
markdown_parsing::html_marks marks;
marks.italic_marks = { "<em><i>", "</i></em>" };

//Parse using the custom marks set
std::cout << markdown_parsing::markdown_to_html(source, marks);
```

# Building
This parser is implemented as a typical single header library.  
Once you have your copy of the ``markdown_parser.hpp``, create a file ``markdown_parser.cpp`` inside your project and paste in the following code:
```cpp
#define MARKDOWN_PARSER_IMPLEMENTATION
#include "markdown_parser.hpp"
```
Once you configure your building environement to build with ``markdown_parser.cpp`` you can use the parser everywhere inside your project.
