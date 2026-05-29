# Slang Factory Project

This repository contains my bachelor project, developed at the **University of Vienna**, Faculty of Computer Science (**Informatics**).

My name is **Oleksandr Volinskyi**.
This project is carried out under the mentorship of **Univ.-Prof. Dr. Helmut Hlavacs**.

The project is related to the research environment of **EDEN – Education, Didactics and Entertainment Computing**, a research group at the Faculty of Computer Science of the University of Vienna. EDEN pursues interdisciplinary research that combines methods and theories from Education, Learning, and Social Sciences to investigate open problems in **Computer Science Education**, **Entertainment Computing**, and education in general, especially in the context of the digital and social transformation of education. The group aims to create sustainable technology-enhanced solutions.

EDEN currently consists of three working groups:

* **Entertainment Computing Group (EC)**, led by **Univ.-Prof. Dr. Helmut Hlavacs**
* **Computer Science Education / Informatik Didaktik Group (CIED)**
* **Technology Enhanced Learning Group (TELG)**

## Project Goal

Goal is to create a Slang factory in C++. It should accept parameters, then create a big string that contains
valid Slang code. I will use the compilation API to compile this code, then use the reflection API to get all reflection information out.

### Current Status

Generation of vertex and fragment shaders, will add some more shader resources like mvp, base_color and maybe texture. Everything is done with slang and I’m using slang api. MAde a small vulkan renderer for demo and I have made a console app that shows some inner infos for debugging.
Future plans, add other types of shaders and try to work with some real engines

## Tech Stack (FOR NOW)
- C++
- CMake
- Slang
- VS Code
- Vulkan
