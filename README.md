# Brow (half a browser)

A basic HTML viewer made using C++, raylib and curl.

<details>
<summary><h2>Screenshot</h2></summary>
<img width="909" height="807" alt="image" src="https://github.com/user-attachments/assets/d8c2d057-d3ca-4669-92c7-5a9b5b48b013" />


</details>

## Features
- Block based rendering with margin and padding.
- Inline text rendering and basic inline div rendering. 
- Inline style support for font-size, color, background-color.
- Window resizing and zoom works.

## Limitations
- Not even remotely compliant with w3 specs.
- No JavaScript support.

## Usage
- To open a website, provide the url as a argument to the binary
- For example:
  ```bash
  ./brow "https://wiby.me/"
  ```
- To open local files, make a simple localhost server using python
  ```bash
  python -m http.server 4040
  ./brow "localhost:4040/demos/index.html"
  ```
- Make sure that the resources is folder is located in the working directory of the program.
- Try opening the index.html file in `demos`. If you end up writing any websites for this make sure to add it to the demos folder and make a pull request!

## Building
- This project uses CMake.
- Make sure you have raylib and curl installed on your system.
- Check if the raylib path in source code points to the correct header files.
- Run these commands in the terminal
  ```bash
  cmake -B build
  cmake --build build
  ```
- The binary will be in the build folder.

## What do i use this for?
This browser has almost no practical use, besides trying to write websites that render properly on this browser(with its numerous limitations) or seeing how modern sites break in amusing ways.
Reading the source code might help understanding how the DOM Tree is made, how CSS inheritance works and how the layout is calculated.
